#include "onboarding_dialog.h"
#include "../settings/settings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QRadioButton>
#include <QPushButton>
#include <QCloseEvent>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QPropertyAnimation>

namespace Tsunami {

OnboardingDialog::OnboardingDialog(QWidget* parent)
    : QDialog(parent)
    , current_step_(0)
{
    setWindowTitle("Welcome to Tsunami");
    setMinimumSize(600, 500); 
    resize(600, 500);
    setModal(true);
    
    // Remove standard window decorations and add custom ones
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    
    applyTheme();
    setupUi();
    
    // Connect settings changes to theme updates
    connect(&Settings::instance(), &Settings::settingsChanged, this, &OnboardingDialog::applyTheme);
}

void OnboardingDialog::setupUi() {
    // Create custom title bar
    QWidget* title_bar = new QWidget(this);
    title_bar->setObjectName("titleBar");
    title_bar->setFixedHeight(32);
    title_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    QHBoxLayout* title_layout = new QHBoxLayout(title_bar);
    title_layout->setContentsMargins(8, 0, 8, 0);
    title_layout->setSpacing(4);
    
    // Window title
    title_ = new QLabel("Welcome to Tsunami");
    // Initial style, will be updated by applyTheme
    title_->setStyleSheet("font-size: 12px; font-weight: 500; margin-left: 8px;");
    title_layout->addWidget(title_);
    title_layout->addStretch();
    
    // Window control buttons - Right side
    QWidget* controls_container = new QWidget(title_bar);
    QHBoxLayout* controls_layout = new QHBoxLayout(controls_container);
    controls_layout->setContentsMargins(0, 0, 0, 0);
    controls_layout->setSpacing(0);
    
    // Minimize button
    min_btn_ = new QPushButton(controls_container);
    min_btn_->setFixedSize(24, 24);
    min_btn_->setToolTip("Minimize");
    min_btn_->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
            padding: 4px;
            margin: 0 1px;
        }
        QPushButton:hover {
            background: rgba(128, 128, 128, 0.2);
        }
    )");
    
    // Maximize button
    max_btn_ = new QPushButton(controls_container);
    max_btn_->setFixedSize(24, 24);
    max_btn_->setToolTip("Maximize");
    max_btn_->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
            padding: 4px;
            margin: 0 1px;
        }
        QPushButton:hover {
            background: rgba(128, 128, 128, 0.2);
        }
    )");
    
    // Close button
    close_btn_ = new QPushButton(controls_container);
    close_btn_->setFixedSize(24, 24);
    close_btn_->setToolTip("Close");
    close_btn_->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            border: none;
            padding: 4px;
            margin: 0 1px;
        }
        QPushButton:hover {
            background: #ef4444;
        }
    )");
    
    connect(close_btn_, &QPushButton::clicked, this, &OnboardingDialog::onClose);
    connect(min_btn_, &QPushButton::clicked, this, &OnboardingDialog::onMinimize);
    connect(max_btn_, &QPushButton::clicked, this, &OnboardingDialog::onMaximize);
    
    controls_layout->addStretch();
    controls_layout->addWidget(min_btn_);
    controls_layout->addWidget(max_btn_);
    controls_layout->addWidget(close_btn_);
    
    title_layout->addWidget(controls_container);
    
    // Main content area
    QWidget* main_widget = new QWidget(this);
    QVBoxLayout* main_layout = new QVBoxLayout(main_widget);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    
    // Stacked widget for different steps
    stacked_widget_ = new QStackedWidget(main_widget);
    
    // Common styles - Radio button style without color (handled by applyTheme)
    QString radioStyle = R"(
        QRadioButton::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #64748b;
            border-radius: 9px;
            background: transparent;
        }
        QRadioButton::indicator:checked {
            background: #3b82f6;
            border-color: #3b82f6;
        }
        QRadioButton {
            font-weight: 500;
            font-size: 14px;
            padding: 4px;
        }
    )";

    // Step 1: Theme selection
    QWidget* theme_step = new QWidget();
    QVBoxLayout* theme_layout = new QVBoxLayout(theme_step);
    theme_layout->setContentsMargins(32, 32, 32, 32);
    theme_layout->setSpacing(20);
    
    QLabel* theme_title = new QLabel("Choose Your Theme");
    // We avoid hardcoding color here, applyTheme will handle text color
    theme_title->setStyleSheet("font-size: 22px; font-weight: bold; margin-bottom: 16px;");
    theme_layout->addWidget(theme_title);
    
    theme_group_ = new QButtonGroup(this);
    
    // Dark theme option
    QWidget* dark_option = new QWidget();
    QVBoxLayout* dark_layout = new QVBoxLayout(dark_option);
    dark_layout->setContentsMargins(0, 0, 0, 0);
    dark_layout->setSpacing(4);
    
    QRadioButton* dark_radio = new QRadioButton("Dark");
    dark_radio->setStyleSheet(radioStyle);
    dark_layout->addWidget(dark_radio);
    
    QLabel* dark_desc = new QLabel("Perfect for late-night browsing with reduced eye strain");
    dark_desc->setStyleSheet("color: #64748b; font-size: 13px; margin-left: 28px;");
    dark_layout->addWidget(dark_desc);
    
    theme_group_->addButton(dark_radio, 0);
    
    // Light theme option
    QWidget* light_option = new QWidget();
    QVBoxLayout* light_layout = new QVBoxLayout(light_option);
    light_layout->setContentsMargins(0, 0, 0, 0);
    light_layout->setSpacing(4);
    
    QRadioButton* light_radio = new QRadioButton("Light");
    light_radio->setStyleSheet(radioStyle);
    light_layout->addWidget(light_radio);
    
    QLabel* light_desc = new QLabel("Bright and clear for daytime use and better visibility");
    light_desc->setStyleSheet("color: #64748b; font-size: 13px; margin-left: 28px;");
    light_layout->addWidget(light_desc);
    
    theme_group_->addButton(light_radio, 1);
    
    // System theme option
    QWidget* system_option = new QWidget();
    QVBoxLayout* system_layout = new QVBoxLayout(system_option);
    system_layout->setContentsMargins(0, 0, 0, 0);
    system_layout->setSpacing(4);
    
    QRadioButton* system_radio = new QRadioButton("System");
    system_radio->setStyleSheet(radioStyle);
    system_layout->addWidget(system_radio);
    
    QLabel* system_desc = new QLabel("Follow your operating system's theme preference");
    system_desc->setStyleSheet("color: #64748b; font-size: 13px; margin-left: 28px;");
    system_layout->addWidget(system_desc);
    
    theme_group_->addButton(system_radio, 2);
    
    theme_layout->addWidget(dark_option);
    theme_layout->addWidget(light_option);
    theme_layout->addWidget(system_option);
    theme_layout->addStretch();
    
    stacked_widget_->addWidget(theme_step);
    
    // Step 2: Search engine selection
    QWidget* search_step = new QWidget();
    QVBoxLayout* search_layout = new QVBoxLayout(search_step);
    search_layout->setContentsMargins(32, 32, 32, 32);
    search_layout->setSpacing(20);
    
    QLabel* search_title = new QLabel("Choose Your Search Engine");
    search_title->setStyleSheet("font-size: 22px; font-weight: bold; margin-bottom: 16px;");
    search_layout->addWidget(search_title);
    
    search_group_ = new QButtonGroup(this);
    
    auto createSearchOption = [&](const QString& name, const QString& desc, int id) {
        QWidget* option = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(option);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(4);
        
        QRadioButton* radio = new QRadioButton(name);
        radio->setStyleSheet(radioStyle);
        layout->addWidget(radio);
        
        QLabel* descLabel = new QLabel(desc);
        descLabel->setStyleSheet("color: #64748b; font-size: 13px; margin-left: 28px;");
        layout->addWidget(descLabel);
        
        search_group_->addButton(radio, id);
        return option;
    };
    
    search_layout->addWidget(createSearchOption("DuckDuckGo", "Privacy-focused search with no tracking", 0));
    search_layout->addWidget(createSearchOption("Brave Search", "Independent search with privacy features", 1));
    search_layout->addWidget(createSearchOption("Google", "Most popular search engine with comprehensive results", 2));
    search_layout->addStretch();
    
    stacked_widget_->addWidget(search_step);
    
    main_layout->addWidget(stacked_widget_);
    
    // Navigation buttons
    QWidget* button_container = new QWidget(main_widget);
    QHBoxLayout* button_layout = new QHBoxLayout(button_container);
    button_layout->setContentsMargins(32, 0, 32, 32);
    button_layout->setSpacing(16);
    
    back_btn_ = new QPushButton("Back");
    back_btn_->setCursor(Qt::PointingHandCursor);
    back_btn_->setDisabled(true);
    connect(back_btn_, &QPushButton::clicked, this, &OnboardingDialog::onBack);
    button_layout->addWidget(back_btn_);
    
    next_btn_ = new QPushButton("Next");
    next_btn_->setCursor(Qt::PointingHandCursor);
    connect(next_btn_, &QPushButton::clicked, this, &OnboardingDialog::onNext);
    button_layout->addWidget(next_btn_);
    
    main_layout->addWidget(button_container);
    
    // Add layout to dialog
    QVBoxLayout* dialog_layout = new QVBoxLayout(this);
    dialog_layout->setContentsMargins(0, 0, 0, 0);
    dialog_layout->setSpacing(0);
    dialog_layout->addWidget(title_bar);
    dialog_layout->addWidget(main_widget);
    
    // Ensure all options are visible with correct size
    resize(600, 500);
}

