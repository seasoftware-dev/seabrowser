#include "history_manager.h"
#include <iostream>
#include <chrono>
#include <filesystem>

namespace SeaBrowser {

HistoryManager& HistoryManager::instance() {
    static HistoryManager instance;
    return instance;
}

HistoryManager::~HistoryManager() {
    if (db_) {
        sqlite3_close(db_);
    }
}

void HistoryManager::init(const std::string& db_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    db_path_ = db_path;
    
    // Ensure directory exists
    auto path = std::filesystem::path(db_path).parent_path();
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }

    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Failed to open history db: " << sqlite3_errmsg(db_) << std::endl;
        return;
    }
    
    ensure_table();
}

void HistoryManager::ensure_table() {
    const char* sql = "CREATE TABLE IF NOT EXISTS visits ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "url TEXT NOT NULL, "
                      "title TEXT, "
                      "timestamp INTEGER);";
                      
    char* err_msg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }
}

void HistoryManager::add_visit(const std::string& url, const std::string& title) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_) return;
    
    // Don't add internal pages
    if (url.find("sea://") == 0) return;
    
    long long timestamp = std::chrono::seconds(std::time(NULL)).count();
    
    const char* sql = "INSERT INTO visits (url, title, timestamp) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, url.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 3, timestamp);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

std::vector<HistoryItem> HistoryManager::get_recent(int limit) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<HistoryItem> items;
    if (!db_) return items;
    
    const char* sql = "SELECT url, title, timestamp FROM visits ORDER BY timestamp DESC LIMIT ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, limit);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            HistoryItem item;
            item.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            
            const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            item.title = title ? title : item.url;
            
            item.timestamp = sqlite3_column_int64(stmt, 2);
            items.push_back(item);
        }
        sqlite3_finalize(stmt);
    }
    
    return items;
}

void HistoryManager::clear_history() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_) return;
    
    const char* sql = "DELETE FROM visits;";
    sqlite3_exec(db_, sql, nullptr, nullptr, nullptr);
}

void HistoryManager::cleanup_history() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!db_) return;
    
    // Default retention: 30 days
    long long threshold = std::chrono::seconds(std::time(NULL)).count() - (30 * 24 * 60 * 60);
    
    const char* sql = "DELETE FROM visits WHERE timestamp < ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, threshold);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

} // namespace SeaBrowser
