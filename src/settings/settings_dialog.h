#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QButtonGroup>
#include <QRadioButton>

namespace Tsunami {

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    
private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    void resetSettings();
    
    QTabWidget* tab_widget_ = nullptr;
    QComboBox* theme_combo_ = nullptr;
    QButtonGroup* search_group_ = nullptr;
    QCheckBox* block_trackers_ = nullptr;
    QCheckBox* block_ads_ = nullptr;
    QCheckBox* https_only_ = nullptr;
    QCheckBox* do_not_track_ = nullptr;
    QCheckBox* block_third_party_cookies_ = nullptr;
    QCheckBox* block_fingerprinting_ = nullptr;
    QCheckBox* disable_webrtc_ = nullptr;
    QCheckBox* restore_tabs_ = nullptr;
    QCheckBox* auto_reload_ = nullptr;
    QLineEdit* homepage_edit_ = nullptr;
    QSpinBox* auto_reload_interval_ = nullptr;
    QSlider* zoom_level_ = nullptr;
    QLabel* zoom_label_ = nullptr;
    QCheckBox* show_bookmarks_bar_ = nullptr;
    QCheckBox* auto_clear_cache_ = nullptr;
    
public slots:
    static void showDialog(QWidget* parent = nullptr);
};

} // namespace Tsunami
