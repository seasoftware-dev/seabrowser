/*
 * Sea Browser - Settings Manager
 * settings_manager.h
 */

#pragma once

#include <string>

namespace SeaBrowser {

enum class CookiePolicy { AcceptAll, BlockThirdParty, BlockAll };
enum class TrackingProtection { Off, Standard, Strict };

struct PrivacySettings {
    CookiePolicy cookie_policy = CookiePolicy::BlockThirdParty;
    TrackingProtection tracking_protection = TrackingProtection::Strict;
    bool https_only = true;
    bool send_dnt = true;
    bool clear_on_exit = false;
};

struct SearchSettings {
    std::string default_engine = "google";
    bool show_suggestions = true;
};

struct AppearanceSettings {
    std::string theme = "system";
    std::string custom_color = "#2563eb";
    int zoom_level = 100;
    bool show_sidebar = false;
    bool vertical_tabs = false;
};

struct GeneralSettings {
    std::string homepage = "sea://newtab";
    std::string download_path;
    std::string user_agent = "chrome";
    bool ask_where_to_save = false;
    bool restore_tabs = true;
    bool setup_completed = false; // New flag for setup wizard
    bool encrypt_settings = false; // Encrypt settings file on disk
};

class SettingsManager {
public:
    static SettingsManager& instance();
    
    void load(const std::string& config_path);
    void save();
    
    // Getters
    PrivacySettings& privacy() { return privacy_; }
    SearchSettings& search() { return search_; }
    AppearanceSettings& appearance() { return appearance_; }
    GeneralSettings& general() { return general_; }
    
    // Setters with auto-save
    void set_cookie_policy(CookiePolicy policy);
    void set_tracking_protection(TrackingProtection level);
    void set_https_only(bool enabled);
    void set_send_dnt(bool enabled);
    void set_clear_on_exit(bool enabled);
    void set_default_search_engine(const std::string& name);
    void set_theme(const std::string& theme);
    void set_custom_color(const std::string& color);
    void set_zoom_level(int level);
    void set_homepage(const std::string& url);
    void set_restore_tabs(bool enabled);
    void set_encrypt_settings(bool enabled);
    void set_vertical_tabs(bool enabled);
    
    // Generic setter for JavaScript bridge
    void set_value(const std::string& key, const std::string& value);
    
private:
    SettingsManager() = default;
    
    std::string config_path_;
    PrivacySettings privacy_;
    SearchSettings search_;
    AppearanceSettings appearance_;
    GeneralSettings general_;
};

} // namespace SeaBrowser
