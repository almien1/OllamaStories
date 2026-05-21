#ifndef OLLAMA_CLI_H
#define OLLAMA_CLI_H
#include <QString>

class OllamaCLI
{
public:
    void compileModel(const QString &projectName, const QString &modelfile);
};

#endif // OLLAMA_CLI_H
