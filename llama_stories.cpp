#include "llama_stories.h"
#include "ui_llama_stories.h"

#include "llama_cpp_chat.h"
#include "llama_cpp_server.h"
#include "input_editbox.h"
#include "gguf_info.h"
#include <QFile>
#include <QDir>
#include <QSignalBlocker>

LlamaStories::LlamaStories(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LlamaStories)
    , m_settings(std::make_shared<QSettings>(QSettings::NativeFormat, QSettings::UserScope, "LlamaStories", "LlamaStories", this))
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);
    ui->storyTab->setCurrentIndex(0);

    connect(ui->txtRunInput, &InputEditbox::enterPressed, this, &LlamaStories::enterPressed);

    m_storyTimer = new QTimer(this);
    resetStoryTimer();
    connect(m_storyTimer, &QTimer::timeout, this, &LlamaStories::storyTimer);

    m_llamaServer = new LlamaCppServer(this);
    connect(m_llamaServer, &LlamaCppServer::ready, this, &LlamaStories::llamaServerReady);
    connect(m_llamaServer, &LlamaCppServer::failed, this, &LlamaStories::llamaServerFailed);
    connect(m_llamaServer, &LlamaCppServer::logOutput, this, [](QString text){ qInfo().noquote() << text; });

    m_llamaOptions.load(*m_settings);
    loadLlamaOptionsIntoUi();
    updateGpuLayersLimit();

    connect(ui->txtLlamaServerPath, &QLineEdit::editingFinished, this, &LlamaStories::llamaOptionChanged);
    connect(ui->txtLlamaModelsDir, &QLineEdit::editingFinished, this, &LlamaStories::llamaOptionChanged);
    connect(ui->cmbLlamaModel, &QComboBox::currentTextChanged, this, &LlamaStories::llamaOptionChanged);
    connect(ui->cmbLlamaModel, &QComboBox::currentTextChanged, this, &LlamaStories::updateGpuLayersLimit);
    connect(ui->spinLlamaContext, &QSpinBox::valueChanged, this, &LlamaStories::llamaOptionChanged);
    connect(ui->spinLlamaGpuLayers, &QSpinBox::valueChanged, this, &LlamaStories::llamaOptionChanged);
    connect(ui->spinLlamaTemp, &QDoubleSpinBox::valueChanged, this, &LlamaStories::llamaOptionChanged);
    connect(ui->spinLlamaTopP, &QDoubleSpinBox::valueChanged, this, &LlamaStories::llamaOptionChanged);
    connect(ui->spinLlamaTopK, &QSpinBox::valueChanged, this, &LlamaStories::llamaOptionChanged);
    connect(ui->spinLlamaMinP, &QDoubleSpinBox::valueChanged, this, &LlamaStories::llamaOptionChanged);
    connect(ui->spinLlamaRepeatPenalty, &QDoubleSpinBox::valueChanged, this, &LlamaStories::llamaOptionChanged);
    connect(ui->spinLlamaRepeatLastN, &QSpinBox::valueChanged, this, &LlamaStories::llamaOptionChanged);
    connect(ui->chkFlashAttention, &QCheckBox::toggled, this, &LlamaStories::llamaOptionChanged);
    connect(ui->cmbChatTemplate, &QComboBox::currentTextChanged, this, &LlamaStories::llamaOptionChanged);
}

LlamaStories::~LlamaStories()
{
    if (m_storyTimer && m_storyTimer->isActive())
    {
        m_storyTimer->stop();
    }
    if (m_conversationThread && m_conversationThread->isRunning())
    {
        m_conversationThread->exit(0);
    }
    if (m_llamaServer)
    {
        m_llamaServer->stop();
    }
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
    ui->txtGlobalPrompt->setText(m_project.m_globalPrompt);
    ui->txtProjectNotes->setText(m_project.m_projectNotes);
    displayStoryList();
    selectStory(m_project.m_selectedStory);
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
    ui->storyTab->setCurrentIndex(0);
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

void LlamaStories::run()
{
    if (m_conversationThread != nullptr)
    {
        m_conversationThread->exit(0);
    }
    ui->txtRunOutput->clear();
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->runTab));
    QCoreApplication::processEvents();

    saveLlamaOptionsFromUi();

    if (m_llamaOptions.modelFile.isEmpty())
    {
        QMessageBox::warning(this, "No model selected", "Choose a .gguf model on the Llama.cpp tab first");
        return;
    }

    if (m_llamaServer->isRunning())
    {
        startConversation();
    }
    else
    {
        ui->txtRunOutput->insertPlainText("Starting llama.cpp server...\n");
        QCoreApplication::processEvents();
        m_startConversationWhenReady = true;
        ui->lblLlamaServerStatus->setText("Starting...");
        ui->btnStartLlamaServer->setEnabled(false);
        m_llamaServer->start(m_llamaOptions);
    }
}

