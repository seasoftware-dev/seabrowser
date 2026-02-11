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

class BookmarksWindow : public QDialog {
    Q_OBJECT
public:
    explicit BookmarksWindow(QWidget* parent = nullptr);
    void applyTheme();

private slots:
    void onAddBookmark();
    void onDeleteBookmark();
    void onItemDoubleClicked(QTableWidgetItem* item);

private:
    QLabel* title_;
    QLineEdit* search_edit_;
    QTableWidget* table_;
    QPushButton* add_btn_;
    QPushButton* delete_btn_;
};

} // namespace Tsunami
