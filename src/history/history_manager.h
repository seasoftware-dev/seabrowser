#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>
#include <mutex>

namespace SeaBrowser {

struct HistoryItem {
    std::string url;
    std::string title;
    long long timestamp;
};

class HistoryManager {
public:
    static HistoryManager& instance();
    
    void init(const std::string& db_path);
    void add_visit(const std::string& url, const std::string& title);
    std::vector<HistoryItem> get_recent(int limit = 10);
    void clear_history();
    void delete_history_item(const std::string& url);
    void cleanup_history();

private:
    HistoryManager() = default;
    ~HistoryManager();
    
    sqlite3* db_ = nullptr;
    std::string db_path_;
    std::mutex mutex_;
    
    void ensure_table();
};

} // namespace SeaBrowser
