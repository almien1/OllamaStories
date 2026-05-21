#include "llama_stories.h"
#include "ui_llama_stories.h"

LlamaStories::LlamaStories(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LlamaStories)
{
    ui->setupUi(this);
}

LlamaStories::~LlamaStories()
{
    delete ui;
}

void LlamaStories::on_actionExit_triggered()
{
    close();
}

