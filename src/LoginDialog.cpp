#include "LoginDialog.hpp"
#include "AuthManager.hpp"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGroupBox>
#include <QtGui/QDesktopServices>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtWebEngineCore/QWebEngineSettings>
#include <QtWebEngineCore/QWebEngineProfile>
#include <obs.h>

#define BLOG(level, msg, ...) blog(level, "[obs-streamelements] " msg, ##__VA_ARGS__)

StreamElementsLoginDialog::StreamElementsLoginDialog(QWidget *parent)
    : QDialog(parent),
      m_tabs(nullptr),
      m_authServer(new StreamElementsAuthServer(this)),
      m_browserStatusLabel(nullptr),
      m_tokenInput(nullptr),
      m_connectTokenBtn(nullptr),
      m_tokenStatusLabel(nullptr),
      m_webView(nullptr)
{
    setWindowTitle(tr("StreamElements Login & Account Connection"));
    resize(720, 560);
    connect(m_authServer, &StreamElementsAuthServer::tokenReceived, this, &StreamElementsLoginDialog::onTokenReceived);
    setupUi();
}

StreamElementsLoginDialog::~StreamElementsLoginDialog()
{
    if (m_authServer) {
        m_authServer->stop();
    }
}

void StreamElementsLoginDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // Header Title
    QLabel *headerTitle = new QLabel(tr("StreamElements SE.Live"), this);
    headerTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #F53656;");
    
    QLabel *headerSubtitle = new QLabel(
        tr("Connect your channel to access Activity Feed, Chat, Overlays, and Media Request inside OBS."), this);
    headerSubtitle->setStyleSheet("font-size: 13px; color: #a0a5b8; margin-bottom: 6px;");

    mainLayout->addWidget(headerTitle);
    mainLayout->addWidget(headerSubtitle);

    m_tabs = new QTabWidget(this);
    m_tabs->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #2e344d; background: #1a1d28; border-radius: 8px; }"
        "QTabBar::tab { background: #232738; color: #a0a5b8; padding: 10px 18px; border-top-left-radius: 6px; border-top-right-radius: 6px; font-weight: 600; }"
        "QTabBar::tab:selected { background: #1a1d28; color: #ffffff; border-bottom: 2px solid #F53656; }"
    );

    m_tabs->addTab(createBrowserAuthTab(), tr("Browser Login (Google / YouTube)"));
    m_tabs->addTab(createTokenAuthTab(), tr("Quick Connect (JWT Token)"));
    m_tabs->addTab(createEmbeddedWebTab(), tr("In-App Browser"));

    mainLayout->addWidget(m_tabs);
}

QWidget* StreamElementsLoginDialog::createBrowserAuthTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    QLabel *infoText = new QLabel(
        tr("<b>Recommended for YouTube & Google Accounts:</b><br>"
           "Signing in directly inside an external browser (e.g. Brave, Chrome, Firefox) prevents Google's "
           "<i>'Browser or app may not be secure'</i> block and reuses your already logged-in session."),
        tab
    );
    infoText->setWordWrap(true);
    infoText->setStyleSheet("background: #232738; padding: 12px; border-radius: 6px; color: #e2e8f0; font-size: 13px; line-height: 1.4;");
    layout->addWidget(infoText);

    QLabel *chooseLabel = new QLabel(tr("Select your streaming platform:"), tab);
    chooseLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #ffffff;");
    layout->addWidget(chooseLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    auto makePlatformBtn = [this, btnLayout](const QString &name, const QString &provider, const QString &bgColor, const QString &textColor) {
        QPushButton *btn = new QPushButton(name);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: %2; font-weight: bold; font-size: 14px; padding: 14px 20px; border-radius: 8px; border: none; }"
            "QPushButton:hover { filter: brightness(1.15); opacity: 0.9; }"
            "QPushButton:pressed { transform: scale(0.98); }"
        ).arg(bgColor, textColor));
        connect(btn, &QPushButton::clicked, this, [this, provider]() {
            onBrowserLoginClicked(provider);
        });
        btnLayout->addWidget(btn);
    };

    makePlatformBtn("YouTube", "youtube", "#FF0000", "#FFFFFF");
    makePlatformBtn("Twitch", "twitch", "#9146FF", "#FFFFFF");
    makePlatformBtn("Kick", "kick", "#53FC18", "#000000");
    makePlatformBtn("X (Twitter)", "x", "#000000", "#FFFFFF");

    layout->addLayout(btnLayout);

    m_browserStatusLabel = new QLabel(tr("Ready. Click your platform to log in via your system browser."), tab);
    m_browserStatusLabel->setAlignment(Qt::AlignCenter);
    m_browserStatusLabel->setStyleSheet("font-size: 13px; color: #94a3b8; margin-top: 10px;");
    layout->addWidget(m_browserStatusLabel);

    QPushButton *switchTabBtn = new QPushButton(tr("Already logged in to StreamElements on Brave? Use Quick Connect (JWT Token) →"), tab);
    switchTabBtn->setCursor(Qt::PointingHandCursor);
    switchTabBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #60a5fa; border: none; font-size: 13px; text-decoration: underline; margin-top: 8px; }"
        "QPushButton:hover { color: #93c5fd; }"
    );
    connect(switchTabBtn, &QPushButton::clicked, this, [this]() {
        if (m_tabs) m_tabs->setCurrentIndex(1);
    });
    layout->addWidget(switchTabBtn);

    layout->addStretch();
    return tab;
}

