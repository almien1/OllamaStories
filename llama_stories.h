#ifndef LLAMA_STORIES_H
#define LLAMA_STORIES_H

#include "story_project.h"
#include "llama_cpp_options.h"

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
    void on_Run_triggered();

    // Main editor
    void on_txtStoryPrompt_textChanged();
    void on_txtStoryNotes_textChanged();
    void on_listStories_itemPressed(QListWidgetItem *item);
    void on_btnNewStory_clicked();
    void on_btnDeleteStory_clicked();

    // Global prompt editor
    void on_txtGlobalPrompt_textChanged();

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
    void on_btnOptimizeForSystem_clicked();
    void on_btnStartLlamaServer_clicked();
    void on_btnStopLlamaServer_clicked();
    void on_chkGpuLayersAll_toggled(bool checked);

    void llamaOptionChanged();
    void updateModelInfo();
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

    void run();
    void sendInput();

    void resetStoryTimer();
    void storyTimer();

    void loadLlamaOptionsIntoUi();
    void saveLlamaOptionsFromUi();
    void refreshLlamaModelList();
    void startConversation();

    enum class TranscriptRole { User, Assistant, Admin };

    // Appends to m_transcriptSegments (coalescing into the last segment if
    // it's the same speaker) and re-renders txtRunOutput from it. Rendered
    // per-segment, rather than as one big markdown string, so each speaker
    // can be tinted differently - still through markdown, so *emphasis* etc.
    // in any of them renders properly rather than showing as literal symbols.
    void appendToTranscript(const QString &text, TranscriptRole role);
    void refreshTranscript();

    // Admin notes (e.g. "Starting llama.cpp server...") are only relevant
    // until whatever they're reporting on resolves - drops them from the
    // transcript once that happens instead of leaving stale status text.
    void removeAdminMessages();

    Ui::LlamaStories *ui;

    StoryProject m_project;

    QTimer *m_storyTimer = nullptr;
    QTime m_storyTime;
    int m_storyDayCount = 1;

    QThread *m_conversationThread = nullptr;

    LlamaCppOptions m_llamaOptions;
    LlamaCppServer *m_llamaServer = nullptr;
    bool m_startConversationWhenReady = false;

    std::shared_ptr<QSettings> m_settings;

    QVector<QAction *> m_mruActions;

    // Raw markdown backing txtRunOutput, split into speaker turns so each
    // can be rendered (and colored) independently - kept separately from
    // the widget since QTextEdit has no incremental markdown-append API.
    struct TranscriptSegment
    {
        TranscriptRole role;
        QString text;
    };
    QVector<TranscriptSegment> m_transcriptSegments;

};
#endif // LLAMA_STORIES_H
