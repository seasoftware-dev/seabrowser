/*
 * Sea Browser - Content Blocker
 * content_blocker.h
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_set>

namespace SeaBrowser {

class ContentBlocker {
public:
    static ContentBlocker& instance();
    
    void load_filter_lists();
    bool is_blocked(const std::string& url);
    size_t blocked_count() const { return blocked_count_; }

private:
    ContentBlocker() = default;
    
    std::unordered_set<std::string> blocked_domains_;
    size_t blocked_count_ = 0;
};

} // namespace SeaBrowser
