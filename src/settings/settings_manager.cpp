/*
 * Sea Browser - Settings Manager
 * settings_manager.cpp - Simple key=value file format (no json-glib dependency)
 */

#include "settings_manager.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <glib.h>

namespace SeaBrowser {

SettingsManager& SettingsManager::instance() {
    static SettingsManager instance;
    return instance;
}

// Encryption helpers
static const std::string ENCRYPTION_KEY = "SeaBrowser_Secret_Key_2024_Wyind_Ryan";

static std::string xor_encrypt(const std::string& input) {
    std::string output = input;
    for (size_t i = 0; i < input.size(); i++) {
        output[i] ^= ENCRYPTION_KEY[i % ENCRYPTION_KEY.size()];
    }
    return output;
}

void SettingsManager::load(const std::string& config_path) {
    config_path_ = config_path;
    auto settings_file = std::filesystem::path(config_path) / "settings.conf";
    
    // Set defaults
    general_.download_path = g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD);
    if (general_.download_path.empty()) {
        general_.download_path = std::string(g_get_home_dir()) + "/Downloads";
    }
    
    if (!std::filesystem::exists(settings_file)) {
        save();
        return;
    }
    
    std::ifstream file(settings_file);
    if (!file.is_open()) return;
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    // Check for encryption
    if (content.rfind("# [ENCRYPTED]\n", 0) == 0) {
        // Read encrypted content
        size_t newline_pos = content.find('\n');
        if (newline_pos != std::string::npos) {
            std::string encrypted_data = content.substr(newline_pos + 1);
            
            // Base64 decode
            gsize out_len = 0;
            guchar* decoded = g_base64_decode(encrypted_data.c_str(), &out_len);
            std::string xor_data(reinterpret_cast<char*>(decoded), out_len);
            g_free(decoded);
            
            // Decrypt
            content = xor_encrypt(xor_data);
            general_.encrypt_settings = true;
        }
    }
    
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        
        auto eq_pos = line.find('=');
        if (eq_pos == std::string::npos) continue;
        
        std::string key = line.substr(0, eq_pos);
        std::string value = line.substr(eq_pos + 1);
        
        // Trim whitespace
        while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) key.pop_back();
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) value.erase(0, 1);
        
        // Parse settings
        if (key == "cookie_policy") {
            if (value == "accept_all") privacy_.cookie_policy = CookiePolicy::AcceptAll;
            else if (value == "block_all") privacy_.cookie_policy = CookiePolicy::BlockAll;
            else privacy_.cookie_policy = CookiePolicy::BlockThirdParty;
        }
        else if (key == "tracking_protection") {
            if (value == "off") privacy_.tracking_protection = TrackingProtection::Off;
            else if (value == "standard") privacy_.tracking_protection = TrackingProtection::Standard;
            else privacy_.tracking_protection = TrackingProtection::Strict;
        }
        else if (key == "https_only") privacy_.https_only = (value == "true" || value == "1");
        else if (key == "send_dnt") privacy_.send_dnt = (value == "true" || value == "1");
        else if (key == "clear_on_exit") privacy_.clear_on_exit = (value == "true" || value == "1");
        else if (key == "default_engine") search_.default_engine = value;
        else if (key == "show_suggestions") search_.show_suggestions = (value == "true" || value == "1");
        else if (key == "theme") appearance_.theme = value;
        else if (key == "zoom_level") appearance_.zoom_level = std::stoi(value);
        else if (key == "show_sidebar") appearance_.show_sidebar = (value == "true" || value == "1");
        else if (key == "homepage") general_.homepage = value;
        else if (key == "download_path") general_.download_path = value;
        else if (key == "ask_where_to_save") general_.ask_where_to_save = (value == "true" || value == "1");
        else if (key == "restore_tabs") general_.restore_tabs = (value == "true" || value == "1");
        else if (key == "encrypt_settings") general_.encrypt_settings = (value == "true" || value == "1");
    }
}

