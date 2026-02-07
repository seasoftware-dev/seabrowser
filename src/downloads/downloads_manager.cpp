/*
 * Sea Browser - Downloads Manager
 * downloads_manager.cpp
 */

#include "downloads_manager.h"
#include <sqlite3.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <glib.h>

namespace SeaBrowser {

DownloadsManager& DownloadsManager::instance() {
    static DownloadsManager instance;
    return instance;
}

void DownloadsManager::init(const std::string& db_path) {
    db_path_ = db_path;
    load();
    initialized_ = true;
}

void DownloadsManager::load() {
    downloads_.clear();
    
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[SeaBrowser] Cannot open downloads database: " << sqlite3_errmsg(db) << std::endl;
        if (db) sqlite3_close(db);
        return;
    }
    
    // Create table if not exists
    const char* create_sql = 
        "CREATE TABLE IF NOT EXISTS downloads ("
        "id TEXT PRIMARY KEY,"
        "url TEXT NOT NULL,"
        "filename TEXT NOT NULL,"
        "path TEXT,"
        "mime_type TEXT,"
        "total_bytes INTEGER DEFAULT 0,"
        "received_bytes INTEGER DEFAULT 0,"
        "state INTEGER DEFAULT 0,"
        "start_time INTEGER,"
        "end_time INTEGER,"
        "error_message TEXT"
        ")";
    
    char* err_msg = nullptr;
    sqlite3_exec(db, create_sql, nullptr, nullptr, &err_msg);
    if (err_msg) {
        std::cerr << "[SeaBrowser] SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }
    
    // Load downloads
    const char* select_sql = 
        "SELECT id, url, filename, path, mime_type, total_bytes, received_bytes, "
        "state, start_time, end_time, error_message FROM downloads ORDER BY start_time DESC";
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Download dl;
            dl.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            dl.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            dl.filename = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            dl.path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            dl.mime_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            dl.total_bytes = sqlite3_column_int64(stmt, 5);
            dl.received_bytes = sqlite3_column_int64(stmt, 6);
            dl.state = static_cast<DownloadState>(sqlite3_column_int(stmt, 7));
            dl.start_time = sqlite3_column_int64(stmt, 8);
            dl.end_time = sqlite3_column_int64(stmt, 9);
            const char* error = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
            if (error) dl.error_message = error;
            downloads_.push_back(dl);
        }
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    std::cout << "[SeaBrowser] Loaded " << downloads_.size() << " downloads" << std::endl;
}

void DownloadsManager::save() {
    // Downloads are saved immediately on state changes
}

std::string DownloadsManager::start_download(const std::string& url, const std::string& suggested_filename) {
    Download dl;
    dl.id = generate_id();
    dl.url = url;
    dl.filename = suggested_filename.empty() ? get_filename_from_url(url) : suggested_filename;
    dl.filename = sanitize_filename(dl.filename);
    dl.state = DownloadState::InProgress;
    dl.start_time = std::time(nullptr);
    
    // Set download path
    const char* download_dir = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
    if (!download_dir) {
        download_dir = g_get_home_dir();
    }
    dl.path = std::string(download_dir) + "/" + dl.filename;
    
    // Check if file exists and append number if needed
    int counter = 1;
    std::string base_path = dl.path;
    while (std::filesystem::exists(dl.path)) {
        size_t dot_pos = base_path.find_last_of('.');
        if (dot_pos != std::string::npos) {
            dl.path = base_path.substr(0, dot_pos) + " (" + std::to_string(counter) + ")" + base_path.substr(dot_pos);
        } else {
            dl.path = base_path + " (" + std::to_string(counter) + ")";
        }
        counter++;
    }
    
    // Save to database
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) == SQLITE_OK) {
        const char* insert_sql = 
            "INSERT INTO downloads (id, url, filename, path, mime_type, total_bytes, received_bytes, "
            "state, start_time, end_time, error_message) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, dl.id.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, dl.url.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, dl.filename.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, dl.path.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, dl.mime_type.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 6, dl.total_bytes);
            sqlite3_bind_int64(stmt, 7, dl.received_bytes);
            sqlite3_bind_int(stmt, 8, static_cast<int>(dl.state));
            sqlite3_bind_int64(stmt, 9, dl.start_time);
            sqlite3_bind_int64(stmt, 10, dl.end_time);
            sqlite3_bind_text(stmt, 11, dl.error_message.c_str(), -1, SQLITE_STATIC);
            
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }
    
    downloads_.insert(downloads_.begin(), dl);
    std::cout << "[SeaBrowser] Started download: " << dl.filename << std::endl;
    
    return dl.id;
}