QWidget* StreamElementsLoginDialog::createTokenAuthTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(14);

    QLabel *info = new QLabel(
        tr("<b>Instant Connect with JWT Token:</b><br>"
           "If you are already logged in to StreamElements on your browser (Brave), you can copy your JWT Token "
           "directly and connect OBS immediately with zero redirects."),
        tab
    );
    info->setWordWrap(true);
    info->setStyleSheet("background: #232738; padding: 12px; border-radius: 6px; color: #e2e8f0; font-size: 13px;");
    layout->addWidget(info);

    QPushButton *openDashBtn = new QPushButton(tr("1. Open StreamElements Account Settings ↗"), tab);
    openDashBtn->setCursor(Qt::PointingHandCursor);
    openDashBtn->setStyleSheet(
        "QPushButton { background: #2e344d; color: #60a5fa; font-weight: 600; padding: 10px 16px; border-radius: 6px; text-align: left; }"
        "QPushButton:hover { background: #3a4260; color: #93c5fd; }"
    );
    connect(openDashBtn, &QPushButton::clicked, this, &StreamElementsLoginDialog::onOpenDashboardClicked);
    layout->addWidget(openDashBtn);

    QLabel *step2 = new QLabel(tr("2. In the browser tab, click <b>'Show Secrets'</b> and copy your <b>JWT Token</b>:"), tab);
    step2->setStyleSheet("color: #cbd5e1; font-size: 13px;");
    layout->addWidget(step2);

    m_tokenInput = new QLineEdit(tab);
    m_tokenInput->setPlaceholderText(tr("Paste your JWT Token here (eyJhbGciOi...)"));
    m_tokenInput->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    m_tokenInput->setStyleSheet(
        "QLineEdit { background: #12141c; color: #ffffff; border: 1px solid #334155; border-radius: 6px; padding: 10px; font-size: 13px; font-family: monospace; }"
        "QLineEdit:focus { border: 1px solid #F53656; }"
    );
    layout->addWidget(m_tokenInput);

    m_connectTokenBtn = new QPushButton(tr("Connect Account"), tab);
    m_connectTokenBtn->setCursor(Qt::PointingHandCursor);
    m_connectTokenBtn->setStyleSheet(
        "QPushButton { background: #F53656; color: #ffffff; font-weight: bold; font-size: 14px; padding: 12px; border-radius: 6px; border: none; }"
        "QPushButton:hover { background: #e02444; }"
    );
    connect(m_connectTokenBtn, &QPushButton::clicked, this, &StreamElementsLoginDialog::onConnectTokenClicked);
    layout->addWidget(m_connectTokenBtn);

    m_tokenStatusLabel = new QLabel("", tab);
    m_tokenStatusLabel->setAlignment(Qt::AlignCenter);
    m_tokenStatusLabel->setStyleSheet("font-size: 13px; color: #94a3b8;");
    layout->addWidget(m_tokenStatusLabel);

    layout->addStretch();
    return tab;
}

QWidget* StreamElementsLoginDialog::createEmbeddedWebTab()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 0, 0, 0);

    // Share persistent profile with all StreamElements docks
    QString storagePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/obs-studio/plugins/obs-streamelements/storage";
    QDir().mkpath(storagePath);

    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->setPersistentStoragePath(storagePath);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);
    profile->setHttpUserAgent("Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36");

    m_webView = new QWebEngineView(tab);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    m_webView->setUrl(QUrl("https://streamelements.com/login"));
    connect(m_webView, &QWebEngineView::urlChanged, this, &StreamElementsLoginDialog::onUrlChanged);

    layout->addWidget(m_webView);
    return tab;
}

