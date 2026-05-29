#ifndef STORY_PROJECT_H
#define STORY_PROJECT_H
#include <QString>
#include <QMap>

class StoryProject
{
public:
    StoryProject();

    bool dirty() { return m_dirty; }
    bool hasFile() { return !m_filename.isEmpty();}
    bool load(const QString &filename);
    bool save();
    bool saveAs(const QString &filename);

    bool writeModelfile(const QString &filename);
    QString asModelfile() const;

    // Storage
    bool m_dirty = true;
    QString m_filename;

    using Stories = QMap<QString , QString>; // name, prompt

    // Settings
    QString m_name; // model name within ollama
    Stories m_stories;
    QString m_selectedStory;
    QString m_model;
    double m_temperature;
    int m_context; // number of tokens to remember
    QString m_globalPrompt; // a system prompt that applies to all story prompts
    QString m_projectNotes; // notes about the whole project or system

};

#endif // STORY_PROJECT_H
