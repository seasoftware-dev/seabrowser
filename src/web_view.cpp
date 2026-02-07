/*
 * Sea Browser - Privacy-focused web browser
 * web_view.cpp - WebView utilities (GTK3)
 */

#include "web_view.h"
#include "application.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <unistd.h>
#include <json-glib/json-glib.h>
#include "settings/settings_manager.h"
#include "history/history_manager.h"
#include "bookmarks/bookmarks_manager.h"
#include "downloads/downloads_manager.h"
#include "extension_manager.h"
#include "browser_window.h"

namespace SeaBrowser {

// Find a resource file by checking multiple locations
static std::string find_resource(const std::string& relative_path) {
    if (!Utils::is_safe_resource_path(relative_path)) {
        std::cerr << "[SeaBrowser] SECURITY: Blocked unsafe resource path: " << relative_path << std::endl;
        return "";
    }
    
    std::string exe_dir = Utils::get_exe_dir();
    
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
            resource_path = "pages/bookmarks.html";
        } else if (page == "downloads") {
            resource_path = "pages/downloads.html";
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
            std::string safe_url = Utils::escape_html(item.url);
            std::string safe_title = Utils::escape_html(item.title);
            recent_html += "<a href='" + safe_url + "' class='site-card'><span class='site-icon'>🌐</span>" + safe_title + "</a>";
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
            std::string safe_title = Utils::escape_json_string(items[i].title);
            std::string safe_url = Utils::escape_json_string(items[i].url);
            history_json += "{\"url\":\"" + safe_url + "\",\"title\":\"" + safe_title + "\",\"timestamp\":" + std::to_string(items[i].timestamp) + "}";
        }
        history_json += "]";
        
