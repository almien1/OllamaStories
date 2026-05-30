#ifndef CONVERSATION_H
#define CONVERSATION_H
#include <QObject>
#include <memory>
#include "ollama-hpp/singleheader/ollama.hpp"

class Conversation : public QObject
{
    Q_OBJECT
public:
    Conversation(const QString &model, const QString &systemPrompt, const QString &firstMessage, QObject *parent);

public slots:
    void start();
    void sendMessage(QString message);
    void cancel();

signals:
    void partialText(QString text);
    void responseFinished();

private:
    void doChat();

    QString m_partialResponse;
    QString m_model;
    ollama::messages m_messageQueue;
    ollama::messages m_options;
    std::shared_ptr<Ollama> m_ollama;
    bool m_cancelled = false;
};

#endif // CONVERSATION_H
