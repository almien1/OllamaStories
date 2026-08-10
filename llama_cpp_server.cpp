#include "llama_cpp_server.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

namespace {
const int kHealthPollIntervalMs = 500;
const int kHealthMaxAttempts = 120; // ~1 minute for the model to load
}

LlamaCppServer::LlamaCppServer(QObject *parent)
    : QObject(parent)
{
}

LlamaCppServer::~LlamaCppServer()
{
    stop();
}

bool LlamaCppServer::isRunning() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

void LlamaCppServer::stop()
{
    if (m_healthTimer)
    {
        m_healthTimer->stop();
    }
    if (m_process)
    {
        m_process->disconnect(this);
        if (m_process->state() != QProcess::NotRunning)
        {
            m_process->kill();
            m_process->waitForFinished(3000);
        }
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void LlamaCppServer::start(const LlamaCppOptions &options)
{
    stop();

    m_baseUrl = options.baseUrl();
    m_healthAttempts = 0;

    if (!m_network)
    {
        m_network = new QNetworkAccessManager(this);
    }

    m_process = new QProcess(this);
    connect(m_process, &QProcess::errorOccurred, this, &LlamaCppServer::onProcessErrorOccurred);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &LlamaCppServer::onProcessFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &LlamaCppServer::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &LlamaCppServer::onReadyReadStandardError);

    m_process->start(options.serverPath, options.serverArguments());

    if (!m_healthTimer)
    {
        m_healthTimer = new QTimer(this);
        connect(m_healthTimer, &QTimer::timeout, this, &LlamaCppServer::checkHealth);
    }
    m_healthTimer->start(kHealthPollIntervalMs);
}

void LlamaCppServer::checkHealth()
{
    if (!isRunning())
    {
        return;
    }

    m_healthAttempts++;
    if (m_healthAttempts > kHealthMaxAttempts)
    {
        m_healthTimer->stop();
        emit failed("Timed out waiting for llama-server to become ready");
        return;
    }

    QNetworkRequest request(QUrl(m_baseUrl + "/health"));
    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError)
        {
            if (m_healthTimer)
            {
                m_healthTimer->stop();
            }
            emit ready();
        }
    });
}

void LlamaCppServer::onProcessErrorOccurred(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart)
    {
        if (m_healthTimer)
        {
            m_healthTimer->stop();
        }
        emit failed("Could not start llama-server. Check the executable path in the llama.cpp options tab.");
    }
}

void LlamaCppServer::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    if (m_healthTimer && m_healthTimer->isActive())
    {
        m_healthTimer->stop();
        emit failed(QString("llama-server exited early (code %1). Check the log for details.").arg(exitCode));
    }
}

void LlamaCppServer::onReadyReadStandardOutput()
{
    if (m_process)
    {
        emit logOutput(QString::fromUtf8(m_process->readAllStandardOutput()));
    }
}

void LlamaCppServer::onReadyReadStandardError()
{
    if (m_process)
    {
        emit logOutput(QString::fromUtf8(m_process->readAllStandardError()));
    }
}