void DownloadsManager::cancel_download(const std::string& id) {
    auto it = std::find_if(downloads_.begin(), downloads_.end(),
        [&id](const Download& dl) { return dl.id == id; });
    
    if (it != downloads_.end()) {
        it->state = DownloadState::Cancelled;
        it->end_time = std::time(nullptr);
        
        // Update database
        sqlite3* db = nullptr;
        if (sqlite3_open(db_path_.c_str(), &db) == SQLITE_OK) {
            const char* update_sql = "UPDATE downloads SET state = ?, end_time = ? WHERE id = ?";
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, static_cast<int>(it->state));
                sqlite3_bind_int64(stmt, 2, it->end_time);
                sqlite3_bind_text(stmt, 3, id.c_str(), -1, SQLITE_STATIC);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
        }
    }
}

void DownloadsManager::pause_download(const std::string& id) {
    auto it = std::find_if(downloads_.begin(), downloads_.end(),
        [&id](const Download& dl) { return dl.id == id; });
    
    if (it != downloads_.end() && it->state == DownloadState::InProgress) {
        it->state = DownloadState::Paused;
        
        // Update database
        sqlite3* db = nullptr;
        if (sqlite3_open(db_path_.c_str(), &db) == SQLITE_OK) {
            const char* update_sql = "UPDATE downloads SET state = ? WHERE id = ?";
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, static_cast<int>(it->state));
                sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_STATIC);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
        }
    }
}

void DownloadsManager::resume_download(const std::string& id) {
    auto it = std::find_if(downloads_.begin(), downloads_.end(),
        [&id](const Download& dl) { return dl.id == id; });
    
    if (it != downloads_.end() && it->state == DownloadState::Paused) {
        it->state = DownloadState::InProgress;
        
        // Update database
        sqlite3* db = nullptr;
        if (sqlite3_open(db_path_.c_str(), &db) == SQLITE_OK) {
            const char* update_sql = "UPDATE downloads SET state = ? WHERE id = ?";
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, static_cast<int>(it->state));
                sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_STATIC);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
        }
    }
}

void DownloadsManager::retry_download(const std::string& id) {
    auto it = std::find_if(downloads_.begin(), downloads_.end(),
        [&id](const Download& dl) { return dl.id == id; });
    
    if (it != downloads_.end()) {
        // Reset state
        it->state = DownloadState::InProgress;
        it->received_bytes = 0;
        it->error_message.clear();
        it->start_time = std::time(nullptr);
        it->end_time = 0;
        
        // Update database
        sqlite3* db = nullptr;
        if (sqlite3_open(db_path_.c_str(), &db) == SQLITE_OK) {
            const char* update_sql = 
                "UPDATE downloads SET state = ?, received_bytes = ?, error_message = ?, "
                "start_time = ?, end_time = ? WHERE id = ?";
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, static_cast<int>(it->state));
                sqlite3_bind_int64(stmt, 2, it->received_bytes);
                sqlite3_bind_text(stmt, 3, "", -1, SQLITE_STATIC);
                sqlite3_bind_int64(stmt, 4, it->start_time);
                sqlite3_bind_int64(stmt, 5, it->end_time);
                sqlite3_bind_text(stmt, 6, id.c_str(), -1, SQLITE_STATIC);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
        }
    }
}

void DownloadsManager::remove_download(const std::string& id) {
    // Remove from database
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) == SQLITE_OK) {
        const char* delete_sql = "DELETE FROM downloads WHERE id = ?";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, delete_sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        sqlite3_close(db);
    }
    
    // Remove from cache
    downloads_.erase(
        std::remove_if(downloads_.begin(), downloads_.end(),
            [&id](const Download& dl) { return dl.id == id; }),
        downloads_.end()
    );
}

void DownloadsManager::clear_completed() {
    // Remove completed and cancelled downloads from database
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path_.c_str(), &db) == SQLITE_OK) {
        const char* delete_sql = "DELETE FROM downloads WHERE state IN (1, 3)";
        sqlite3_exec(db, delete_sql, nullptr, nullptr, nullptr);
        sqlite3_close(db);
    }
    
    // Remove from cache
    downloads_.erase(
        std::remove_if(downloads_.begin(), downloads_.end(),
            [](const Download& dl) { 
                return dl.state == DownloadState::Completed || 
                       dl.state == DownloadState::Cancelled; 
            }),
        downloads_.end()
    );
}

std::vector<Download> DownloadsManager::get_all_downloads() const {
    return downloads_;
}

std::vector<Download> DownloadsManager::get_active_downloads() const {
    std::vector<Download> active;
    for (const auto& dl : downloads_) {
        if (dl.state == DownloadState::InProgress || dl.state == DownloadState::Paused) {
            active.push_back(dl);
        }
    }
    return active;
}

Download* DownloadsManager::get_download(const std::string& id) {
    auto it = std::find_if(downloads_.begin(), downloads_.end(),
        [&id](const Download& dl) { return dl.id == id; });
    
    if (it != downloads_.end()) {
        return &(*it);
    }
    return nullptr;
}

void DownloadsManager::set_progress_callback(std::function<void(const Download&)> callback) {
    progress_callback_ = callback;
}

