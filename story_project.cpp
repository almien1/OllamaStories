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
                    QJsonValue storyJson = storiesobj.value(name);
                    if (storyJson.isString())
                    {
                        // original format
                        Story story;
                        story.prompt = storyJson.toString();
                        m_stories.insert(name, story);
                    }
                    else
                    {
                        // original format
                        Story story;
                        story.prompt = storyJson.toObject().value("prompt").toString();
                        story.notes = storyJson.toObject().value("notes").toString();
                        m_stories.insert(name, story);

                    }
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
        json.insert("global_prompt", m_globalPrompt);
        json.insert("current", m_selectedStory);
        json.insert("project_notes", m_projectNotes);
        QJsonObject stories;
        for (const auto &name : m_stories.keys())
        {
            QJsonObject story;
            story.insert("prompt", m_stories.value(name).prompt);
            story.insert("notes", m_stories.value(name).notes);
            stories.insert(name, story);
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

QString StoryProject::combinedPrompt() const
{
    return m_stories.value(m_selectedStory).prompt + "\n\n" + m_globalPrompt;
}
