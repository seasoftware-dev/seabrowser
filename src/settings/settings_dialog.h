/*
 * Sea Browser - Settings Dialog (GTK3)
 * settings_dialog.h
 */

#pragma once

#include <gtk/gtk.h>
#include <string>

namespace SeaBrowser {

class SettingsDialog {
public:
    static void show(GtkWindow* parent);

private:
    static GtkWidget* create_page(const char* title, const char* icon_name);
    static GtkWidget* create_section(GtkWidget* page, const char* title);
    static void add_row(GtkWidget* section, GtkWidget* widget, const char* title, const char* subtitle = nullptr);
    
    // Callbacks
    static void on_theme_changed(GtkComboBoxText* combo, gpointer user_data);
    static void on_user_agent_changed(GtkComboBoxText* combo, gpointer user_data);
    static void on_tracking_protection_toggled(GtkSwitch* sw, gpointer user_data);
};

} // namespace SeaBrowser
