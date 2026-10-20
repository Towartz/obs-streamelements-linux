#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

class StreamElementsAuthServer : public QObject {
    Q_OBJECT

public:
    explicit StreamElementsAuthServer(QObject *parent = nullptr);
    ~StreamElementsAuthServer() override;

    bool start(quint16 preferredPort = 4269);
    void stop();
    bool isListening() const;
    quint16 port() const;

signals:
    void tokenReceived(const QString &token);
    void authFailed(const QString &reason);

private slots:
    void onNewConnection();
    void onClientReadyRead();

private:
    void handleHttpRequest(QTcpSocket *socket, const QByteArray &data);
    void sendResponse(QTcpSocket *socket, int statusCode, const QString &statusText, const QString &contentType, const QByteArray &body);

    QTcpServer *m_tcpServer = nullptr;
    quint16 m_port = 0;
};
