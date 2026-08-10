#include "llama_cpp_options.h"
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
    repeatPenalty = 1.1;
    repeatLastN = 256;
    flashAttention = true;
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
    repeatPenalty = settings.value("repeatPenalty", repeatPenalty).toDouble();
    repeatLastN = settings.value("repeatLastN", repeatLastN).toInt();
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
    settings.setValue("repeatPenalty", repeatPenalty);
    settings.setValue("repeatLastN", repeatLastN);
    settings.setValue("flashAttention", flashAttention);
    settings.endGroup();
}