void DownloadsManager::update_progress(const std::string& id, uint64_t received, uint64_t total, double speed) {
    auto it = std::find_if(downloads_.begin(), downloads_.end(),
        [&id](const Download& dl) { return dl.id == id; });
    
    if (it != downloads_.end()) {
        it->received_bytes = received;
        it->total_bytes = total;
        it->speed = speed;
        
        if (progress_callback_) {
            progress_callback_(*it);
        }
    }
}

void DownloadsManager::complete_download(const std::string& id) {
    auto it = std::find_if(downloads_.begin(), downloads_.end(),
        [&id](const Download& dl) { return dl.id == id; });
    
    if (it != downloads_.end()) {
        it->state = DownloadState::Completed;
        it->end_time = std::time(nullptr);
        it->received_bytes = it->total_bytes;
        
        // Update database
        sqlite3* db = nullptr;
        if (sqlite3_open(db_path_.c_str(), &db) == SQLITE_OK) {
            const char* update_sql = 
                "UPDATE downloads SET state = ?, end_time = ?, received_bytes = ? WHERE id = ?";
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, static_cast<int>(it->state));
                sqlite3_bind_int64(stmt, 2, it->end_time);
                sqlite3_bind_int64(stmt, 3, it->received_bytes);
                sqlite3_bind_text(stmt, 4, id.c_str(), -1, SQLITE_STATIC);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
        }
        
        if (progress_callback_) {
            progress_callback_(*it);
        }
        
        std::cout << "[SeaBrowser] Download completed: " << it->filename << std::endl;
    }
}

void DownloadsManager::fail_download(const std::string& id, const std::string& error) {
    auto it = std::find_if(downloads_.begin(), downloads_.end(),
        [&id](const Download& dl) { return dl.id == id; });
    
    if (it != downloads_.end()) {
        it->state = DownloadState::Failed;
        it->end_time = std::time(nullptr);
        it->error_message = error;
        
        // Update database
        sqlite3* db = nullptr;
        if (sqlite3_open(db_path_.c_str(), &db) == SQLITE_OK) {
            const char* update_sql = 
                "UPDATE downloads SET state = ?, end_time = ?, error_message = ? WHERE id = ?";
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, static_cast<int>(it->state));
                sqlite3_bind_int64(stmt, 2, it->end_time);
                sqlite3_bind_text(stmt, 3, error.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 4, id.c_str(), -1, SQLITE_STATIC);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
        }
        
        if (progress_callback_) {
            progress_callback_(*it);
        }
        
        std::cerr << "[SeaBrowser] Download failed: " << it->filename << " - " << error << std::endl;
    }
}

void DownloadsManager::open_file(const std::string& path) {
    std::string cmd = "xdg-open \"" + path + "\" &";
    std::system(cmd.c_str());
}

void DownloadsManager::show_in_folder(const std::string& path) {
    std::filesystem::path file_path(path);
    std::string folder = file_path.parent_path().string();
    std::string cmd = "xdg-open \"" + folder + "\" &";
    std::system(cmd.c_str());
}

void DownloadsManager::open_downloads_folder() {
    const char* download_dir = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
    if (!download_dir) {
        download_dir = g_get_home_dir();
    }
    std::string cmd = "xdg-open \"" + std::string(download_dir) + "\" &";
    std::system(cmd.c_str());
}

std::string DownloadsManager::generate_id() {
    return std::to_string(std::time(nullptr)) + "_" + std::to_string(rand());
}

std::string DownloadsManager::get_filename_from_url(const std::string& url) {
    size_t last_slash = url.find_last_of('/');
    if (last_slash != std::string::npos && last_slash < url.length() - 1) {
        std::string filename = url.substr(last_slash + 1);
        // Remove query parameters
        size_t query_pos = filename.find('?');
        if (query_pos != std::string::npos) {
            filename = filename.substr(0, query_pos);
        }
        // URL decode
        std::string decoded;
        for (size_t i = 0; i < filename.length(); i++) {
            if (filename[i] == '%' && i + 2 < filename.length()) {
                int hex = std::stoi(filename.substr(i + 1, 2), nullptr, 16);
                decoded += static_cast<char>(hex);
                i += 2;
            } else if (filename[i] == '+') {
                decoded += ' ';
            } else {
                decoded += filename[i];
            }
        }
        return decoded.empty() ? "download" : decoded;
    }
    return "download";
}

std::string DownloadsManager::sanitize_filename(const std::string& filename) {
    std::string sanitized = filename;
    // Remove or replace invalid characters
    for (char& c : sanitized) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || 
            c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }
    // Limit length
    if (sanitized.length() > 255) {
        size_t ext_pos = sanitized.find_last_of('.');
        if (ext_pos != std::string::npos && ext_pos > 200) {
            sanitized = sanitized.substr(0, 200) + sanitized.substr(ext_pos);
        } else {
            sanitized = sanitized.substr(0, 255);
        }
    }
    return sanitized;
}

} // namespace SeaBrowser
