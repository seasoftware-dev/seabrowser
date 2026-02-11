#include "update_manager.h"
#include "version.h"
#include "settings/settings.h"
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>
#include <QDebug>
#include <QDateTime>

namespace Tsunami {

UpdateManager::UpdateManager(QObject* parent)
    : QObject(parent)
    , networkManager_(new QNetworkAccessManager(this))
{
    parseRepoFromUrl("https://github.com/seasoftware-dev/tsunami/");
}

void UpdateManager::parseRepoFromUrl(const QString& url) {
    QUrl qurl(url);
    QString path = qurl.path();
    QStringList parts = path.split('/', Qt::SkipEmptyParts);
    if (parts.size() >= 2) {
        owner_ = parts[0];
        repo_ = parts[1];
    }
}

QString UpdateManager::getUpdateUrl(const QString& owner, const QString& repo) {
    return QString("https://api.github.com/repos/%1/%2/releases/latest").arg(owner, repo);
}

QString UpdateManager::currentVersion() {
    return QString::fromLatin1(TUNDMAJI_VERSION);
}

QString UpdateManager::getPlatformPattern() {
#if defined(Q_OS_WIN)
    return "windows";
#elif defined(Q_OS_MACOS)
    return "macos";
#elif defined(Q_OS_ANDROID)
    return "android";
#else
    if (isDistribution("fedora") || isDistribution("rhel") || isDistribution("centos")) {
        return "rpm";
    } else if (isDistribution("ubuntu") || isDistribution("debian") || isDistribution("mint")) {
        return "deb";
    }
    return "AppImage";
#endif
}

bool UpdateManager::isDistribution(const QString& distro) {
    QFile osRelease("/etc/os-release");
    if (osRelease.open(QIODevice::ReadOnly)) {
        QByteArray content = osRelease.readAll();
        QString contentStr = QString::fromLatin1(content);
        osRelease.close();
        return contentStr.contains(distro, Qt::CaseInsensitive);
    }
    return false;
}

QString UpdateManager::getAssetUrl(const QJsonArray& assets, const QString& pattern) {
    for (const QJsonValue& assetValue : assets) {
        QJsonObject asset = assetValue.toObject();
        QString name = asset["name"].toString();
        if (name.contains(pattern, Qt::CaseInsensitive)) {
            return asset["browser_download_url"].toString();
        }
    }
    return QString();
}

void UpdateManager::checkForUpdates(bool silent) {
    QUrl url(getUpdateUrl(owner_, repo_));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Tsunami/" + currentVersion());
    request.setRawHeader("Accept", "application/vnd.github.v3+json");

    silentCheck_ = silent;

    QNetworkReply* reply = networkManager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onCheckReply(reply);
    });
}

void UpdateManager::onCheckReply(QNetworkReply* reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        error_ = reply->errorString();
        emit updateError(error_);
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        error_ = "Invalid response from GitHub";
        emit updateError(error_);
        return;
    }

    parseReleaseInfo(doc);

    if (updateInfo_.updateAvailable && !silentCheck_) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Update Available"));
        msgBox.setText(tr("Tsunami %1 is available!\n\nYou are currently using %2.\n\nRelease Notes:\n%3")
                          .arg(updateInfo_.version)
                          .arg(updateInfo_.currentVersion)
                          .arg(updateInfo_.releaseNotes));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        QAbstractButton* yesButton = msgBox.button(QMessageBox::Yes);
        yesButton->setText(tr("Download & Install"));
        msgBox.button(QMessageBox::No)->setText(tr("Not Now"));

        if (msgBox.exec() == QMessageBox::Yes) {
            downloadUpdate();
        }
    }

    emit updateCheckFinished(updateInfo_);
}

QString UpdateManager::getPlatformDisplayName() {
#if defined(Q_OS_WIN)
    return "Windows";
#elif defined(Q_OS_MACOS)
    return "macOS";
#elif defined(Q_OS_ANDROID)
    return "Android";
#else
    if (isDistribution("fedora") || isDistribution("rhel") || isDistribution("centos")) {
        return "Linux (RPM)";
    } else if (isDistribution("ubuntu") || isDistribution("debian") || isDistribution("mint")) {
        return "Linux (DEB)";
    }
    return "Linux (AppImage)";
#endif
}

