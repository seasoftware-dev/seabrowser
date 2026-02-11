#pragma once

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>

namespace Tsunami {

class DownloadsWindow : public QDialog {
    Q_OBJECT
public:
    explicit DownloadsWindow(QWidget* parent = nullptr);
    void applyTheme();

private slots:
    void onOpenFolder();
    void onClearCompleted();
    void onItemActivated(QTableWidgetItem* item);

private:
    QLabel* title_;
    QTableWidget* table_;
    QPushButton* open_folder_btn_;
    QPushButton* clear_btn_;
};

} // namespace Tsunami
