#include "llama_stories.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LlamaStories w;
    w.show();
    return QApplication::exec();
}
