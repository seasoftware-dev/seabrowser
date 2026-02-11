#pragma once

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>

namespace SeaBrowser {

class DownloadsWindow : public QDialog {
    Q_OBJECT
public:
    explicit DownloadsWindow(QWidget* parent = nullptr);
    void applyDarkBlueTheme();

private slots:
    void onOpenFolder();
    void onClearCompleted();
    void onItemActivated(QTableWidgetItem* item);

private:
    QTableWidget* table_;
    QPushButton* open_folder_btn_;
    QPushButton* clear_btn_;
};

} // namespace SeaBrowser
