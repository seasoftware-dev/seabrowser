/*
 * Sea Browser - address_bar.h
 */

#pragma once

#include <gtk/gtk.h>
#include <string>
#include <vector>

namespace SeaBrowser {

// Address bar functionality is currently in browser_window.cpp
// This file exists for future expansion (autocomplete, suggestions)

struct SearchEngine {
    std::string name;
    std::string url_template;  // %s = search query
    std::string icon;
    bool is_default = false;
};

inline std::vector<SearchEngine> get_search_engines() {
    return {
        {"Google", "https://www.google.com/search?q=%s", "🔍", true},
        {"DuckDuckGo", "https://duckduckgo.com/?q=%s", "🦆", false},
        {"Brave Search", "https://search.brave.com/search?q=%s", "🦁", false},
        {"Ecosia", "https://www.ecosia.org/search?q=%s", "🌳", false},
        {"Startpage", "https://www.startpage.com/sp/search?query=%s", "🔒", false},
        {"Qwant", "https://www.qwant.com/?q=%s", "🔵", false},
    };
}

} // namespace SeaBrowser
