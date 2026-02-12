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
#include <QButtonGroup>
#include <QRadioButton>
#include <QScrollArea>
#include <QFrame>

namespace Tsunami {

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Settings - Tsunami");
    setMinimumSize(900, 600);
    resize(900, 600);
    setModal(true);
    
    bool isDark = Settings::instance().getDarkMode();
    
    QString bgColor = isDark ? "#0f172a" : "#ffffff";
    QString textColor = isDark ? "#e2e8f0" : "#1e293b";
    QString inputBg = isDark ? "#1e293b" : "#ffffff";
    QString borderColor = isDark ? "#334155" : "#cbd5e1";
    QString accentColor = "#3b82f6";
    
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    
    // Scroll area for content
    QScrollArea* scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable(true);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    main_layout->addWidget(scroll_area);
    
    // Content widget
    QWidget* content_widget = new QWidget();
    content_widget->setStyleSheet(QString("background-color: %1;").arg(bgColor));
    scroll_area->setWidget(content_widget);
    
    QVBoxLayout* content_layout = new QVBoxLayout(content_widget);
    content_layout->setContentsMargins(24, 24, 24, 24);
    content_layout->setSpacing(20);
    
    // Title
    QLabel* title = new QLabel("Settings");
    title->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1;").arg(accentColor));
    content_layout->addWidget(title);
    
    // Theme
    QLabel* theme_label = new QLabel("Theme:");
    theme_label->setStyleSheet(QString("color: %1;").arg(textColor));
    theme_combo_ = new QComboBox();
    theme_combo_->addItem("Dark", "dark");
    theme_combo_->addItem("Light", "light");
    theme_combo_->addItem("System", "system");
    theme_combo_->setStyleSheet(QString("padding: 8px 12px; border: 1px solid %1; border-radius: 6px; background: %2; color: %3; min-width: 150px;").arg(borderColor, inputBg, textColor));
    
    QHBoxLayout* theme_row = new QHBoxLayout();
    theme_row->addWidget(theme_label);
    theme_row->addWidget(theme_combo_);
    theme_row->addStretch();
    content_layout->addLayout(theme_row);
    
    // Privacy Section
    QLabel* privacy_header = new QLabel("Privacy & Security");
    privacy_header->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; margin-top: 16px;").arg(accentColor));
    content_layout->addWidget(privacy_header);
    
    QGridLayout* privacy_grid = new QGridLayout();
    privacy_grid->setColumnStretch(0, 1);
    privacy_grid->setColumnStretch(1, 1);
    privacy_grid->setHorizontalSpacing(16);
    privacy_grid->setVerticalSpacing(8);
    
    QString checkbox_style = QString("QCheckBox { color: %1; spacing: 8px; } QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; border: 1px solid %2; background: %3; } QCheckBox::indicator:checked { background: %4; border-color: %4; }").arg(textColor, borderColor, inputBg, accentColor);
    
    block_trackers_ = new QCheckBox("Block Trackers");
    block_trackers_->setChecked(true);
    block_trackers_->setStyleSheet(checkbox_style);
    privacy_grid->addWidget(block_trackers_, 0, 0);
    
    block_ads_ = new QCheckBox("Block Ads");
    block_ads_->setChecked(true);
    block_ads_->setStyleSheet(checkbox_style);
    privacy_grid->addWidget(block_ads_, 0, 1);
    
    https_only_ = new QCheckBox("HTTPS Only");
    https_only_->setStyleSheet(checkbox_style);
    privacy_grid->addWidget(https_only_, 1, 0);
    
    do_not_track_ = new QCheckBox("Do Not Track");
    do_not_track_->setChecked(true);
    do_not_track_->setStyleSheet(checkbox_style);
    privacy_grid->addWidget(do_not_track_, 1, 1);
    
    block_third_party_cookies_ = new QCheckBox("Block Third-Party Cookies");
    block_third_party_cookies_->setChecked(true);
    block_third_party_cookies_->setStyleSheet(checkbox_style);
    privacy_grid->addWidget(block_third_party_cookies_, 2, 0);
    
    block_fingerprinting_ = new QCheckBox("Block Fingerprinting");
    block_fingerprinting_->setChecked(true);
    block_fingerprinting_->setStyleSheet(checkbox_style);
    privacy_grid->addWidget(block_fingerprinting_, 2, 1);
    
    disable_webrtc_ = new QCheckBox("Disable WebRTC");
    disable_webrtc_->setStyleSheet(checkbox_style);
    privacy_grid->addWidget(disable_webrtc_, 3, 0);
    
    content_layout->addLayout(privacy_grid);
    
    // Search Engine Section
    QLabel* search_header = new QLabel("Search Engine");
    search_header->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; margin-top: 16px;").arg(accentColor));
    content_layout->addWidget(search_header);
    
    QString radio_style = QString("QRadioButton { color: %1; spacing: 8px; } QRadioButton::indicator { width: 18px; height: 18px; border-radius: 9px; border: 1px solid %2; background: %3; } QRadioButton::indicator:checked { background: %4; border-color: %4; }").arg(textColor, borderColor, inputBg, accentColor);
    
