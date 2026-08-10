#include "llama_cpp_chat.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

LlamaCppChat::LlamaCppChat(const QString &baseUrl, const QString &apiKey, const QString &systemPrompt, const QString &firstMessage,
                           const LlamaCppOptions &options, QObject *parent)
    : QObject(parent)
    , m_baseUrl(baseUrl)
    , m_apiKey(apiKey)
    , m_options(options)
{
    if (!systemPrompt.trimmed().isEmpty())
    {
        QJsonObject system;
        system["role"] = "system";
        system["content"] = systemPrompt;
        m_messages.append(system);
    }

    QJsonObject user;
    user["role"] = "user";
    user["content"] = firstMessage;
    m_messages.append(user);
}

void LlamaCppChat::start()
{
    doChat();
}

void LlamaCppChat::sendMessage(QString message)
{
    QJsonObject user;
    user["role"] = "user";
    user["content"] = message;
    m_messages.append(user);
    doChat();
}

void LlamaCppChat::cancel()
{
    m_cancelled = true;
    if (m_currentReply)
    {
        m_currentReply->abort();
    }
}

void LlamaCppChat::doChat()
{
    if (!m_network)
    {
        m_network = new QNetworkAccessManager(this);
    }

    QJsonObject body;
    body["messages"] = m_messages;
    body["stream"] = true;
    body["temperature"] = m_options.temperature;
    body["top_p"] = m_options.topP;
    body["top_k"] = m_options.topK;
    body["min_p"] = m_options.minP;
    body["repeat_penalty"] = m_options.repeatPenalty;

    QNetworkRequest request(QUrl(m_baseUrl + "/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_apiKey.isEmpty())
    {
        request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    }

    m_partialResponse.clear();
    m_lineBuffer.clear();

    m_currentReply = m_network->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(m_currentReply, &QNetworkReply::readyRead, this, &LlamaCppChat::onReadyRead);
    connect(m_currentReply, &QNetworkReply::finished, this, &LlamaCppChat::onFinished);
}

void LlamaCppChat::onReadyRead()
{
    m_lineBuffer += m_currentReply->readAll();

    int newlineIndex;
    while ((newlineIndex = m_lineBuffer.indexOf('\n')) != -1)
    {
        QByteArray line = m_lineBuffer.left(newlineIndex).trimmed();
        m_lineBuffer.remove(0, newlineIndex + 1);
        processLine(line);
    }
}

void LlamaCppChat::processLine(const QByteArray &line)
{
    if (!line.startsWith("data:"))
    {
        return;
    }

    QByteArray payload = line.mid(5).trimmed();
    if (payload.isEmpty() || payload == "[DONE]")
    {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isObject())
    {
        return;
    }

    QJsonArray choices = doc.object().value("choices").toArray();
    if (choices.isEmpty())
    {
        return;
    }

    QString content = choices.first().toObject().value("delta").toObject().value("content").toString();
    if (!content.isEmpty())
    {
        m_partialResponse += content;
        emit partialText(content);
    }
}

void LlamaCppChat::onFinished()
{
    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;

    const bool wasCancelled = m_cancelled;
    m_cancelled = false;

    if (!wasCancelled && reply->error() != QNetworkReply::NoError)
    {
        emit errorOccurred(reply->errorString());
    }
    else if (!wasCancelled)
    {
        QJsonObject assistant;
        assistant["role"] = "assistant";
        assistant["content"] = m_partialResponse;
        m_messages.append(assistant);
        emit responseFinished();
    }

    reply->deleteLater();
}