void StreamElementsLoginDialog::onBrowserLoginClicked(const QString &provider)
{
    m_browserStatusLabel->setText(tr("Starting local authentication listener..."));
    m_browserStatusLabel->setStyleSheet("color: #60a5fa; font-weight: bold;");

    if (!m_authServer->isListening()) {
        if (!m_authServer->start(4269)) {
            m_browserStatusLabel->setText(tr("Failed to start local listener. Please use the Quick Connect (JWT Token) tab."));
            m_browserStatusLabel->setStyleSheet("color: #ef4444;");
            return;
        }
    }

    quint16 port = m_authServer->port();

    // StreamElements Citadel / OAuth requires an authorized domain for the return parameter.
    // selive.streamelements.com/auth/connect/<provider>/callback is the official return target,
    // which then forwards the JWT token to our local loopback server on http://localhost:<port>/auth/connect/<provider>/token.
    QString seliveCallback = QString("https://selive.streamelements.com/auth/connect/%1/callback?port=%2")
                                 .arg(provider)
                                 .arg(port);
    QString encodedReturn = QString::fromUtf8(QUrl::toPercentEncoding(seliveCallback));
    QString authUrl = QString("https://id.streamelements.com/auth/%1?return=%2").arg(provider, encodedReturn);

    // selive pre-auth flow: browser hits this, checks healthz on localhost:port, then redirects to authUrl
    QString preAuthUrl = QString("https://selive.streamelements.com/auth/%1/pre-auth?port=%2&authUrl=%3")
                             .arg(provider)
                             .arg(port)
                             .arg(QString::fromUtf8(QUrl::toPercentEncoding(authUrl)));

    m_browserStatusLabel->setText(tr("Opened %1 authentication in your browser.\nPlease complete login in Brave...").arg(provider.toUpper()));
    m_browserStatusLabel->setStyleSheet("color: #fbbf24; font-weight: bold;");

    BLOG(LOG_INFO, "Opening pre-auth in system browser: %s", preAuthUrl.toUtf8().constData());
    QDesktopServices::openUrl(QUrl(preAuthUrl));
}

void StreamElementsLoginDialog::onTokenReceived(const QString &token)
{
    BLOG(LOG_INFO, "Token received from browser callback");

    QString username, error;
    bool success = StreamElementsAuthManager::instance().validateAndApplyToken(token, username, error);
    if (success) {
        m_browserStatusLabel->setText(tr("✓ Successfully connected as %1!").arg(username));
        m_browserStatusLabel->setStyleSheet("color: #4ade80; font-weight: bold; font-size: 15px;");
        emit loginSuccessful(username);
        QTimer::singleShot(1000, this, &QDialog::accept);
    } else {
        m_browserStatusLabel->setText(tr("Authentication failed: %1").arg(error));
        m_browserStatusLabel->setStyleSheet("color: #ef4444;");
    }
}

void StreamElementsLoginDialog::onOpenDashboardClicked()
{
    QDesktopServices::openUrl(QUrl("https://streamelements.com/dashboard/account/channels"));
}

void StreamElementsLoginDialog::onConnectTokenClicked()
{
    QString token = m_tokenInput->text().trimmed();
    if (token.startsWith('"') && token.endsWith('"')) {
        token = token.mid(1, token.length() - 2).trimmed();
    }
    if (token.startsWith("Bearer ", Qt::CaseInsensitive)) {
        token = token.mid(7).trimmed();
    }

    if (token.isEmpty()) {
        m_tokenStatusLabel->setText(tr("Please paste your StreamElements JWT Token"));
        m_tokenStatusLabel->setStyleSheet("color: #ef4444;");
        return;
    }

    m_tokenStatusLabel->setText(tr("Validating token with StreamElements..."));
    m_tokenStatusLabel->setStyleSheet("color: #60a5fa;");

    QString username, error;
    bool success = StreamElementsAuthManager::instance().validateAndApplyToken(token, username, error);
    if (success) {
        m_tokenStatusLabel->setText(tr("✓ Successfully connected as %1!").arg(username));
        m_tokenStatusLabel->setStyleSheet("color: #4ade80; font-weight: bold; font-size: 15px;");
        emit loginSuccessful(username);
        QTimer::singleShot(1000, this, &QDialog::accept);
    } else {
        m_tokenStatusLabel->setText(tr("Error: %1").arg(error));
        m_tokenStatusLabel->setStyleSheet("color: #ef4444;");
    }
}

void StreamElementsLoginDialog::onUrlChanged(const QUrl &url)
{
    QString urlStr = url.toString();
    BLOG(LOG_INFO, "In-app login navigation: %s", urlStr.toUtf8().constData());

    // Once redirected to dashboard or themes, user is successfully authenticated!
    if (urlStr.contains("streamelements.com/dashboard") ||
        urlStr.contains("streamelements.com/activity-feed") ||
        urlStr.contains("streamelements.com/overlays")) {
        BLOG(LOG_INFO, "In-app StreamElements authentication successful!");
        emit loginSuccessful(tr("Connected"));
        accept();
    }
}
