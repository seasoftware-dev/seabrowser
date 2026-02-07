/*
 * Sea Browser - Tab Manager (GTK3)
 * tab_manager.h
 */

#pragma once

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <vector>
#include <string>

namespace SeaBrowser {

class TabManager {
public:
    explicit TabManager(GtkNotebook* notebook, GtkEntry* url_entry, GtkWindow* window);
    
    // Core actions
    GtkWidget* create_tab(const std::string& url, bool switch_to = true);
    void close_current_tab();
    void close_tab(GtkWidget* page);
    
    // Getters
    WebKitWebView* get_current_web_view();
    GtkWidget* get_current_tab_widget();
    int get_tab_count();
    GtkNotebook* get_notebook() const { return notebook_; }

private:
    GtkNotebook* notebook_;
    GtkEntry* url_entry_;
    GtkWindow* window_;
    
    // Callbacks
    static void on_tab_switched(GtkNotebook* notebook, GtkWidget* page, guint page_num, gpointer user_data);
    static void on_tab_close_clicked(GtkButton* btn, gpointer user_data);
    static void on_load_changed(WebKitWebView* view, WebKitLoadEvent event, gpointer user_data);
    static void on_title_changed(GObject* object, GParamSpec* pspec, gpointer user_data);
};

} // namespace SeaBrowser
