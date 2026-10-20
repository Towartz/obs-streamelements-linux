#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtWidgets/QMainWindow>
#include <memory>
#include <map>
#include "BrowserDock.hpp"

class StreamElementsBrowserWidgetManager : public QObject {
    Q_OBJECT

public:
    explicit StreamElementsBrowserWidgetManager(QMainWindow *mainWindow, QObject *parent = nullptr);
    virtual ~StreamElementsBrowserWidgetManager() = default;

    void registerDefaultDocks();
    void toggleDock(const QString &dockId);
    StreamElementsBrowserDock* getDock(const QString &dockId);

private:
    QMainWindow *m_mainWindow;
    std::map<QString, StreamElementsBrowserDock*> m_docks;
};