        size_t pos = content.find("{{HISTORY_DATA}}");
        if (pos != std::string::npos) content.replace(pos, 16, history_json);
    }
    
    // For bookmarks.html, inject bookmarks data
    if (resource_path == "pages/bookmarks.html") {
        auto bookmarks = BookmarksManager::instance().get_all_bookmarks();
        std::string bookmarks_json = "[";
        for (size_t i = 0; i < bookmarks.size(); ++i) {
            if (i > 0) bookmarks_json += ",";
            bookmarks_json += "{";
            bookmarks_json += "\"id\":\"" + Utils::escape_json_string(bookmarks[i].id) + "\",";
            bookmarks_json += "\"title\":\"" + Utils::escape_json_string(bookmarks[i].title) + "\",";
            bookmarks_json += "\"url\":\"" + Utils::escape_json_string(bookmarks[i].url) + "\",";
            bookmarks_json += "\"folder\":\"" + Utils::escape_json_string(bookmarks[i].folder) + "\",";
            bookmarks_json += "\"dateAdded\":" + std::to_string(bookmarks[i].date_added);
            bookmarks_json += "}";
        }
        bookmarks_json += "]";
        
        size_t pos = content.find("{{BOOKMARKS_DATA}}");
        if (pos != std::string::npos) content.replace(pos, 18, bookmarks_json);
    }
    
    // For downloads.html, inject downloads data
    if (resource_path == "pages/downloads.html") {
        auto downloads = DownloadsManager::instance().get_all_downloads();
        std::string downloads_json = "[";
        for (size_t i = 0; i < downloads.size(); ++i) {
            if (i > 0) downloads_json += ",";
            downloads_json += "{";
            downloads_json += "\"id\":\"" + Utils::escape_json_string(downloads[i].id) + "\",";
            downloads_json += "\"url\":\"" + Utils::escape_json_string(downloads[i].url) + "\",";
            downloads_json += "\"filename\":\"" + Utils::escape_json_string(downloads[i].filename) + "\",";
            downloads_json += "\"path\":\"" + Utils::escape_json_string(downloads[i].path) + "\",";
            downloads_json += "\"mime_type\":\"" + Utils::escape_json_string(downloads[i].mime_type) + "\",";
            downloads_json += "\"totalBytes\":" + std::to_string(downloads[i].total_bytes) + ",";
            downloads_json += "\"receivedBytes\":" + std::to_string(downloads[i].received_bytes) + ",";
            downloads_json += "\"speed\":" + std::to_string(downloads[i].speed) + ",";
            downloads_json += "\"state\":\"" + std::string(downloads[i].state == DownloadState::InProgress ? "in-progress" : 
                                                                  downloads[i].state == DownloadState::Completed ? "completed" : 
                                                                  downloads[i].state == DownloadState::Failed ? "failed" : 
                                                                  downloads[i].state == DownloadState::Cancelled ? "cancelled" : "paused") + "\",";
            downloads_json += "\"startTime\":" + std::to_string(downloads[i].start_time) + ",";
            downloads_json += "\"endTime\":" + std::to_string(downloads[i].end_time);
            downloads_json += "}";
        }
        downloads_json += "]";
        
        size_t pos = content.find("{{DOWNLOADS_DATA}}");
        if (pos != std::string::npos) content.replace(pos, 18, downloads_json);
    }
    
    // For settings.html, inject current settings
    if (resource_path == "pages/settings.html") {
        auto& settings = SettingsManager::instance();
        std::string settings_json = "{";
        settings_json += "\"theme\":\"" + Utils::escape_json_string(settings.appearance().theme) + "\",";
        settings_json += "\"custom_color\":\"" + Utils::escape_json_string(settings.appearance().custom_color) + "\",";
        settings_json += "\"zoom\":" + std::to_string(settings.appearance().zoom_level) + ",";
        settings_json += "\"homepage\":\"" + Utils::escape_json_string(settings.general().homepage) + "\",";
        settings_json += "\"default_engine\":\"" + Utils::escape_json_string(settings.search().default_engine) + "\",";
        settings_json += "\"https_only\":" + std::string(settings.privacy().https_only ? "true" : "false") + ",";
        settings_json += "\"send_dnt\":" + std::string(settings.privacy().send_dnt ? "true" : "false") + ",";
        settings_json += "\"clear_on_exit\":" + std::string(settings.privacy().clear_on_exit ? "true" : "false") + ",";
        settings_json += "\"restore_tabs\":" + std::string(settings.general().restore_tabs ? "true" : "false") + ",";
        settings_json += "\"encrypt_settings\":" + std::string(settings.general().encrypt_settings ? "true" : "false") + ",";
        settings_json += "\"vertical_tabs\":" + std::string(settings.appearance().vertical_tabs ? "true" : "false");
        settings_json += "}";

        size_t pos = content.find("{{SETTINGS_DATA}}");
        if (pos != std::string::npos) content.replace(pos, 17, settings_json);
    }

    // For extensions.html, inject installed extensions
    if (resource_path == "pages/extensions.html") {
        auto& ext_mgr = ExtensionManager::instance();
        auto extensions = ext_mgr.get_extensions();
        std::string ext_json = "[";

        for (size_t i = 0; i < extensions.size(); ++i) {
             if (i > 0) ext_json += ",";
             const auto& ext = extensions[i];

             // Get type string
             std::string type_str = "JavaScript";
             if (ext.type == ExtensionType::Chrome) type_str = "Chrome";
             else if (ext.type == ExtensionType::Firefox) type_str = "Firefox";

             ext_json += "{";
             ext_json += "\"name\":\"" + Utils::escape_json_string(ext.name) + "\",";
             ext_json += "\"description\":\"" + Utils::escape_json_string(ext.description) + "\",";
             ext_json += "\"version\":\"" + Utils::escape_json_string(ext.version) + "\",";
             ext_json += "\"author\":\"" + Utils::escape_json_string(ext.author) + "\",";
             ext_json += "\"type\":\"" + Utils::escape_json_string(type_str) + "\",";
             ext_json += "\"enabled\":" + std::string(ext.enabled ? "true" : "false");
             ext_json += "}";
        }

        ext_json += "]";

        size_t pos = content.find("{{EXTENSIONS_DATA}}");
        if (pos != std::string::npos) content.replace(pos, 19, ext_json);
    }
    
    GInputStream* stream = g_memory_input_stream_new_from_data(g_strdup(content.c_str()), -1, g_free);
    webkit_uri_scheme_request_finish(request, stream, -1, mime_type.c_str());
    g_object_unref(stream);
}

