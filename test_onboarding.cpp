#include <QApplication>
#include <QTimer>
#include <QMessageBox>
#include "src/settings/settings.h"
#include "src/ui/onboarding_dialog.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Test settings functionality
    QMessageBox::information(nullptr, "Test", "Testing settings functionality...");
    
    // Create onboarding dialog
    Tsunami::OnboardingDialog onboarding;
    onboarding.show();
    
    return app.exec();
}