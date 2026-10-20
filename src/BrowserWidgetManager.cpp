#include "BrowserWidgetManager.hpp"
#include <obs.h>
#include <obs-frontend-api.h>

#define BLOG(level, msg, ...) blog(level, "[obs-streamelements] " msg, ##__VA_ARGS__)

StreamElementsBrowserWidgetManager::StreamElementsBrowserWidgetManager(QMainWindow *mainWindow, QObject *parent)
    : QObject(parent),
      m_mainWindow(mainWindow)
{
}

void StreamElementsBrowserWidgetManager::registerDefaultDocks()
{
    if (!m_mainWindow) return;

    // 1. Activity Feed Dock
    auto *activityFeed = new StreamElementsBrowserDock(
        "se_activity_feed",
        tr("StreamElements Activity Feed"),
        QUrl("https://streamelements.com/activity-feed"),
        m_mainWindow
    );
    m_docks["se_activity_feed"] = activityFeed;
    obs_frontend_add_custom_qdock("se_activity_feed", static_cast<void*>(activityFeed));

    // 2. Chat Dock
    auto *chatDock = new StreamElementsBrowserDock(
        "se_chat",
        tr("StreamElements Chat"),
        QUrl("https://streamelements.com/chat"),
        m_mainWindow
    );
    m_docks["se_chat"] = chatDock;
    obs_frontend_add_custom_qdock("se_chat", static_cast<void*>(chatDock));

    // 3. Media Request Dock
    auto *mediaDock = new StreamElementsBrowserDock(
        "se_media_request",
        tr("StreamElements Media Request"),
        QUrl("https://streamelements.com/media-request"),
        m_mainWindow
    );
    m_docks["se_media_request"] = mediaDock;
    obs_frontend_add_custom_qdock("se_media_request", static_cast<void*>(mediaDock));

    BLOG(LOG_INFO, "Registered StreamElements default browser docks (Activity Feed, Chat, Media Request)");
}

void StreamElementsBrowserWidgetManager::toggleDock(const QString &dockId)
{
    auto it = m_docks.find(dockId);
    if (it != m_docks.end() && it->second) {
        bool visible = it->second->isVisible();
        it->second->setVisible(!visible);
        if (!visible) {
            it->second->raise();
        }
    }
}

StreamElementsBrowserDock* StreamElementsBrowserWidgetManager::getDock(const QString &dockId)
{
    auto it = m_docks.find(dockId);
    if (it != m_docks.end()) {
        return it->second;
    }
    return nullptr;
}
