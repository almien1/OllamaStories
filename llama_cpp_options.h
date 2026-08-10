#ifndef LLAMA_CPP_OPTIONS_H
#define LLAMA_CPP_OPTIONS_H
#include <QString>
#include <QStringList>

class QSettings;

// Options for running a local llama.cpp server/CLI, tuned by default for
// long, in-character roleplay chats rather than short factual answers.
class LlamaCppOptions
{
public:
    LlamaCppOptions();

    void load(QSettings &settings);
    void save(QSettings &settings) const;

    QString modelPath() const;
    QString cliExecutablePath() const;
    QString baseUrl() const;

    // Arguments to launch `llama-server`, ready to serve chat completions.
    QStringList serverArguments() const;

    // Arguments to launch `llama-cli` for an interactive, detached terminal
    // roleplay session using the same sampling/model settings. Takes a path
    // to a file holding the system prompt (avoids cmd.exe quoting issues).
    QStringList cliArguments(const QString &systemPromptFilePath) const;

    // Resets the sampling parameters (not paths/model) to the recommended
    // roleplay-friendly defaults.
    void applyRoleplayDefaults();

    QString serverPath = "llama-server.exe";
    QString modelsDir;
    QString modelFile;
    QString host = "127.0.0.1";

    // Not user-configurable: LlamaCppServer assigns a free port to this just
    // before launching, since we're the only client that ever talks to it.
    int port = 0;

    int contextSize = 8192;
    int gpuLayers = 999; // offload as many layers to GPU as possible

    double temperature = 0.8;
    double topP = 0.95;
    int topK = 64;
    double minP = 0.05;
    double repeatPenalty = 1.1;
    int repeatLastN = 256;

    bool flashAttention = true;

    // When true, pass --jinja so llama.cpp uses the chat template embedded
    // in the model itself (best for roleplay finetunes with their own
    // format). When false, chatTemplate names an explicit override.
    bool useModelChatTemplate = true;
    QString chatTemplate;
};

#endif // LLAMA_CPP_OPTIONS_H
