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

    QString combinedPrompt() const;

    // Storage
    bool m_dirty = true;
    QString m_filename;

    using Story = struct{QString prompt; QString notes;};
    using Stories = QMap<QString , Story>;

    // Settings
    Stories m_stories;
    QString m_selectedStory;
    QString m_globalPrompt; // a system prompt that applies to all story prompts
    QString m_projectNotes; // notes about the whole project or system

};

#endif // STORY_PROJECT_H
