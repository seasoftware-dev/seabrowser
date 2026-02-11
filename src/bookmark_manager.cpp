#include "bookmark_manager.h"
#include "settings/settings.h"
#include <QFile>
#include <QDir>
#include <QUuid>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

namespace Tsunami {

BookmarkManager::BookmarkManager() : QObject(nullptr) {
    loadBookmarks();
}

BookmarkManager::~BookmarkManager() {
    saveBookmarks();
}

void BookmarkManager::loadBookmarks() {
    rootArray_ = QJsonArray();
}

void BookmarkManager::saveBookmarks() {
    // saveBookmarks();
}

QString BookmarkManager::generateId() {
    return QUuid::createUuid().toString();
}

QString BookmarkManager::getBookmarksFile() const {
    return Tsunami::Settings::instance().getConfigPath();
}

void BookmarkManager::addBookmark(const QString& title, const QString& url, const QString& folder) {
    QJsonObject bookmark;
    bookmark["id"] = generateId();
    bookmark["title"] = title;
    bookmark["url"] = url;
    bookmark["folder"] = folder;
    bookmark["isFolder"] = false;
    bookmark["position"] = rootArray_.size();
    
    if (folder.isEmpty()) {
        rootArray_.append(bookmark);
    }
    
    saveBookmarks();
    emit bookmarksChanged();
}

void BookmarkManager::addFolder(const QString& name, const QString& parentFolder) {
    QJsonObject folder;
    folder["id"] = generateId();
    folder["title"] = name;
    folder["folder"] = parentFolder;
    folder["isFolder"] = true;
    folder["children"] = QJsonArray();
    folder["position"] = rootArray_.size();
    
    rootArray_.append(folder);
    saveBookmarks();
    emit bookmarksChanged();
}

void BookmarkManager::removeBookmark(const QString& id) {
    for (int i = 0; i < rootArray_.size(); ++i) {
        if (rootArray_[i].toObject()["id"].toString() == id) {
            rootArray_.removeAt(i);
            break;
        }
    }
    saveBookmarks();
    emit bookmarksChanged();
}

void BookmarkManager::removeFolder(const QString& folderName) {
    QJsonArray newArray;
    for (const QJsonValue& val : rootArray_) {
        QJsonObject obj = val.toObject();
        if (obj["title"].toString() != folderName) {
            newArray.append(obj);
        }
    }
    rootArray_ = newArray;
    saveBookmarks();
    emit bookmarksChanged();
}

void BookmarkManager::updateBookmark(const QString& id, const QString& title, const QString& url) {
    for (int i = 0; i < rootArray_.size(); ++i) {
        QJsonObject obj = rootArray_[i].toObject();
        if (obj["id"].toString() == id) {
            obj["title"] = title;
            obj["url"] = url;
            rootArray_[i] = obj;
            break;
        }
    }
    saveBookmarks();
    emit bookmarksChanged();
}

void BookmarkManager::moveBookmark(const QString& id, const QString& newFolder) {
    for (int i = 0; i < rootArray_.size(); ++i) {
        QJsonObject obj = rootArray_[i].toObject();
        if (obj["id"].toString() == id) {
            obj["folder"] = newFolder;
            rootArray_[i] = obj;
            break;
        }
    }
    saveBookmarks();
    emit bookmarksChanged();
}

QList<Bookmark> BookmarkManager::getBookmarks(const QString& folder) const {
    QList<Bookmark> bookmarks;
    for (const QJsonValue& val : rootArray_) {
        QJsonObject obj = val.toObject();
        if (obj["folder"].toString() == folder) {
            Bookmark b;
            b.id = obj["id"].toString();
            b.title = obj["title"].toString();
            b.url = obj["url"].toString();
            b.folder = obj["folder"].toString();
            b.isFolder = obj["isFolder"].toBool();
            b.position = obj["position"].toInt();
            bookmarks.append(b);
        }
    }
    return bookmarks;
}

QList<Bookmark> BookmarkManager::getAllBookmarks() const {
    return getBookmarks("");
}

QList<QString> BookmarkManager::getFolders() const {
    QList<QString> folders;
    for (const QJsonValue& val : rootArray_) {
        QJsonObject obj = val.toObject();
        if (obj["isFolder"].toBool()) {
            folders.append(obj["title"].toString());
        }
    }
    return folders;
}

Bookmark BookmarkManager::findBookmark(const QString& id) const {
    for (const QJsonValue& val : rootArray_) {
        QJsonObject obj = val.toObject();
        if (obj["id"].toString() == id) {
            Bookmark b;
            b.id = obj["id"].toString();
            b.title = obj["title"].toString();
            b.url = obj["url"].toString();
            b.folder = obj["folder"].toString();
            b.isFolder = obj["isFolder"].toBool();
            b.position = obj["position"].toInt();
            return b;
        }
    }
    return Bookmark();
}

void BookmarkManager::importFromHtml(const QString& filePath) {
    // Simple HTML bookmark import
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QString content = file.readAll();
    file.close();
    
    // Parse bookmarks from DL/DD tags
    QStringList lines = content.split("\n");
    for (const QString& line : lines) {
        if (line.contains("<DT><A HREF=")) {
            int start = line.indexOf("<A HREF=\"") + 9;
            int end = line.indexOf("\"", start);
            QString url = line.mid(start, end - start);
            
            start = line.indexOf(">") + 1;
            end = line.indexOf("</A>");
            QString title = line.mid(start, end - start);
            
            if (!url.isEmpty() && !title.isEmpty()) {
                addBookmark(title, url);
            }
        }
    }
}

void BookmarkManager::exportToHtml(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    
    QString html = "<!DOCTYPE NETSCAPE-Bookmark-file-1>\n";
    html += "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\n";
    html += "<TITLE>Bookmarks</TITLE>\n";
    html += "<H1>Bookmarks</H1>\n";
    html += "<DL><p>\n";
    
    for (const QJsonValue& val : rootArray_) {
        QJsonObject obj = val.toObject();
        if (!obj["isFolder"].toBool()) {
            html += QString("    <DT><A HREF=\"%1\">%2</A>\n").arg(obj["url"].toString(), obj["title"].toString());
        }
    }
    
    html += "</DL><p>\n";
    file.write(html.toUtf8());
    file.close();
}

} // namespace Tsunami