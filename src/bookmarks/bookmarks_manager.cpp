/*
 * Sea Browser - Bookmarks Manager
 * bookmarks_manager.cpp
 */

#include "bookmarks_manager.h"
#include <sqlite3.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace SeaBrowser {

BookmarksManager& BookmarksManager::instance() {
    static BookmarksManager instance;
    return instance;
}

void BookmarksManager::init(const std::string& db_path) {
    db_path_ = db_path;
    load();
    initialized_ = true;
}

void BookmarksManager::load() {
    bookmarks_.clear();
    
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[SeaBrowser] Cannot open bookmarks database: " << sqlite3_errmsg(db) << std::endl;
        if (db) sqlite3_close(db);
        return;
    }
    
    // Create table if not exists
    const char* create_sql = 
        "CREATE TABLE IF NOT EXISTS bookmarks ("
        "id TEXT PRIMARY KEY,"
        "title TEXT NOT NULL,"
        "url TEXT NOT NULL,"
        "folder TEXT DEFAULT 'Other Bookmarks',"
        "date_added INTEGER"
        ")";
    
    char* err_msg = nullptr;
    sqlite3_exec(db, create_sql, nullptr, nullptr, &err_msg);
    if (err_msg) {
        std::cerr << "[SeaBrowser] SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }
    
    // Load bookmarks
    const char* select_sql = "SELECT id, title, url, folder, date_added FROM bookmarks ORDER BY date_added DESC";
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Bookmark bm;
            bm.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            bm.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            bm.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            bm.folder = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            bm.date_added = sqlite3_column_int64(stmt, 4);
            bookmarks_.push_back(bm);
        }
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    std::cout << "[SeaBrowser] Loaded " << bookmarks_.size() << " bookmarks" << std::endl;
}

void BookmarksManager::save() {
    // Bookmarks are saved immediately on add/update/delete
}

void BookmarksManager::add_bookmark(const Bookmark& bookmark) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[SeaBrowser] Cannot open bookmarks database" << std::endl;
        return;
    }
    
    const char* insert_sql = 
        "INSERT OR REPLACE INTO bookmarks (id, title, url, folder, date_added) "
        "VALUES (?, ?, ?, ?, ?)";
    
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, bookmark.id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, bookmark.title.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, bookmark.url.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, bookmark.folder.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 5, bookmark.date_added);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    
    // Update cache
    bookmarks_.push_back(bookmark);
    std::cout << "[SeaBrowser] Added bookmark: " << bookmark.title << std::endl;
}

void BookmarksManager::delete_bookmark(const std::string& id) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK) {
        return;
    }
    
    const char* delete_sql = "DELETE FROM bookmarks WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db, delete_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    
    // Update cache
    bookmarks_.erase(
        std::remove_if(bookmarks_.begin(), bookmarks_.end(),
            [&id](const Bookmark& bm) { return bm.id == id; }),
        bookmarks_.end()
    );
    
    std::cout << "[SeaBrowser] Deleted bookmark: " << id << std::endl;
}

void BookmarksManager::update_bookmark(const Bookmark& bookmark) {
    delete_bookmark(bookmark.id);
    add_bookmark(bookmark);
}

std::vector<Bookmark> BookmarksManager::get_all_bookmarks() const {
    return bookmarks_;
}

std::vector<Bookmark> BookmarksManager::get_bookmarks_in_folder(const std::string& folder) const {
    std::vector<Bookmark> result;
    for (const auto& bm : bookmarks_) {
        if (bm.folder == folder) {
            result.push_back(bm);
        }
    }
    return result;
}

std::vector<std::string> BookmarksManager::get_folders() const {
    std::vector<std::string> folders;
    for (const auto& bm : bookmarks_) {
        if (std::find(folders.begin(), folders.end(), bm.folder) == folders.end()) {
            folders.push_back(bm.folder);
        }
    }
    if (folders.empty()) {
        folders.push_back("Bookmarks Bar");
        folders.push_back("Other Bookmarks");
    }
    return folders;
}

bool BookmarksManager::is_bookmarked(const std::string& url) const {
    for (const auto& bm : bookmarks_) {
        if (bm.url == url) {
            return true;
        }
    }
    return false;
}

void BookmarksManager::toggle_bookmark(const std::string& url, const std::string& title) {
    // Check if already bookmarked
    for (const auto& bm : bookmarks_) {
        if (bm.url == url) {
            delete_bookmark(bm.id);
            return;
        }
    }
    
    // Add new bookmark
    Bookmark bm;
    bm.id = std::to_string(std::time(nullptr)) + url;
    bm.title = title.empty() ? url : title;
    bm.url = url;
    bm.folder = "Other Bookmarks";
    bm.date_added = std::time(nullptr);
    
    add_bookmark(bm);
}

} // namespace SeaBrowser
