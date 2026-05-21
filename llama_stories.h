#ifndef LLAMA_STORIES_H
#define LLAMA_STORIES_H

#include <QMainWindow>
#include <QProcess> // just while we figure out where it should go
#include "story_project.h"
#include "ollama_cli.h"

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
    void on_actionExit_triggered();
    void on_actionOpenProject_triggered();
    void on_actionSave_triggered();

    void on_txtMainPrompt_textChanged();

    void on_txtGlobalPrompt_textChanged();

    void on_cmbModel_currentTextChanged(const QString &arg1);

    void on_slideTemp_valueChanged(int value);

    void on_slideContext_valueChanged(int value);

    void on_actionCompileAndRun_triggered();

    void on_txtProjectName_textChanged(const QString &arg1);

    void on_actionCompile_triggered();

    void on_Run_triggered();

    void handleProcessStdout();
    void handleProcessStderr();
    void handleProcessExit(int exitCode, QProcess::ExitStatus exitStatus);

    void on_pushButton_clicked();

private:

    void displayLoadedProject();
    bool compileProject();
    bool run();
    void closeRunner();

    Ui::LlamaStories *ui;

    StoryProject m_project;
    OllamaCLI m_ai;

    std::shared_ptr<QProcess> m_runner = nullptr;

};
#endif // LLAMA_STORIES_H
