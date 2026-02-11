#include "onboarding_dialog.h"
#include "../settings/settings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QPixmap>
#include <QPainter>
#include <QCloseEvent>
#include <QDebug>

namespace Tsunami {

OnboardingDialog::OnboardingDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Welcome to Tsunami");
    setMinimumSize(500, 400);
    resize(500, 420);
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    applyTheme();
    setupUi();
}

void OnboardingDialog::applyTheme() {
    QString accentColor = Settings::instance().getAccentColor();
    if (accentColor.isEmpty()) accentColor = "#3b82f6";

    setStyleSheet(QString(R"(
        QDialog {
            background-color: #0f172a;
            color: #f8fafc;
        }
        QLabel {
            color: #f8fafc;
        }
        QRadioButton {
            color: #f8fafc;
            spacing: 8px;
            font-size: 14px;
        }
        QRadioButton::indicator {
            width: 18px;
            height: 18px;
            border-radius: 9px;
            border: 2px solid #475569;
        }
        QRadioButton::indicator:checked {
            background: %1;
            border-color: %1;
        }
        QPushButton {
            background: linear-gradient(135deg, %1, #2563eb);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 24px;
            font-weight: 600;
            font-size: 14px;
        }
        QPushButton:hover {
            opacity: 0.9;
        }
        QPushButton:disabled {
            background: #334155;
            color: #64748b;
        }
    )").arg(accentColor));
}

void OnboardingDialog::setupUi() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(24, 20, 24, 16);
    main_layout->setSpacing(12);

    // Logo
    QLabel* logo = new QLabel();
    logo->setFixedSize(64, 64);
    logo->setAlignment(Qt::AlignCenter);

    QPixmap p(64, 64);
    p.fill(Qt::transparent);
    QPainter painter(&p);
    painter.setRenderHint(QPainter::Antialiasing);

    QString accentColor = Settings::instance().getAccentColor();
    if (accentColor.isEmpty()) accentColor = "#3b82f6";

    QLinearGradient gradient(0, 0, 64, 64);
    gradient.setColorAt(0, QColor(accentColor));
    gradient.setColorAt(1, QColor("#2563eb"));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(0, 0, 64, 64, 12, 12);

    painter.setFont(QFont("Arial", 28, QFont::Bold));
    painter.setPen(Qt::white);
    painter.drawText(QRectF(0, 0, 64, 64), Qt::AlignCenter, "T");
    logo->setPixmap(p);
    main_layout->addWidget(logo, 0, Qt::AlignCenter);

    // Title
    QLabel* title = new QLabel("Welcome to Tsunami");
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #f8fafc;");
    title->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(title, 0, Qt::AlignCenter);

    QLabel* subtitle = new QLabel("A fast, private, beautiful web browser");
    subtitle->setStyleSheet("font-size: 13px; color: #94a3b8; margin-bottom: 16px;");
    subtitle->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(subtitle, 0, Qt::AlignCenter);

    // Stacked widget for steps
    stacked_widget_ = new QStackedWidget();
    main_layout->addWidget(stacked_widget_, 1);

    // Step 1: Theme
    QWidget* themeStep = createThemeStep();
    stacked_widget_->addWidget(themeStep);

    // Step 2: Color
    QWidget* colorStep = createColorStep();
    stacked_widget_->addWidget(colorStep);

    // Step 3: Search
    QWidget* searchStep = createSearchStep();
    stacked_widget_->addWidget(searchStep);

    // Navigation buttons
    QHBoxLayout* nav_layout = new QHBoxLayout();
    nav_layout->addStretch();

    back_btn_ = new QPushButton("Back");
    back_btn_->setEnabled(false);
    back_btn_->setStyleSheet("QPushButton { background: #1e293b; color: #94a3b8; border: 1px solid #334155; border-radius: 8px; padding: 10px 20px; } QPushButton:hover:not(:disabled) { background: #334155; } QPushButton:disabled { opacity: 0.5; }");
    connect(back_btn_, &QPushButton::clicked, this, &OnboardingDialog::onBack);
    nav_layout->addWidget(back_btn_);

    next_btn_ = new QPushButton("Continue");
    next_btn_->setStyleSheet("QPushButton { background: " + accentColor + "; color: white; border: none; border-radius: 8px; padding: 10px 24px; font-weight: 600; } QPushButton:hover { opacity: 0.9; }");
    connect(next_btn_, &QPushButton::clicked, this, &OnboardingDialog::onNext);
    nav_layout->addWidget(next_btn_);

    main_layout->addLayout(nav_layout);

    current_step_ = 0;
    updateButtons();
}

QWidget* OnboardingDialog::createThemeStep() {
    QWidget* step = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(step);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    QLabel* label = new QLabel("Choose your theme:");
    label->setStyleSheet("font-size: 15px; font-weight: 600; color: #f8fafc;");
    layout->addWidget(label);

    theme_group_ = new QButtonGroup(this);

    QRadioButton* darkBtn = new QRadioButton("Dark Mode");
    darkBtn->setChecked(true);
    darkBtn->setProperty("theme", "dark");
    theme_group_->addButton(darkBtn);
    layout->addWidget(darkBtn);

    QRadioButton* lightBtn = new QRadioButton("Light Mode");
    lightBtn->setProperty("theme", "light");
    theme_group_->addButton(lightBtn);
    layout->addWidget(lightBtn);

    layout->addStretch();
    return step;
}

QWidget* OnboardingDialog::createColorStep() {
    QWidget* step = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(step);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    QLabel* label = new QLabel("Pick an accent color:");
    label->setStyleSheet("font-size: 15px; font-weight: 600; color: #f8fafc;");
    layout->addWidget(label);

    QHBoxLayout* color_layout = new QHBoxLayout();
    color_layout->setSpacing(8);
    color_layout->addStretch();

    QStringList colors = {"#3b82f6", "#ef4444", "#22c55e", "#f59e0b", "#a855f7", "#ec4899", "#06b6d4"};
    QStringList names = {"Blue", "Red", "Green", "Orange", "Purple", "Pink", "Cyan"};

    color_group_ = new QButtonGroup(this);

    for (int i = 0; i < colors.size(); ++i) {
        QRadioButton* btn = new QRadioButton();
        btn->setFixedSize(40, 40);
        btn->setStyleSheet(QString(R"(
            QRadioButton {
                border-radius: 20px;
                background: %1;
            }
            QRadioButton::indicator {
                width: 32px;
                height: 32px;
                border-radius: 16px;
                border: 2px solid transparent;
                background: transparent;
            }
            QRadioButton::indicator:hover {
                border-color: white;
            }
            QRadioButton::indicator:checked {
                border-color: white;
            }
        )").arg(colors[i]));
        btn->setProperty("color", colors[i]);
        btn->setProperty("name", names[i]);
        color_group_->addButton(btn);
        color_layout->addWidget(btn);
    }

    color_layout->addStretch();
    layout->addLayout(color_layout);
    layout->addStretch();

    color_group_->buttons().first()->setChecked(true);
    return step;
}

QWidget* OnboardingDialog::createSearchStep() {
    QWidget* step = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(step);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    QLabel* label = new QLabel("Choose your search engine:");
    label->setStyleSheet("font-size: 15px; font-weight: 600; color: #f8fafc;");
    layout->addWidget(label);

    search_group_ = new QButtonGroup(this);

    QStringList engines = {"duckduckgo", "brave", "google", "bing"};
    QStringList engineNames = {"DuckDuckGo", "Brave Search", "Google", "Bing"};

    for (int i = 0; i < engines.size(); ++i) {
        QRadioButton* btn = new QRadioButton(engineNames[i]);
        btn->setProperty("engine", engines[i]);
        if (i == 0) btn->setChecked(true);
        search_group_->addButton(btn);
        layout->addWidget(btn);
    }

    layout->addStretch();
    return step;
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
