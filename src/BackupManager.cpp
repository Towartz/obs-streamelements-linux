#include "BackupManager.hpp"
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QProcess>
#include <obs.h>

#define BLOG(level, msg, ...) blog(level, "[obs-streamelements] " msg, ##__VA_ARGS__)

QString StreamElementsBackupManager::getObsConfigDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/obs-studio";
}

QString StreamElementsBackupManager::createBackupPackage(const QString &backupName)
{
    QString configDir = getObsConfigDir();
    QString timeStamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString filename = backupName.isEmpty() ? QString("se_backup_%1.zip").arg(timeStamp) : backupName;
    if (!filename.endsWith(".zip")) {
        filename += ".zip";
    }

    QString destPath = QDir::tempPath() + "/" + filename;

    QProcess process;
    process.setWorkingDirectory(configDir);
    process.start("zip", QStringList() << "-r" << destPath << "basic/scenes" << "basic/profiles");
    if (!process.waitForFinished(15000) || process.exitCode() != 0) {
        BLOG(LOG_ERROR, "Failed to create backup package: %s", process.readAllStandardError().constData());
        return QString();
    }

    BLOG(LOG_INFO, "Created user environment backup package at %s", destPath.toUtf8().constData());
    return destPath;
}

bool StreamElementsBackupManager::restoreBackupPackage(const QString &packagePath)
{
    if (!QFile::exists(packagePath)) {
        BLOG(LOG_ERROR, "Backup package does not exist: %s", packagePath.toUtf8().constData());
        return false;
    }

    QString configDir = getObsConfigDir();
    QProcess process;
    process.setWorkingDirectory(configDir);
    process.start("unzip", QStringList() << "-o" << packagePath);
    if (!process.waitForFinished(20000) || process.exitCode() != 0) {
        BLOG(LOG_ERROR, "Failed to restore backup package: %s", process.readAllStandardError().constData());
        return false;
    }

    BLOG(LOG_INFO, "Restored user environment from %s", packagePath.toUtf8().constData());
    return true;
}

QJsonObject StreamElementsBackupManager::handleCreateBackup(const QJsonObject &params)
{
    QString name = params.value("name").toString();
    QString path = createBackupPackage(name);

    QJsonObject res;
    res["success"] = !path.isEmpty();
    res["packagePath"] = path;
    return res;
}

QJsonObject StreamElementsBackupManager::handleQueryBackupContents(const QJsonObject &params)
{
    QString path = params.value("packagePath").toString();
    QJsonObject res;
    QJsonArray files;

    if (QFile::exists(path)) {
        QProcess process;
        process.start("unzip", QStringList() << "-l" << path);
        if (process.waitForFinished(5000)) {
            QString out = QString::fromUtf8(process.readAllStandardOutput());
            const auto lines = out.split('\n');
            for (const auto &line : lines) {
                QString trimmed = line.trimmed();
                if (trimmed.startsWith("basic/")) {
                    files.append(trimmed);
                }
            }
        }
    }

    res["files"] = files;
    return res;
}

QJsonObject StreamElementsBackupManager::handleRestoreBackup(const QJsonObject &params)
{
    QString path = params.value("packagePath").toString();
    bool ok = restoreBackupPackage(path);

    QJsonObject res;
    res["success"] = ok;
    return res;
}
