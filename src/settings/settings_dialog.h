#pragma once

#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>

namespace Tsunami {

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    
private:
    void setupUi();
    void applyTheme();
    void loadSettings();
    void saveSettings();
    void resetSettings();
    void onSettingsChanged();
    
    QComboBox* theme_combo_;
    QComboBox* accent_combo_;
    QComboBox* search_engine_;
    QCheckBox* block_trackers_;
    QCheckBox* block_ads_;
    QCheckBox* https_only_;
    QCheckBox* do_not_track_;
    QCheckBox* block_third_party_cookies_;
    QCheckBox* block_fingerprinting_;
    QCheckBox* disable_webrtc_;
    QCheckBox* dark_mode_check_;
    QCheckBox* restore_tabs_;
    QCheckBox* auto_reload_;
    QCheckBox* show_bookmarks_bar_;
    QCheckBox* auto_clear_cache_;
    QLineEdit* homepage_edit_;
    QSlider* zoom_level_;
    QLabel* zoom_label_;
    QSpinBox* auto_reload_interval_;
    
public slots:
    static void showDialog(QWidget* parent = nullptr);
};

} // namespace Tsunami