void LlamaStories::startConversation()
{
    if (m_conversationThread != nullptr)
    {
        m_conversationThread->exit(0);
    }

    QString systemPrompt = m_project.combinedPrompt();
    LlamaCppChat *worker = new LlamaCppChat(m_llamaServer->baseUrl(), m_llamaServer->apiKey(), systemPrompt, "hello, who are you?", m_llamaOptions, nullptr); // no parent because we will call moveToThread
    m_conversationThread = new QThread(this);
    worker->moveToThread(m_conversationThread);

    connect(worker, &LlamaCppChat::partialText, this, [this](QString text){partialText(text);});
    connect(worker, &LlamaCppChat::responseFinished, this, [this](){responseFinished();});
    connect(worker, &LlamaCppChat::errorOccurred, this, [this](QString error){
        QMessageBox::warning(this, "llama.cpp error", error);
    });

    connect(m_conversationThread, &QThread::started, worker, &LlamaCppChat::start);

    connect(m_conversationThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(m_conversationThread, &QThread::finished, m_conversationThread, &QObject::deleteLater);
    connect(this, &QObject::destroyed, m_conversationThread, &QThread::quit);
    connect(this, &LlamaStories::sendMessage, worker, &LlamaCppChat::sendMessage);
    m_conversationThread->start();
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

void LlamaStories::on_Run_triggered()
{
    run();
}

void LlamaStories::enterPressed()
{
    sendInput();
}

void LlamaStories::sendInput()
{
    if ((m_conversationThread != nullptr) && m_conversationThread->isRunning())
    {
        QString question = ui->txtRunInput->toPlainText();
        showQuestion(question);
        ui->txtRunInput->clear();
        QCoreApplication::processEvents();

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
     ui->txtRunOutput->insertPlainText("\n\n");
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
    ui->txtRunOutput->insertPlainText("\n\n");
    QCoreApplication::processEvents();
}

void LlamaStories::on_btnNewStory_clicked()
{
    QString name = QInputDialog::getText(this, "New story", "Story name");
    if (!name.isEmpty())
    {
        m_project.m_stories.insert(name, StoryProject::Story());
        displayStoryList();
        selectStory(name);
        displaySelectedStory();
    }
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

void LlamaStories::on_actionOpenInTerminal_triggered()
{
    saveLlamaOptionsFromUi();

    if (m_llamaOptions.modelFile.isEmpty())
    {
        QMessageBox::warning(this, "No model selected", "Choose a .gguf model on the Llama.cpp tab first");
        return;
    }

    // Written to disk (rather than passed as a -p argument) so cmd.exe doesn't
    // have to survive quoting a multi-line, possibly-quote-containing prompt.
    QString promptFile = QDir::temp().filePath("LlamaStories_system_prompt.txt");
    QFile file(promptFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file.write(m_project.combinedPrompt().toUtf8());
        file.close();
    }

    QStringList args = {"/c", "start", "cmd.exe", "/K", m_llamaOptions.cliExecutablePath()};
    args += m_llamaOptions.cliArguments(promptFile);
    QProcess::startDetached("cmd.exe", args);
}

void LlamaStories::on_actionCopy_triggered()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(m_project.combinedPrompt());
}

void LlamaStories::on_btnRenameStory_clicked()
{
    if (m_project.m_stories.contains(m_project.m_selectedStory))
    {
        QString name = QInputDialog::getText(this, "Rename story", "New name", QLineEdit::Normal, m_project.m_selectedStory);
        if (!name.isEmpty())
        {
            m_project.m_stories[name] = m_project.m_stories[m_project.m_selectedStory];
            m_project.m_stories.remove(m_project.m_selectedStory);
            m_project.m_selectedStory = name;
            displayStoryList();
            selectStory(name);
            displaySelectedStory();
        }
    }

}

void LlamaStories::on_btnTimePlay_pressed()
{
    if (m_storyTimer && !m_storyTimer->isActive())
    {
        storyTimer();
        m_storyTimer->start(6000);
    }
}

void LlamaStories::on_btnTimePause_pressed()
{
    if (m_storyTimer)
    {
        m_storyTimer->stop();
    }
}

void LlamaStories::on_btnTimeReset_pressed()
{
    resetStoryTimer();
}

void LlamaStories::resetStoryTimer()
{
    m_storyTime.setHMS(6,0,0);
    m_storyDayCount = 1;
    ui->txtStoryTime->setTime(m_storyTime);
}

void LlamaStories::storyTimer()
{
    const bool debugTimer = false;
    if ((m_conversationThread != nullptr) && m_conversationThread->isRunning())
    {
        QTime oldTime = m_storyTime;
        m_storyTime = m_storyTime.addSecs(20 * 60);

        ui->txtStoryTime->setTime(m_storyTime);

        if (m_storyTime < oldTime)
        {
            m_storyDayCount += 1;
        }

        QString prompt = QString("It is %1:%2 on day %3 ")
                             .arg(m_storyTime.hour(), 2, 10, QChar('0'))
                             .arg(m_storyTime.minute(), 2, 10, QChar('0'))
                             .arg(m_storyDayCount);

        if (debugTimer)
        {
            showQuestion(prompt);
        }

        emit sendMessage(prompt);
    }

}
void LlamaStories::on_btnTimeStep_pressed()
{
    // Single step
    storyTimer();
}

void LlamaStories::loadLlamaOptionsIntoUi()
{
    ui->txtLlamaServerPath->setText(m_llamaOptions.serverPath);
    ui->txtLlamaModelsDir->setText(m_llamaOptions.modelsDir);
    ui->spinLlamaContext->setValue(m_llamaOptions.contextSize);
    ui->chkGpuLayersAll->setChecked(m_llamaOptions.gpuLayersAll);
    ui->spinLlamaGpuLayers->setEnabled(!m_llamaOptions.gpuLayersAll);
    ui->spinLlamaGpuLayers->setValue(m_llamaOptions.gpuLayers);
    ui->spinLlamaTemp->setValue(m_llamaOptions.temperature);
    ui->spinLlamaTopP->setValue(m_llamaOptions.topP);
    ui->spinLlamaTopK->setValue(m_llamaOptions.topK);
    ui->spinLlamaMinP->setValue(m_llamaOptions.minP);
    ui->spinLlamaRepeatPenalty->setValue(m_llamaOptions.repeatPenalty);
    ui->spinLlamaRepeatLastN->setValue(m_llamaOptions.repeatLastN);
    ui->chkFlashAttention->setChecked(m_llamaOptions.flashAttention);
    ui->chkUseModelTemplate->setChecked(m_llamaOptions.useModelChatTemplate);
    ui->cmbChatTemplate->setEnabled(!m_llamaOptions.useModelChatTemplate);
    ui->cmbChatTemplate->setCurrentText(m_llamaOptions.chatTemplate);

    refreshLlamaModelList();
    ui->cmbLlamaModel->setCurrentText(m_llamaOptions.modelFile);
}

void LlamaStories::saveLlamaOptionsFromUi()
{
    m_llamaOptions.serverPath = ui->txtLlamaServerPath->text();
    m_llamaOptions.modelsDir = ui->txtLlamaModelsDir->text();
    m_llamaOptions.modelFile = ui->cmbLlamaModel->currentText();
    m_llamaOptions.contextSize = ui->spinLlamaContext->value();
    m_llamaOptions.gpuLayersAll = ui->chkGpuLayersAll->isChecked();
    m_llamaOptions.gpuLayers = ui->spinLlamaGpuLayers->value();
    m_llamaOptions.temperature = ui->spinLlamaTemp->value();
    m_llamaOptions.topP = ui->spinLlamaTopP->value();
    m_llamaOptions.topK = ui->spinLlamaTopK->value();
    m_llamaOptions.minP = ui->spinLlamaMinP->value();
    m_llamaOptions.repeatPenalty = ui->spinLlamaRepeatPenalty->value();
    m_llamaOptions.repeatLastN = ui->spinLlamaRepeatLastN->value();
    m_llamaOptions.flashAttention = ui->chkFlashAttention->isChecked();
    m_llamaOptions.useModelChatTemplate = ui->chkUseModelTemplate->isChecked();
    m_llamaOptions.chatTemplate = ui->cmbChatTemplate->currentText();
    m_llamaOptions.save(*m_settings);
}

void LlamaStories::refreshLlamaModelList()
{
    QString previous = ui->cmbLlamaModel->currentText();

    // Repopulating fires currentTextChanged (e.g. clear() briefly leaves it
    // empty, and Qt auto-selects item 0 the moment the list goes from empty
    // to non-empty) - block signals so none of that transient state gets
    // persisted via llamaOptionChanged before we restore the real selection.
    QSignalBlocker blocker(ui->cmbLlamaModel);

    ui->cmbLlamaModel->clear();

    QDir dir(ui->txtLlamaModelsDir->text());
    if (dir.exists())
    {
        ui->cmbLlamaModel->addItems(dir.entryList({"*.gguf"}, QDir::Files, QDir::Name));
    }

    if (!previous.isEmpty())
    {
        ui->cmbLlamaModel->setCurrentText(previous);
    }
}

void LlamaStories::on_btnBrowseLlamaServer_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Locate llama-server", ui->txtLlamaServerPath->text(),
                                                      "llama-server (llama-server*.exe);;All files (*)");
    if (!filename.isEmpty())
    {
        ui->txtLlamaServerPath->setText(filename);
        saveLlamaOptionsFromUi();
    }
}

void LlamaStories::on_btnBrowseModelsDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select models folder", ui->txtLlamaModelsDir->text());
    if (!dir.isEmpty())
    {
        ui->txtLlamaModelsDir->setText(dir);
        saveLlamaOptionsFromUi();
        refreshLlamaModelList();
    }
}

