#include "bookmarks_window.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

namespace SeaBrowser {

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
    QLabel* title = new QLabel("Bookmarks");
    title->setStyleSheet("font-size: 20px; font-weight: 600; color: #3b82f6;");
    header_layout->addWidget(title);
    header_layout->addStretch();

    search_edit_ = new QLineEdit();
    search_edit_->setPlaceholderText("Search bookmarks...");
    search_edit_->setFixedWidth(200);
    search_edit_->setStyleSheet(R"(
        QLineEdit {
            background-color: #1e293b;
            color: #e2e8f0;
            border: 1px solid #334155;
            border-radius: 8px;
            padding: 8px 12px;
        }
    )");
    header_layout->addWidget(search_edit_);

    add_btn_ = new QPushButton("Add Bookmark");
    add_btn_->setStyleSheet(R"(
        QPushButton {
            background-color: #3b82f6;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 600;
        }
    )");
    connect(add_btn_, &QPushButton::clicked, this, &BookmarksWindow::onAddBookmark);
    header_layout->addWidget(add_btn_);

    delete_btn_ = new QPushButton("Delete");
    delete_btn_->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(239, 68, 68, 0.15);
            color: #ef4444;
            border: 1px solid rgba(239, 68, 68, 0.3);
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: 600;
        }
    )");
    connect(delete_btn_, &QPushButton::clicked, this, &BookmarksWindow::onDeleteBookmark);
    header_layout->addWidget(delete_btn_);

    main_layout->addLayout(header_layout);

    table_ = new QTableWidget(0, 3);
    table_->setObjectName("bookmarksTable");
    table_->setHorizontalHeaderLabels(QStringList() << "Title" << "URL" << "Folder");
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setAlternatingRowColors(true);

    table_->setStyleSheet(R"(
        QTableWidget {
            background-color: #0f172a;
            border: 1px solid #1e293b;
            border-radius: 8px;
            color: #e2e8f0;
            font-size: 13px;
        }
        QTableWidget::item {
            padding: 10px 12px;
            border-bottom: 1px solid #1e293b;
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
    )");

    connect(table_, &QTableWidget::itemDoubleClicked, this, &BookmarksWindow::onItemDoubleClicked);
    main_layout->addWidget(table_);

    applyDarkBlueTheme();
}

void BookmarksWindow::applyDarkBlueTheme() {
    setStyleSheet(R"(
        QDialog {
            background-color: #030712;
            color: #e2e8f0;
        }
    )");
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

} // namespace SeaBrowser
