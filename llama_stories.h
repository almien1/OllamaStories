#ifndef LLAMA_STORIES_H
#define LLAMA_STORIES_H

#include "story_project.h"
#include "ollama_cli.h"
#include "llama_cpp_options.h"

class Ollama;
class QSettings;
class QListWidgetItem;
class LlamaCppServer;

QT_BEGIN_NAMESPACE
namespace Ui {
class LlamaStories;
}
QT_END_NAMESPACE

class LlamaStories : public QMainWindow
{
    Q_OBJECT

public:
    explicit LlamaStories(QWidget *parent = nullptr);
    ~LlamaStories() override;

public slots:
    // Interactons with m_conversationThread
    void partialText(QString text);
    void responseFinished();
    void showQuestion(QString text);

    void gotModelList(QStringList models);

    // Custom textbox for input
    void enterPressed();

signals:
    void sendMessage(QString text);

protected:
    void closeEvent(QCloseEvent *event) override;


private slots:
    // Menu
    void on_actionOpenProject_triggered();
    void on_actionSave_triggered();
    void on_actionCopy_triggered();
    void on_actionOpenInTerminal_triggered();
    void on_actionExit_triggered();

    // Toolbar
    void on_actionCompileAndRun_triggered();
    void on_actionCompile_triggered();
    void on_Run_triggered();

    // Main editor
    void on_txtStoryPrompt_textChanged();
    void on_txtStoryNotes_textChanged();
    void on_listStories_itemPressed(QListWidgetItem *item);
    void on_btnNewStory_clicked();
    void on_btnDeleteStory_clicked();

    // Global prompt editor
    void on_txtGlobalPrompt_textChanged();

    // Settings
    void on_txtProjectName_textChanged(const QString &arg1);
    void on_cmbModel_currentTextChanged(const QString &arg1);
    void on_btnRefreshModelList_pressed();
    void on_slideTemp_valueChanged(int value);
    void on_slideContext_valueChanged(int value);

    // Notes
    void on_txtProjectNotes_textChanged();
    void on_btnRenameStory_clicked();

    void on_btnTimePlay_pressed();
    void on_btnTimePause_pressed();
    void on_btnTimeReset_pressed();
    void on_btnTimeStep_pressed();

    // Llama.cpp options tab
    void on_btnBrowseLlamaServer_clicked();
    void on_btnBrowseModelsDir_clicked();
    void on_btnRefreshLlamaModels_clicked();
    void on_btnRoleplayDefaults_clicked();
    void on_btnStartLlamaServer_clicked();
    void on_btnStopLlamaServer_clicked();
    void on_chkUseModelTemplate_toggled(bool checked);

    void llamaOptionChanged();
    void llamaServerReady();
    void llamaServerFailed(QString error);

private:
    void loadProject(const QString &filename);
    void saveProject();
    void displayLoadedProject();
    void displayStoryList();
    void displaySelectedStory();
    void selectStory(const QString &name);
    QString projectDirectory();

    bool compileProject();
    void run();
    void sendInput();

    void updateModelList();

    void resetStoryTimer();
    void storyTimer();

    void loadLlamaOptionsIntoUi();
    void saveLlamaOptionsFromUi();
    void refreshLlamaModelList();
    void startConversation();

    Ui::LlamaStories *ui;

    StoryProject m_project;

    QTimer *m_storyTimer = nullptr;
    QTime m_storyTime;
    int m_storyDayCount = 1;

    OllamaCLI m_ai;
    QThread *m_conversationThread = nullptr;

    LlamaCppOptions m_llamaOptions;
    LlamaCppServer *m_llamaServer = nullptr;
    bool m_startConversationWhenReady = false;

    std::shared_ptr<QSettings> m_settings;

    QVector<QAction *> m_mruActions;

};
#endif // LLAMA_STORIES_H
