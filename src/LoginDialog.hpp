#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtCore/QUrl>
#include <QtCore/QString>
#include "AuthServer.hpp"

class StreamElementsLoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit StreamElementsLoginDialog(QWidget *parent = nullptr);
    ~StreamElementsLoginDialog() override;

signals:
    void loginSuccessful(const QString &username);

private slots:
    void onUrlChanged(const QUrl &url);
    void onBrowserLoginClicked(const QString &provider);
    void onConnectTokenClicked();
    void onOpenDashboardClicked();
    void onTokenReceived(const QString &token);

private:
    void setupUi();
    QWidget* createBrowserAuthTab();
    QWidget* createTokenAuthTab();
    QWidget* createEmbeddedWebTab();

    QTabWidget *m_tabs = nullptr;

    StreamElementsAuthServer *m_authServer = nullptr;
    QLabel *m_browserStatusLabel = nullptr;

    QLineEdit *m_tokenInput = nullptr;
    QPushButton *m_connectTokenBtn = nullptr;
    QLabel *m_tokenStatusLabel = nullptr;

    QWebEngineView *m_webView = nullptr;
};
