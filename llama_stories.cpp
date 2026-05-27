#include "llama_stories.h"
#include "ui_llama_stories.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "ollama-hpp/singleheader/ollama.hpp"

LlamaStories::LlamaStories(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LlamaStories)
    , m_server(std::make_shared<Ollama>("http://localhost:11434"))
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);

    /*
    if (m_project.load(R"(C:\Users\user\repos\LlamaWorkspace\project.json)"))
    {
        displayLoadedProject();
    }
    */

    updateModelList();
}

LlamaStories::~LlamaStories()
{
    delete ui;
}

void LlamaStories::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void LlamaStories::on_actionExit_triggered()
{
    close();
}


void LlamaStories::on_actionOpenProject_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "open project", "../../../../LlamaWorkspace", "*.json");
    if (!filename.isEmpty())
    {
        if (m_project.load(filename))
        {
            displayLoadedProject();
        }
        else
        {
            QMessageBox::warning(this, "Could not open project", "Could not open project");
        }
    }
}

void LlamaStories::saveProject()
{
    if (m_project.dirty())
    {
        bool fail = false;
        if (m_project.hasFile())
        {
            fail = !m_project.save();
        }
        else
        {
            QString filename = QFileDialog::getSaveFileName(this, "save project");
            if (!filename.isEmpty())
            {
                fail = !m_project.saveAs(filename);
            }
        }
        if (fail)
        {
            QMessageBox::warning(this, "Failed", "Couldn't save project");
        }
    }
}

void LlamaStories::on_actionSave_triggered()
{
    saveProject();
}

void LlamaStories::displayLoadedProject()
{
    ui->txtProjectName->setText(m_project.m_name);
    ui->txtStoryPrompt->setText(m_project.m_stories.value(m_project.m_selectedStory));
    ui->txtGlobalPrompt->setText(m_project.m_globalPrompt);
    ui->cmbModel->setEditText(m_project.m_model);
    ui->slideTemp->setValue(m_project.m_temperature * 100);
    ui->slideContext->setValue(m_project.m_context);
    displayStoryList();
}

void LlamaStories::displayStoryList()
{
    ui->listStories->clear();
    for (QString name : m_project.m_stories.keys())
    {
        ui->listStories->addItem(name);
    }
}

void LlamaStories::displaySelectedStory()
{
    ui->txtStoryPrompt->setText(m_project.m_stories.value(m_project.m_selectedStory));
}

void LlamaStories::selectStory(const QString &name)
{
    m_project.m_selectedStory = name;

    for(int i = 0; i < ui->listStories->count(); i++)
    {
        if (ui->listStories->item(i)->text() == name)
        {
            ui->listStories->setCurrentRow(i);
            break;
        }
    }
}

bool LlamaStories::compileProject()
{
    saveProject();

    ui->actionCompile->setEnabled(false);
    ui->actionCompileAndRun->setEnabled(false);

    QString modelFile = R"(C:\Users\user\repos\LlamaWorkspace\Modelfile)";
    m_project.writeModelfile(modelFile);
    bool success = m_ai.compileModel(m_project.m_name, modelFile);

    ui->actionCompile->setEnabled(true);
    ui->actionCompileAndRun->setEnabled(true);
    return success;
}

void LlamaStories::run()
{
    if (m_server->is_running())
    {
        ui->tabWidget->setCurrentIndex(3);
        QCoreApplication::processEvents();

        //auto response = m_server->generate(m_project.m_name.toStdString(), "Hello?");
        //ui->txtRunOutput->setPlainText(QString::fromStdString(response.as_json_string()));

        std::function<bool(const ollama::response&)> response_callback = std::bind(
            &LlamaStories::on_receive_response,
            this,
            std::placeholders::_1
            );

        ollama::message message("user", "Hello?");

        ollama::options options; //["seed"], ["temperature"], ["num_predict"]

        ollama::chat(m_project.m_name.toStdString(), message, response_callback, options);


    }

}

bool LlamaStories::on_receive_response(const ollama::response& response)
{
    //if (response.as_json()["done"]==true) std::cout << std::endl;

    QString newText = QString::fromStdString(response.as_simple_string());

    // This would add a newline every time
    // ui->txtRunOutput->append(newText);

    ui->txtRunOutput->moveCursor(QTextCursor::End);
    ui->txtRunOutput->insertPlainText(newText);
    QCoreApplication::processEvents();

    // Return true to continue streaming, or false to stop immediately
    return true;
}

void LlamaStories::updateModelList()
{

    if (m_server->is_running())
    {
        ui->cmbModel->clear();
        for (const auto &model : m_server->list_models())
        {
            ui->cmbModel->addItem(QString::fromStdString(model));
        }
        ui->cmbModel->setCurrentText(m_project.m_model);
    }

}


void LlamaStories::on_txtStoryPrompt_textChanged()
{
    if (!m_project.m_selectedStory.isEmpty())
    {
        m_project.m_stories[m_project.m_selectedStory] = ui->txtStoryPrompt->toPlainText();
    }
}

void LlamaStories::on_txtGlobalPrompt_textChanged()
{
    m_project.m_globalPrompt = ui->txtGlobalPrompt->toPlainText();
}

void LlamaStories::on_txtProjectName_textChanged(const QString &text)
{
    m_project.m_name = text;
}

void LlamaStories::on_cmbModel_currentTextChanged(const QString &text)
{
    m_project.m_model = text;
}

void LlamaStories::on_slideTemp_valueChanged(int value)
{
    m_project.m_temperature = 0.01 * value;
}

void LlamaStories::on_slideContext_valueChanged(int value)
{
    m_project.m_context = value;
}

void LlamaStories::on_actionCompileAndRun_triggered()
{
    compileProject();
    run();
}

void LlamaStories::on_actionCompile_triggered()
{
    if (!compileProject())
    {
        QMessageBox::warning(this, "couldn't compile", "couldn't compile project");
    }
}


void LlamaStories::on_Run_triggered()
{
    run();
}

void LlamaStories::on_pushButton_clicked()
{

    // TODO; talk
}

void LlamaStories::on_btnNewStory_clicked()
{
    QString name = QInputDialog::getText(this, "New story", "Story name");
    m_project.m_stories.insert(name, "");
    displayStoryList();
    selectStory(name);
    displaySelectedStory();
}

void LlamaStories::on_btnDeleteStory_clicked()
{
    if (!m_project.m_selectedStory.isEmpty())
    {
        m_project.m_stories.remove(m_project.m_selectedStory);
        displayStoryList();
        if (!m_project.m_stories.isEmpty())
        {
            m_project.m_selectedStory =  m_project.m_stories.first();
            displaySelectedStory();
        }
    }
}


void LlamaStories::on_listStories_itemPressed(QListWidgetItem *item)
{
    m_project.m_selectedStory = item->text();

    displaySelectedStory();
}

