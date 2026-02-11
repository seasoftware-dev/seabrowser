#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHeaderView>

namespace SeaBrowser {

class HistoryWindow : public QDialog {
    Q_OBJECT
public:
    explicit HistoryWindow(QWidget* parent = nullptr);
    void applyDarkBlueTheme();

private slots:
    void onClearHistory();
    void onItemDoubleClicked(QTableWidgetItem* item);

private:
    QTableWidget* table_;
    QPushButton* clear_btn_;
    QLineEdit* search_edit_;
};

} // namespace SeaBrowser
