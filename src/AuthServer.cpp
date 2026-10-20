#include "AuthServer.hpp"
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <obs.h>

#define BLOG(level, msg, ...) blog(level, "[obs-streamelements] " msg, ##__VA_ARGS__)

StreamElementsAuthServer::StreamElementsAuthServer(QObject *parent)
    : QObject(parent),
      m_tcpServer(new QTcpServer(this))
{
    connect(m_tcpServer, &QTcpServer::newConnection, this, &StreamElementsAuthServer::onNewConnection);
}

StreamElementsAuthServer::~StreamElementsAuthServer()
{
    stop();
}

bool StreamElementsAuthServer::start(quint16 preferredPort)
{
    stop();

    // First attempt to bind to preferred port (4269 is standard for SE.Live)
    if (m_tcpServer->listen(QHostAddress::LocalHost, preferredPort)) {
        m_port = m_tcpServer->serverPort();
        BLOG(LOG_INFO, "StreamElements OAuth loopback server listening on http://127.0.0.1:%d", m_port);
        return true;
    }

    // If preferred port is busy, fallback to any available port
    if (m_tcpServer->listen(QHostAddress::LocalHost, 0)) {
        m_port = m_tcpServer->serverPort();
        BLOG(LOG_INFO, "StreamElements OAuth loopback server listening on fallback port http://127.0.0.1:%d", m_port);
        return true;
    }

    BLOG(LOG_ERROR, "Failed to start StreamElements OAuth loopback server: %s",
         m_tcpServer->errorString().toUtf8().constData());
    return false;
}

void StreamElementsAuthServer::stop()
{
    if (m_tcpServer && m_tcpServer->isListening()) {
        m_tcpServer->close();
        BLOG(LOG_INFO, "Stopped StreamElements OAuth loopback server");
    }
    m_port = 0;
}

bool StreamElementsAuthServer::isListening() const
{
    return m_tcpServer && m_tcpServer->isListening();
}

quint16 StreamElementsAuthServer::port() const
{
    return m_port;
}

void StreamElementsAuthServer::onNewConnection()
{
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *socket = m_tcpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &StreamElementsAuthServer::onClientReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
}

void StreamElementsAuthServer::onClientReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray data = socket->readAll();
    handleHttpRequest(socket, data);
}

void StreamElementsAuthServer::sendResponse(QTcpSocket *socket, int statusCode, const QString &statusText, const QString &contentType, const QByteArray &body)
{
    QString header = QString("HTTP/1.1 %1 %2\r\n"
                             "Content-Type: %3; charset=utf-8\r\n"
                             "Content-Length: %4\r\n"
                             "Access-Control-Allow-Origin: *\r\n"
                             "Access-Control-Allow-Private-Network: true\r\n"
                             "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                             "Access-Control-Allow-Headers: *\r\n"
                             "Access-Control-Max-Age: 86400\r\n"
                             "Connection: close\r\n"
                             "\r\n")
                     .arg(statusCode)
                     .arg(statusText)
                     .arg(contentType)
                     .arg(body.size());

    socket->write(header.toUtf8());
    if (!body.isEmpty()) {
        socket->write(body);
    }
    socket->flush();
    socket->disconnectFromHost();
}

