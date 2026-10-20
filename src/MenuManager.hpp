#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtGui/QAction>
#include <memory>

class StreamElementsBrowserWidgetManager;

class StreamElementsMenuManager : public QObject {
    Q_OBJECT

public:
    explicit StreamElementsMenuManager(QMainWindow *mainWindow, StreamElementsBrowserWidgetManager *browserManager, QObject *parent = nullptr);
    virtual ~StreamElementsMenuManager();

    void Update();
    void SetLoggedIn(bool loggedIn, const QString &username = QString());
    bool IsLoggedIn() const { return m_isLoggedIn; }

    QMenu* menu() const { return m_seMenu; }

public slots:
    void onLoginAction();
    void onSetupBotTipping();
    void onBrowseOverlays();
    void onLiveSupport();
    void onCheckForUpdates();
    void onReportIssue();
    void onToggleActivityFeed();
    void onToggleChat();
    void onToggleMediaRequest();

private:
    void setupUi();
    void applyTheme();

    QMainWindow *m_mainWindow;
    StreamElementsBrowserWidgetManager *m_browserManager;
    QMenu *m_seMenu;
    QMenu *m_setupMenu;
    QMenu *m_docksMenu;
    QMenu *m_helpMenu;

    QAction *m_loginAction;
    QAction *m_botTippingAction;
    QAction *m_browseOverlaysAction;
    QAction *m_importAction;
    
    QAction *m_activityFeedAction;
    QAction *m_chatAction;
    QAction *m_mediaRequestAction;

    QAction *m_liveSupportAction;
    QAction *m_checkForUpdatesAction;
    QAction *m_reportIssueAction;

    bool m_isLoggedIn;
    QString m_username;
};
