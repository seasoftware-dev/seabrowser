/*
 * Sea Browser - Settings Window
 * settings_window.h
 */

#pragma once

#include <adwaita.h>

namespace SeaBrowser {

class SettingsWindow {
public:
    static void show(GtkWindow* parent);

private:
    static GtkWidget* create_window(GtkWindow* parent);
};

} // namespace SeaBrowser
