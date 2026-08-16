#include "llama_cpp_options.h"
#include "gguf_info.h"
#include "gpu_info.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>

LlamaCppOptions::LlamaCppOptions() {}

void LlamaCppOptions::applyRoleplayDefaults()
{
    temperature = 0.8;
    topP = 0.95;
    topK = 64;
    minP = 0.05;
    flashAttention = true;
}

QString LlamaCppOptions::applySystemOptimizedDefaults()
{
    // Sampling isn't hardware-dependent - same values as the general
    // roleplay defaults.
    applyRoleplayDefaults();

    auto fallback = [this](const QString &reason) -> QString
    {
        // Same numbers the pre-dynamic version of this button used: sized
        // for a 12GB-VRAM GPU running the curated 8-13B roleplay models at
        // Q4_K_M/Q5_K_M (see models/recommended_models.html).
        contextSize = 16384;
        gpuLayersAll = true;
        gpuLayers = 999;
        return QString("Couldn't tailor this to your hardware (%1) - used built-in defaults for a 12GB-class GPU.")
            .arg(reason);
    };

    if (modelFile.isEmpty())
    {
        return fallback("no model selected");
    }

    GgufInfo model = readGgufInfo(modelPath());
    if (!model.valid || model.blockCount <= 0)
    {
        return fallback("couldn't read the model's metadata");
    }

    qint64 modelBytes = QFileInfo(modelPath()).size();
    if (modelBytes <= 0)
    {
        return fallback("couldn't read the model file's size");
    }

    // Per-head key/value dimension: prefer the explicit metadata keys, fall
    // back to hidden-size / head-count (standard pre-GQA sizing) if the file
    // omits them.
    int headCountKv = model.headCountKv > 0 ? model.headCountKv : model.headCount;
    int headDim = model.keyLength > 0 ? model.keyLength
                : (model.headCount > 0 && model.embeddingLength > 0) ? model.embeddingLength / model.headCount
                                                                      : 0;
    int valueDim = model.valueLength > 0 ? model.valueLength : headDim;
    if (headCountKv <= 0 || headDim <= 0)
    {
        return fallback("the model's attention shape is missing from its metadata");
    }

    GpuVramInfo gpu = queryPrimaryGpuVram();
    if (!gpu.valid || gpu.freeBytes == 0)
    {
        return fallback("couldn't detect GPU memory");
    }

    // Default llama.cpp KV cache is fp16 (2 bytes/element) for both K and V;
    // this app doesn't set --cache-type-k/v, so that default always applies.
    constexpr qint64 bytesPerKvElement = 2;
    qint64 kvBytesPerToken = qint64(headCountKv) * (headDim + valueDim) * bytesPerKvElement * model.blockCount;

    // Reserve headroom beyond weights+KV for compute buffers and whatever
    // else is sharing the card - freeBytes is already "budget minus current
    // usage", so this is on top of that.
    constexpr qint64 safetyMarginBytes = qint64(1024) * 1024 * 1024;
    qint64 usableBytes = qint64(gpu.freeBytes) - safetyMarginBytes;

    QString gpuLabel = gpu.adapterName.isEmpty() ? QStringLiteral("your GPU") : gpu.adapterName;
    QString gpuGiB = QString::number(gpu.freeBytes / (1024.0 * 1024.0 * 1024.0), 'f', 1);

    if (usableBytes > modelBytes && kvBytesPerToken > 0)
    {
        qint64 ctx = (usableBytes - modelBytes) / kvBytesPerToken;
        if (model.trainedContextLength > 0)
        {
            ctx = qMin(ctx, qint64(model.trainedContextLength));
        }
        ctx = qBound(qint64(512), ctx, qint64(131072));
        ctx -= ctx % 512; // match the context spinbox's step, keeps the number tidy

        contextSize = int(ctx);
        gpuLayersAll = true;
        gpuLayers = 999;

        return QString("Fit to %1 (%2 GB free): all %3 layers on GPU, context %4.")
            .arg(gpuLabel, gpuGiB, QString::number(model.blockCount), QString::number(contextSize));
    }

    // Whole model doesn't fit - keep a modest fixed context and offload as
    // many layers as fit, estimating VRAM/layer as an even split of the file
    // size. Layers aren't perfectly uniform in size, so treat this as a
    // starting point to fine-tune GPU layers from by hand, not a guarantee.
    contextSize = 8192;
    qint64 vramForLayers = usableBytes - kvBytesPerToken * contextSize;
    qint64 bytesPerLayer = modelBytes / model.blockCount;
    qint64 layers = bytesPerLayer > 0 ? vramForLayers / bytesPerLayer : 0;
    layers = qBound(qint64(0), layers, qint64(model.blockCount));

    gpuLayersAll = false;
    gpuLayers = int(layers);

    return QString("%1 (%2 GB free) can't fit the whole model: offloading %3/%4 layers, context kept at %5.")
        .arg(gpuLabel, gpuGiB, QString::number(gpuLayers), QString::number(model.blockCount),
             QString::number(contextSize));
}

