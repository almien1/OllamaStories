#ifndef LLAMA_STORIES_H
#define LLAMA_STORIES_H

#include <QMainWindow>
#include "story_project.h"

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

private:

    void displayLoadedProject();

    Ui::LlamaStories *ui;

    StoryProject m_project;

};
#endif // LLAMA_STORIES_H
