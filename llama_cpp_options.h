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

    // Resets sampling to the roleplay defaults, then sizes context size and
    // GPU offload to fit the currently selected model into this machine's
    // actual free VRAM (queried via DXGI) using the model's own GGUF
    // metadata for the KV-cache/layer-size math. Falls back to a fixed
    // estimate for a 12GB-class GPU if no model is selected, its metadata
    // can't be read, or VRAM can't be queried (old Windows, no GPU, etc).
    // Returns a one-line explanation of what it did, for display in the UI.
    QString applySystemOptimizedDefaults();

    QString serverPath = "llama-server.exe";
    QString modelsDir;
    QString modelFile;
    QString host = "127.0.0.1";

    // Not user-configurable: LlamaCppServer assigns a free port and a random
    // per-launch API key to these just before launching, since we're the
    // only client that ever talks to it. The key stops other local
    // processes - or a malicious webpage's JS - from using the server
    // while it's running.
    int port = 0;
    QString apiKey;

    int contextSize = 8192;

    // When true, offload every layer to the GPU (llama.cpp's own "999 always
    // means all" convention) regardless of how many layers the model has.
    // When false, gpuLayers gives an explicit count instead.
    bool gpuLayersAll = true;
    int gpuLayers = 999;

    double temperature = 0.8;
    double topP = 0.95;
    int topK = 64;
    double minP = 0.05;

    // Everything below is deliberately not user-configurable: these are a
    // matched pair of repetition controls sized from the shape of a roleplay
    // turn (a few paragraphs from the model, one from the user, ~400-650
    // tokens), not values someone should need to hand-tune per model.
    //
    // repeatPenalty/repeatLastN are mild and short-range - just enough to
    // stop the model getting stuck stuttering on a single token. DRY handles
    // phrase-level repetition (whole recurring sentences/openers) instead:
    // it only penalizes actual repeated multi-token *sequences* (broken by
    // the default sequence breakers: newline, ':', '"', '*'), never a lone
    // repeated word, so its window can safely span several turns (2048
    // tokens) without the "avoids saying names" problem a classic repeat
    // penalty would have at that range.
    double repeatPenalty = 1.05;
    int repeatLastN = 64;
    double dryMultiplier = 0.8;
    double dryBase = 1.75;
    int dryAllowedLength = 2;
    int dryPenaltyLastN = 2048;

    bool flashAttention = true;
};

#endif // LLAMA_CPP_OPTIONS_H
