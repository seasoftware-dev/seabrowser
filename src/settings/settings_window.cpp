/*
 * Sea Browser - Settings Window
 * settings_window.cpp
 */

#include "settings_window.h"
#include "settings_manager.h"

namespace SeaBrowser {

void SettingsWindow::show(GtkWindow* parent) {
    auto window = create_window(parent);
    gtk_window_present(GTK_WINDOW(window));
}

GtkWidget* SettingsWindow::create_window(GtkWindow* parent) {
    auto window = adw_preferences_window_new();
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    
    auto& settings = SettingsManager::instance();
    
    // Privacy page
    auto privacy_page = adw_preferences_page_new();
    adw_preferences_page_set_title(privacy_page, "Privacy & Security");
    adw_preferences_page_set_icon_name(privacy_page, "security-high-symbolic");
    
    auto privacy_group = adw_preferences_group_new();
    adw_preferences_group_set_title(privacy_group, "Tracking Protection");
    
    auto tracking_row = adw_combo_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(tracking_row), "Tracking Protection Level");
    auto tracking_model = gtk_string_list_new(nullptr);
    gtk_string_list_append(tracking_model, "Off");
    gtk_string_list_append(tracking_model, "Standard");
    gtk_string_list_append(tracking_model, "Strict");
    adw_combo_row_set_model(ADW_COMBO_ROW(tracking_row), G_LIST_MODEL(tracking_model));
    adw_combo_row_set_selected(ADW_COMBO_ROW(tracking_row), static_cast<int>(settings.privacy().tracking_protection));
    adw_preferences_group_add(privacy_group, GTK_WIDGET(tracking_row));
    
    auto https_row = adw_switch_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(https_row), "HTTPS-Only Mode");
    adw_switch_row_set_active(ADW_SWITCH_ROW(https_row), settings.privacy().https_only);
    adw_preferences_group_add(privacy_group, GTK_WIDGET(https_row));
    
    auto dnt_row = adw_switch_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(dnt_row), "Send Do Not Track");
    adw_switch_row_set_active(ADW_SWITCH_ROW(dnt_row), settings.privacy().send_dnt);
    adw_preferences_group_add(privacy_group, GTK_WIDGET(dnt_row));
    
    adw_preferences_page_add(privacy_page, privacy_group);
    
    // Cookie group
    auto cookie_group = adw_preferences_group_new();
    adw_preferences_group_set_title(cookie_group, "Cookies");
    
    auto cookie_row = adw_combo_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(cookie_row), "Cookie Policy");
    auto cookie_model = gtk_string_list_new(nullptr);
    gtk_string_list_append(cookie_model, "Accept All");
    gtk_string_list_append(cookie_model, "Block Third-Party");
    gtk_string_list_append(cookie_model, "Block All");
    adw_combo_row_set_model(ADW_COMBO_ROW(cookie_row), G_LIST_MODEL(cookie_model));
    adw_combo_row_set_selected(ADW_COMBO_ROW(cookie_row), static_cast<int>(settings.privacy().cookie_policy));
    adw_preferences_group_add(cookie_group, GTK_WIDGET(cookie_row));
    
    adw_preferences_page_add(privacy_page, cookie_group);
    adw_preferences_window_add(ADW_PREFERENCES_WINDOW(window), privacy_page);
    
    // Search page
    auto search_page = adw_preferences_page_new();
    adw_preferences_page_set_title(search_page, "Search");
    adw_preferences_page_set_icon_name(search_page, "system-search-symbolic");
    
    auto search_group = adw_preferences_group_new();
    adw_preferences_group_set_title(search_group, "Default Search Engine");
    
    auto engine_row = adw_combo_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(engine_row), "Search Engine");
    auto engine_model = gtk_string_list_new(nullptr);
    gtk_string_list_append(engine_model, "Google");
    gtk_string_list_append(engine_model, "DuckDuckGo");
    gtk_string_list_append(engine_model, "Brave Search");
    gtk_string_list_append(engine_model, "Ecosia");
    gtk_string_list_append(engine_model, "Startpage");
    gtk_string_list_append(engine_model, "Qwant");
    adw_combo_row_set_model(ADW_COMBO_ROW(engine_row), G_LIST_MODEL(engine_model));
    adw_preferences_group_add(search_group, GTK_WIDGET(engine_row));
    
    adw_preferences_page_add(search_page, search_group);
    adw_preferences_window_add(ADW_PREFERENCES_WINDOW(window), search_page);
    
    return window;
}

} // namespace SeaBrowser
