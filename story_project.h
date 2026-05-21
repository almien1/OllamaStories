#ifndef STORY_PROJECT_H
#define STORY_PROJECT_H
#include <QString>

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

    // Settings
    QString m_prompt;
    QString m_model;
    double m_temperature;
    int m_context;

    // References
    QString m_globalsFile;

};

#endif // STORY_PROJECT_H
