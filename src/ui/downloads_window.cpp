#include "downloads_window.h"
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

namespace SeaBrowser {

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
    QLabel* title = new QLabel("Downloads");
    title->setStyleSheet("font-size: 20px; font-weight: 600; color: #e2e8f0;");
    header_layout->addWidget(title);
    header_layout->addStretch();

    open_folder_btn_ = new QPushButton("Open Folder");
    open_folder_btn_->setStyleSheet(R"(
        QPushButton {
            background-color: #3b82f6;
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
    )");
    connect(open_folder_btn_, &QPushButton::clicked, this, &DownloadsWindow::onOpenFolder);

    clear_btn_ = new QPushButton("Clear Completed");
    clear_btn_->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(239, 68, 68, 0.15);
            color: #ef4444;
            border: 1px solid rgba(239, 68, 68, 0.3);
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: rgba(239, 68, 68, 0.25);
        }
    )");
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
    table_->setStyleSheet(R"(
        QTableWidget {
            background-color: #0f172a;
            border: 1px solid #1e293b;
            border-radius: 8px;
            gridline-color: #1e293b;
            color: #e2e8f0;
            font-size: 13px;
        }
        QTableWidget::item {
            padding: 10px 12px;
            border-bottom: 1px solid #1e293b;
        }
        QTableWidget::item:selected {
            background-color: rgba(59, 130, 246, 0.2);
        }
        QHeaderView::section {
            background-color: #0f172a;
            color: #64748b;
            padding: 10px 12px;
            font-weight: 600;
            font-size: 12px;
            text-transform: uppercase;
            border-bottom: 1px solid #1e293b;
        }
        QScrollBar:vertical {
            background: #0f172a;
            width: 10px;
        }
        QScrollBar::handle:vertical {
            background: #334155;
            border-radius: 5px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: #475569;
        }
    )");

    main_layout->addWidget(table_);

    applyDarkBlueTheme();
}

void DownloadsWindow::applyDarkBlueTheme() {
    setStyleSheet(R"(
        QDialog {
            background-color: #030712;
            color: #e2e8f0;
        }
        QLabel {
            color: #e2e8f0;
        }
    )");
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

} // namespace SeaBrowser
