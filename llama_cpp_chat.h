#ifndef LLAMA_CPP_CHAT_H
#define LLAMA_CPP_CHAT_H
#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QByteArray>
#include "llama_cpp_options.h"

class QNetworkAccessManager;
class QNetworkReply;

// Drives a streaming chat conversation against a running llama-server's
// OpenAI-compatible /v1/chat/completions endpoint. Mirrors Conversation's
// slot/signal shape so it can be dropped into the same worker-thread setup.
class LlamaCppChat : public QObject
{
    Q_OBJECT
public:
    LlamaCppChat(const QString &baseUrl, const QString &apiKey, const QString &systemPrompt, const QString &firstMessage,
                 const LlamaCppOptions &options, QObject *parent);

public slots:
    void start();
    void sendMessage(QString message);
    void cancel();

signals:
    void partialText(QString text);
    void responseFinished();
    void errorOccurred(QString error);

private slots:
    void onReadyRead();
    void onFinished();

private:
    void doChat();
    void processLine(const QByteArray &line);

    QNetworkAccessManager *m_network = nullptr;
    QNetworkReply *m_currentReply = nullptr;

    QString m_baseUrl;
    QString m_apiKey;
    QJsonArray m_messages;
    LlamaCppOptions m_options;

    QString m_partialResponse;
    QByteArray m_lineBuffer;
    bool m_cancelled = false;
};

#endif // LLAMA_CPP_CHAT_H