    search_group_ = new QButtonGroup(this);
    
    QRadioButton* duckduckgo_radio = new QRadioButton("DuckDuckGo");
    duckduckgo_radio->setStyleSheet(radio_style);
    search_group_->addButton(duckduckgo_radio, 0);
    content_layout->addWidget(duckduckgo_radio);
    
    QRadioButton* brave_radio = new QRadioButton("Brave Search");
    brave_radio->setStyleSheet(radio_style);
    search_group_->addButton(brave_radio, 1);
    content_layout->addWidget(brave_radio);
    
    QRadioButton* google_radio = new QRadioButton("Google");
    google_radio->setStyleSheet(radio_style);
    search_group_->addButton(google_radio, 2);
    content_layout->addWidget(google_radio);
    
    // Startup Section
    QLabel* startup_header = new QLabel("Startup");
    startup_header->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; margin-top: 16px;").arg(accentColor));
    content_layout->addWidget(startup_header);
    
    QHBoxLayout* homepage_row = new QHBoxLayout();
    QLabel* homepage_label = new QLabel("Homepage:");
    homepage_label->setStyleSheet(QString("color: %1; min-width: 80px;").arg(textColor));
    homepage_row->addWidget(homepage_label);
    homepage_edit_ = new QLineEdit();
    homepage_edit_->setPlaceholderText("about:blank");
    homepage_edit_->setStyleSheet(QString("padding: 8px 12px; border: 1px solid %1; border-radius: 6px; background: %2; color: %3;").arg(borderColor, inputBg, textColor));
    homepage_edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    homepage_row->addWidget(homepage_edit_);
    content_layout->addLayout(homepage_row);
    
    restore_tabs_ = new QCheckBox("Restore tabs from last session");
    restore_tabs_->setChecked(true);
    restore_tabs_->setStyleSheet(checkbox_style);
    content_layout->addWidget(restore_tabs_);
    
    auto_reload_ = new QCheckBox("Auto-reload pages");
    auto_reload_->setStyleSheet(checkbox_style);
    content_layout->addWidget(auto_reload_);
    
    QHBoxLayout* reload_row = new QHBoxLayout();
    QLabel* reload_label = new QLabel("Interval:");
    reload_label->setStyleSheet(QString("color: %1; min-width: 80px;").arg(textColor));
    reload_row->addWidget(reload_label);
    auto_reload_interval_ = new QSpinBox();
    auto_reload_interval_->setRange(5, 3600);
    auto_reload_interval_->setValue(30);
    auto_reload_interval_->setEnabled(false);
    auto_reload_interval_->setStyleSheet(QString("padding: 8px 12px; border: 1px solid %1; border-radius: 6px; background: %2; color: %3;").arg(borderColor, inputBg, textColor));
    reload_row->addWidget(auto_reload_interval_);
    reload_row->addStretch();
    content_layout->addLayout(reload_row);
    
    connect(auto_reload_, &QCheckBox::toggled, auto_reload_interval_, &QSpinBox::setEnabled);
    
    // Advanced Section
    QLabel* advanced_header = new QLabel("Advanced");
    advanced_header->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1; margin-top: 16px;").arg(accentColor));
    content_layout->addWidget(advanced_header);
    
    QHBoxLayout* zoom_row = new QHBoxLayout();
    QLabel* zoom_label = new QLabel("Zoom:");
    zoom_label->setStyleSheet(QString("color: %1; min-width: 80px;").arg(textColor));
    zoom_row->addWidget(zoom_label);
    zoom_level_ = new QSlider(Qt::Horizontal);
    zoom_level_->setRange(25, 200);
    zoom_level_->setValue(100);
    zoom_level_->setStyleSheet(QString("QSlider::groove:horizontal { height: 6px; background: %1; border-radius: 3px; } QSlider::handle:horizontal { background: %2; width: 18px; margin: -6px 0; border-radius: 4px; }").arg(borderColor, accentColor));
    zoom_level_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    zoom_row->addWidget(zoom_level_);
    zoom_label_ = new QLabel("100%");
    zoom_label_->setStyleSheet(QString("color: %1; min-width: 50px;").arg(textColor));
    zoom_row->addWidget(zoom_label_);
    content_layout->addLayout(zoom_row);
    
    connect(zoom_level_, &QSlider::valueChanged, this, [this](int value) {
        zoom_label_->setText(QString::number(value) + "%");
    });
    
    show_bookmarks_bar_ = new QCheckBox("Show bookmarks bar");
    show_bookmarks_bar_->setStyleSheet(checkbox_style);
    content_layout->addWidget(show_bookmarks_bar_);
    
    auto_clear_cache_ = new QCheckBox("Auto-clear cache on exit");
    auto_clear_cache_->setStyleSheet(checkbox_style);
    content_layout->addWidget(auto_clear_cache_);
    
