#include "AuthManager.hpp"
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QDateTime>
#include <curl/curl.h>
#include <obs.h>

#define BLOG(level, msg, ...) blog(level, "[obs-streamelements] " msg, ##__VA_ARGS__)

static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total = size * nmemb;
    std::string *s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), total);
    return total;
}

StreamElementsAuthManager& StreamElementsAuthManager::instance()
{
    static StreamElementsAuthManager s_instance;
    return s_instance;
}

StreamElementsAuthManager::StreamElementsAuthManager()
{
}

QString StreamElementsAuthManager::getAuthFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/obs-studio/plugins/obs-streamelements";
    QDir().mkpath(configDir);
    return configDir + "/auth.json";
}

void StreamElementsAuthManager::init()
{
    if (loadAuthFromFile()) {
        BLOG(LOG_INFO, "Loaded stored StreamElements session for user: %s (Provider: %s)",
             m_username.toUtf8().constData(), m_provider.toUtf8().constData());
        injectCookies();
        emit authStateChanged(true, m_username);
    } else {
        BLOG(LOG_INFO, "No stored StreamElements session found");
    }
}

bool StreamElementsAuthManager::loadAuthFromFile()
{
    QFile file(getAuthFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject obj = doc.object();
    m_token = obj.value("token").toString();
    m_username = obj.value("username").toString();
    m_provider = obj.value("provider").toString();
    m_channelId = obj.value("channelId").toString();
    m_displayName = obj.value("displayName").toString();

    if (m_token.isEmpty() || m_username.isEmpty()) {
        m_isLoggedIn = false;
        return false;
    }

    m_isLoggedIn = true;
    return true;
}

bool StreamElementsAuthManager::saveAuthToFile()
{
    QJsonObject obj;
    obj["token"] = m_token;
    obj["username"] = m_username;
    obj["provider"] = m_provider;
    obj["channelId"] = m_channelId;
    obj["displayName"] = m_displayName;

    QJsonDocument doc(obj);
    QFile file(getAuthFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        BLOG(LOG_ERROR, "Failed to save auth.json to %s", getAuthFilePath().toUtf8().constData());
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool StreamElementsAuthManager::validateAndApplyToken(const QString &token, QString &outUsername, QString &outError)
{
    QString cleanToken = token.trimmed();
    if (cleanToken.isEmpty()) {
        outError = "Token is empty";
        return false;
    }

    // Call StreamElements API channels/me to validate token and fetch profile
    CURL *curl = curl_easy_init();
    if (!curl) {
        outError = "Failed to initialize HTTP client";
        return false;
    }

    std::string responseString;
    struct curl_slist *headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + cleanToken.toStdString();
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64) SE.Live-Linux/1.0");

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.streamelements.com/kappa/v2/channels/me");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        outError = QString("Network error: %1").arg(curl_easy_strerror(res));
        BLOG(LOG_ERROR, "Token validation network error: %s", outError.toUtf8().constData());
        return false;
    }

    if (httpCode != 200) {
        outError = QString("Invalid StreamElements token (HTTP %1)").arg(httpCode);
        BLOG(LOG_WARNING, "Token validation failed: HTTP %ld, Body: %s", httpCode, responseString.c_str());
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(responseString));
    if (!doc.isObject()) {
        outError = "Invalid JSON response from StreamElements API";
        return false;
    }

    QJsonObject channel = doc.object();
    m_token = cleanToken;
    m_channelId = channel.value("_id").toString();
    m_username = channel.value("username").toString();
    m_displayName = channel.value("displayName").toString();
    if (m_displayName.isEmpty()) {
        m_displayName = m_username;
    }
    m_provider = channel.value("provider").toString();
    m_isLoggedIn = true;

    outUsername = !m_displayName.isEmpty() ? m_displayName : m_username;

    saveAuthToFile();
    injectCookies();

    BLOG(LOG_INFO, "Successfully authenticated as %s (%s, provider: %s)",
         m_username.toUtf8().constData(), m_channelId.toUtf8().constData(), m_provider.toUtf8().constData());

    emit authStateChanged(true, outUsername);
    return true;
}

void StreamElementsAuthManager::injectCookies()
{
    if (m_token.isEmpty()) return;

    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QWebEngineCookieStore *store = profile->cookieStore();

    auto setCookieForDomain = [&](const QString &name, const QString &val, const QString &domain, const QUrl &origin) {
        QNetworkCookie cookie;
        cookie.setName(name.toUtf8());
        cookie.setValue(val.toUtf8());
        cookie.setDomain(domain);
        cookie.setPath("/");
        cookie.setExpirationDate(QDateTime::currentDateTime().addYears(2));
        cookie.setSecure(true);
        cookie.setHttpOnly(false);
        store->setCookie(cookie, origin);
    };

    setCookieForDomain("token", m_token, ".streamelements.com", QUrl("https://streamelements.com"));
    setCookieForDomain("creator", m_token, ".streamelements.com", QUrl("https://streamelements.com"));

    setCookieForDomain("token", m_token, ".streamelements.com", QUrl("https://selive.streamelements.com"));
    setCookieForDomain("creator", m_token, ".streamelements.com", QUrl("https://selive.streamelements.com"));

    BLOG(LOG_INFO, "Injected StreamElements session cookies into QWebEngineProfile");
}

void StreamElementsAuthManager::logout()
{
    BLOG(LOG_INFO, "Logging out user: %s", m_username.toUtf8().constData());

    m_isLoggedIn = false;
    m_token.clear();
    m_username.clear();
    m_provider.clear();
    m_channelId.clear();
    m_displayName.clear();

    QFile file(getAuthFilePath());
    if (file.exists()) {
        file.remove();
    }

    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    QWebEngineCookieStore *store = profile->cookieStore();
    store->deleteAllCookies();

    emit authStateChanged(false, "");
}