void LlamaStories::on_btnRefreshLlamaModels_clicked()
{
    refreshLlamaModelList();
}

void LlamaStories::on_btnRoleplayDefaults_clicked()
{
    m_llamaOptions.applyRoleplayDefaults();
    loadLlamaOptionsIntoUi();
    saveLlamaOptionsFromUi();
}

void LlamaStories::on_btnStartLlamaServer_clicked()
{
    saveLlamaOptionsFromUi();

    if (m_llamaOptions.modelFile.isEmpty())
    {
        QMessageBox::warning(this, "No model selected", "Choose a .gguf model on the Llama.cpp tab first");
        return;
    }

    ui->lblLlamaServerStatus->setText("Starting...");
    ui->btnStartLlamaServer->setEnabled(false);
    m_llamaServer->start(m_llamaOptions);
}

void LlamaStories::on_btnStopLlamaServer_clicked()
{
    m_llamaServer->stop();
    ui->lblLlamaServerStatus->setText("Stopped");
    ui->lblLlamaServerStatus->setToolTip("");
    ui->btnStartLlamaServer->setEnabled(true);
    ui->btnStopLlamaServer->setEnabled(false);
}

void LlamaStories::on_chkUseModelTemplate_toggled(bool checked)
{
    ui->cmbChatTemplate->setEnabled(!checked);
    saveLlamaOptionsFromUi();
}