void OnboardingDialog::onBack() {
    if (current_step_ > 0) {
        current_step_--;
        stacked_widget_->setCurrentIndex(current_step_);
        updateButtons();
    }
}

void OnboardingDialog::onNext() {
    if (current_step_ < 1) {
        current_step_++;
        stacked_widget_->setCurrentIndex(current_step_);
        updateButtons();
    } else {
        // Last step (1) - Finish
        saveSettings();
        accept();
    }
}

void OnboardingDialog::updateButtons() {
    back_btn_->setDisabled(current_step_ == 0);
    next_btn_->setText(current_step_ == 2 ? "Finish" : "Next");
}

void OnboardingDialog::saveSettings() {
    // Save theme
    int theme_index = theme_group_->checkedId();
    if (theme_index == 0) {
        Settings::instance().setTheme("dark");
        Settings::instance().setDarkMode(true);
    } else if (theme_index == 1) {
        Settings::instance().setTheme("light");
        Settings::instance().setDarkMode(false);
    } else {
        Settings::instance().setTheme("system");
        Settings::instance().setDarkMode(false); // System will handle this
    }
    
    // Save search engine
    int search_index = search_group_->checkedId();
    QString searchEngine;
    switch (search_index) {
        case 0: searchEngine = "duckduckgo"; break;
        case 1: searchEngine = "brave"; break;
        case 2: searchEngine = "google"; break;
    }
    Settings::instance().setSearchEngine(searchEngine);
    
    Settings::instance().setFirstRun(false);
}

