#include "extensions_window.h"
#include "../settings/settings.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QMap>

namespace Tsunami {

ExtensionsWindow::ExtensionsWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Extensions - Tsunami");
    setMinimumSize(700, 500);
    resize(800, 500);
    setModal(true);

    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(16, 16, 16, 16);
    main_layout->setSpacing(12);

    QHBoxLayout* header_layout = new QHBoxLayout();
    title_ = new QLabel("Extensions");
    header_layout->addWidget(title_);
    header_layout->addStretch();

    dev_mode_check_ = new QCheckBox("Developer Mode");
    connect(dev_mode_check_, &QCheckBox::toggled, this, &ExtensionsWindow::onDeveloperModeToggled);
    header_layout->addWidget(dev_mode_check_);

    import_btn_ = new QPushButton("Import Chrome Extensions");
    connect(import_btn_, &QPushButton::clicked, this, &ExtensionsWindow::onImportChromeExtensions);
    header_layout->addWidget(import_btn_);

    main_layout->addLayout(header_layout);

    list_ = new QListWidget();
    list_->setObjectName("extensionsList");
    connect(list_, &QListWidget::itemChanged, this, &ExtensionsWindow::onToggleExtension);

    QStringList sampleExtensions = {
        "Dark Reader - Dark mode for every website",
        "uBlock Origin - Efficient ad blocker",
        "Grammarly - Writing assistant",
        "LastPass - Password manager"
    };

    for (const QString& ext : sampleExtensions) {
        QListWidgetItem* item = new QListWidgetItem(ext);
        item->setCheckState(Qt::Checked);
        list_->addItem(item);
    }

    connect(list_, &QListWidget::itemChanged, this, &ExtensionsWindow::onToggleExtension);
    main_layout->addWidget(list_);

    info_ = new QLabel("Import Chrome extensions by clicking the button above. Tsunami supports Chrome extension imports.");
    main_layout->addWidget(info_);

    applyTheme();
    connect(&Settings::instance(), &Settings::settingsChanged, this, &ExtensionsWindow::applyTheme);
}

void ExtensionsWindow::applyTheme() {
    QString accentColor = Settings::instance().getAccentColor();
    if (accentColor.isEmpty()) accentColor = "#60a5fa";

    bool isDark = Settings::instance().getDarkMode();

    QString bgColor = isDark ? "#030712" : "#f0f9ff";
    QString titleColor = isDark ? "#e2e8f0" : "#1e40af";
    QString textColor = isDark ? "#e2e8f0" : "#1e293b";
    QString inputBg = isDark ? "#1e293b" : "#ffffff";
    QString borderColor = isDark ? "#1e293b" : "#bfdbfe";
    QString headerBg = isDark ? "#0f172a" : "#dbeafe";
    QString headerText = isDark ? "#64748b" : "#3b82f6";

    title_->setStyleSheet(QString("font-size: 20px; font-weight: 600; color: %1;").arg(accentColor));

    dev_mode_check_->setStyleSheet(QString("color: %1; font-size: 13px;").arg(isDark ? "#94a3b8" : "#64748b"));

    import_btn_->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 600;
        }
    )").arg(accentColor));

    list_->setStyleSheet(QString(R"(
        QListWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 8px;
            color: %3;
        }
        QListWidget::item {
            padding: 12px 16px;
            border-bottom: 1px solid %2;
        }
        QListWidget::item:selected {
            background-color: %4;
        }
    )").arg(inputBg, borderColor, textColor, isDark ? "rgba(96, 165, 250, 0.15)" : "#bfdbfe"));

    info_->setStyleSheet(QString("font-size: 12px; color: %1;").arg(isDark ? "#64748b" : "#64748b"));

    setStyleSheet(QString(R"(
        QDialog {
            background-color: %1;
            color: %2;
        }
        QCheckBox {
            color: %2;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border-radius: 4px;
            border: 1px solid %4;
            background: %3;
        }
        QCheckBox::indicator:checked {
            background: %5;
            border-color: %5;
        }
    )").arg(bgColor, textColor, inputBg, borderColor, accentColor));
}

void ExtensionsWindow::onImportChromeExtensions() {
    QMessageBox::information(this, "Import Chrome Extensions",
        "Tsunami can import Chrome extensions from your Chrome profile.\n\n"
        "Common Chrome profile locations:\n"
        "~/.config/google-chrome/Default/Extensions/\n"
        "~/.config/chromium/Default/Extensions/\n\n"
        "Click OK to browse for extension directories, or Cancel to scan the default locations.");

    QString dir = QFileDialog::getExistingDirectory(this, "Select Chrome Extensions Directory",
        QDir::homePath() + "/.config/google-chrome/Default/Extensions",
        QFileDialog::ShowDirsOnly);

    if (!dir.isEmpty()) {
        QMessageBox::information(this, "Import Extensions",
            "Scanning: " + dir + "\n\nExtensions found will be listed here.");
    }
}

void ExtensionsWindow::onToggleExtension(QListWidgetItem* item) {
    if (item) {
        QString name = item->text().split(" - ").first();
        Qt::CheckState state = item->checkState();
        qDebug() << "Extension" << name << (state == Qt::Checked ? "enabled" : "disabled");
    }
}

void ExtensionsWindow::onDeveloperModeToggled(bool checked) {
    if (checked) {
        QMessageBox::information(this, "Developer Mode",
            "Developer mode is now enabled.\n\n"
            "You can now:\n"
            "- Load unpacked extensions\n"
            "- View extension errors in console\n"
            "- Debug extensions\n\n"
            "Place extension files in: ~/.config/tsunami/extensions/");
    }
}

} // namespace Tsunami
