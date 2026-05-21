#include "ollama_cli.h"
#include <QProcess>
#include <QDebug>

void OllamaCLI::compileModel(const QString &projectName, const QString &modelfile)
{
    QProcess p;
    p.start("ollama", {"create", projectName, "-f", modelfile});

    if (p.waitForFinished())
    {
        qInfo() << p.readAllStandardOutput();
        qInfo() << p.readAllStandardError();
        qInfo() << "exited with code" << p.exitCode();
    }
    else
    {
        p.kill();
    }
}