void SettingsManager::save() {
    auto settings_file = std::filesystem::path(config_path_) / "settings.conf";
    std::filesystem::create_directories(config_path_);
    
    std::stringstream ss;
    ss << "# Sea Browser Settings\n\n";
    
    // Privacy
    ss << "# Privacy\n";
    ss << "cookie_policy=";
    switch (privacy_.cookie_policy) {
        case CookiePolicy::AcceptAll: ss << "accept_all"; break;
        case CookiePolicy::BlockAll: ss << "block_all"; break;
        default: ss << "block_third_party"; break;
    }
    ss << "\n";
    
    ss << "tracking_protection=";
    switch (privacy_.tracking_protection) {
        case TrackingProtection::Off: ss << "off"; break;
        case TrackingProtection::Standard: ss << "standard"; break;
        default: ss << "strict"; break;
    }
    ss << "\n";
    
    ss << "https_only=" << (privacy_.https_only ? "true" : "false") << "\n";
    ss << "send_dnt=" << (privacy_.send_dnt ? "true" : "false") << "\n";
    ss << "clear_on_exit=" << (privacy_.clear_on_exit ? "true" : "false") << "\n";
    
    // Search
    ss << "\n# Search\n";
    ss << "default_engine=" << search_.default_engine << "\n";
    ss << "show_suggestions=" << (search_.show_suggestions ? "true" : "false") << "\n";
    
    // Appearance
    ss << "\n# Appearance\n";
    ss << "theme=" << appearance_.theme << "\n";
    ss << "zoom_level=" << appearance_.zoom_level << "\n";
    ss << "show_sidebar=" << (appearance_.show_sidebar ? "true" : "false") << "\n";
    
    // General
    ss << "\n# General\n";
    ss << "homepage=" << general_.homepage << "\n";
    ss << "download_path=" << general_.download_path << "\n";
    ss << "ask_where_to_save=" << (general_.ask_where_to_save ? "true" : "false") << "\n";
    ss << "restore_tabs=" << (general_.restore_tabs ? "true" : "false") << "\n";
    ss << "encrypt_settings=" << (general_.encrypt_settings ? "true" : "false") << "\n";

    std::ofstream file(settings_file);
    if (!file.is_open()) {
        g_warning("Failed to save settings to %s", settings_file.c_str());
        return;
    }

    if (general_.encrypt_settings) {
        std::string raw_content = ss.str();
        std::string encrypted_content = xor_encrypt(raw_content);
        gchar* base64_encoded = g_base64_encode((const guchar*)encrypted_content.c_str(), encrypted_content.length());
        
        file << "# [ENCRYPTED]\n";
        file << base64_encoded;
        g_free(base64_encoded);
    } else {
        file << ss.str();
    }
}

void SettingsManager::set_cookie_policy(CookiePolicy policy) {
    privacy_.cookie_policy = policy;
    save();
}

void SettingsManager::set_tracking_protection(TrackingProtection level) {
    privacy_.tracking_protection = level;
    save();
}

void SettingsManager::set_https_only(bool enabled) {
    privacy_.https_only = enabled;
    save();
}

void SettingsManager::set_default_search_engine(const std::string& name) {
    search_.default_engine = name;
    save();
}

void SettingsManager::set_theme(const std::string& theme) {
    appearance_.theme = theme;
    save();
}

void SettingsManager::set_homepage(const std::string& url) {
    general_.homepage = url;
    save();
}

void SettingsManager::set_encrypt_settings(bool enabled) {
    general_.encrypt_settings = enabled;
    save();
}

void SettingsManager::set_restore_tabs(bool enabled) {
    general_.restore_tabs = enabled;
    save();
}

void SettingsManager::set_send_dnt(bool enabled) {
    privacy_.send_dnt = enabled;
    save();
}

void SettingsManager::set_clear_on_exit(bool enabled) {
    privacy_.clear_on_exit = enabled;
    save();
}

void SettingsManager::set_zoom_level(int level) {
    appearance_.zoom_level = level;
    save();
}

void SettingsManager::set_value(const std::string& key, const std::string& value) {
    // Generic setter for JavaScript bridge - maps string keys to proper setters
    if (key == "theme") set_theme(value);
    else if (key == "zoom" || key == "zoom-level") set_zoom_level(std::stoi(value));
    else if (key == "homepage") set_homepage(value);
    else if (key == "restore-tabs" || key == "restore_tabs") set_restore_tabs(value == "true" || value == "1");
    else if (key == "search-engine" || key == "default_engine") set_default_search_engine(value);
    else if (key == "https-only") set_https_only(value == "true" || value == "1");
    else if (key == "send-dnt") set_send_dnt(value == "true" || value == "1");
    else if (key == "clear-on-exit") set_clear_on_exit(value == "true" || value == "1");
    else if (key == "encrypt-settings" || key == "encrypt_settings") set_encrypt_settings(value == "true" || value == "1");
    else if (key == "tracking-protection") {
        if (value == "off") set_tracking_protection(TrackingProtection::Off);
        else if (value == "standard") set_tracking_protection(TrackingProtection::Standard);
        else set_tracking_protection(TrackingProtection::Strict);
    }
    else if (key == "cookie-policy") {
        if (value == "accept-all") set_cookie_policy(CookiePolicy::AcceptAll);
        else if (value == "block-all") set_cookie_policy(CookiePolicy::BlockAll);
        else set_cookie_policy(CookiePolicy::BlockThirdParty);
    }
    else {
        g_warning("Unknown setting key: %s", key.c_str());
    }
}

} // namespace SeaBrowser
