/*
 * Sea Browser - Privacy-focused web browser
 * web_view.cpp - WebView utilities (GTK3)
 */

#include "web_view.h"
#include "application.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <iostream>
#include <limits.h>
#include <unistd.h>
#include "settings/settings_manager.h"
#include "history/history_manager.h"

namespace SeaBrowser {

// Get the directory where the executable is located
static std::string get_exe_dir() {
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        std::string path(buf);
        size_t pos = path.rfind('/');
        if (pos != std::string::npos) {
            return path.substr(0, pos);
        }
    }
    return ".";
}

// Find a resource file by checking multiple locations
static std::string find_resource(const std::string& relative_path) {
    std::string exe_dir = get_exe_dir();
    
    // List of paths to check, in order of priority
    std::vector<std::string> search_paths = {
        exe_dir + "/../data/" + relative_path,  // Dev: build-output/../data/
        exe_dir + "/data/" + relative_path,      // Release: alongside binary
        "/usr/share/seabrowser/data/" + relative_path,  // System install
        "data/" + relative_path,                 // CWD fallback
        "../data/" + relative_path               // CWD parent fallback
    };
    
    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            std::cout << "[SeaBrowser] Found resource: " << path << std::endl;
            return std::filesystem::absolute(path).string();
        }
    }
    
    std::cerr << "[SeaBrowser] ERROR: Resource not found: " << relative_path << std::endl;
    std::cerr << "[SeaBrowser] Searched in:" << std::endl;
    for (const auto& path : search_paths) {
        std::cerr << "  - " << path << std::endl;
    }
    return "";
}

