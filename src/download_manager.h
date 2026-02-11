#pragma once

#include <QObject>
#include <QWebEngineDownloadRequest>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>

namespace Tsunami {

struct DownloadItem {
    QString id;
    QString url;
    QString filename;
    QString downloadPath;
    qint64 totalBytes = 0;
    qint64 receivedBytes = 0;
    int progress = 0;
    QWebEngineDownloadRequest::DownloadState state = QWebEngineDownloadRequest::DownloadInProgress;
    QString mimeType;
    QDateTime startTime;
    bool isPaused = false;
};

class DownloadManager : public QObject {
    Q_OBJECT

public:
    static DownloadManager& instance() {
        static DownloadManager instance;
        return instance;
    }

    void startDownload(QWebEngineDownloadRequest* download);
    void pauseDownload(const QString& id);
    void resumeDownload(const QString& id);
    void cancelDownload(const QString& id);
    void removeDownload(const QString& id);
    
    const QList<DownloadItem>& getDownloads() const { return downloads_; }
    QString getDownloadPath(const QString& filename) const;
    
    signals:
    void downloadAdded(const DownloadItem& item);
    void downloadUpdated(const DownloadItem& item);
    void downloadCompleted(const QString& id);
    void downloadCanceled(const QString& id);

private:
    DownloadManager();
    ~DownloadManager();
    DownloadManager(const DownloadManager&) = delete;
    DownloadManager& operator=(const DownloadManager&) = delete;
    
    QList<DownloadItem> downloads_;
    QString defaultDownloadPath_;
    
    QString generateId();
    DownloadItem* findDownload(const QString& id);
};

} // namespace Tsunami