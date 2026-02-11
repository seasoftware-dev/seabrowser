#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>

namespace Tsunami {

class Settings : public QObject {
    Q_OBJECT
public:
    static Settings& instance();
    
    // Load/Save
    void load();
    void save();
    void reset();
    
    // Getters
    QString getTheme() const { return theme_; }
    bool getDarkMode() const { return dark_mode_; }
    QString getAccentColor() const { return accent_color_; }
    QString getSearchEngine() const { return search_engine_; }
    QString getHomepage() const { return homepage_; }
    bool getRestoreTabs() const { return restore_tabs_; }
    bool getBlockTrackers() const { return block_trackers_; }
    bool getBlockAds() const { return block_ads_; }
    bool getHttpsOnly() const { return https_only_; }
    bool isFirstRun() const { return first_run_; }
    bool getDoNotTrack() const { return do_not_track_; }
    bool getBlockThirdPartyCookies() const { return block_third_party_cookies_; }
    bool getBlockFingerprinting() const { return block_fingerprinting_; }
    bool getDisableWebRTC() const { return disable_webrtc_; }
    bool getAutoClearCache() const { return auto_clear_cache_; }
    int getZoomLevel() const { return zoom_level_; }
    bool getShowBookmarksBar() const { return show_bookmarks_bar_; }
    bool getAutoReload() const { return auto_reload_; }
    int getAutoReloadInterval() const { return auto_reload_interval_; }

    // Setters
    void setTheme(const QString& theme) { theme_ = theme; save(); emit settingsChanged(); }
    void setDarkMode(bool dark) { dark_mode_ = dark; save(); emit settingsChanged(); }
    void setAccentColor(const QString& color) { accent_color_ = color; save(); emit settingsChanged(); }
    void setSearchEngine(const QString& engine) { search_engine_ = engine; save(); }
    void setHomepage(const QString& homepage) { homepage_ = homepage; save(); }
    void setRestoreTabs(bool restore) { restore_tabs_ = restore; save(); }
    void setBlockTrackers(bool block) { block_trackers_ = block; save(); emit settingsChanged(); }
    void setBlockAds(bool block) { block_ads_ = block; save(); emit settingsChanged(); }
    void setHttpsOnly(bool https) { https_only_ = https; save(); emit settingsChanged(); }
    void setFirstRun(bool first) { first_run_ = first; save(); }
    void setDoNotTrack(bool dnt) { do_not_track_ = dnt; save(); emit settingsChanged(); }
    void setBlockThirdPartyCookies(bool block) { block_third_party_cookies_ = block; save(); emit settingsChanged(); }
    void setBlockFingerprinting(bool block) { block_fingerprinting_ = block; save(); emit settingsChanged(); }
    void setDisableWebRTC(bool disable) { disable_webrtc_ = disable; save(); emit settingsChanged(); }
    void setAutoClearCache(bool clear) { auto_clear_cache_ = clear; save(); }
    void setZoomLevel(int zoom) { zoom_level_ = zoom; save(); }
    void setShowBookmarksBar(bool show) { show_bookmarks_bar_ = show; save(); }
    void setAutoReload(bool reload) { auto_reload_ = reload; save(); }
    void setAutoReloadInterval(int interval) { auto_reload_interval_ = interval; save(); }

signals:
    void settingsChanged();

public:
    QString getConfigPath() const;

private:
    Settings();
    ~Settings() = default;
    
    // Settings values
    QString theme_ = "dark";
    bool dark_mode_ = true;
    QString accent_color_ = "#3b82f6";
    QString search_engine_ = "duckduckgo";
    QString homepage_ = "";
    bool restore_tabs_ = true;
    bool block_trackers_ = true;
    bool block_ads_ = true;
    bool https_only_ = false;
    bool first_run_ = true;
    bool do_not_track_ = true;
    bool block_third_party_cookies_ = true;
    bool block_fingerprinting_ = true;
    bool disable_webrtc_ = false;
    bool auto_clear_cache_ = false;
    int zoom_level_ = 100;
    bool show_bookmarks_bar_ = false;
    bool auto_reload_ = false;
    int auto_reload_interval_ = 30;
};

} // namespace Tsunami
