#pragma once

#include <QtWidgets/QDockWidget>
#include <QtWidgets/QWidget>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtCore/QUrl>
#include <QtCore/QString>

class StreamElementsBrowserDock : public QDockWidget {
    Q_OBJECT

public:
    explicit StreamElementsBrowserDock(const QString &dockId, const QString &title, const QUrl &url, QWidget *parent = nullptr);
    virtual ~StreamElementsBrowserDock() = default;

    QString dockId() const { return m_dockId; }
    void reload();
    void setUrl(const QUrl &url);

private:
    void setupUi();

    QString m_dockId;
    QUrl m_url;
    QWebEngineView *m_webView;
};
