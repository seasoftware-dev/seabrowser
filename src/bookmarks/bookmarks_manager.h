/*
 * Sea Browser - Bookmarks Manager
 * bookmarks_manager.h
 */

#pragma once

#include <string>
#include <vector>
#include <ctime>

namespace SeaBrowser {

struct Bookmark {
    std::string id;
    std::string title;
    std::string url;
    std::string folder;
    std::time_t date_added;
};

class BookmarksManager {
public:
    static BookmarksManager& instance();
    
    void init(const std::string& db_path);
    void load();
    void save();
    
    // Bookmark operations
    void add_bookmark(const Bookmark& bookmark);
    void delete_bookmark(const std::string& id);
    void update_bookmark(const Bookmark& bookmark);
    
    // Getters
    std::vector<Bookmark> get_all_bookmarks() const;
    std::vector<Bookmark> get_bookmarks_in_folder(const std::string& folder) const;
    std::vector<std::string> get_folders() const;
    
    // Check if URL is bookmarked
    bool is_bookmarked(const std::string& url) const;
    
    // Toggle bookmark for URL
    void toggle_bookmark(const std::string& url, const std::string& title);
    
private:
    BookmarksManager() = default;
    
    std::string db_path_;
    std::vector<Bookmark> bookmarks_;
    bool initialized_ = false;
};

} // namespace SeaBrowser