void WebView::sea_scheme_request_handler(WebKitURISchemeRequest* request, gpointer /*user_data*/) {
    const gchar* uri = webkit_uri_scheme_request_get_uri(request);
    std::string uri_str = uri;
    std::string resource_path;
    
    std::cout << "[SeaBrowser] Handling URI: " << uri_str << std::endl;
    
    // Parse the URI to determine what file to load
    if (uri_str.starts_with("sea://")) {
        std::string page = uri_str.substr(6);  // Remove "sea://"
        
        // Map page names to files
        if (page == "newtab" || page == "start" || page.empty()) {
            resource_path = "pages/newtab.html";
        } else if (page == "setup") {
            resource_path = "pages/setup.html";
        } else if (page == "settings") {
            resource_path = "pages/settings.html";
        } else if (page == "history") {
            resource_path = "pages/history.html";
        } else if (page == "extensions") {
            resource_path = "pages/extensions.html";
        } else if (page == "bookmarks") {
            resource_path = "pages/history.html";
        } else if (page.starts_with("resources/")) {
            // Direct resource request (for CSS, images, etc.)
            resource_path = page.substr(10);  // Remove "resources/"
        } else {
            // Unknown page - 404
            std::string html = "<html><body style='background:#1a1a2e;color:white;font-family:sans-serif;padding:2em;'>"
                              "<h1>404 - Page Not Found</h1>"
                              "<p>The page <code>" + page + "</code> does not exist.</p>"
                              "<p><a href='sea://newtab' style='color:#00d4ff;'>Go to New Tab</a></p>"
                              "</body></html>";
            GInputStream* stream = g_memory_input_stream_new_from_data(g_strdup(html.c_str()), -1, g_free);
            webkit_uri_scheme_request_finish(request, stream, -1, "text/html");
            g_object_unref(stream);
            return;
        }
    }
    
    // Find the actual file
    std::string file_path = find_resource(resource_path);
    
    if (file_path.empty()) {
        // Resource not found
        std::string html = "<html><body style='background:#1a1a2e;color:white;font-family:sans-serif;padding:2em;'>"
                          "<h1>Error: Resource Not Found</h1>"
                          "<p>Could not find: <code>" + resource_path + "</code></p>"
                          "</body></html>";
        GInputStream* stream = g_memory_input_stream_new_from_data(g_strdup(html.c_str()), -1, g_free);
        webkit_uri_scheme_request_finish(request, stream, -1, "text/html");
        g_object_unref(stream);
        return;
    }
    
    // Determine MIME type
    std::string mime_type = "text/html";
    if (file_path.ends_with(".css")) mime_type = "text/css";
    else if (file_path.ends_with(".js")) mime_type = "application/javascript";
    else if (file_path.ends_with(".png")) mime_type = "image/png";
    else if (file_path.ends_with(".jpg") || file_path.ends_with(".jpeg")) mime_type = "image/jpeg";
    else if (file_path.ends_with(".svg")) mime_type = "image/svg+xml";
    else if (file_path.ends_with(".json")) mime_type = "application/json";
    
    // Read and serve the file
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::string html = "<html><body><h1>Error reading file</h1></body></html>";
        GInputStream* stream = g_memory_input_stream_new_from_data(g_strdup(html.c_str()), -1, g_free);
        webkit_uri_scheme_request_finish(request, stream, -1, "text/html");
        g_object_unref(stream);
        return;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    // For newtab.html, inject dynamic content
    if (resource_path == "pages/newtab.html") {
        // Inject search URL
        std::string engine = SettingsManager::instance().search().default_engine;
        std::string search_url = "https://www.google.com/search?q=";
        if (engine == "duckduckgo") search_url = "https://duckduckgo.com/?q=";
        else if (engine == "bing") search_url = "https://www.bing.com/search?q=";
        else if (engine == "brave") search_url = "https://search.brave.com/search?q=";
        
        size_t pos = content.find("{{SEARCH_URL}}");
        if (pos != std::string::npos) content.replace(pos, 14, search_url);
        
        // Inject recent sites
        std::string recent_html;
        auto items = HistoryManager::instance().get_recent(4);
        for (const auto& item : items) {
            recent_html += "<a href='" + item.url + "' class='site-card'><span class='site-icon'>🌐</span>" + item.title + "</a>";
        }
        if (recent_html.empty()) {
            recent_html = "<div style='opacity:0.5;'>No recent history</div>";
        }
        
        pos = content.find("{{RECENT_SITES}}");
        if (pos != std::string::npos) content.replace(pos, 16, recent_html);
    }
    
    // For history.html, inject real history data
    if (resource_path == "pages/history.html") {
        auto items = HistoryManager::instance().get_recent(100);
        std::string history_json = "[";
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) history_json += ",";
            // Escape quotes in title and url
            std::string safe_title = items[i].title;
            std::string safe_url = items[i].url;
            for (auto& c : safe_title) if (c == '"') c = '\'';
            for (auto& c : safe_url) if (c == '"') c = '\'';
            history_json += "{\"url\":\"" + safe_url + "\",\"title\":\"" + safe_title + "\",\"timestamp\":" + std::to_string(items[i].timestamp) + "}";
        }
        history_json += "]";
        
        size_t pos = content.find("{{HISTORY_DATA}}");
        if (pos != std::string::npos) content.replace(pos, 16, history_json);
    }
    
    // For settings.html, inject current settings
    if (resource_path == "pages/settings.html") {
        auto& settings = SettingsManager::instance();
        std::string settings_json = "{";
        settings_json += "\"theme\":\"" + settings.appearance().theme + "\",";
        settings_json += "\"zoom\":" + std::to_string(settings.appearance().zoom_level) + ",";
        settings_json += "\"homepage\":\"" + settings.general().homepage + "\",";
        settings_json += "\"default_engine\":\"" + settings.search().default_engine + "\",";
        settings_json += "\"https_only\":" + std::string(settings.privacy().https_only ? "true" : "false") + ",";
        settings_json += "\"send_dnt\":" + std::string(settings.privacy().send_dnt ? "true" : "false") + ",";
        settings_json += "\"clear_on_exit\":" + std::string(settings.privacy().clear_on_exit ? "true" : "false") + ",";
        settings_json += "\"restore_tabs\":" + std::string(settings.general().restore_tabs ? "true" : "false") + ",";
        settings_json += "\"encrypt_settings\":" + std::string(settings.general().encrypt_settings ? "true" : "false");
        settings_json += "}";
        
        size_t pos = content.find("{{SETTINGS_DATA}}");
        if (pos != std::string::npos) content.replace(pos, 17, settings_json);
    }
    
    GInputStream* stream = g_memory_input_stream_new_from_data(g_strdup(content.c_str()), -1, g_free);
    webkit_uri_scheme_request_finish(request, stream, -1, mime_type.c_str());
    g_object_unref(stream);
}

std::string WebView::generate_internal_page(const std::string& /*page*/) {
    return "";
}

} // namespace SeaBrowser
