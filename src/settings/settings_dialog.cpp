#include "settings_dialog.h"
#include "settings/settings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QTabWidget>
#include <QSlider>
#include <QSpinBox>

namespace Tsunami {

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Settings - Tsunami");
    setMinimumSize(600, 500);
    resize(700, 550);
    setModal(true);
    
    applyTheme();
    setupUi();
    loadSettings();
    
    connect(&Settings::instance(), &Settings::settingsChanged, this, &SettingsDialog::onSettingsChanged);
}

void SettingsDialog::applyTheme() {
    QString accentColor = Settings::instance().getAccentColor();
    if (accentColor.isEmpty()) accentColor = "#3b82f6";
    
    bool isDark = Settings::instance().getDarkMode();
    
    QString bgColor = isDark ? "#0f172a" : "#ffffff";
    QString textColor = isDark ? "#e2e8f0" : "#1e293b";
    QString inputBg = isDark ? "#1e293b" : "#f1f5f9";
    QString borderColor = isDark ? "#334155" : "#e2e8f0";
    QString headerColor = isDark ? "#3b82f6" : "#2563eb";
    
    setStyleSheet(QString(R"(
        QDialog {
            background-color: %1;
            color: %2;
        }
        QLabel {
            color: %2;
        }
        QComboBox {
            background-color: %3;
            color: %2;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 8px 12px;
            min-width: 150px;
            selection-background-color: %5;
            selection-color: #ffffff;
        }
        QComboBox:hover {
            border-color: %5;
        }
        QLineEdit {
            background-color: %3;
            color: %2;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 8px 12px;
        }
        QLineEdit:focus {
            border-color: %5;
        }
        QCheckBox {
            color: %2;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 4px;
            border: 1px solid %4;
            background: %3;
        }
        QCheckBox::indicator:hover {
            border-color: %5;
        }
        QCheckBox::indicator:checked {
            background: %5;
            border-color: %5;
        }
        QSlider::groove:horizontal {
            background: %3;
            height: 6px;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: %5;
            width: 18px;
            margin: -6px 0;
            border-radius: 9px;
        }
        QSlider::sub-page:horizontal {
            background: %5;
            border-radius: 3px;
        }
        QSpinBox {
            background-color: %3;
            color: %2;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 4px 8px;
        }
        QTabWidget::pane {
            background-color: %1;
            border: 1px solid %4;
            border-radius: 8px;
        }
        QTabBar::tab {
            background-color: %3;
            color: %2;
            padding: 10px 20px;
            border-radius: 6px;
            margin-right: 4px;
        }
        QTabBar::tab:selected {
            background-color: %5;
            color: #ffffff;
        }
        QTabBar::tab:hover:!selected {
            border: 1px solid %5;
        }
    )").arg(bgColor, textColor, inputBg, borderColor, accentColor));
}

void SettingsDialog::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(24, 24, 24, 24);
    main_layout->setSpacing(16);
    
    // Title with accent color
    QString accentColor = Settings::instance().getAccentColor();
    QLabel* title = new QLabel("Settings");
    title->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1; margin-bottom: 8px;").arg(accentColor));
    main_layout->addWidget(title);
    
    QLabel* subtitle = new QLabel("Customize your browsing experience");
    subtitle->setStyleSheet("font-size: 13px; color: #64748b; margin-bottom: 16px;");
    main_layout->addWidget(subtitle);
    
    // Tab Widget
    QTabWidget* tab_widget = new QTabWidget();
    tab_widget->setDocumentMode(true);
    
    // Appearance Tab
    QWidget* appearance_tab = new QWidget();
    QVBoxLayout* appearance_layout = new QVBoxLayout(appearance_tab);
    appearance_layout->setContentsMargins(20, 20, 20, 20);
    appearance_layout->setSpacing(16);
    
    QLabel* appearance_header = new QLabel("Appearance");
    appearance_header->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(accentColor));
    appearance_layout->addWidget(appearance_header);
    
    // Theme
    QHBoxLayout* theme_layout = new QHBoxLayout();
    theme_layout->addWidget(new QLabel("Theme:"));
    theme_combo_ = new QComboBox();
    theme_combo_->addItem("Dark", "dark");
    theme_combo_->addItem("Light", "light");
    theme_combo_->addItem("System", "system");
    theme_layout->addWidget(theme_combo_);
    theme_layout->addStretch();
    appearance_layout->addLayout(theme_layout);
    
    // Dark Mode
    dark_mode_check_ = new QCheckBox("Dark Mode");
    appearance_layout->addWidget(dark_mode_check_);
    
    // Accent Color
    QHBoxLayout* accent_layout = new QHBoxLayout();
    accent_layout->addWidget(new QLabel("Accent Color:"));
    accent_combo_ = new QComboBox();
    accent_combo_->addItem("Blue", "#3b82f6");
    accent_combo_->addItem("Red", "#ef4444");
    accent_combo_->addItem("Green", "#22c55e");
    accent_combo_->addItem("Orange", "#f59e0b");
    accent_combo_->addItem("Purple", "#a855f7");
    accent_combo_->addItem("Pink", "#ec4899");
    accent_combo_->addItem("Cyan", "#06b6d4");
    accent_layout->addWidget(accent_combo_);
    accent_layout->addStretch();
    appearance_layout->addLayout(accent_layout);
    
    appearance_layout->addStretch();
    tab_widget->addTab(appearance_tab, "Appearance");
    
    // Privacy Tab
    QWidget* privacy_tab = new QWidget();
    QVBoxLayout* privacy_layout = new QVBoxLayout(privacy_tab);
    privacy_layout->setContentsMargins(20, 20, 20, 20);
    privacy_layout->setSpacing(16);
    
    QLabel* privacy_header = new QLabel("Privacy & Security");
    privacy_header->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(accentColor));
    privacy_layout->addWidget(privacy_header);
    
    block_trackers_ = new QCheckBox("Block Trackers");
    privacy_layout->addWidget(block_trackers_);
    
    block_ads_ = new QCheckBox("Block Ads");
    privacy_layout->addWidget(block_ads_);
    
    https_only_ = new QCheckBox("HTTPS-Only Mode");
    privacy_layout->addWidget(https_only_);
    
    do_not_track_ = new QCheckBox("Send Do Not Track Header");
    privacy_layout->addWidget(do_not_track_);
    
    block_third_party_cookies_ = new QCheckBox("Block Third-Party Cookies");
    privacy_layout->addWidget(block_third_party_cookies_);
    
    block_fingerprinting_ = new QCheckBox("Block Fingerprinting");
    privacy_layout->addWidget(block_fingerprinting_);
    
    disable_webrtc_ = new QCheckBox("Disable WebRTC (prevents IP leaks)");
    privacy_layout->addWidget(disable_webrtc_);
    
    privacy_layout->addStretch();
    tab_widget->addTab(privacy_tab, "Privacy");
    
    // Search Tab
    QWidget* search_tab = new QWidget();
    QVBoxLayout* search_layout = new QVBoxLayout(search_tab);
    search_layout->setContentsMargins(20, 20, 20, 20);
    search_layout->setSpacing(16);
    
    QLabel* search_header = new QLabel("Search Engine");
    search_header->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(accentColor));
    search_layout->addWidget(search_header);
    
    QHBoxLayout* search_engine_layout = new QHBoxLayout();
    search_engine_layout->addWidget(new QLabel("Default Search Engine:"));
    search_engine_ = new QComboBox();
    search_engine_->addItem("DuckDuckGo", "duckduckgo");
    search_engine_->addItem("Brave Search", "brave");
    search_engine_->addItem("Google", "google");
    search_engine_->addItem("Bing", "bing");
    search_engine_->addItem("Startpage", "startpage");
    search_engine_->addItem("Qwant", "qwant");
    search_engine_layout->addWidget(search_engine_);
    search_engine_layout->addStretch();
    search_layout->addLayout(search_engine_layout);
    
    search_layout->addStretch();
    tab_widget->addTab(search_tab, "Search");
    
    // Startup Tab
    QWidget* startup_tab = new QWidget();
    QVBoxLayout* startup_layout = new QVBoxLayout(startup_tab);
    startup_layout->setContentsMargins(20, 20, 20, 20);
    startup_layout->setSpacing(16);
    
    QLabel* startup_header = new QLabel("Startup");
    startup_header->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(accentColor));
    startup_layout->addWidget(startup_header);
    
    QHBoxLayout* homepage_layout = new QHBoxLayout();
    homepage_layout->addWidget(new QLabel("Homepage:"));
    homepage_edit_ = new QLineEdit();
    homepage_edit_->setPlaceholderText("https://... or tsunami://newtab");
    homepage_layout->addWidget(homepage_edit_);
    startup_layout->addLayout(homepage_layout);
    
    restore_tabs_ = new QCheckBox("Restore tabs on startup");
    startup_layout->addWidget(restore_tabs_);
    
    auto_reload_ = new QCheckBox("Auto-reload pages");
    startup_layout->addWidget(auto_reload_);
    
    QHBoxLayout* reload_layout = new QHBoxLayout();
    reload_layout->addWidget(new QLabel("Auto-reload interval:"));
    auto_reload_interval_ = new QSpinBox();
    auto_reload_interval_->setRange(5, 3600);
    auto_reload_interval_->setSuffix(" seconds");
    reload_layout->addWidget(auto_reload_interval_);
    reload_layout->addStretch();
    startup_layout->addLayout(reload_layout);
    
    startup_layout->addStretch();
    tab_widget->addTab(startup_tab, "Startup");
    
    // Advanced Tab
    QWidget* advanced_tab = new QWidget();
    QVBoxLayout* advanced_layout = new QVBoxLayout(advanced_tab);
    advanced_layout->setContentsMargins(20, 20, 20, 20);
    advanced_layout->setSpacing(16);
    
    QLabel* advanced_header = new QLabel("Advanced");
    advanced_header->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(accentColor));
    advanced_layout->addWidget(advanced_header);
    
    QHBoxLayout* zoom_layout = new QHBoxLayout();
    zoom_layout->addWidget(new QLabel("Default Zoom:"));
    zoom_level_ = new QSlider(Qt::Horizontal);
    zoom_level_->setRange(25, 200);
    zoom_level_->setValue(100);
    zoom_layout->addWidget(zoom_level_);
    zoom_label_ = new QLabel("100%");
    zoom_layout->addWidget(zoom_label_);
    connect(zoom_level_, &QSlider::valueChanged, this, [this](int value) {
        zoom_label_->setText(QString::number(value) + "%");
    });
    advanced_layout->addLayout(zoom_layout);
    
    show_bookmarks_bar_ = new QCheckBox("Show bookmarks bar by default");
    advanced_layout->addWidget(show_bookmarks_bar_);
    
    auto_clear_cache_ = new QCheckBox("Auto-clear cache on exit");
    advanced_layout->addWidget(auto_clear_cache_);
    
    advanced_layout->addStretch();
    tab_widget->addTab(advanced_tab, "Advanced");
    
    main_layout->addWidget(tab_widget);
    
    // Buttons
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();
    
    QPushButton* reset_btn = new QPushButton("Reset to Defaults");
    reset_btn->setStyleSheet(QString("QPushButton { background-color: %1; color: white; border: none; border-radius: 6px; padding: 10px 20px; } QPushButton:hover { opacity: 0.9; }").arg(Settings::instance().getAccentColor()));
    connect(reset_btn, &QPushButton::clicked, this, &SettingsDialog::resetSettings);
    button_layout->addWidget(reset_btn);
    
    QPushButton* cancel_btn = new QPushButton("Cancel");
    cancel_btn->setStyleSheet("QPushButton { background-color: #475569; color: white; border: none; border-radius: 6px; padding: 10px 20px; } QPushButton:hover { background-color: #64748b; }");
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
    button_layout->addWidget(cancel_btn);
    
    QPushButton* save_btn = new QPushButton("Save Changes");
    save_btn->setStyleSheet(QString("QPushButton { background-color: %1; color: white; border: none; border-radius: 6px; padding: 10px 20px; font-weight: 600; } QPushButton:hover { opacity: 0.9; }").arg(Settings::instance().getAccentColor()));
    connect(save_btn, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    button_layout->addWidget(save_btn);
    
    main_layout->addLayout(button_layout);
}

