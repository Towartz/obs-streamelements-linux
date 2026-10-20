#include "MenuManager.hpp"
#include "BrowserWidgetManager.hpp"
#include "LoginDialog.hpp"
#include "AuthManager.hpp"
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <obs.h>
#include <obs-frontend-api.h>

#define BLOG(level, msg, ...) blog(level, "[obs-streamelements] " msg, ##__VA_ARGS__)

StreamElementsMenuManager::StreamElementsMenuManager(QMainWindow *mainWindow, StreamElementsBrowserWidgetManager *browserManager, QObject *parent)
    : QObject(parent),
      m_mainWindow(mainWindow),
      m_browserManager(browserManager),
      m_seMenu(nullptr),
      m_setupMenu(nullptr),
      m_docksMenu(nullptr),
      m_helpMenu(nullptr),
      m_loginAction(nullptr),
      m_botTippingAction(nullptr),
      m_browseOverlaysAction(nullptr),
      m_importAction(nullptr),
      m_activityFeedAction(nullptr),
      m_chatAction(nullptr),
      m_mediaRequestAction(nullptr),
      m_liveSupportAction(nullptr),
      m_checkForUpdatesAction(nullptr),
      m_reportIssueAction(nullptr),
      m_isLoggedIn(false)
{
    setupUi();
    applyTheme();
}

StreamElementsMenuManager::~StreamElementsMenuManager()
{
    if (m_seMenu && m_mainWindow && m_mainWindow->menuBar()) {
        m_mainWindow->menuBar()->removeAction(m_seMenu->menuAction());
        delete m_seMenu;
        m_seMenu = nullptr;
    }
}

void StreamElementsMenuManager::setupUi()
{
    if (!m_mainWindow || !m_mainWindow->menuBar()) {
        BLOG(LOG_WARNING, "Main window or menu bar not available");
        return;
    }

    QMenuBar *menuBar = m_mainWindow->menuBar();
    
    // Insert StreamElements before Help menu
    QAction *helpAction = nullptr;
    const auto actions = menuBar->actions();
    for (QAction *action : actions) {
        if (action->text().contains("Help", Qt::CaseInsensitive)) {
            helpAction = action;
            break;
        }
    }

    m_seMenu = new QMenu(tr("StreamElements"), menuBar);

    m_loginAction = new QAction(tr("Login"), m_seMenu);
    m_loginAction->setObjectName("seLoginAction");
    connect(m_loginAction, &QAction::triggered, this, &StreamElementsMenuManager::onLoginAction);
    m_seMenu->addAction(m_loginAction);

    m_seMenu->addSeparator();

    m_setupMenu = new QMenu(tr("Stream Setup"), m_seMenu);
    m_botTippingAction = m_setupMenu->addAction(tr("Setup Bot && Tipping"), this, &StreamElementsMenuManager::onSetupBotTipping);
    m_browseOverlaysAction = m_setupMenu->addAction(tr("Browse Overlays..."), this, &StreamElementsMenuManager::onBrowseOverlays);
    m_seMenu->addMenu(m_setupMenu);

    m_docksMenu = new QMenu(tr("SE.Live Docks"), m_seMenu);
    m_activityFeedAction = m_docksMenu->addAction(tr("Activity Feed"), this, &StreamElementsMenuManager::onToggleActivityFeed);
    m_chatAction = m_docksMenu->addAction(tr("Chat"), this, &StreamElementsMenuManager::onToggleChat);
    m_mediaRequestAction = m_docksMenu->addAction(tr("Media Request"), this, &StreamElementsMenuManager::onToggleMediaRequest);
    m_seMenu->addMenu(m_docksMenu);

    m_seMenu->addSeparator();

    m_helpMenu = new QMenu(tr("Help"), m_seMenu);
    m_liveSupportAction = m_helpMenu->addAction(tr("StreamElements Live Support"), this, &StreamElementsMenuManager::onLiveSupport);
    m_checkForUpdatesAction = m_helpMenu->addAction(tr("Check for Updates"), this, &StreamElementsMenuManager::onCheckForUpdates);
    m_reportIssueAction = m_helpMenu->addAction(tr("Report an Issue"), this, &StreamElementsMenuManager::onReportIssue);
    m_seMenu->addMenu(m_helpMenu);

    if (helpAction) {
        menuBar->insertMenu(helpAction, m_seMenu);
    } else {
        menuBar->addMenu(m_seMenu);
    }

    connect(&StreamElementsAuthManager::instance(), &StreamElementsAuthManager::authStateChanged,
            this, [this](bool loggedIn, const QString &username) {
        SetLoggedIn(loggedIn, username);
        if (m_browserManager) {
            auto *feed = m_browserManager->getDock("se_activity_feed");
            if (feed) feed->reload();
            auto *chat = m_browserManager->getDock("se_chat");
            if (chat) chat->reload();
            auto *media = m_browserManager->getDock("se_media_request");
            if (media) media->reload();
        }
    });

    if (StreamElementsAuthManager::instance().isLoggedIn()) {
        SetLoggedIn(true, StreamElementsAuthManager::instance().username());
    }

    BLOG(LOG_INFO, "StreamElements top-level menu registered successfully");
}

