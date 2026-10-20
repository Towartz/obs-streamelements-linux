#include "BrowserDock.hpp"
#include <QtWebEngineCore/QWebEngineSettings>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QToolBar>
#include <QtGui/QAction>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <obs.h>

#define BLOG(level, msg, ...) blog(level, "[obs-streamelements] " msg, ##__VA_ARGS__)

StreamElementsBrowserDock::StreamElementsBrowserDock(const QString &dockId, const QString &title, const QUrl &url, QWidget *parent)
    : QDockWidget(title, parent),
      m_dockId(dockId),
      m_url(url),
      m_webView(nullptr)
{
    setObjectName(dockId);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    setupUi();
}

void StreamElementsBrowserDock::setupUi()
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Ensure persistent storage directory exists for StreamElements web session cookies & storage
    QString storagePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/obs-studio/plugins/obs-streamelements/storage";
    QDir().mkpath(storagePath);

    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->setPersistentStoragePath(storagePath);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    profile->setHttpUserAgent("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36");

    m_webView = new QWebEngineView(container);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    if (!m_url.isEmpty()) {
        m_webView->setUrl(m_url);
    }

    layout->addWidget(m_webView);
    setWidget(container);

    BLOG(LOG_INFO, "Initialized StreamElementsBrowserDock '%s' pointing to %s",
         m_dockId.toUtf8().constData(), m_url.toString().toUtf8().constData());
}

void StreamElementsBrowserDock::reload()
{
    if (m_webView) {
        m_webView->reload();
    }
}

void StreamElementsBrowserDock::setUrl(const QUrl &url)
{
    m_url = url;
    if (m_webView) {
        m_webView->setUrl(url);
    }
}
