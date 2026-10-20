#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <functional>
#include <map>
#include <vector>

using ApiMethodHandler = std::function<QJsonObject(const QJsonObject &params)>;

class StreamElementsApiServer : public QObject {
    Q_OBJECT

public:
    explicit StreamElementsApiServer(quint16 port = 8092, QObject *parent = nullptr);
    virtual ~StreamElementsApiServer();

    bool start();
    void stop();
    quint16 port() const { return m_port; }

    void registerHandler(const QString &method, ApiMethodHandler handler);
    void broadcastEvent(const QString &eventName, const QJsonObject &data);

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString &message);
    void onSocketDisconnected();

private:
    void setupDefaultHandlers();

    quint16 m_port;
    QWebSocketServer *m_server;
    std::vector<QWebSocket*> m_clients;
    std::map<QString, ApiMethodHandler> m_handlers;
};
