#include "download_manager.h"
#include "settings/settings.h"
#include <QWebEngineDownloadRequest>
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QDebug>

using namespace Tsunami;

namespace Tsunami {

DownloadManager::DownloadManager() : QObject(nullptr) {
    defaultDownloadPath_ = Settings::instance().getHomepage();
    QDir dir(defaultDownloadPath_);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

DownloadManager::~DownloadManager() {}

QString DownloadManager::generateId() {
    return QUuid::createUuid().toString();
}

DownloadItem* DownloadManager::findDownload(const QString& id) {
    for (auto& item : downloads_) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

void DownloadManager::startDownload(QWebEngineDownloadRequest* download) {
    DownloadItem item;
    item.id = generateId();
    item.url = download->url().toString();
    item.filename = download->downloadFileName();
    item.downloadPath = getDownloadPath(item.filename);
    item.totalBytes = download->totalBytes();
    item.state = QWebEngineDownloadRequest::DownloadInProgress;
    item.mimeType = download->mimeType();
    item.startTime = QDateTime::currentDateTime();
    
    downloads_.append(item);
    
    download->setDownloadDirectory(defaultDownloadPath_);
    download->setDownloadFileName(item.filename);
    download->accept();
    
    emit downloadAdded(item);
}

QString DownloadManager::getDownloadPath(const QString& filename) const {
    return defaultDownloadPath_ + "/" + filename;
}

void DownloadManager::pauseDownload(const QString& id) {
    DownloadItem* item = findDownload(id);
    if (item && item->state == QWebEngineDownloadRequest::DownloadInProgress) {
        item->isPaused = true;
        emit downloadUpdated(*item);
    }
}

void DownloadManager::resumeDownload(const QString& id) {
    DownloadItem* item = findDownload(id);
    if (item && item->isPaused) {
        item->isPaused = false;
        emit downloadUpdated(*item);
    }
}

void DownloadManager::cancelDownload(const QString& id) {
    DownloadItem* item = findDownload(id);
    if (item) {
        item->state = QWebEngineDownloadRequest::DownloadCancelled;
        emit downloadCanceled(id);
    }
}

void DownloadManager::removeDownload(const QString& id) {
    for (int i = 0; i < downloads_.size(); ++i) {
        if (downloads_[i].id == id) {
            downloads_.removeAt(i);
            break;
        }
    }
}

} // namespace Tsunami