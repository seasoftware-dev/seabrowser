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

namespace Tsunami {

OnboardingDialog::OnboardingDialog(QWidget* parent)
    : QDialog(parent)
    , current_step_(0)
{
    setWindowTitle("Welcome to Tsunami");
    setMinimumSize(500, 400);
    resize(550, 420);
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    applyTheme();
    setupUi();
    connect(&Settings::instance(), &Settings::settingsChanged, this, &OnboardingDialog::applyTheme);
}

void OnboardingDialog::applyTheme() {
    QString accentColor = Settings::instance().getAccentColor();
    if (accentColor.isEmpty()) accentColor = "#60a5fa";

    bool isDark = Settings::instance().getDarkMode();

    QString bgColor = isDark ? "#030712" : "#f0f9ff";
    QString titleColor = isDark ? "#f8fafc" : "#1e40af";
    QString subtitleColor = isDark ? "#94a3b8" : "#64748b";
    QString cardBg = isDark ? "#0f172a" : "#ffffff";
    QString borderColor = isDark ? "#1e293b" : "#bfdbfe";
    QString textColor = isDark ? "#f8fafc" : "#1e293b";
    QString optionBg = isDark ? "#1e293b" : "#eff6ff";
    QString optionHover = isDark ? "#334155" : "#dbeafe";

    setStyleSheet(QString(R"(
        QDialog {
            background-color: %1;
        }
    )").arg(bgColor));

    if (title_) {
        title_->setStyleSheet(QString("font-size: 22px; font-weight: bold; color: %1;").arg(titleColor));
    }
    if (subtitle_) {
        subtitle_->setStyleSheet(QString("font-size: 13px; color: %1; margin-bottom: 16px;").arg(subtitleColor));
    }

    if (back_btn_) {
        back_btn_->setStyleSheet(QString(R"(
            QPushButton {
                background: %1;
                color: %2;
                border: 1px solid %3;
                border-radius: 8px;
                padding: 10px 20px;
            }
            QPushButton:hover:not(:disabled) {
                background: %4;
            }
            QPushButton:disabled {
                opacity: 0.5;
            }
        )").arg(cardBg, subtitleColor, borderColor, optionBg));
    }

    if (next_btn_) {
        next_btn_->setStyleSheet(QString(R"(
            QPushButton {
                background: %1;
                color: white;
                border: none;
                border-radius: 8px;
                padding: 10px 24px;
                font-weight: 600;
            }
            QPushButton:hover {
                opacity: 0.9;
            }
        )").arg(accentColor));
    }

    updateOptionButtons();
}

void OnboardingDialog::updateOptionButtons() {
    QString accentColor = Settings::instance().getAccentColor();
    if (accentColor.isEmpty()) accentColor = "#60a5fa";

    bool isDark = Settings::instance().getDarkMode();

    QString cardBg = isDark ? "#1e293b" : "#ffffff";
    QString borderColor = isDark ? "#334155" : "#bfdbfe";
    QString textColor = isDark ? "#f8fafc" : "#1e293b";
    QString optionBg = isDark ? "#0f172a" : "#eff6ff";
    QString optionHover = isDark ? "#1e293b" : "#dbeafe";
    QString selectedBg = isDark ? QString("rgba(96, 165, 250, 0.2)") : "#bfdbfe";

    QStringList steps = {"theme_step", "color_step", "search_step"};
    for (const QString& stepName : steps) {
        QLayout* layout = nullptr;
        if (stepName == "theme_step") {
            layout = theme_layout_;
        } else if (stepName == "color_step") {
            layout = color_layout_;
        } else if (stepName == "search_step") {
            layout = search_layout_;
        }

        if (layout) {
            for (int i = 0; i < layout->count(); ++i) {
                QWidget* widget = layout->itemAt(i)->widget();
                if (widget && widget->inherits("QAbstractButton")) {
                    QRadioButton* btn = qobject_cast<QRadioButton*>(widget);
                    if (btn) {
                        bool isChecked = btn->isChecked();
                        btn->setStyleSheet(QString(R"(
                            QRadioButton {
                                background: %1;
                                color: %2;
                                border: 1px solid %3;
                                border-radius: 8px;
                                padding: 12px 16px;
                                spacing: 10px;
                            }
                            QRadioButton:hover {
                                background: %4;
                            }
                            QRadioButton::indicator {
                                width: 18px;
                                height: 18px;
                                border-radius: 9px;
                                border: 2px solid %3;
                            }
                            QRadioButton::indicator:checked {
                                background: %5;
                                border: 2px solid %5;
                            }
                        )").arg(isChecked ? selectedBg : optionBg, textColor, borderColor,
                                isDark ? optionBg : optionHover, accentColor));
                    }
                }
            }
        }
    }
}

void OnboardingDialog::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(32, 28, 32, 20);
    main_layout->setSpacing(0);

    title_ = new QLabel("Welcome to Tsunami");
    title_->setStyleSheet("font-size: 22px; font-weight: bold; color: #f8fafc;");
    main_layout->addWidget(title_);

    subtitle_ = new QLabel("Let's customize your browsing experience");
    subtitle_->setStyleSheet("font-size: 13px; color: #94a3b8; margin-bottom: 16px;");
    main_layout->addWidget(subtitle_);

    stacked_widget_ = new QStackedWidget();
    stacked_widget_->setObjectName("onboardingStack");
    stacked_widget_->setStyleSheet(R"(
        QStackedWidget {
            background: transparent;
            border: none;
        }
    )");

    theme_layout_ = new QVBoxLayout();
    theme_layout_->setSpacing(10);

    QLabel* theme_label = new QLabel("Choose your theme");
    theme_label->setStyleSheet("font-size: 15px; font-weight: 600; color: #f8fafc;");
    theme_layout_->addWidget(theme_label);

    theme_group_ = new QButtonGroup(this);

    QRadioButton* dark_btn = new QRadioButton("Dark Mode");
    dark_btn->setProperty("theme", "dark");
    dark_btn->setChecked(true);
    theme_layout_->addWidget(dark_btn);
    theme_group_->addButton(dark_btn);

    QRadioButton* light_btn = new QRadioButton("Light Mode");
    light_btn->setProperty("theme", "light");
    theme_layout_->addWidget(light_btn);
    theme_group_->addButton(light_btn);

    QWidget* theme_widget = new QWidget();
    theme_widget->setLayout(theme_layout_);
    stacked_widget_->addWidget(theme_widget);

    color_layout_ = new QVBoxLayout();
    color_layout_->setSpacing(10);

    QLabel* color_label = new QLabel("Pick an accent color");
    color_label->setStyleSheet("font-size: 15px; font-weight: 600; color: #f8fafc;");
    color_layout_->addWidget(color_label);

    color_group_ = new QButtonGroup(this);

    QStringList colors = {
        "#60a5fa|Blue",
        "#a78bfa|Purple",
        "#34d399|Green",
        "#f472b6|Pink",
        "#fbbf24|Orange",
        "#f87171|Red"
    };

    QHBoxLayout* color_row1 = new QHBoxLayout();
    QHBoxLayout* color_row2 = new QHBoxLayout();

    for (int i = 0; i < colors.size(); ++i) {
        QStringList parts = colors[i].split("|");
        QRadioButton* color_btn = new QRadioButton(parts[1]);
        color_btn->setProperty("color", parts[0]);
        if (i == 0) color_btn->setChecked(true);
        color_group_->addButton(color_btn);
        if (i < 3) {
            color_row1->addWidget(color_btn);
        } else {
            color_row2->addWidget(color_btn);
        }
    }

    color_layout_->addLayout(color_row1);
    color_layout_->addLayout(color_row2);

    QWidget* color_widget = new QWidget();
    color_widget->setLayout(color_layout_);
    stacked_widget_->addWidget(color_widget);

    search_layout_ = new QVBoxLayout();
    search_layout_->setSpacing(10);

    QLabel* search_label = new QLabel("Select your search engine");
    search_label->setStyleSheet("font-size: 15px; font-weight: 600; color: #f8fafc;");
    search_layout_->addWidget(search_label);

    search_group_ = new QButtonGroup(this);

    QStringList engines = {
        "duckduckgo|DuckDuckGo - Privacy-first search",
        "google|Google - Fast and comprehensive",
        "bing|Microsoft Bing - AI-powered search"
    };

    for (const QString& engine : engines) {
        QStringList parts = engine.split("|");
        QRadioButton* engine_btn = new QRadioButton(parts[1]);
        engine_btn->setProperty("engine", parts[0]);
        if (parts[0] == "duckduckgo") engine_btn->setChecked(true);
        search_layout_->addWidget(engine_btn);
        search_group_->addButton(engine_btn);
    }

    QWidget* search_widget = new QWidget();
    search_widget->setLayout(search_layout_);
    stacked_widget_->addWidget(search_widget);

    main_layout->addWidget(stacked_widget_, 1);

    QHBoxLayout* btn_layout = new QHBoxLayout();
    btn_layout->setSpacing(12);

    back_btn_ = new QPushButton("Back");
    back_btn_->setEnabled(false);
    back_btn_->setStyleSheet("QPushButton { background: #1e293b; color: #94a3b8; border: 1px solid #334155; border-radius: 8px; padding: 10px 20px; } QPushButton:hover:not(:disabled) { background: #334155; } QPushButton:disabled { opacity: 0.5; }");
    connect(back_btn_, &QPushButton::clicked, this, &OnboardingDialog::onBack);
    btn_layout->addWidget(back_btn_);

    btn_layout->addStretch();

    next_btn_ = new QPushButton("Continue");
    next_btn_->setStyleSheet("QPushButton { background: #3b82f6; color: white; border: none; border-radius: 8px; padding: 10px 24px; font-weight: 600; } QPushButton:hover { opacity: 0.9; }");
    connect(next_btn_, &QPushButton::clicked, this, &OnboardingDialog::onNext);
    btn_layout->addWidget(next_btn_);

    main_layout->addLayout(btn_layout);

    applyTheme();
}

void OnboardingDialog::onBack() {
    if (current_step_ > 0) {
        current_step_--;
        stacked_widget_->setCurrentIndex(current_step_);
        updateButtons();
    }
}

void OnboardingDialog::onNext() {
    if (current_step_ < 2) {
        current_step_++;
        stacked_widget_->setCurrentIndex(current_step_);
        updateButtons();
    } else {
        saveSettings();
        accept();
    }
}

void OnboardingDialog::updateButtons() {
    back_btn_->setEnabled(current_step_ > 0);

    if (current_step_ >= 2) {
        next_btn_->setText("Start Browsing");
    } else {
        next_btn_->setText("Continue");
    }
}

void OnboardingDialog::closeEvent(QCloseEvent* event) {
    if (current_step_ < 2) {
        event->ignore();
    } else {
        event->accept();
    }
}

void OnboardingDialog::saveSettings() {
    auto& settings = Settings::instance();

    QAbstractButton* themeBtn = theme_group_->checkedButton();
    if (themeBtn) {
        bool isDark = themeBtn->property("theme").toString() == "dark";
        settings.setDarkMode(isDark);
    }

    QAbstractButton* colorBtn = color_group_->checkedButton();
    if (colorBtn) {
        settings.setAccentColor(colorBtn->property("color").toString());
    }

    QAbstractButton* searchBtn = search_group_->checkedButton();
    if (searchBtn) {
        settings.setSearchEngine(searchBtn->property("engine").toString());
    }

    settings.setFirstRun(false);
    emit settings.settingsChanged();

    qDebug() << "Onboarding settings saved";
}

} // namespace Tsunami