QString LlamaCppOptions::modelPath() const
{
    if (QFileInfo(modelFile).isAbsolute())
    {
        return modelFile;
    }
    return QDir(modelsDir).filePath(modelFile);
}

QString LlamaCppOptions::cliExecutablePath() const
{
    return QFileInfo(serverPath).dir().filePath("llama-cli.exe");
}

QString LlamaCppOptions::baseUrl() const
{
    return QString("http://%1:%2").arg(host).arg(port);
}

QStringList LlamaCppOptions::serverArguments() const
{
    QStringList args = {
        "-m", modelPath(),
        "-c", QString::number(contextSize),
        "-ngl", QString::number(gpuLayersAll ? 999 : gpuLayers),
        "--port", QString::number(port),
        "--host", host,
        "--temp", QString::number(temperature),
        "--top-p", QString::number(topP),
        "--top-k", QString::number(topK),
        "--min-p", QString::number(minP),
        "--repeat-penalty", QString::number(repeatPenalty),
        "--repeat-last-n", QString::number(repeatLastN),
        "--dry-multiplier", QString::number(dryMultiplier),
        "--dry-base", QString::number(dryBase),
        "--dry-allowed-length", QString::number(dryAllowedLength),
        "--dry-penalty-last-n", QString::number(dryPenaltyLastN),
    };

    if (flashAttention)
    {
        args += {"-fa", "on"};
    }

    // Always defer to the chat template embedded in the model itself, so it
    // sees prompts in the format it was actually trained on.
    args += "--jinja";

    if (!apiKey.isEmpty())
    {
        args += {"--api-key", apiKey};
    }

    return args;
}

QStringList LlamaCppOptions::cliArguments(const QString &systemPromptFilePath) const
{
    QStringList args = {
        "-m", modelPath(),
        "-c", QString::number(contextSize),
        "-ngl", QString::number(gpuLayersAll ? 999 : gpuLayers),
        "--temp", QString::number(temperature),
        "--top-p", QString::number(topP),
        "--top-k", QString::number(topK),
        "--min-p", QString::number(minP),
        "--repeat-penalty", QString::number(repeatPenalty),
        "--repeat-last-n", QString::number(repeatLastN),
        "--dry-multiplier", QString::number(dryMultiplier),
        "--dry-base", QString::number(dryBase),
        "--dry-allowed-length", QString::number(dryAllowedLength),
        "--dry-penalty-last-n", QString::number(dryPenaltyLastN),
        "-cnv",
    };

    if (flashAttention)
    {
        args += {"-fa", "on"};
    }

    args += "--jinja";

    if (!systemPromptFilePath.isEmpty())
    {
        args += {"--system-prompt-file", systemPromptFilePath};
    }

    return args;
}

void LlamaCppOptions::load(QSettings &settings)
{
    settings.beginGroup("llamacpp");
    serverPath = settings.value("serverPath", serverPath).toString();
    modelsDir = settings.value("modelsDir", modelsDir).toString();
    modelFile = settings.value("modelFile", modelFile).toString();
    host = settings.value("host", host).toString();
    contextSize = settings.value("contextSize", contextSize).toInt();
    gpuLayersAll = settings.value("gpuLayersAll", gpuLayersAll).toBool();
    gpuLayers = settings.value("gpuLayers", gpuLayers).toInt();
    temperature = settings.value("temperature", temperature).toDouble();
    topP = settings.value("topP", topP).toDouble();
    topK = settings.value("topK", topK).toInt();
    minP = settings.value("minP", minP).toDouble();
    flashAttention = settings.value("flashAttention", flashAttention).toBool();
    settings.endGroup();
}

void LlamaCppOptions::save(QSettings &settings) const
{
    settings.beginGroup("llamacpp");
    settings.setValue("serverPath", serverPath);
    settings.setValue("modelsDir", modelsDir);
    settings.setValue("modelFile", modelFile);
    settings.setValue("host", host);
    settings.setValue("contextSize", contextSize);
    settings.setValue("gpuLayersAll", gpuLayersAll);
    settings.setValue("gpuLayers", gpuLayers);
    settings.setValue("temperature", temperature);
    settings.setValue("topP", topP);
    settings.setValue("topK", topK);
    settings.setValue("minP", minP);
    settings.setValue("flashAttention", flashAttention);
    settings.endGroup();
}
