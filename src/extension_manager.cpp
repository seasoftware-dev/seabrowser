#include "extension_manager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <glib.h>
#include <json-glib/json-glib.h>
#include "utils.h"

namespace SeaBrowser {

ExtensionManager& ExtensionManager::instance() {
    static ExtensionManager inst;
    return inst;
}

void ExtensionManager::init() {
    load_from_dir();
}

void ExtensionManager::load_from_dir() {
    extensions_.clear();
    std::string config_dir = std::string(g_get_user_config_dir()) + "/seabrowser/extensions";

    if (!std::filesystem::exists(config_dir)) {
        std::filesystem::create_directories(config_dir);
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(config_dir)) {
        if (entry.is_directory()) {
            // Check for Chrome extension (manifest.json)
            auto manifest_path = entry.path() / "manifest.json";
            if (std::filesystem::exists(manifest_path)) {
                if (load_chrome_extension(entry.path())) continue;
            }

            // Check for Firefox extension (manifest.json with different structure)
            auto firefox_manifest = entry.path() / "manifest.json";
            if (std::filesystem::exists(firefox_manifest)) {
                if (load_firefox_extension(entry.path())) continue;
            }
        } else if (entry.path().extension() == ".js") {
            // Plain JavaScript extension
            std::ifstream file(entry.path());
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                extensions_.push_back({
                    entry.path().filename().string(),
                    "Plain JavaScript extension",
                    "1.0.0",
                    "User",
                    ExtensionType::JavaScript,
                    content,
                    "",
                    true
                });
                std::cout << "[SeaBrowser] Loaded JS extension: " << entry.path().filename() << std::endl;
            }
        }
    }
}

bool ExtensionManager::load_chrome_extension(const std::string& path) {
    auto manifest_path = std::filesystem::path(path) / "manifest.json";
    std::ifstream manifest_file(manifest_path);
    if (!manifest_file.is_open()) return false;

    std::stringstream buffer;
    buffer << manifest_file.rdbuf();
    std::string manifest_content = buffer.str();

    // Parse manifest JSON
    JsonParser* parser = json_parser_new();
    if (!json_parser_load_from_data(parser, manifest_content.c_str(), -1, nullptr)) {
        g_object_unref(parser);
        return false;
    }

    JsonNode* root = json_parser_get_root(parser);
    JsonObject* root_obj = json_node_get_object(root);

    // Get basic info
    const char* name = json_object_get_string_member(root_obj, "name");
    const char* version = json_object_get_string_member(root_obj, "version");
    const char* desc = json_object_get_string_member(root_obj, "description");

    if (!name) name = "Unknown Extension";
    if (!version) version = "1.0.0";
    if (!desc) desc = "No description";

    // Get content scripts
    JsonArray* content_scripts = nullptr;
    if (json_object_has_member(root_obj, "content_scripts")) {
        content_scripts = json_object_get_array_member(root_obj, "content_scripts");
    }

    std::string combined_script = "";
    if (content_scripts) {
        for (guint i = 0; i < json_array_get_length(content_scripts); i++) {
            JsonNode* script_node = json_array_get_element(content_scripts, i);
            JsonObject* script_obj = json_node_get_object(script_node);

            if (json_object_has_member(script_obj, "js")) {
                JsonArray* js_files = json_object_get_array_member(script_obj, "js");
                for (guint j = 0; j < json_array_get_length(js_files); j++) {
                    const char* js_file = json_array_get_string_element(js_files, j);
                    auto js_path = std::filesystem::path(path) / js_file;

                    if (std::filesystem::exists(js_path)) {
                        std::ifstream js_file(js_path);
                        std::string js_content((std::istreambuf_iterator<char>(js_file)),
                                             std::istreambuf_iterator<char>());
                        combined_script += "\n// ===== " + js_path.filename().string() + " =====\n";
                        combined_script += js_content + "\n";
                    }
                }
            }
        }
    }

    // If no content scripts, look for background script
    if (combined_script.empty() && json_object_has_member(root_obj, "background")) {
        JsonNode* bg_node = json_object_get_member(root_obj, "background");
        if (JSON_NODE_HOLDS_OBJECT(bg_node)) {
            JsonObject* bg_obj = json_node_get_object(bg_node);
            if (json_object_has_member(bg_obj, "scripts")) {
                JsonArray* bg_scripts = json_object_get_array_member(bg_obj, "scripts");
                for (guint i = 0; i < json_array_get_length(bg_scripts); i++) {
                    const char* script_file = json_array_get_string_element(bg_scripts, i);
                    auto script_path = std::filesystem::path(path) / script_file;

                    if (std::filesystem::exists(script_path)) {
                        std::ifstream script_file(script_path);
                        std::string script_content((std::istreambuf_iterator<char>(script_file)),
                                                std::istreambuf_iterator<char>());
                        combined_script += "\n// ===== Background Script =====\n";
                        combined_script += script_content + "\n";
                    }
                }
            }
        }
    }

    if (!combined_script.empty()) {
        extensions_.push_back({
            name,
            desc,
            version,
            "",
            ExtensionType::Chrome,
            combined_script,
            manifest_content,
            true
        });
        std::cout << "[SeaBrowser] Loaded Chrome extension: " << name << " v" << version << std::endl;
    }

    g_object_unref(parser);
    return !combined_script.empty();
}

bool ExtensionManager::load_firefox_extension(const std::string& path) {
    // Firefox extensions have similar manifest structure to Chrome
    // For now, we use the same loading logic
    return load_chrome_extension(path);
}

void ExtensionManager::inject_extensions(WebKitWebView* web_view) {
    auto manager = webkit_web_view_get_user_content_manager(web_view);

    for (const auto& ext : extensions_) {
        if (!ext.enabled) continue;

        auto script = webkit_user_script_new(
            ext.script.c_str(),
            WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
            WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
            nullptr, nullptr
        );
        webkit_user_content_manager_add_script(manager, script);
        webkit_user_script_unref(script);
    }
}

void ExtensionManager::reload_extensions() {
    load_from_dir();
}

void ExtensionManager::toggle_extension(const std::string& name, bool enabled) {
    for (auto& ext : extensions_) {
        if (ext.name == name) {
            ext.enabled = enabled;
            std::cout << "[SeaBrowser] Extension toggled: " << name << " -> " << (enabled ? "ON" : "OFF") << std::endl;
            break;
        }
    }
}

bool ExtensionManager::install_extension(const std::string& path) {
    // TODO: Implement extension installation from file
    std::cout << "[SeaBrowser] Extension installation not yet implemented: " << path << std::endl;
    return false;
}

void ExtensionManager::uninstall_extension(const std::string& name) {
    // TODO: Implement extension uninstallation
    std::cout << "[SeaBrowser] Extension uninstallation not yet implemented: " << name << std::endl;
}

} // namespace SeaBrowser
