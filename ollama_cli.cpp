#include "ollama_cli.h"
#include <QProcess>
#include <QDebug>

bool OllamaCLI::compileModel(const QString &projectName, const QString &modelfile)
{
    QProcess p;
    p.start("ollama", {"create", projectName, "-f", modelfile});

    if (p.waitForFinished())
    {
        qInfo() << p.readAllStandardOutput();
        qInfo() << p.readAllStandardError();
        return (p.exitCode() == 0);
    }
    else
    {
        p.kill();
        return false;
    }
}