void UpdateManager::parseReleaseInfo(const QJsonDocument& doc) {
    QJsonObject release = doc.object();
    QString latestVersion = release["tag_name"].toString();
    if (latestVersion.startsWith('v')) {
        latestVersion = latestVersion.mid(1);
    }

    updateInfo_.currentVersion = currentVersion();
    updateInfo_.version = latestVersion;
    updateInfo_.releaseNotes = release["body"].toString();
    updateInfo_.updateAvailable = false;

    QVersionNumber current = parseVersion(updateInfo_.currentVersion);
    QVersionNumber latest = parseVersion(latestVersion);

    if (latest > current) {
        updateInfo_.updateAvailable = true;

        QJsonArray assets = release["assets"].toArray();
        updateInfo_.downloadUrl = getAssetUrl(assets, getPlatformPattern());
        updateInfo_.fileName = QFileInfo(QUrl(updateInfo_.downloadUrl).path()).fileName();

        if (!assets.isEmpty()) {
            for (const QJsonValue& assetValue : assets) {
                QJsonObject asset = assetValue.toObject();
                QString name = asset["name"].toString();
                if (name == updateInfo_.fileName) {
                    updateInfo_.fileSize = asset["size"].toVariant().toLongLong();
                    break;
                }
            }
        }
    }
}

QVersionNumber UpdateManager::parseVersion(const QString& versionString) {
    QString cleaned = versionString;
    cleaned.remove('v').remove('V');
    return QVersionNumber::fromString(cleaned);
}

void UpdateManager::downloadUpdate() {
    if (updateInfo_.downloadUrl.isEmpty()) {
        error_ = "No download URL found for your platform";
        emit updateError(error_);
        return;
    }

    downloading_ = true;
    downloadProgress_ = 0;

    tempDownloadPath_ = getUpdateFilePath();

    QNetworkRequest request(QUrl(updateInfo_.downloadUrl));
    request.setHeader(QNetworkRequest::UserAgentHeader, "Tsunami/" + currentVersion());

    currentDownload_ = networkManager_->get(request);

    connect(currentDownload_, &QNetworkReply::readyRead, this, &UpdateManager::onDownloadReadyRead);
    connect(currentDownload_, &QNetworkReply::finished, this, &UpdateManager::onDownloadFinished);
    connect(currentDownload_, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &UpdateManager::onDownloadError);
}

void UpdateManager::onDownloadReadyRead() {
    if (currentDownload_) {
        QByteArray data = currentDownload_->readAll();
        QFile file(tempDownloadPath_);
        if (file.open(QIODevice::Append)) {
            file.write(data);
            file.close();
        }

        downloadProgress_ += data.size();
        emit updateDownloadProgress(downloadProgress_, updateInfo_.fileSize);
    }
}

void UpdateManager::onDownloadFinished() {
    downloading_ = false;

    if (currentDownload_->error() != QNetworkReply::NoError) {
        error_ = currentDownload_->errorString();
        emit updateError(error_);
        currentDownload_->deleteLater();
        currentDownload_ = nullptr;
        return;
    }

    QFileInfo fileInfo(tempDownloadPath_);
    if (fileInfo.size() == 0) {
        error_ = "Downloaded file is empty";
        emit updateError(error_);
        currentDownload_->deleteLater();
        currentDownload_ = nullptr;
        return;
    }

    currentDownload_->deleteLater();
    currentDownload_ = nullptr;

    emit updateDownloaded(tempDownloadPath_);
    installUpdateAndRestart();
}

void UpdateManager::onDownloadError(QNetworkReply::NetworkError error) {
    Q_UNUSED(error);
    downloading_ = false;
    error_ = currentDownload_->errorString();
    emit updateError(error_);
}

QString UpdateManager::getUpdateFilePath() {
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return tempDir + "/tsunami_update.zip";
}

