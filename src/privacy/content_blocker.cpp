/*
 * Sea Browser - Content Blocker
 * content_blocker.cpp
 */

#include "content_blocker.h"

namespace SeaBrowser {

ContentBlocker& ContentBlocker::instance() {
    static ContentBlocker instance;
    return instance;
}

void ContentBlocker::load_filter_lists() {
    blocked_domains_ = {
        "google-analytics.com", "googletagmanager.com", "doubleclick.net",
        "googlesyndication.com", "facebook.net", "connect.facebook.net",
        "analytics.twitter.com", "ads.twitter.com", "hotjar.com",
        "mixpanel.com", "segment.io", "amplitude.com"
    };
}

bool ContentBlocker::is_blocked(const std::string& url) {
    for (const auto& domain : blocked_domains_) {
        if (url.find(domain) != std::string::npos) {
            blocked_count_++;
            return true;
        }
    }
    return false;
}

} // namespace SeaBrowser
