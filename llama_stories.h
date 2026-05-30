#ifndef LLAMA_STORIES_H
#define LLAMA_STORIES_H

#include <QMainWindow>
#include <QThread>
#include "story_project.h"
#include "ollama_cli.h"

class Ollama;
class QSettings;
class QListWidgetItem;

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

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // Menu
    void on_actionOpenProject_triggered();
    void on_actionSave_triggered();
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
    void on_slideTemp_valueChanged(int value);
    void on_slideContext_valueChanged(int value);

    // Notes
    void on_txtProjectNotes_textChanged();

    // Interactive
    void on_pushButton_clicked();


    // Interactons with m_conversationThread
    void partialText(QString text);
    void responseFinished();

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

    void updateModelList();

    Ui::LlamaStories *ui;

    StoryProject m_project;

    OllamaCLI m_ai;
    QThread *m_conversationThread = nullptr;

    std::shared_ptr<QSettings> m_settings;

    QVector<QAction *> m_mruActions;

};
#endif // LLAMA_STORIES_H