void OnboardingDialog::closeEvent(QCloseEvent* event) {
    if (Settings::instance().isFirstRun()) {
        // If closing during first run (without finishing), enable exit
        event->accept();
        QApplication::exit(0);
    } else {
        event->accept();
    }
}

void OnboardingDialog::applyTheme() {
    // Default to dark mode for first run (before settings are saved)
    bool isDark = true; // Always use dark theme for onboarding
    QString bgColor = "#0f172a";
    QString textColor = "#e2e8f0";
    QString btnBg = "#1e293b";
    QString btnBorder = "#334155";
    
    setStyleSheet(QString("QDialog { background-color: %1; color: %2; } QLabel { color: %2; } QRadioButton { color: %2; }").arg(bgColor, textColor));
    
    if (title_) {
        title_->setStyleSheet(QString("font-size: 12px; font-weight: 500; color: %1; margin-left: 8px;").arg(textColor));
    }
    
    if (back_btn_) {
        back_btn_->setStyleSheet(QString(R"(
            QPushButton {
                background: %1;
                color: %2;
                border: 1px solid %3;
                border-radius: 8px;
                padding: 12px 24px;
                font-weight: 500;
                font-size: 14px;
            }
            QPushButton:hover {
                background: %3;
                border-color: #3b82f6;
            }
            QPushButton:disabled {
                background: %1;
                border-color: %3;
                color: #94a3b8;
                opacity: 0.5;
            }
        )").arg(btnBg, textColor, btnBorder));
    }
    
    if (next_btn_) {
        next_btn_->setStyleSheet(R"(
            QPushButton {
                background: #3b82f6;
                color: #ffffff;
                border: 1px solid #3b82f6;
                border-radius: 8px;
                padding: 12px 24px;
                font-weight: 600;
                font-size: 14px;
            }
            QPushButton:hover {
                background: #2563eb;
                border-color: #2563eb;
            }
        )");
    }
    
    // We don't have direct access to all radio buttons easily unless we stored them?
    // Actually, setStyleSheet on the QDialog propagates to children unless overridden.
    // In setupUi, I changed QRadioButton stylesheet to NOT include color!
    // So QRadioButton { color: %2; } in this function's setStyleSheet should apply to all of them.
    // That works!
}

void OnboardingDialog::onMinimize() {
    showMinimized();
}

void OnboardingDialog::onMaximize() {
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void OnboardingDialog::onClose() {
    close();
}

} // namespace Tsunami