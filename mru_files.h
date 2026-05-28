#ifndef MRU_FILES_H
#define MRU_FILES_H
#include <QSettings>

class MruFiles
{
public:
    MruFiles(std::weak_ptr<QSettings> settings, int size = 5);
    void newEntry(const QString &name);
    QString recentDirectory() const;
    QStringList m_values;

private:
    void save();
    static QString valueName(int i);
    std::weak_ptr<QSettings> m_settings;
    int m_size;
};

#endif // MRU_FILES_H
