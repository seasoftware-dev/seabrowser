/*
 * Sea Browser - Privacy-focused web browser
 * browser_window.h - Main browser window (GTK3)
 */

#pragma once

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <string>
#include <vector>
#include <memory>
#include "tab_manager.h"

namespace SeaBrowser {

// Make this available to tab_manager
void configure_web_view(WebKitWebView* view);

class BrowserWindow {
public:
    static GtkWidget* create(GtkApplication* app);
    
private:
    static GtkWidget* setup_header_bar(GtkWidget* window, GtkWidget* web_view, GtkWidget* url_entry);
    static void on_url_activate(GtkEntry* entry, gpointer user_data);
    static void on_back_clicked(GtkButton* button, gpointer user_data);
    static void on_forward_clicked(GtkButton* button, gpointer user_data);
    static void on_reload_clicked(GtkButton* button, gpointer user_data);
    static void on_home_clicked(GtkButton* button, gpointer user_data);
    static void on_new_tab_clicked(GtkButton* button, gpointer user_data);
    static void on_settings_clicked(GtkMenuItem* item, gpointer user_data);
    
    static std::string process_url_input(const std::string& input);
    static bool is_url(const std::string& input);
    static std::string get_search_url(const std::string& query);
};

} // namespace SeaBrowser
