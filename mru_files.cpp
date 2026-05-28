#include "mru_files.h"
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

MruFiles::MruFiles(std::weak_ptr<QSettings> settings, int size) :
    m_settings(settings),
    m_size(size)
{
    if (auto settings = m_settings.lock())
    {
        for (int i = 0; i < m_size; i++)
        {
            QString value = settings->value(valueName(i)).toString();
            if (!value.isEmpty())
            {
                m_values.append(value);
            }
        }
    }
}

void MruFiles::newEntry(const QString &name)
{
    if (m_values.contains(name))
    {
        m_values.removeAll(name);
    }

    if (m_values.size() >= m_size)
    {
        m_values.resize(m_size - 1);
    }

    m_values.push_front(name);
    save();
}

QString MruFiles::recentDirectory() const
{
    if (m_values.empty())
    {
        return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    else
    {
        return QFileInfo(m_values.first()).dir().absolutePath();
    }
}

void MruFiles::save()
{
    if (auto settings = m_settings.lock())
    {
        int i = 0;
        for (const QString &value : std::as_const(m_values))
        {
            settings->setValue(valueName(i), value);
            i++;
        }
    }
}

QString MruFiles::valueName(int i)
{
    return QString("mru%1").arg(i, 2, 10, QChar('0'));
}
