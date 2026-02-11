#include "bookmarks_window.h"
#include "../settings/settings.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

namespace Tsunami {

BookmarksWindow::BookmarksWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Bookmarks - Tsunami");
    setMinimumSize(700, 500);
    resize(800, 500);
    setModal(true);

    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(16, 16, 16, 16);
    main_layout->setSpacing(12);

    QHBoxLayout* header_layout = new QHBoxLayout();
    title_ = new QLabel("Bookmarks");
    header_layout->addWidget(title_);
    header_layout->addStretch();

    search_edit_ = new QLineEdit();
    search_edit_->setPlaceholderText("Search bookmarks...");
    search_edit_->setFixedWidth(200);
    header_layout->addWidget(search_edit_);

    add_btn_ = new QPushButton("Add Bookmark");
    connect(add_btn_, &QPushButton::clicked, this, &BookmarksWindow::onAddBookmark);
    header_layout->addWidget(add_btn_);

    delete_btn_ = new QPushButton("Delete");
    connect(delete_btn_, &QPushButton::clicked, this, &BookmarksWindow::onDeleteBookmark);
    header_layout->addWidget(delete_btn_);

    main_layout->addLayout(header_layout);

    table_ = new QTableWidget(0, 3);
    table_->setObjectName("bookmarksTable");
    table_->setHorizontalHeaderLabels(QStringList() << "Title" << "URL" << "Folder");
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setAlternatingRowColors(true);

    connect(table_, &QTableWidget::itemDoubleClicked, this, &BookmarksWindow::onItemDoubleClicked);
    main_layout->addWidget(table_);

    applyTheme();
    connect(&Settings::instance(), &Settings::settingsChanged, this, &BookmarksWindow::applyTheme);
}

void BookmarksWindow::applyTheme() {
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

    search_edit_->setStyleSheet(QString(R"(
        QLineEdit {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 8px 12px;
        }
    )").arg(inputBg, textColor, borderColor));

    add_btn_->setStyleSheet(QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 600;
        }
    )").arg(accentColor));

    delete_btn_->setStyleSheet(QString(R"(
        QPushButton {
            background-color: rgba(239, 68, 68, 0.15);
            color: #dc2626;
            border: 1px solid rgba(239, 68, 68, 0.3);
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 600;
        }
    )"));

    table_->setStyleSheet(QString(R"(
        QTableWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 8px;
            color: %3;
            font-size: 13px;
        }
        QTableWidget::item {
            padding: 10px 12px;
            border-bottom: 1px solid %2;
        }
        QHeaderView::section {
            background-color: %4;
            color: %5;
            padding: 10px 12px;
            font-weight: 600;
            font-size: 12px;
            text-transform: uppercase;
            border-bottom: 1px solid %2;
        }
    )").arg(inputBg, borderColor, textColor, headerBg, headerText));

    setStyleSheet(QString(R"(
        QDialog {
            background-color: %1;
            color: %2;
        }
    )").arg(bgColor, textColor));
}

void BookmarksWindow::onAddBookmark() {
    QString url = QInputDialog::getText(this, "Add Bookmark", "Enter URL:");
    if (!url.isEmpty()) {
        int row = table_->rowCount();
        table_->insertRow(row);
        table_->setItem(row, 0, new QTableWidgetItem(url.split("/").last()));
        table_->setItem(row, 1, new QTableWidgetItem(url));
        table_->setItem(row, 2, new QTableWidgetItem("General"));
    }
}

void BookmarksWindow::onDeleteBookmark() {
    int row = table_->currentRow();
    if (row >= 0) {
        table_->removeRow(row);
    }
}

void BookmarksWindow::onItemDoubleClicked(QTableWidgetItem* item) {
    Q_UNUSED(item)
    int row = table_->currentRow();
    QTableWidgetItem* urlItem = table_->item(row, 1);
    if (urlItem) {
        accept();
    }
}

} // namespace Tsunami
