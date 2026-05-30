#include "llama_stories.h"
#include "ui_llama_stories.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTemporaryFile>
#include "conversation.h"

LlamaStories::LlamaStories(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LlamaStories)
    , m_settings(std::make_shared<QSettings>(QSettings::NativeFormat, QSettings::UserScope, "LlamaStories", "LlamaStories", this))
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);
    ui->storyTab->setCurrentIndex(0);

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
    QString filename = QFileDialog::getOpenFileName(this, "open project", projectDirectory(), "*.json");
    if (!filename.isEmpty())
    {
        loadProject(filename);
    }
}

void LlamaStories::loadProject(const QString &filename)
{
    if (m_project.load(filename))
    {
        m_settings->setValue("recent_directory", QFileInfo(filename).dir().absolutePath());
        displayLoadedProject();
    }
    else
    {
        QMessageBox::warning(this, "Could not open project", "Could not open project");
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
            QString filename = QFileDialog::getSaveFileName(this, "save project", projectDirectory(), "*.json");
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
    ui->txtGlobalPrompt->setText(m_project.m_globalPrompt);
    ui->cmbModel->setEditText(m_project.m_model);
    ui->slideTemp->setValue(m_project.m_temperature * 100);
    ui->slideContext->setValue(m_project.m_context);
    ui->txtProjectNotes->setText(m_project.m_projectNotes);
    displayStoryList();
    displaySelectedStory();
}

void LlamaStories::displayStoryList()
{
    ui->listStories->clear();
    for (const QString &name : m_project.m_stories.keys())
    {
        ui->listStories->addItem(name);
    }
}

void LlamaStories::displaySelectedStory()
{
    ui->txtStoryPrompt->setText(m_project.m_stories.value(m_project.m_selectedStory).prompt);
    ui->txtStoryNotes->setText(m_project.m_stories.value(m_project.m_selectedStory).notes);
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

QString LlamaStories::projectDirectory()
{
    return m_settings->value("recent_directory").toString();
}

bool LlamaStories::compileProject()
{
    bool success = false;
    saveProject();

    ui->actionCompile->setEnabled(false);
    ui->actionCompileAndRun->setEnabled(false);
    QCoreApplication::processEvents();

    QTemporaryFile modelFile;
    if (modelFile.open())
    {
        m_project.writeModelfile(modelFile.fileName());
        success = m_ai.compileModel(m_project.m_name, modelFile.fileName());
    }

    ui->actionCompile->setEnabled(true);
    ui->actionCompileAndRun->setEnabled(true);
    QCoreApplication::processEvents();

    return success;
}

void LlamaStories::run()
{
    if (m_conversationThread != nullptr)
    {
        // TODO kill the old one
    }
    ui->tabWidget->setCurrentIndex(3);
    QCoreApplication::processEvents();

    Conversation *worker = new Conversation(m_project.m_name, "", "hello", this);
    m_conversationThread = new QThread(this);
    worker->moveToThread(m_conversationThread);

    connect(worker, &Conversation::partialText, this, [this](QString text){partialText(text);});
    connect(worker, &Conversation::responseFinished, this, [this](){responseFinished();});

    connect(m_conversationThread, &QThread::started, worker, &Conversation::start);

    connect(m_conversationThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(m_conversationThread, &QThread::finished, m_conversationThread, &QObject::deleteLater);
    connect(this, &QObject::destroyed, m_conversationThread, &QThread::quit);
    connect(this, &LlamaStories::sendMessage, worker, &Conversation::sendMessage);
    m_conversationThread->start();

}

void LlamaStories::updateModelList()
{

}


void LlamaStories::on_txtStoryPrompt_textChanged()
{
    if (!m_project.m_selectedStory.isEmpty())
    {
        m_project.m_stories[m_project.m_selectedStory].prompt = ui->txtStoryPrompt->toPlainText();
    }
}

void LlamaStories::on_txtStoryNotes_textChanged()
{
    if (!m_project.m_selectedStory.isEmpty())
    {
        m_project.m_stories[m_project.m_selectedStory].notes = ui->txtStoryNotes->toPlainText();
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
    if ((m_conversationThread != nullptr) && m_conversationThread->isRunning())
    {
        QString question = ui->txtRunInput->toPlainText();
        showQuestion(question);

        emit sendMessage(question);
    }
    else
    {
        QMessageBox::information(this, "no conversation", "Need to run first");
    }
}

void LlamaStories::showQuestion(QString text)
{
    ui->txtRunOutput->moveCursor(QTextCursor::End);
    ui->txtRunOutput->insertPlainText(text);
     ui->txtRunOutput->insertPlainText("\n\n-- sending question --");
    QCoreApplication::processEvents();
}

void LlamaStories::partialText(QString text)
{
    ui->txtRunOutput->moveCursor(QTextCursor::End);
    ui->txtRunOutput->insertPlainText(text);
    QCoreApplication::processEvents();
}

void LlamaStories::responseFinished()
{
    ui->txtRunOutput->moveCursor(QTextCursor::End);
    ui->txtRunOutput->insertPlainText("\n\n-- end of response --");
    QCoreApplication::processEvents();
}

void LlamaStories::on_btnNewStory_clicked()
{
    QString name = QInputDialog::getText(this, "New story", "Story name");
    m_project.m_stories.insert(name, StoryProject::Story());
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
            m_project.m_selectedStory = m_project.m_stories.firstKey();
            displaySelectedStory();
        }
    }
}


void LlamaStories::on_listStories_itemPressed(QListWidgetItem *item)
{
    m_project.m_selectedStory = item->text();

    displaySelectedStory();
}

void LlamaStories::on_txtProjectNotes_textChanged()
{
    m_project.m_projectNotes = ui->txtProjectNotes->toPlainText();
}

