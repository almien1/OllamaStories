#ifndef LLAMA_CPP_SERVER_H
#define LLAMA_CPP_SERVER_H
#include <QObject>
#include <QString>
#include <QProcess>
#include "llama_cpp_options.h"

class QNetworkAccessManager;
class QTimer;

// Manages a local `llama-server` subprocess and reports back once its
// HTTP API is ready to accept chat requests.
class LlamaCppServer : public QObject
{
    Q_OBJECT
public:
    explicit LlamaCppServer(QObject *parent = nullptr);
    ~LlamaCppServer() override;

    void start(const LlamaCppOptions &options);
    void stop();
    bool isRunning() const;
    QString baseUrl() const { return m_baseUrl; }

signals:
    void ready();
    void failed(QString error);
    void logOutput(QString text);

private slots:
    void checkHealth();
    void onProcessErrorOccurred(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    QProcess *m_process = nullptr;
    QNetworkAccessManager *m_network = nullptr;
    QTimer *m_healthTimer = nullptr;
    QString m_baseUrl;
    int m_healthAttempts = 0;
};

#endif // LLAMA_CPP_SERVER_H
