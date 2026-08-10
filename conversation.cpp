#include "conversation.h"

Conversation::Conversation(const QString &model, const QString &systemPrompt, const QString &firstMessage, QObject *parent) :
    QObject(parent),
    m_model(model),
    m_ollama(std::make_shared<Ollama>())
{
    if (!systemPrompt.isEmpty())
    {
        m_messageQueue.push_back(ollama::message("system", systemPrompt.toStdString()));
    }
    m_messageQueue.push_back(ollama::message("user", firstMessage.toStdString()));
}

void Conversation::start()
{
    doChat();
}

void Conversation::sendMessage(QString message)
{
    m_messageQueue.push_back(ollama::message("user", message.toStdString()));
    doChat();
}

void Conversation::cancel()
{
    m_cancelled = true;
}

void Conversation::doChat()
{
    std::function<bool(const ollama::response&)> callback = [this](const ollama::response&responseObj){
        QString response = QString::fromStdString(responseObj.as_simple_string());

        m_partialResponse += response;
        if (!response.trimmed().startsWith("⌀"))
        {
            emit partialText(response);
        }

        if (responseObj.as_json()["done"] == true)
        {
            m_messageQueue.push_back(ollama::message("assistant", m_partialResponse.toStdString()));
            emit responseFinished();
        }

        if (m_cancelled)
        {
            m_cancelled = false;
            return false;
        }
        return true;
    };

    m_ollama->chat(m_model.toStdString(), m_messageQueue, callback, m_options);

}
