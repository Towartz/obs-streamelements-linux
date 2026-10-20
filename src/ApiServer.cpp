#include "ApiServer.hpp"
#include "BackupManager.hpp"
#include <obs.h>
#include <obs-frontend-api.h>
#include <algorithm>

#define BLOG(level, msg, ...) blog(level, "[obs-streamelements] " msg, ##__VA_ARGS__)

StreamElementsApiServer::StreamElementsApiServer(quint16 port, QObject *parent)
    : QObject(parent),
      m_port(port),
      m_server(nullptr)
{
    setupDefaultHandlers();
}

StreamElementsApiServer::~StreamElementsApiServer()
{
    stop();
}

bool StreamElementsApiServer::start()
{
    if (m_server) {
        return true;
    }

    m_server = new QWebSocketServer(QStringLiteral("StreamElements OBS Bridge"),
                                    QWebSocketServer::NonSecureMode, this);

    // Try starting on default port, or find available port between 8092 and 8102
    quint16 listenPort = m_port;
    bool listening = false;
    for (int i = 0; i < 10; ++i) {
        if (m_server->listen(QHostAddress::LocalHost, listenPort + i)) {
            m_port = listenPort + i;
            listening = true;
            break;
        }
    }

    if (!listening) {
        BLOG(LOG_ERROR, "Failed to start StreamElementsApiServer on localhost ports %u-%u: %s",
             listenPort, listenPort + 9, m_server->errorString().toUtf8().constData());
        delete m_server;
        m_server = nullptr;
        return false;
    }

    connect(m_server, &QWebSocketServer::newConnection, this, &StreamElementsApiServer::onNewConnection);
    BLOG(LOG_INFO, "StreamElementsApiServer listening on ws://127.0.0.1:%u", m_port);
    return true;
}

void StreamElementsApiServer::stop()
{
    if (m_server) {
        for (auto *client : m_clients) {
            client->close();
            client->deleteLater();
        }
        m_clients.clear();
        m_server->close();
        delete m_server;
        m_server = nullptr;
        BLOG(LOG_INFO, "StreamElementsApiServer stopped");
    }
}

void StreamElementsApiServer::registerHandler(const QString &method, ApiMethodHandler handler)
{
    m_handlers[method] = handler;
}

void StreamElementsApiServer::onNewConnection()
{
    if (!m_server) return;

    QWebSocket *socket = m_server->nextPendingConnection();
    if (!socket) return;

    m_clients.push_back(socket);
    BLOG(LOG_INFO, "StreamElements client connected from %s",
         socket->peerAddress().toString().toUtf8().constData());

    connect(socket, &QWebSocket::textMessageReceived, this, &StreamElementsApiServer::onTextMessageReceived);
    connect(socket, &QWebSocket::disconnected, this, &StreamElementsApiServer::onSocketDisconnected);
}

void StreamElementsApiServer::onTextMessageReceived(const QString &message)
{
    QWebSocket *senderSocket = qobject_cast<QWebSocket *>(sender());
    if (!senderSocket) return;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        BLOG(LOG_WARNING, "Invalid JSON received: %s", parseError.errorString().toUtf8().constData());
        return;
    }

    QJsonObject req = doc.object();
    QJsonValue idVal = req.value("id");
    QString method = req.value("method").toString();
    QJsonObject params = req.value("params").toObject();

    QJsonObject response;
    if (!idVal.isUndefined()) {
        response["id"] = idVal;
    }

    auto it = m_handlers.find(method);
    if (it != m_handlers.end()) {
        try {
            QJsonObject result = it->second(params);
            response["result"] = result;
        } catch (const std::exception &e) {
            QJsonObject err;
            err["code"] = -32000;
            err["message"] = QString::fromUtf8(e.what());
            response["error"] = err;
        }
    } else {
        BLOG(LOG_WARNING, "Unknown RPC method requested: %s", method.toUtf8().constData());
        QJsonObject err;
        err["code"] = -32601;
        err["message"] = QString("Method not found: %1").arg(method);
        response["error"] = err;
    }

    QJsonDocument respDoc(response);
    senderSocket->sendTextMessage(QString::fromUtf8(respDoc.toJson(QJsonDocument::Compact)));
}

void StreamElementsApiServer::onSocketDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (!socket) return;

    auto it = std::find(m_clients.begin(), m_clients.end(), socket);
    if (it != m_clients.end()) {
        m_clients.erase(it);
    }
    socket->deleteLater();
    BLOG(LOG_INFO, "StreamElements client disconnected");
}

