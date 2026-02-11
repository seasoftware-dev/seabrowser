#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

namespace SeaBrowser {

class ExtensionsWindow : public QDialog {
    Q_OBJECT
public:
    explicit ExtensionsWindow(QWidget* parent = nullptr);
    void applyDarkBlueTheme();

private slots:
    void onImportChromeExtensions();
    void onToggleExtension(QListWidgetItem* item);
    void onDeveloperModeToggled(bool checked);

private:
    QListWidget* list_;
    QPushButton* import_btn_;
    QCheckBox* dev_mode_check_;
    QList<QMap<QString, QString>> extensions_;
};

} // namespace SeaBrowser
