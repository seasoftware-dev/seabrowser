#include "downloads_window.h"
#include "../settings/settings.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QStyle>
#include <QMessageBox>
#include <QDir>
#include <QHeaderView>

namespace Tsunami {

DownloadsWindow::DownloadsWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Downloads - Tsunami");
    setMinimumSize(700, 500);
    resize(800, 550);

    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(16, 16, 16, 16);
    main_layout->setSpacing(12);

    QHBoxLayout* header_layout = new QHBoxLayout();
    title_ = new QLabel("Downloads");
    header_layout->addWidget(title_);
    header_layout->addStretch();

    open_folder_btn_ = new QPushButton("Open Folder");
    connect(open_folder_btn_, &QPushButton::clicked, this, &DownloadsWindow::onOpenFolder);

    clear_btn_ = new QPushButton("Clear Completed");
    connect(clear_btn_, &QPushButton::clicked, this, &DownloadsWindow::onClearCompleted);

    header_layout->addWidget(open_folder_btn_);
    header_layout->addWidget(clear_btn_);
    main_layout->addLayout(header_layout);

    table_ = new QTableWidget(0, 4);
    table_->setObjectName("downloadsTable");
    table_->setHorizontalHeaderLabels(QStringList() << "Name" << "Status" << "Size" << "Location");
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->verticalHeader()->setVisible(false);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setAlternatingRowColors(true);
    connect(table_, &QTableWidget::itemActivated, this, &DownloadsWindow::onItemActivated);

    main_layout->addWidget(table_);

    applyTheme();
    connect(&Settings::instance(), &Settings::settingsChanged, this, &DownloadsWindow::applyTheme);
}

void DownloadsWindow::applyTheme() {
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
    QString selectedBg = isDark ? "rgba(96, 165, 250, 0.2)" : "#bfdbfe";

    title_->setStyleSheet(QString("font-size: 20px; font-weight: 600; color: %1;").arg(titleColor));

    open_folder_btn_->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton:hover {
            opacity: 0.9;
        }
    )").arg(accentColor));

    clear_btn_->setStyleSheet(QString(R"(
        QPushButton {
            background-color: rgba(239, 68, 68, 0.15);
            color: #dc2626;
            border: 1px solid rgba(239, 68, 68, 0.3);
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: rgba(239, 68, 68, 0.25);
        }
    )"));

    table_->setStyleSheet(QString(R"(
        QTableWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 8px;
            gridline-color: %2;
            color: %3;
            font-size: 13px;
        }
        QTableWidget::item {
            padding: 10px 12px;
            border-bottom: 1px solid %2;
        }
        QTableWidget::item:selected {
            background-color: %4;
        }
        QHeaderView::section {
            background-color: %5;
            color: %6;
            padding: 10px 12px;
            font-weight: 600;
            font-size: 12px;
            text-transform: uppercase;
            border-bottom: 1px solid %2;
        }
        QScrollBar:vertical {
            background: %1;
            width: 10px;
        }
        QScrollBar::handle:vertical {
            background: %7;
            border-radius: 5px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: %8;
        }
    )").arg(inputBg, borderColor, textColor, selectedBg, headerBg, headerText,
           isDark ? "#334155" : "#93c5fd", isDark ? "#475569" : "#60a5fa"));

    setStyleSheet(QString(R"(
        QDialog {
            background-color: %1;
            color: %2;
        }
        QLabel {
            color: %2;
        }
    )").arg(bgColor, textColor));
}

void DownloadsWindow::onOpenFolder() {
    QMessageBox::information(this, "Downloads", "Opening downloads folder...");
}

void DownloadsWindow::onClearCompleted() {
    QMessageBox::information(this, "Downloads", "Clearing completed downloads...");
}

void DownloadsWindow::onItemActivated(QTableWidgetItem* item) {
    Q_UNUSED(item)
}

} // namespace Tsunami
