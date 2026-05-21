#include "llama_stories.h"
#include "ui_llama_stories.h"
#include <QFileDialog>
#include <QMessageBox>

LlamaStories::LlamaStories(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LlamaStories)
{
    ui->setupUi(this);

    if (m_project.load(R"(C:\Users\user\repos\LlamaWorkspace\project.json)"))
    {
        displayLoadedProject();
    }
    else
    {
        QMessageBox::warning(this, "couldn't load", "couldn't load project");
    }

}

LlamaStories::~LlamaStories()
{
    delete ui;
}

void LlamaStories::on_actionExit_triggered()
{
    close();
}


void LlamaStories::on_actionOpenProject_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, "open project");
    if (!filename.isEmpty())
    {
        m_project.load(filename);
    }
}


void LlamaStories::on_actionSave_triggered()
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

void LlamaStories::displayLoadedProject()
{
    ui->txtMainPrompt->setText(m_project.m_prompt);
    ui->cmbModel->setEditText(m_project.m_model);
    ui->slideTemp->setValue(m_project.m_temperature * 100);
    ui->slideContext->setValue(m_project.m_context);
    ui->txtGlobalFile->setText(m_project.m_globalsFile);
}


void LlamaStories::on_txtMainPrompt_textChanged()
{
    m_project.m_prompt = ui->txtMainPrompt->toPlainText();
}


void LlamaStories::on_txtGlobalPrompt_textChanged()
{
    // todo save this somewhere else (in m_project.m_globalsFile)
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
    m_project.writeModelfile(R"(C:\Users\user\repos\LlamaWorkspace\Modelfile)");
}

