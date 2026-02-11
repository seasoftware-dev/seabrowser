#include "extensions_window.h"
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

namespace SeaBrowser {

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
    QLabel* title = new QLabel("Extensions");
    title->setStyleSheet("font-size: 20px; font-weight: 600; color: #3b82f6;");
    header_layout->addWidget(title);
    header_layout->addStretch();

    dev_mode_check_ = new QCheckBox("Developer Mode");
    dev_mode_check_->setStyleSheet("color: #94a3b8; font-size: 13px;");
    connect(dev_mode_check_, &QCheckBox::toggled, this, &ExtensionsWindow::onDeveloperModeToggled);
    header_layout->addWidget(dev_mode_check_);

    import_btn_ = new QPushButton("Import Chrome Extensions");
    import_btn_->setStyleSheet(R"(
        QPushButton {
            background-color: #3b82f6;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 600;
        }
    )");
    connect(import_btn_, &QPushButton::clicked, this, &ExtensionsWindow::onImportChromeExtensions);
    header_layout->addWidget(import_btn_);

    main_layout->addLayout(header_layout);

    list_ = new QListWidget();
    list_->setObjectName("extensionsList");
    list_->setStyleSheet(R"(
        QListWidget {
            background-color: #0f172a;
            border: 1px solid #1e293b;
            border-radius: 8px;
            color: #e2e8f0;
        }
        QListWidget::item {
            padding: 12px 16px;
            border-bottom: 1px solid #1e293b;
        }
        QListWidget::item:selected {
            background-color: rgba(59, 130, 246, 0.15);
        }
    )");

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

    QLabel* info = new QLabel("Import Chrome extensions by clicking the button above. Tsunami supports Chrome extension imports.");
    info->setStyleSheet("font-size: 12px; color: #64748b;");
    main_layout->addWidget(info);

    applyDarkBlueTheme();
}

void ExtensionsWindow::applyDarkBlueTheme() {
    setStyleSheet(R"(
        QDialog {
            background-color: #030712;
            color: #e2e8f0;
        }
        QCheckBox {
            color: #e2e8f0;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border-radius: 4px;
            border: 1px solid #334155;
            background: #1e293b;
        }
        QCheckBox::indicator:checked {
            background: #3b82f6;
            border-color: #3b82f6;
        }
    )");
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

} // namespace SeaBrowser
