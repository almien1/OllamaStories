#include "story_project.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStringList>

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
            m_name = json.value("project_name").toString("untitled_llama_story");
            m_model = json.value("model").toString("gemma3:12b");
            m_temperature = json.value("temperature").toDouble();
            m_context = json.value("context_size").toInt(16384);
            m_globalPrompt = json.value("global_prompt").toString("");
            m_selectedStory = json.value("current").toString("");
            m_projectNotes = json.value("project_notes").toString("");

            m_stories.clear();
            QJsonValue stories = json.value("stories");
            if (stories.isObject())
            {
                QJsonObject storiesobj = stories.toObject();
                for (auto name : storiesobj.keys())
                {
                    m_stories.insert(name, storiesobj.value(name).toString());
                }
            }
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
        json.insert("project_name", m_name);
        json.insert("global_prompt", m_globalPrompt);
        json.insert("model", m_model);
        json.insert("temperature", m_temperature);
        json.insert("context_size", m_context);
        json.insert("current", m_selectedStory);
        json.insert("project_notes", m_projectNotes);
        QJsonObject stories;
        for (const auto &name : m_stories.keys())
        {
            stories.insert(name, m_stories.value(name));
        }
        json.insert("stories", stories);

        if (file.write(QJsonDocument(json).toJson(QJsonDocument::Indented)) > 0)
        {
            success = true;
            m_filename = filename;
        }
    }
    return success;
}

bool StoryProject::writeModelfile(const QString &filename)
{
    bool success = false;
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(asModelfile().toUtf8());
        success = true;
    }
    return success;
}

QString StoryProject::asModelfile() const
{
    QStringList lines;
    lines.append(QString("FROM %1").arg(m_model));
    lines.append(QString("PARAMETER temperature %1").arg(m_temperature));
    lines.append(QString("PARAMETER num_ctx %1").arg(m_context));

    lines.append(R"(SYSTEM """)");
    lines.append(m_stories[m_selectedStory].split("\n"));
    lines.append("");
    lines.append(m_globalPrompt.split("\n"));
    lines.append(R"(""")");
    return lines.join("\n");
}