    content_layout->addStretch();
    
    // Button Row
    QWidget* button_container = new QWidget();
    button_container->setStyleSheet(QString("background-color: %1; border-top: 1px solid %2;").arg(bgColor, borderColor));
    QHBoxLayout* button_layout = new QHBoxLayout(button_container);
    button_layout->setContentsMargins(24, 16, 24, 16);
    button_layout->setSpacing(12);
    
    QPushButton* reset_btn = new QPushButton("Reset");
    reset_btn->setStyleSheet(QString("padding: 10px 24px; border: 1px solid %1; border-radius: 6px; background: %2; color: %3;").arg(borderColor, inputBg, textColor));
    connect(reset_btn, &QPushButton::clicked, this, &SettingsDialog::resetSettings);
    button_layout->addWidget(reset_btn);
    
    button_layout->addStretch();
    
    QPushButton* cancel_btn = new QPushButton("Cancel");
    cancel_btn->setStyleSheet(QString("padding: 10px 24px; border: 1px solid %1; border-radius: 6px; background: %2; color: %3;").arg(borderColor, inputBg, textColor));
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
    button_layout->addWidget(cancel_btn);
    
    QPushButton* save_btn = new QPushButton("Save");
    save_btn->setStyleSheet(QString("padding: 10px 24px; background: %1; color: white; border: none; border-radius: 6px;").arg(accentColor));
    connect(save_btn, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    connect(save_btn, &QPushButton::clicked, this, &QDialog::accept);
    button_layout->addWidget(save_btn);
    
    main_layout->addWidget(button_container);
    
    loadSettings();
}

void SettingsDialog::loadSettings() {
    auto& settings = Settings::instance();
    
    QString theme = settings.getTheme();
    int theme_index = theme_combo_->findData(theme);
    if (theme_index >= 0) theme_combo_->setCurrentIndex(theme_index);
    
    block_trackers_->setChecked(settings.getBlockTrackers());
    block_ads_->setChecked(settings.getBlockAds());
    https_only_->setChecked(settings.getHttpsOnly());
    do_not_track_->setChecked(settings.getDoNotTrack());
    block_third_party_cookies_->setChecked(settings.getBlockThirdPartyCookies());
    block_fingerprinting_->setChecked(settings.getBlockFingerprinting());
    disable_webrtc_->setChecked(settings.getDisableWebRTC());
    
    QString searchEngine = settings.getSearchEngine();
    int search_id = 0;
    if (searchEngine == "brave") search_id = 1;
    else if (searchEngine == "google") search_id = 2;
    QAbstractButton* search_btn = search_group_->button(search_id);
    if (search_btn) search_btn->setChecked(true);
    
    homepage_edit_->setText(settings.getHomepage());
    restore_tabs_->setChecked(settings.getRestoreTabs());
    auto_reload_->setChecked(settings.getAutoReload());
    auto_reload_interval_->setValue(settings.getAutoReloadInterval());
    
    zoom_level_->setValue(settings.getZoomLevel());
    zoom_label_->setText(QString::number(settings.getZoomLevel()) + "%");
    show_bookmarks_bar_->setChecked(settings.getShowBookmarksBar());
    auto_clear_cache_->setChecked(settings.getAutoClearCache());
}

void SettingsDialog::saveSettings() {
    auto& settings = Settings::instance();
    
    QString theme = theme_combo_->currentData().toString();
    settings.setTheme(theme);
    settings.setDarkMode(theme == "dark");
    
    settings.setBlockTrackers(block_trackers_->isChecked());
    settings.setBlockAds(block_ads_->isChecked());
    settings.setHttpsOnly(https_only_->isChecked());
    settings.setDoNotTrack(do_not_track_->isChecked());
    settings.setBlockThirdPartyCookies(block_third_party_cookies_->isChecked());
    settings.setBlockFingerprinting(block_fingerprinting_->isChecked());
    settings.setDisableWebRTC(disable_webrtc_->isChecked());
    
    int search_id = search_group_->checkedId();
    QString searchEngine = "duckduckgo";
    if (search_id == 1) searchEngine = "brave";
    else if (search_id == 2) searchEngine = "google";
    settings.setSearchEngine(searchEngine);
    
    settings.setHomepage(homepage_edit_->text());
    settings.setRestoreTabs(restore_tabs_->isChecked());
    settings.setAutoReload(auto_reload_->isChecked());
    settings.setAutoReloadInterval(auto_reload_interval_->value());
    
    settings.setZoomLevel(zoom_level_->value());
    settings.setShowBookmarksBar(show_bookmarks_bar_->isChecked());
    settings.setAutoClearCache(auto_clear_cache_->isChecked());
    
    settings.save();
}

void SettingsDialog::resetSettings() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Reset Settings", 
        "Reset all settings to defaults?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        Settings::instance().reset();
        loadSettings();
    }
}

void SettingsDialog::showDialog(QWidget* parent) {
    SettingsDialog dialog(parent);
    dialog.exec();
}

} // namespace Tsunami
