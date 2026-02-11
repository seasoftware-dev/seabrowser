#include "history_window.h"
#include "../settings/settings.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QDateTime>

namespace Tsunami {

HistoryWindow::HistoryWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("History - Tsunami");
    setMinimumSize(700, 500);
    resize(800, 500);
    setModal(true);

    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(16, 16, 16, 16);
    main_layout->setSpacing(12);

    QHBoxLayout* header_layout = new QHBoxLayout();
    title_ = new QLabel("History");
    header_layout->addWidget(title_);
    header_layout->addStretch();

    search_edit_ = new QLineEdit();
    search_edit_->setPlaceholderText("Search history...");
    search_edit_->setFixedWidth(200);
    header_layout->addWidget(search_edit_);

    clear_btn_ = new QPushButton("Clear All");
    connect(clear_btn_, &QPushButton::clicked, this, &HistoryWindow::onClearHistory);
    header_layout->addWidget(clear_btn_);

    main_layout->addLayout(header_layout);

    table_ = new QTableWidget(10, 3);
    table_->setObjectName("historyTable");
    table_->setHorizontalHeaderLabels(QStringList() << "Title" << "URL" << "Visited");
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setAlternatingRowColors(true);

    QStringList sampleHistory = {
        "New Tab - Tsunami|tsunami://newtab|Just now",
        "Sea Software|https://seasoftware.dev|2 hours ago",
        "GitHub|https://github.com|Yesterday",
        "DuckDuckGo|https://duckduckgo.com|2 days ago"
    };

    for (int i = 0; i < sampleHistory.size(); ++i) {
        QStringList parts = sampleHistory[i].split("|");
        for (int j = 0; j < parts.size(); ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(parts[j]);
            table_->setItem(i, j, item);
        }
    }

    connect(table_, &QTableWidget::itemDoubleClicked, this, &HistoryWindow::onItemDoubleClicked);
    main_layout->addWidget(table_);

    applyTheme();
    connect(&Settings::instance(), &Settings::settingsChanged, this, &HistoryWindow::applyTheme);
}

void HistoryWindow::applyTheme() {
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

    clear_btn_->setStyleSheet(QString(R"(
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

void HistoryWindow::onClearHistory() {
    if (QMessageBox::warning(this, "Clear History",
        "Are you sure you want to clear all browsing history?",
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        table_->setRowCount(0);
        QMessageBox::information(this, "History", "Browsing history has been cleared.");
    }
}

void HistoryWindow::onItemDoubleClicked(QTableWidgetItem* item) {
    Q_UNUSED(item)
    int row = table_->currentRow();
    QTableWidgetItem* urlItem = table_->item(row, 1);
    if (urlItem) {
        accept();
    }
}

} // namespace Tsunami
