#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkCookie>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWebEngineCore/QWebEngineCookieStore>

class StreamElementsAuthManager : public QObject {
    Q_OBJECT

public:
    static StreamElementsAuthManager& instance();

    void init();
    bool isLoggedIn() const { return m_isLoggedIn; }
    QString token() const { return m_token; }
    QString username() const { return m_username; }
    QString provider() const { return m_provider; }
    QString channelId() const { return m_channelId; }
    QString displayName() const { return m_displayName; }

    bool validateAndApplyToken(const QString &token, QString &outUsername, QString &outError);
    void logout();
    void injectCookies();

signals:
    void authStateChanged(bool loggedIn, const QString &username);

private:
    StreamElementsAuthManager();
    ~StreamElementsAuthManager() override = default;
    StreamElementsAuthManager(const StreamElementsAuthManager&) = delete;
    StreamElementsAuthManager& operator=(const StreamElementsAuthManager&) = delete;

    QString getAuthFilePath() const;
    bool loadAuthFromFile();
    bool saveAuthToFile();

    bool m_isLoggedIn = false;
    QString m_token;
    QString m_username;
    QString m_provider;
    QString m_channelId;
    QString m_displayName;
};
