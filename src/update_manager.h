#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QVersionNumber>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>

namespace Tsunami {

struct UpdateInfo {
    bool updateAvailable = false;
    QString version;
    QString releaseNotes;
    QString downloadUrl;
    QString fileName;
    qint64 fileSize = 0;
    QString currentVersion;
};

class UpdateManager : public QObject {
    Q_OBJECT
public:
    explicit UpdateManager(QObject* parent = nullptr);

    void checkForUpdates(bool silent = false);
    void downloadUpdate();
    void installUpdateAndRestart();

    bool isDownloading() const { return downloading_; }
    qint64 downloadProgress() const { return downloadProgress_; }
    QString errorString() const { return error_; }

    static QString getUpdateUrl(const QString& owner, const QString& repo);
    static QString currentVersion();
    static QString getPlatformPattern();
    static bool isDistribution(const QString& distro);
    static QString getPlatformDisplayName();

signals:
    void updateCheckFinished(const Tsunami::UpdateInfo& info);
    void updateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void updateDownloaded(const QString& filePath);
    void updateError(const QString& error);
    void restartRequested();

private slots:
    void onCheckReply(QNetworkReply* reply);
    void onDownloadReadyRead();
    void onDownloadFinished();
    void onDownloadError(QNetworkReply::NetworkError error);

private:
    void parseReleaseInfo(const QJsonDocument& doc);
    QVersionNumber parseVersion(const QString& versionString);
    QString getAssetUrl(const QJsonArray& assets, const QString& pattern);
    QString getCurrentExecutablePath();
    QString getUpdateFilePath();
    void cleanupUpdateFiles();
    void launchUpdater(const QString& updateFilePath);
    QString createUpdaterScript(const QString& currentPath, const QString& updateFilePath);
    void parseRepoFromUrl(const QString& url);

    QNetworkAccessManager* networkManager_ = nullptr;
    UpdateInfo updateInfo_;
    QString owner_;
    QString repo_;
    bool downloading_ = false;
    qint64 downloadProgress_ = 0;
    QString error_;
    QString tempDownloadPath_;
    QNetworkReply* currentDownload_ = nullptr;
    bool silentCheck_ = false;
};

} // namespace Tsunami
