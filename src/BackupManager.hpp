#pragma once

#include <QtCore/QString>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

class StreamElementsBackupManager {
public:
    static QString getObsConfigDir();
    static QString createBackupPackage(const QString &backupName = QString());
    static bool restoreBackupPackage(const QString &packagePath);

    static QJsonObject handleCreateBackup(const QJsonObject &params);
    static QJsonObject handleQueryBackupContents(const QJsonObject &params);
    static QJsonObject handleRestoreBackup(const QJsonObject &params);
};