void SettingsDialog::loadSettings() {
    auto& settings = Settings::instance();
    
    // Theme
    QString theme = settings.getTheme();
    int theme_index = theme_combo_->findData(theme);
    if (theme_index >= 0) theme_combo_->setCurrentIndex(theme_index);
    
    // Dark mode
    dark_mode_check_->setChecked(settings.getDarkMode());
    
    // Accent color
    QString accent = settings.getAccentColor();
    int accent_index = accent_combo_->findData(accent);
    if (accent_index >= 0) accent_combo_->setCurrentIndex(accent_index);
    
    // Privacy
    block_trackers_->setChecked(settings.getBlockTrackers());
    block_ads_->setChecked(settings.getBlockAds());
    https_only_->setChecked(settings.getHttpsOnly());
    do_not_track_->setChecked(settings.getDoNotTrack());
    block_third_party_cookies_->setChecked(settings.getBlockThirdPartyCookies());
    block_fingerprinting_->setChecked(settings.getBlockFingerprinting());
    disable_webrtc_->setChecked(settings.getDisableWebRTC());
    
    // Search
    QString engine = settings.getSearchEngine();
    int engine_index = search_engine_->findData(engine);
    if (engine_index >= 0) search_engine_->setCurrentIndex(engine_index);
    
    // Startup
    homepage_edit_->setText(settings.getHomepage());
    restore_tabs_->setChecked(settings.getRestoreTabs());
    auto_reload_->setChecked(settings.getAutoReload());
    auto_reload_interval_->setValue(settings.getAutoReloadInterval());
    
    // Advanced
    zoom_level_->setValue(settings.getZoomLevel());
    zoom_label_->setText(QString::number(settings.getZoomLevel()) + "%");
    show_bookmarks_bar_->setChecked(settings.getShowBookmarksBar());
    auto_clear_cache_->setChecked(settings.getAutoClearCache());
}

