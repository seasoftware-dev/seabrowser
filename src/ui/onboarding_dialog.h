#pragma once

#include <QDialog>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QString>
#include <QWidget>
#include <QCloseEvent>

namespace Tsunami {

class OnboardingDialog : public QDialog {
    Q_OBJECT
public:
    explicit OnboardingDialog(QWidget* parent = nullptr);

private slots:
    void onBack();
    void onNext();
    void updateButtons();
    void saveSettings();
    void onClose();
    void onMinimize();
    void onMaximize();

private:
    void setupUi();
    void applyTheme();
    void updateOptionButtons();
    void closeEvent(QCloseEvent* event) override;

    QStackedWidget* stacked_widget_ = nullptr;
    QButtonGroup* theme_group_ = nullptr;
    QButtonGroup* search_group_ = nullptr;
    QPushButton* back_btn_ = nullptr;
    QPushButton* next_btn_ = nullptr;
    QLabel* title_ = nullptr;
    QLabel* subtitle_ = nullptr;
    QVBoxLayout* theme_layout_ = nullptr;
    QVBoxLayout* search_layout_ = nullptr;
    QPushButton* min_btn_ = nullptr;
    QPushButton* max_btn_ = nullptr;
    QPushButton* close_btn_ = nullptr;
    int current_step_ = 0;
};

} // namespace Tsunami
