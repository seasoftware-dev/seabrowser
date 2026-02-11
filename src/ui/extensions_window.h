#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

namespace Tsunami {

class ExtensionsWindow : public QDialog {
    Q_OBJECT
public:
    explicit ExtensionsWindow(QWidget* parent = nullptr);
    void applyTheme();

private slots:
    void onImportChromeExtensions();
    void onToggleExtension(QListWidgetItem* item);
    void onDeveloperModeToggled(bool checked);

private:
    QLabel* title_;
    QCheckBox* dev_mode_check_;
    QPushButton* import_btn_;
    QListWidget* list_;
    QLabel* info_;
};

} // namespace Tsunami
