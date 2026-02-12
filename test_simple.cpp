#include <QApplication>
#include <QTimer>
#include <QMessageBox>
#include "src/settings/settings.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Test settings functionality
    QMessageBox::information(nullptr, "Test", "Testing settings functionality...");
    
    // Test settings
    auto& settings = Tsunami::Settings::instance();
    settings.setAccentColor("#ff0000");
    
    QMessageBox::information(nullptr, "Test", "Settings test completed!");
    
    return app.exec();
}