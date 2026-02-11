#include "settings.h"
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QDebug>
#include <QApplication>

namespace Tsunami {

Settings& Settings::instance() {
    static Settings instance;
    return instance;
}

Settings::Settings() : QObject() {
    load();
}

QString Settings::getConfigPath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    return configDir + "/tsunami_settings.json";
}

void Settings::load() {
    QString path = getConfigPath();
    QFile file(path);
    
    if (!file.exists()) {
        qDebug() << "Settings file does not exist, using defaults";
        save();
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot read settings file:" << path;
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        qWarning() << "Invalid settings format";
        return;
    }
    
    QJsonObject obj = doc.object();
    
    theme_ = obj["theme"].toString("dark");
    dark_mode_ = obj["dark_mode"].toBool(true);
    accent_color_ = obj["accent_color"].toString("#3b82f6");
    search_engine_ = obj["search_engine"].toString("duckduckgo");
    homepage_ = obj["homepage"].toString("");
    restore_tabs_ = obj["restore_tabs"].toBool(true);
    block_trackers_ = obj["block_trackers"].toBool(true);
    block_ads_ = obj["block_ads"].toBool(true);
    https_only_ = obj["https_only"].toBool(false);
    first_run_ = obj["first_run"].toBool(true);
    do_not_track_ = obj["do_not_track"].toBool(true);
    block_third_party_cookies_ = obj["block_third_party_cookies"].toBool(true);
    block_fingerprinting_ = obj["block_fingerprinting"].toBool(true);
    disable_webrtc_ = obj["disable_webrtc"].toBool(false);
    auto_clear_cache_ = obj["auto_clear_cache"].toBool(false);
    zoom_level_ = obj["zoom_level"].toInt(100);
    show_bookmarks_bar_ = obj["show_bookmarks_bar"].toBool(false);
    auto_reload_ = obj["auto_reload"].toBool(false);
    auto_reload_interval_ = obj["auto_reload_interval"].toInt(30);
    
    qDebug() << "Settings loaded from:" << path;
}

void Settings::save() {
    QJsonObject obj;
    obj["theme"] = theme_;
    obj["dark_mode"] = dark_mode_;
    obj["accent_color"] = accent_color_;
    obj["search_engine"] = search_engine_;
    obj["homepage"] = homepage_;
    obj["restore_tabs"] = restore_tabs_;
    obj["block_trackers"] = block_trackers_;
    obj["block_ads"] = block_ads_;
    obj["https_only"] = https_only_;
    obj["first_run"] = first_run_;
    obj["do_not_track"] = do_not_track_;
    obj["block_third_party_cookies"] = block_third_party_cookies_;
    obj["block_fingerprinting"] = block_fingerprinting_;
    obj["disable_webrtc"] = disable_webrtc_;
    obj["auto_clear_cache"] = auto_clear_cache_;
    obj["zoom_level"] = zoom_level_;
    obj["show_bookmarks_bar"] = show_bookmarks_bar_;
    obj["auto_reload"] = auto_reload_;
    obj["auto_reload_interval"] = auto_reload_interval_;
    
    QJsonDocument doc(obj);
    
    QString path = getConfigPath();
    QFile file(path);
    
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "Settings saved to:" << path;
    } else {
        qWarning() << "Cannot write settings file:" << path;
    }
}

void Settings::reset() {
    theme_ = "dark";
    dark_mode_ = true;
    accent_color_ = "#3b82f6";
    search_engine_ = "duckduckgo";
    homepage_ = "";
    restore_tabs_ = true;
    block_trackers_ = true;
    block_ads_ = true;
    https_only_ = false;
    first_run_ = true;
    do_not_track_ = true;
    block_third_party_cookies_ = true;
    block_fingerprinting_ = true;
    disable_webrtc_ = false;
    auto_clear_cache_ = false;
    zoom_level_ = 100;
    show_bookmarks_bar_ = false;
    auto_reload_ = false;
    auto_reload_interval_ = 30;
    save();
    emit settingsChanged();
}

} // namespace Tsunami
