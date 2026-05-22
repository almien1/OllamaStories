#include "llama_stories.h"
#include "ui_llama_stories.h"
#include <QFileDialog>
#include <QMessageBox>

LlamaStories::LlamaStories(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LlamaStories)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);
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

void LlamaStories::closeEvent(QCloseEvent *event)
{
    if (m_runner != nullptr)
    {
        closeRunner();
    }

    event->accept();
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
    ui->txtMainPrompt->setText(m_project.m_prompt);
    ui->txtGlobalPrompt->setText(m_project.m_globalPrompt);
    ui->cmbModel->setEditText(m_project.m_model);
    ui->slideTemp->setValue(m_project.m_temperature * 100);
    ui->slideContext->setValue(m_project.m_context);
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
    // Close any existing session
    if (m_runner != nullptr)
    {
        closeRunner();
    }

    // New process
    m_runner = std::make_shared<QProcess>();

    if (m_runner != nullptr)
    {

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("TERM", "dumb"); // Strips out the terminal layout engine noise
        m_runner->setProcessEnvironment(env);

        ui->tabWidget->setCurrentWidget(ui->runTab);

        connect(m_runner.get(), &QProcess::readyReadStandardOutput, this, &LlamaStories::handleProcessStdout);
        connect(m_runner.get(), &QProcess::readyReadStandardError, this, &LlamaStories::handleProcessStderr);
        connect(m_runner.get(), qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &LlamaStories::handleProcessExit);

        m_runner->start("ollama", {"run", m_project.m_name});
        qInfo() << "Run the start command for" << m_project.m_name;

    }
    else
    {
        qWarning() << "couldn't create a QProcess";
    }
}

void LlamaStories::closeRunner()
{
    m_runner->kill();
    if (m_runner->waitForFinished())
    {
        // ? what if not ?
    }
    m_runner.reset();
}


void LlamaStories::on_txtMainPrompt_textChanged()
{
    m_project.m_prompt = ui->txtMainPrompt->toPlainText();
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

void LlamaStories::handleProcessStdout()
{
    QByteArray outputData = m_runner->readAllStandardOutput();
    QString outputString = QString::fromUtf8(outputData);
    qInfo() << outputString;
}

void LlamaStories::handleProcessStderr()
{
    QByteArray outputData = m_runner->readAllStandardError();
    QString outputString = QString::fromUtf8(outputData);
    qWarning() << outputString;

}

void LlamaStories::handleProcessExit(int exitCode, QProcess::ExitStatus)
{
    qInfo() << "process finished with code" << exitCode;
}


void LlamaStories::on_pushButton_clicked()
{
    if (m_runner != nullptr)
    {
        QString text = ui->txtRunInput->toPlainText();
        if (!text.isEmpty())
        {
            ui->txtRunInput->clear();
            qInfo() << "Sending it " << text;
            if (m_runner->write(text.toUtf8()) > 0)
            {
            }
        }
    }
    else
    {
        qWarning() << "nothing running to talk to";
    }

}


void LlamaStories::on_listStories_itemSelectionChanged()
{

}