void SettingsDialog::saveSettings() {
    auto& settings = Settings::instance();
    
    // Theme
    settings.setTheme(theme_combo_->currentData().toString());
    settings.setDarkMode(dark_mode_check_->isChecked());
    settings.setAccentColor(accent_combo_->currentData().toString());
    
    // Privacy
    settings.setBlockTrackers(block_trackers_->isChecked());
    settings.setBlockAds(block_ads_->isChecked());
    settings.setHttpsOnly(https_only_->isChecked());
    settings.setDoNotTrack(do_not_track_->isChecked());
    settings.setBlockThirdPartyCookies(block_third_party_cookies_->isChecked());
    settings.setBlockFingerprinting(block_fingerprinting_->isChecked());
    settings.setDisableWebRTC(disable_webrtc_->isChecked());
    
    // Search
    settings.setSearchEngine(search_engine_->currentData().toString());
    
    // Startup
    settings.setHomepage(homepage_edit_->text());
    settings.setRestoreTabs(restore_tabs_->isChecked());
    settings.setAutoReload(auto_reload_->isChecked());
    settings.setAutoReloadInterval(auto_reload_interval_->value());
    
    // Advanced
    settings.setZoomLevel(zoom_level_->value());
    settings.setShowBookmarksBar(show_bookmarks_bar_->isChecked());
    settings.setAutoClearCache(auto_clear_cache_->isChecked());
    
    QMessageBox::information(this, "Settings Saved", "Your settings have been saved and applied.");
    accept();
}

void SettingsDialog::resetSettings() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Reset Settings", 
        "Are you sure you want to reset all settings to defaults?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        Settings::instance().reset();
        applyTheme();
        loadSettings();
        QMessageBox::information(this, "Settings Reset", "All settings have been reset to defaults.");
    }
}

void SettingsDialog::onSettingsChanged() {
    applyTheme();
}

void SettingsDialog::showDialog(QWidget* parent) {
    SettingsDialog dialog(parent);
    dialog.exec();
}

} // namespace Tsunami