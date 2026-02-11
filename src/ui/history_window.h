#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QHeaderView>

namespace Tsunami {

class HistoryWindow : public QDialog {
    Q_OBJECT
public:
    explicit HistoryWindow(QWidget* parent = nullptr);
    void applyTheme();

private slots:
    void onClearHistory();
    void onItemDoubleClicked(QTableWidgetItem* item);

private:
    QLabel* title_;
    QLineEdit* search_edit_;
    QTableWidget* table_;
    QPushButton* clear_btn_;
};

} // namespace Tsunami
