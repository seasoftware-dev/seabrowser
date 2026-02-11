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

class BookmarksWindow : public QDialog {
    Q_OBJECT
public:
    explicit BookmarksWindow(QWidget* parent = nullptr);
    void applyDarkBlueTheme();

private slots:
    void onAddBookmark();
    void onDeleteBookmark();
    void onItemDoubleClicked(QTableWidgetItem* item);

private:
    QTableWidget* table_;
    QPushButton* add_btn_;
    QPushButton* delete_btn_;
    QLineEdit* search_edit_;
};

} // namespace SeaBrowser