QString UpdateManager::getCurrentExecutablePath() {
    return QApplication::applicationFilePath();
}

void UpdateManager::installUpdateAndRestart() {
    cleanupUpdateFiles();
    launchUpdater(tempDownloadPath_);
}

void UpdateManager::cleanupUpdateFiles() {
    QDir tempDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    QStringList updateFiles = QStringList() << "tsunami_update*"
                                             << "tsunami_updater"
                                             << "tsunami_updater.exe";
    for (const QString& file : updateFiles) {
        tempDir.remove(file);
    }
}

void UpdateManager::launchUpdater(const QString& updateFilePath) {
    QString executablePath = getCurrentExecutablePath();
    QString updaterPath = createUpdaterScript(executablePath, updateFilePath);

#if defined(Q_OS_WIN)
    QProcess::startDetached("powershell.exe", {
        "-WindowStyle", "Hidden",
        "-ExecutionPolicy", "Bypass",
        "-File", updaterPath
    });
#else
    QProcess::startDetached("/bin/bash", {"-c", updaterPath});
#endif

    QApplication::quit();
}

QString UpdateManager::createUpdaterScript(const QString& currentPath, const QString& updateFilePath) {
    QString scriptPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

#if defined(Q_OS_WIN)
    scriptPath += "/tsunami_updater.ps1";
    QFile file(scriptPath);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << "# Tsunami Updater\n";
        stream << "param([string]$UpdateFile, [string]$CurrentPath)\n\n";
        stream << "Start-Sleep -Seconds 3\n\n";
        stream << "$ErrorActionPreference = 'Stop'\n\n";
        stream << "try {\n";
        stream << "    $process = Get-Process -Name \"Tsunami\" -ErrorAction SilentlyContinue\n";
        stream << "    if ($process) { $process.WaitForExit(15000) }\n\n";
        stream << "    Expand-Archive -Path $UpdateFile -DestinationPath $env:TEMP\\tsunami_new -Force\n";
        stream << "    Copy-Item -Path \"$env:TEMP\\tsunami_new\\*\" -Destination (Split-Path $CurrentPath) -Recurse -Force\n";
        stream << "    Remove-Item -Path $UpdateFile -Force -ErrorAction SilentlyContinue\n";
        stream << "    Remove-Item -Path \"$env:TEMP\\tsunami_new\" -Recurse -Force -ErrorAction SilentlyContinue\n";
        stream << "    Start-Process -FilePath $CurrentPath\n";
        stream << "} catch {\n";
        stream << "    Write-Error \"Update failed: $_\"\n";
        stream << "    exit 1\n";
        stream << "}\n";
        file.close();
    }
#else
    scriptPath += "/tsunami_updater.sh";
    QFile file(scriptPath);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << "#!/bin/bash\n";
        stream << "# Tsunami Updater\n\n";
        stream << "UPDATE_FILE=\"" << QDir::toNativeSeparators(updateFilePath) << "\"\n";
        stream << "CURRENT_PATH=\"" << QDir::toNativeSeparators(currentPath) << "\"\n\n";
        stream << "sleep 3\n\n";
        stream << "while pgrep -x \"Tsunami\" > /dev/null; do sleep 1; done\n\n";
        stream << "TEMP_DIR=$(mktemp -d)\n";
        stream << "unzip -o \"$UPDATE_FILE\" -d \"$TEMP_DIR\"\n";
        stream << "cp -R \"$TEMP_DIR\"/* \"$(dirname \"$CURRENT_PATH\")/\"\n";
        stream << "rm -f \"$UPDATE_FILE\"\n";
        stream << "rm -rf \"$TEMP_DIR\"\n";
        stream << "chmod +x \"$CURRENT_PATH\"\n";
        stream << "nohup \"$CURRENT_PATH\" > /dev/null 2>&1 &\n";
        file.close();
    }
    QProcess::execute("chmod", {"+x", scriptPath});
#endif
    return scriptPath;
}

} // namespace Tsunami
