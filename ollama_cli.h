#ifndef OLLAMA_CLI_H
#define OLLAMA_CLI_H
#include <QString>

class OllamaCLI
{
public:
    bool compileModel(const QString &projectName, const QString &modelfile);
};

#endif // OLLAMA_CLI_H
