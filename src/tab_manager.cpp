/*
 * Sea Browser - Tab Manager (GTK3)
 * tab_manager.cpp
 */

#include "tab_manager.h"
#include "web_view.h"
#include "settings/settings_manager.h"
#include "history/history_manager.h"
#include "browser_window.h" // For config functions
#include <iostream>

namespace SeaBrowser {

// Forward declare helper from browser_window
// In a fuller refactor we'd move this to a shared utils file
extern void configure_web_view(WebKitWebView* view);

struct TabData {
    TabManager* manager;
    GtkWidget* tab_label;
    GtkWidget* spinner;
    GtkWidget* web_view;
};

TabManager::TabManager(GtkNotebook* notebook, GtkEntry* url_entry, GtkWindow* window) 
    : notebook_(notebook), url_entry_(url_entry), window_(window) {
    
    g_signal_connect(notebook_, "switch-page", G_CALLBACK(on_tab_switched), this);
}

GtkWidget* TabManager::create_tab(const std::string& url, bool switch_to) {
    auto web_view = webkit_web_view_new();
    configure_web_view(WEBKIT_WEB_VIEW(web_view));
    
    // Scrolled window for viewport
    auto scrolled = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_container_add(GTK_CONTAINER(scrolled), web_view);
    
    // Tab Label Widget (HBox with Spinner, Label, Close Button)
    auto tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    auto spinner = gtk_spinner_new();
    auto label = gtk_label_new("New Tab");
    auto close_btn = gtk_button_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_MENU);
    
    gtk_widget_set_tooltip_text(close_btn, "Close Tab");
    gtk_button_set_relief(GTK_BUTTON(close_btn), GTK_RELIEF_NONE);
    
    gtk_box_pack_start(GTK_BOX(tab_box), spinner, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_box), close_btn, FALSE, FALSE, 0);
    gtk_widget_show_all(tab_box);
    
    // Store data for callbacks
    auto* data = new TabData{this, label, spinner, web_view};
    g_object_set_data(G_OBJECT(scrolled), "tab-data", data);
    
    // Signals
    g_signal_connect(close_btn, "clicked", G_CALLBACK(on_tab_close_clicked), scrolled);
    g_signal_connect(web_view, "load-changed", G_CALLBACK(on_load_changed), data);
    g_signal_connect(web_view, "notify::title", G_CALLBACK(on_title_changed), data);
    
    // Mouse Navigation (Back/Forward buttons)
    g_signal_connect(web_view, "button-press-event", G_CALLBACK(+[](GtkWidget*, GdkEventButton* event, gpointer data) -> gboolean {
        auto* tab_data = static_cast<TabData*>(data);
        auto* view = WEBKIT_WEB_VIEW(tab_data->web_view);
        
        if (event->type == GDK_BUTTON_PRESS) {
            if (event->button == 8) { // Back button
                if (webkit_web_view_can_go_back(view)) {
                    webkit_web_view_go_back(view);
                    return TRUE;
                }
            } else if (event->button == 9) { // Forward button
                if (webkit_web_view_can_go_forward(view)) {
                    webkit_web_view_go_forward(view);
                    return TRUE;
                }
            }
        }
        return FALSE;
    }), data);
    
    int index = gtk_notebook_append_page(notebook_, scrolled, tab_box);
    gtk_widget_show_all(scrolled);
    
    if (switch_to) {
        gtk_notebook_set_current_page(notebook_, index);
    }
    
    // Register script message handler for settings
    auto content_manager = webkit_web_view_get_user_content_manager(WEBKIT_WEB_VIEW(web_view));
    
    // Settings message handler
    g_signal_connect(content_manager, "script-message-received::settings", 
        G_CALLBACK(+[](WebKitUserContentManager*, WebKitJavascriptResult* js_result, gpointer) {
            JSCValue* value = webkit_javascript_result_get_js_value(js_result);
            
            if (jsc_value_is_object(value)) {
                JSCValue* key_val = jsc_value_object_get_property(value, "key");
                JSCValue* val_val = jsc_value_object_get_property(value, "value");
                
                if (key_val && val_val) {
                    char* key = jsc_value_to_string(key_val);
                    char* val = jsc_value_to_string(val_val);
                    
                    std::cout << "[SeaBrowser] Setting: " << key << " = " << val << std::endl;
                    SettingsManager::instance().set_value(key, val);
                    
                    g_free(key);
                    g_free(val);
                }
            }
        }), nullptr);
    webkit_user_content_manager_register_script_message_handler(content_manager, "settings");
    
    // History clear message handler
    g_signal_connect(content_manager, "script-message-received::history",
        G_CALLBACK(+[](WebKitUserContentManager*, WebKitJavascriptResult* js_result, gpointer) {
            JSCValue* value = webkit_javascript_result_get_js_value(js_result);
            if (jsc_value_is_object(value)) {
                JSCValue* action_val = jsc_value_object_get_property(value, "action");
                if (action_val) {
                    char* action = jsc_value_to_string(action_val);
                    if (strcmp(action, "clear") == 0) {
                        HistoryManager::instance().clear_history();
                        std::cout << "[SeaBrowser] History cleared" << std::endl;
                    }
                    g_free(action);
                }
            }
        }), nullptr);
    webkit_user_content_manager_register_script_message_handler(content_manager, "history");
    
    // Load the URL - use load_uri for ALL URLs including sea:// so the scheme handler is triggered
    if (!url.empty()) {
        std::cout << "[SeaBrowser] Tab loading: " << url << std::endl;
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web_view), url.c_str());
    }
    
    return scrolled;
}

