#include "src/settings/settings.h"
#include "src/ui/onboarding_dialog.h"
#include "src/browser_window.h"
#include <QApplication>
#include <QTimer>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Create browser window
    Tsunami::BrowserWindow window;
    window.show();
    
    // Create settings dialog
    Tsunami::SettingsDialog settingsDialog;
    
    // Test settings functionality
    QTimer::singleShot(2000, [&]() {
        // Open settings dialog
        settingsDialog.showDialog(&window);
    });
    
    return app.exec();
}