void StreamElementsMenuManager::applyTheme()
{
    if (m_seMenu) {
        m_seMenu->setStyleSheet(
            "QMenu::item#seLoginAction { color: #F53656; font-weight: bold; }"
            "QMenu::item:selected { background-color: #2b2e38; }"
        );
    }
}

void StreamElementsMenuManager::SetLoggedIn(bool loggedIn, const QString &username)
{
    m_isLoggedIn = loggedIn;
    m_username = username;
    Update();
}

void StreamElementsMenuManager::Update()
{
    if (!m_loginAction) return;

    if (m_isLoggedIn) {
        m_loginAction->setText(tr("Logout (%1)").arg(m_username.isEmpty() ? tr("User") : m_username));
    } else {
        m_loginAction->setText(tr("Login"));
    }
}

void StreamElementsMenuManager::onLoginAction()
{
    if (m_isLoggedIn) {
        auto res = QMessageBox::question(m_mainWindow, tr("Logout"),
            tr("Are you sure you want to sign out of StreamElements?"),
            QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::Yes) {
            StreamElementsAuthManager::instance().logout();
        }
    } else {
        StreamElementsLoginDialog dialog(m_mainWindow);
        dialog.exec();
    }
}

void StreamElementsMenuManager::onSetupBotTipping()
{
    QDesktopServices::openUrl(QUrl("https://streamelements.com/dashboard/tipping-settings"));
}

void StreamElementsMenuManager::onBrowseOverlays()
{
    QDesktopServices::openUrl(QUrl("https://streamelements.com/dashboard/themes"));
}

void StreamElementsMenuManager::onLiveSupport()
{
    QDesktopServices::openUrl(QUrl("https://strms.net/discord"));
}

void StreamElementsMenuManager::onCheckForUpdates()
{
    QMessageBox::information(m_mainWindow, tr("Check for Updates"),
        tr("StreamElements Linux Plugin is up to date (Version 26.9.4 Native Linux)."));
}

void StreamElementsMenuManager::onReportIssue()
{
    QDesktopServices::openUrl(QUrl("https://support.streamelements.com/"));
}

void StreamElementsMenuManager::onToggleActivityFeed()
{
    BLOG(LOG_INFO, "Toggling Activity Feed Dock");
    if (m_browserManager) {
        m_browserManager->toggleDock("se_activity_feed");
    }
}

void StreamElementsMenuManager::onToggleChat()
{
    BLOG(LOG_INFO, "Toggling Chat Dock");
    if (m_browserManager) {
        m_browserManager->toggleDock("se_chat");
    }
}

void StreamElementsMenuManager::onToggleMediaRequest()
{
    BLOG(LOG_INFO, "Toggling Media Request Dock");
    if (m_browserManager) {
        m_browserManager->toggleDock("se_media_request");
    }
}