void TabManager::close_current_tab() {
    int page = gtk_notebook_get_current_page(notebook_);
    if (page != -1) {
        gtk_notebook_remove_page(notebook_, page);
        
        // precise behavior: close window if no tabs left
        if (gtk_notebook_get_n_pages(notebook_) == 0) {
            gtk_window_close(window_);
        }
    }
}

void TabManager::close_tab(GtkWidget* page) {
    int page_num = gtk_notebook_page_num(notebook_, page);
    if (page_num != -1) {
        gtk_notebook_remove_page(notebook_, page_num);
        
        // precise behavior: close window if no tabs left
        if (gtk_notebook_get_n_pages(notebook_) == 0) {
            gtk_window_close(window_);
        }
    }
}

WebKitWebView* TabManager::get_current_web_view() {
    int page = gtk_notebook_get_current_page(notebook_);
    if (page == -1) return nullptr;
    
    auto scrolled = gtk_notebook_get_nth_page(notebook_, page);
    auto data = (TabData*)g_object_get_data(G_OBJECT(scrolled), "tab-data");
    return WEBKIT_WEB_VIEW(data->web_view);
}

int TabManager::get_tab_count() {
    return gtk_notebook_get_n_pages(notebook_);
}

void TabManager::on_tab_switched(GtkNotebook*, GtkWidget* page, guint, gpointer user_data) {
    auto self = static_cast<TabManager*>(user_data);
    auto data = (TabData*)g_object_get_data(G_OBJECT(page), "tab-data");
    
    if (data && data->web_view) {
        // Update URL bar
        auto uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(data->web_view));
        if (uri && !g_str_has_prefix(uri, "sea://")) {
             gtk_entry_set_text(self->url_entry_, uri);
             gtk_entry_set_progress_fraction(self->url_entry_, 0.0); // Reset
        } else {
             gtk_entry_set_text(self->url_entry_, "");
        }
        
        // Update window title
        auto title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(data->web_view));
        std::string win_title = title ? std::string(title) + " - Sea Browser" : "Sea Browser";
        gtk_window_set_title(self->window_, win_title.c_str());
    }
}

void TabManager::on_tab_close_clicked(GtkButton*, gpointer user_data) {
    auto page = GTK_WIDGET(user_data);
    auto data = (TabData*)g_object_get_data(G_OBJECT(page), "tab-data");
    data->manager->close_tab(page);
}

void TabManager::on_load_changed(WebKitWebView* view, WebKitLoadEvent event, gpointer user_data) {
    auto data = static_cast<TabData*>(user_data);
    
    if (event == WEBKIT_LOAD_STARTED) {
        gtk_spinner_start(GTK_SPINNER(data->spinner));
    } else if (event == WEBKIT_LOAD_FINISHED) {
        gtk_spinner_stop(GTK_SPINNER(data->spinner));
    }
    
    // Only update URL bar if this is the ACTIVE tab
    if (data->manager->get_current_web_view() == view) {
        if (event == WEBKIT_LOAD_COMMITTED) {
            auto uri = webkit_web_view_get_uri(view);
            if (uri && !g_str_has_prefix(uri, "sea://")) {
                gtk_entry_set_text(data->manager->url_entry_, uri);
                
                // Add to history
                auto title = webkit_web_view_get_title(view);
                HistoryManager::instance().add_visit(uri, title ? title : uri);
            }
        }
    }
}

void TabManager::on_title_changed(GObject* object, GParamSpec*, gpointer user_data) {
    auto view = WEBKIT_WEB_VIEW(object);
    auto data = static_cast<TabData*>(user_data);
    auto title = webkit_web_view_get_title(view);
    
    if (title) {
        // Truncate for tab label if too long
        std::string s_title = title;
        if (s_title.empty()) s_title = "New Tab";
        if (s_title.length() > 20) s_title = s_title.substr(0, 17) + "...";
        gtk_label_set_text(GTK_LABEL(data->tab_label), s_title.c_str());
        
        // Update window title if active
        if (data->manager->get_current_web_view() == view) {
            std::string win_title = std::string(title) + " - Sea Browser";
            gtk_window_set_title(data->manager->window_, win_title.c_str());
        }
    }
}

} // namespace SeaBrowser
