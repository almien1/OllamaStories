#ifndef LLAMA_STORIES_H
#define LLAMA_STORIES_H

#include <QMainWindow>

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

private:
    Ui::LlamaStories *ui;
};
#endif // LLAMA_STORIES_H
