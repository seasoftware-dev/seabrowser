/*
 * Sea Browser - Downloads Manager
 * downloads_manager.h
 */

#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <cstdint>
#include <functional>

namespace SeaBrowser {

enum class DownloadState {
    InProgress,
    Completed,
    Failed,
    Cancelled,
    Paused
};

struct Download {
    std::string id;
    std::string url;
    std::string filename;
    std::string path;
    std::string mime_type;
    uint64_t total_bytes = 0;
    uint64_t received_bytes = 0;
    double speed = 0;
    DownloadState state = DownloadState::InProgress;
    std::time_t start_time = 0;
    std::time_t end_time = 0;
    std::string error_message;
};

class DownloadsManager {
public:
    static DownloadsManager& instance();
    
    void init(const std::string& db_path);
    void load();
    void save();
    
    // Download operations
    std::string start_download(const std::string& url, const std::string& suggested_filename = "");
    void cancel_download(const std::string& id);
    void pause_download(const std::string& id);
    void resume_download(const std::string& id);
    void retry_download(const std::string& id);
    void remove_download(const std::string& id);
    void clear_completed();
    
    // Getters
    std::vector<Download> get_all_downloads() const;
    std::vector<Download> get_active_downloads() const;
    Download* get_download(const std::string& id);
    
    // Progress callback
    void set_progress_callback(std::function<void(const Download&)> callback);
    void update_progress(const std::string& id, uint64_t received, uint64_t total, double speed);
    void complete_download(const std::string& id);
    void fail_download(const std::string& id, const std::string& error);
    
    // File operations
    void open_file(const std::string& path);
    void show_in_folder(const std::string& path);
    void open_downloads_folder();
    
private:
    DownloadsManager() = default;
    
    std::string db_path_;
    std::vector<Download> downloads_;
    bool initialized_ = false;
    std::function<void(const Download&)> progress_callback_;
    
    std::string generate_id();
    std::string get_filename_from_url(const std::string& url);
    std::string sanitize_filename(const std::string& filename);
};

} // namespace SeaBrowser
