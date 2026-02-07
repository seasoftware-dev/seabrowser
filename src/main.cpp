/*
 * Sea Browser - Privacy-focused web browser
 * main.cpp - Application entry point (GTK3 version)
 */

#include <cstdlib>
#include <iostream>

// Forward declaration
namespace SeaBrowser {
    int Application_run(int argc, char* argv[]);
}

// CRITICAL: Must set CSD environment variables BEFORE any GTK includes or calls
// This ensures the custom titlebar works on ALL desktop environments including KDE
void setup_environment() {
    // Force Client-Side Decorations (custom titlebar) - MUST be before gtk_init
    setenv("GTK_CSD", "1", 1);
    
    // Disable native window decorations for all GTK apps (helps with KDE)
    setenv("GTK_THEME", "", 1);
    
    // Force GTK to not use native decorations
    setenv("GTK_OVERLAY_SCROLLING", "1", 1);
    
    // KDE/Plasma specific: Force CSD even on KDE
    setenv("KWIN_FORCE_CSD", "1", 1);
    
    // Qt/GTK integration - disable Qt from interfering with GTK CSD
    setenv("QT_QPA_PLATFORMTHEME", "", 1);
    
    // Disable GTK from using server-side decorations
    setenv("GTK_USE_PORTAL", "0", 1);
    
    std::cout << "[SeaBrowser] Environment configured for custom titlebar" << std::endl;
}

int main(int argc, char* argv[]) {
    // Set environment variables FIRST, before ANY GTK calls
    setup_environment();
    
    // Forward to the actual application implementation
    return SeaBrowser::Application_run(argc, argv);
}