void StreamElementsAuthServer::handleHttpRequest(QTcpSocket *socket, const QByteArray &data)
{
    QString request = QString::fromUtf8(data);
    int firstLineEnd = request.indexOf("\r\n");
    if (firstLineEnd == -1) {
        firstLineEnd = request.indexOf('\n');
    }
    if (firstLineEnd == -1) return;

    QString requestLine = request.left(firstLineEnd);
    QStringList parts = requestLine.split(' ');
    if (parts.size() < 2) return;

    QString method = parts[0].toUpper();
    QString rawPath = parts[1];

    BLOG(LOG_INFO, "Loopback HTTP request: %s %s", method.toUtf8().constData(), rawPath.toUtf8().constData());

    // 1. Handle CORS Preflight / Private Network Access
    if (method == "OPTIONS") {
        sendResponse(socket, 204, "No Content", "text/plain", "");
        return;
    }

    QUrl url("http://127.0.0.1" + rawPath);
    QString path = url.path();
    QUrlQuery query(url);

    // 2. Health check endpoint (checked by selive pre-auth)
    if (path == "/healthz") {
        sendResponse(socket, 200, "OK", "text/plain", "ok");
        return;
    }

    // 3. Handle POST token or callback
    if (method == "POST") {
        int bodyStart = request.indexOf("\r\n\r\n");
        QByteArray body;
        if (bodyStart != -1) {
            body = data.mid(bodyStart + 4);
        }

        QString foundToken;
        QJsonDocument doc = QJsonDocument::fromJson(body);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("token") && obj["token"].isString()) {
                foundToken = obj["token"].toString();
            } else if (obj.contains("channel") && obj["channel"].isObject()) {
                foundToken = obj["channel"].toObject().value("token").toString();
            }
        }

        if (!foundToken.isEmpty()) {
            BLOG(LOG_INFO, "Captured StreamElements JWT token via HTTP POST");
            sendResponse(socket, 200, "OK", "application/json", "{\"status\":\"ok\"}");
            emit tokenReceived(foundToken);
            return;
        }

        sendResponse(socket, 200, "OK", "application/json", "{\"status\":\"ok\"}");
        return;
    }

    // 4. Handle GET callback
    if (method == "GET") {
        QString token = query.queryItemValue("token");
        if (token.isEmpty()) {
            token = query.queryItemValue("creator");
        }

        // Beautiful completion page
        QByteArray html = 
            "<!DOCTYPE html>"
            "<html><head><meta charset='utf-8'><title>StreamElements Authenticated</title>"
            "<style>"
            "body { background: #12141c; color: #fff; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; display: flex; align-items: center; justify-content: center; height: 100vh; margin: 0; }"
            ".card { background: #1c2030; padding: 44px; border-radius: 16px; text-align: center; box-shadow: 0 16px 32px rgba(0,0,0,0.4); max-width: 480px; border: 1px solid #2e344d; }"
            ".check { font-size: 52px; color: #4ade80; margin-bottom: 16px; }"
            "h1 { font-size: 24px; margin: 0 0 10px; color: #fff; }"
            "p { color: #94a3b8; font-size: 15px; line-height: 1.5; margin: 0 0 20px; }"
            ".btn { background: #f53656; color: #fff; border: none; padding: 10px 24px; border-radius: 8px; font-weight: 600; cursor: pointer; text-decoration: none; display: inline-block; }"
            "</style></head><body>"
            "<div class='card'>"
            "<div class='check'>&#10003;</div>"
            "<h1>StreamElements Connected!</h1>"
            "<p>You are now signed into StreamElements on OBS Studio. You can safely close this browser window and return to OBS.</p>"
            "<button class='btn' onclick='window.close()'>Close Window</button>"
            "</div>"
            "<script>"
            "const hash = window.location.hash.substring(1);"
            "const p = new URLSearchParams(window.location.search || hash);"
            "const t = p.get('token') || p.get('creator');"
            "if (t) {"
            "  fetch('/auth/connect/token', {"
            "    method: 'POST',"
            "    headers: {'Content-Type': 'application/json'},"
            "    body: JSON.stringify({token: t})"
            "  }).then(() => setTimeout(() => window.close(), 1200));"
            "} else {"
            "  setTimeout(() => window.close(), 2000);"
            "}"
            "</script></body></html>";

        sendResponse(socket, 200, "OK", "text/html", html);

        if (!token.isEmpty()) {
            BLOG(LOG_INFO, "Captured StreamElements JWT token via HTTP GET query string");
            emit tokenReceived(token);
        }
        return;
    }

    sendResponse(socket, 404, "Not Found", "text/plain", "Not Found");
}
