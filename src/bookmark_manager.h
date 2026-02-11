#pragma once

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QList>
#include <QUrl>

namespace Tsunami {

struct Bookmark {
    QString id;
    QString title;
    QString url;
    QString folder;
    QString icon;
    int position = 0;
    bool isFolder = false;
    QList<Bookmark> children;
};

class BookmarkManager : public QObject {
    Q_OBJECT

public:
    static BookmarkManager& instance() {
        static BookmarkManager instance;
        return instance;
    }

    void loadBookmarks();
    void saveBookmarks();
    
    void addBookmark(const QString& title, const QString& url, const QString& folder = "");
    void removeBookmark(const QString& id);
    void updateBookmark(const QString& id, const QString& title, const QString& url);
    void moveBookmark(const QString& id, const QString& newFolder);
    
    void addFolder(const QString& name, const QString& parentFolder = "");
    void removeFolder(const QString& folderName);
    
    QList<Bookmark> getBookmarks(const QString& folder = "") const;
    QList<Bookmark> getAllBookmarks() const;
    QList<QString> getFolders() const;
    
    Bookmark findBookmark(const QString& id) const;
    
    void importFromHtml(const QString& filePath);
    void exportToHtml(const QString& filePath);

signals:
    void bookmarksChanged();

private:
    BookmarkManager();
    ~BookmarkManager();
    BookmarkManager(const BookmarkManager&) = delete;
    BookmarkManager& operator=(const BookmarkManager&) = delete;
    
    QJsonArray rootArray_;
    QList<Bookmark> parseBookmarks(const QJsonArray& array, const QString& folder = "") const;
    QString generateId();
    
    QString getBookmarksFile() const;
};

} // namespace Tsunami