void LlamaStories::on_chkGpuLayersAll_toggled(bool checked)
{
    ui->spinLlamaGpuLayers->setEnabled(!checked);
    saveLlamaOptionsFromUi();
}

void LlamaStories::llamaOptionChanged()
{
    saveLlamaOptionsFromUi();
}

void LlamaStories::updateGpuLayersLimit()
{
    saveLlamaOptionsFromUi();

    GgufInfo info = readGgufBlockCount(m_llamaOptions.modelPath());
    int maxLayers = info.valid ? info.blockCount : 999;

    ui->spinLlamaGpuLayers->setMaximum(maxLayers);
    ui->spinLlamaGpuLayers->setToolTip(info.valid
        ? QString("This model has %1 layers. Lower this if it doesn't fit in VRAM.").arg(maxLayers)
        : "How many layers to offload to the GPU. Lower this if the model doesn't fit in VRAM (pick a valid .gguf model to see its exact layer count here).");
}

void LlamaStories::llamaServerReady()
{
    ui->lblLlamaServerStatus->setText("Ready");
    ui->lblLlamaServerStatus->setToolTip(m_llamaServer->baseUrl());
    ui->btnStartLlamaServer->setEnabled(false);
    ui->btnStopLlamaServer->setEnabled(true);

    if (m_startConversationWhenReady)
    {
        m_startConversationWhenReady = false;
        startConversation();
    }
}

void LlamaStories::llamaServerFailed(QString error)
{
    ui->lblLlamaServerStatus->setText("Error");
    ui->lblLlamaServerStatus->setToolTip(error);
    ui->btnStartLlamaServer->setEnabled(true);
    ui->btnStopLlamaServer->setEnabled(false);

    m_startConversationWhenReady = false;
    QMessageBox::warning(this, "llama.cpp error", error);
}