void StreamElementsApiServer::broadcastEvent(const QString &eventName, const QJsonObject &data)
{
    QJsonObject msg;
    msg["event"] = eventName;
    msg["data"] = data;

    QString serialized = QString::fromUtf8(QJsonDocument(msg).toJson(QJsonDocument::Compact));
    for (auto *client : m_clients) {
        if (client && client->isValid()) {
            client->sendTextMessage(serialized);
        }
    }
}

void StreamElementsApiServer::setupDefaultHandlers()
{
    // 1. getHostProperties
    registerHandler("getHostProperties", [](const QJsonObject &) -> QJsonObject {
        QJsonObject res;
        res["name"] = "obs-studio";
        res["version"] = "32.2.2";
        res["platform"] = "linux";
        res["plugin_version"] = "26.9.4.994";
        return res;
    });

    // 2. getAllScenes
    registerHandler("getAllScenes", [](const QJsonObject &) -> QJsonObject {
        QJsonObject res;
        QJsonArray scenesArray;
        char **sceneNames = obs_frontend_get_scene_names();
        if (sceneNames) {
            for (char **name = sceneNames; *name != nullptr; ++name) {
                QJsonObject sceneObj;
                sceneObj["name"] = QString::fromUtf8(*name);
                sceneObj["id"] = QString::fromUtf8(*name);
                scenesArray.append(sceneObj);
            }
            bfree(sceneNames);
        }
        res["scenes"] = scenesArray;
        return res;
    });

    // 3. getCurrentScene
    registerHandler("getCurrentScene", [](const QJsonObject &) -> QJsonObject {
        QJsonObject res;
        obs_source_t *currentScene = obs_frontend_get_current_scene();
        if (currentScene) {
            const char *name = obs_source_get_name(currentScene);
            res["name"] = QString::fromUtf8(name ? name : "");
            obs_source_release(currentScene);
        } else {
            res["name"] = "";
        }
        return res;
    });

    // 4. setCurrentSceneById
    registerHandler("setCurrentSceneById", [](const QJsonObject &params) -> QJsonObject {
        QString sceneName = params.value("sceneId").toString();
        if (sceneName.isEmpty()) {
            sceneName = params.value("name").toString();
        }
        obs_source_t *source = obs_get_source_by_name(sceneName.toUtf8().constData());
        if (source) {
            obs_frontend_set_current_scene(source);
            obs_source_release(source);
        }
        QJsonObject res;
        res["success"] = (source != nullptr);
        return res;
    });

    // 5. getStreamingStatus
    registerHandler("getStreamingStatus", [](const QJsonObject &) -> QJsonObject {
        QJsonObject res;
        res["streaming"] = obs_frontend_streaming_active();
        res["recording"] = obs_frontend_recording_active();
        res["paused"] = obs_frontend_recording_paused();
        return res;
    });

    // 6. requestStreamingStart
    registerHandler("requestStreamingStart", [](const QJsonObject &) -> QJsonObject {
        obs_frontend_streaming_start();
        QJsonObject res;
        res["success"] = true;
        return res;
    });

    // 7. requestStreamingStop
    registerHandler("requestStreamingStop", [](const QJsonObject &) -> QJsonObject {
        obs_frontend_streaming_stop();
        QJsonObject res;
        res["success"] = true;
        return res;
    });

    // 8. getAllSceneCollections
    registerHandler("getAllSceneCollections", [](const QJsonObject &) -> QJsonObject {
        QJsonObject res;
        QJsonArray collsArray;
        char **colls = obs_frontend_get_scene_collections();
        if (colls) {
            for (char **c = colls; *c != nullptr; ++c) {
                collsArray.append(QString::fromUtf8(*c));
            }
            bfree(colls);
        }
        res["collections"] = collsArray;
        return res;
    });

    // 9. getUserInterfaceState
    registerHandler("getUserInterfaceState", [](const QJsonObject &) -> QJsonObject {
        QJsonObject res;
        res["studioMode"] = obs_frontend_preview_program_mode_active();
        return res;
    });

    // 10. Backup & Restore
    registerHandler("createUserEnvironmentBackupPackage", StreamElementsBackupManager::handleCreateBackup);
    registerHandler("queryUserEnvironmentBackupPackageContents", StreamElementsBackupManager::handleQueryBackupContents);
    registerHandler("restoreUserEnvironmentBackupPackageContents", StreamElementsBackupManager::handleRestoreBackup);
}