void WebView::on_script_message_received(WebKitUserContentManager* /*manager*/, WebKitJavascriptResult* result, gpointer /*user_data*/) {
    JSCValue* value = webkit_javascript_result_get_js_value(result);
    if (!jsc_value_is_string(value)) return;

    gchar* msg = jsc_value_to_string(value);
    std::string message = msg;
    g_free(msg);

    std::cout << "[SeaBrowser] JS Bridge Message: " << message << std::endl; // DEBUG LOG

    try {
        auto json_parser = json_parser_new();
        if (json_parser_load_from_data(json_parser, message.c_str(), -1, nullptr)) {
            auto root = json_parser_get_root(json_parser);
            auto object = json_node_get_object(root);

            if (json_object_has_member(object, "action")) {
                std::string action = json_object_get_string_member(object, "action");

                if (action == "saveSetting") {
                    std::string key = json_object_get_string_member(object, "key");
                    std::string val = json_object_get_string_member(object, "value");
                    SettingsManager::instance().set_value(key, val);
                } else if (action == "clearHistory") {
                    HistoryManager::instance().clear_history();
                } else if (action == "deleteHistoryItem") {
                    std::string url = json_object_get_string_member(object, "url");
                    HistoryManager::instance().delete_history_item(url);
                } else if (action == "toggleExtension") {
                    std::string name = json_object_get_string_member(object, "name");
                    bool enabled = json_object_get_boolean_member(object, "enabled");
                    ExtensionManager::instance().toggle_extension(name, enabled);
                } else if (action == "addBookmark") {
                    if (json_object_has_member(object, "bookmark")) {
                        auto bm_obj = json_object_get_object_member(object, "bookmark");
                        Bookmark bm;
                        bm.id = json_object_get_string_member(bm_obj, "id");
                        bm.title = json_object_get_string_member(bm_obj, "title");
                        bm.url = json_object_get_string_member(bm_obj, "url");
                        bm.folder = json_object_get_string_member(bm_obj, "folder");
                        bm.date_added = json_object_get_int_member(bm_obj, "dateAdded");
                        BookmarksManager::instance().add_bookmark(bm);
                    }
                } else if (action == "deleteBookmark") {
                    std::string id = json_object_get_string_member(object, "id");
                    BookmarksManager::instance().delete_bookmark(id);
                } else if (action == "updateBookmark") {
                    if (json_object_has_member(object, "bookmark")) {
                        auto bm_obj = json_object_get_object_member(object, "bookmark");
                        Bookmark bm;
                        bm.id = json_object_get_string_member(bm_obj, "id");
                        bm.title = json_object_get_string_member(bm_obj, "title");
                        bm.url = json_object_get_string_member(bm_obj, "url");
                        bm.folder = json_object_get_string_member(bm_obj, "folder");
                        bm.date_added = json_object_get_int_member(bm_obj, "dateAdded");
                        BookmarksManager::instance().update_bookmark(bm);
                    }
                } else if (action == "pauseDownload") {
                    std::string id = json_object_get_string_member(object, "id");
                    DownloadsManager::instance().pause_download(id);
                } else if (action == "cancelDownload") {
                    std::string id = json_object_get_string_member(object, "id");
                    DownloadsManager::instance().cancel_download(id);
                } else if (action == "retryDownload") {
                    std::string id = json_object_get_string_member(object, "id");
                    DownloadsManager::instance().retry_download(id);
                } else if (action == "removeDownload") {
                    std::string id = json_object_get_string_member(object, "id");
                    DownloadsManager::instance().remove_download(id);
                } else if (action == "clearCompletedDownloads") {
                    DownloadsManager::instance().clear_completed();
                } else if (action == "openFile") {
                    std::string path = json_object_get_string_member(object, "path");
                    DownloadsManager::instance().open_file(path);
                } else if (action == "showInFolder") {
                    std::string path = json_object_get_string_member(object, "path");
                    DownloadsManager::instance().show_in_folder(path);
                } else if (action == "openDownloadsFolder") {
                    DownloadsManager::instance().open_downloads_folder();
                }
            }
        }
        g_object_unref(json_parser);
    } catch (...) {
        std::cerr << "[SeaBrowser] Error parsing bridge message: " << message << std::endl;
    }
}

void WebView::setup_internal_page_bridge(WebKitWebView* web_view) {
    auto manager = webkit_web_view_get_user_content_manager(web_view);
    webkit_user_content_manager_register_script_message_handler(manager, "sea");
    g_signal_connect(manager, "script-message-received::sea", G_CALLBACK(on_script_message_received), nullptr);
}

std::string WebView::generate_internal_page(const std::string& /*page*/) {
    return "";
}

} // namespace SeaBrowser
