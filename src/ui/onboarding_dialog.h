#pragma once

#include <QDialog>
#include <QButtonGroup>
#include <QStackedWidget>
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

private:
    void setupUi();
    void applyTheme();
    void closeEvent(QCloseEvent* event) override;

    QWidget* createThemeStep();
    QWidget* createColorStep();
    QWidget* createSearchStep();

    QStackedWidget* stacked_widget_ = nullptr;
    QButtonGroup* theme_group_ = nullptr;
    QButtonGroup* color_group_ = nullptr;
    QButtonGroup* search_group_ = nullptr;
    QPushButton* back_btn_ = nullptr;
    QPushButton* next_btn_ = nullptr;
    int current_step_ = 0;
};

} // namespace Tsunami
