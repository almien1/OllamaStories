#include "story_project.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

StoryProject::StoryProject() {}

bool StoryProject::load(const QString &filename)
{
    bool success = false;
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly))
    {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject())
        {
            success = true;
            QJsonObject json = doc.object();
            m_prompt = json.value("prompt").toString("");
            m_model = json.value("model").toString("gemma3:12b");
            m_temperature = json.value("temperature").toDouble();
            m_context = json.value("context_size").toInt(16384);
            m_globalsFile = json.value("globals_file").toString("");

            m_filename = filename;
        }
    }
    return success;
}

bool StoryProject::save()
{
    return saveAs(m_filename);
}

bool StoryProject::saveAs(const QString &filename)
{
    bool success = false;
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
        QJsonObject json;
        json.insert("prompt", m_prompt);
        json.insert("model", m_model);
        json.insert("temperature", m_temperature);
        json.insert("context_size", m_context);
        json.insert("globals_file", m_globalsFile);

        if (file.write(QJsonDocument(json).toJson(QJsonDocument::Indented)) > 0)
        {
            success = true;
        }
    }
    return success;
}
