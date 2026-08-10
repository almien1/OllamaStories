#include "model_list.h"

ModelList::ModelList()
    : QObject(nullptr)
{

}

void ModelList::start()
{
    Ollama llama;
    QStringList models;
    for (auto model : llama.list_models())
    {
        models.append(QString::fromStdString(model));
    }
    emit modelList(models);
}
