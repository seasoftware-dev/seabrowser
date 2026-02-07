/*
 * Sea Browser - Settings Dialog (GTK3)
 * settings_dialog.cpp
 */

#include "settings_dialog.h"
#include "settings_manager.h"
#include <gtk/gtk.h>

namespace SeaBrowser {

void SettingsDialog::show(GtkWindow* parent) {
    // Create dialog with transient parent
    auto dialog = gtk_dialog_new_with_buttons("Settings", parent,
        (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_USE_HEADER_BAR),
        nullptr, nullptr);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 800, 600);
    gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
    
    auto content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    auto stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_UP_DOWN);
    
    auto stack_sidebar = gtk_stack_sidebar_new();
    gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(stack_sidebar), GTK_STACK(stack));
    // Set size request for sidebar
    gtk_widget_set_size_request(stack_sidebar, 200, -1);
    
    auto hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(hbox), stack_sidebar, FALSE, FALSE, 0);
    
    // Separator
    auto sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(hbox), sep, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(hbox), stack, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(content_area), hbox);
    
    // --- General Page ---
    auto general_page = create_page("General", "preferences-system-symbolic");
    
    auto startup_section = create_section(general_page, "Startup");
    auto homepage_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(homepage_entry), SettingsManager::instance().general().homepage.c_str());
    add_row(startup_section, homepage_entry, "Homepage", "Page to open on startup/new tab");
    
    gtk_stack_add_titled(GTK_STACK(stack), general_page, "general", "General");
    
    // --- Appearance Page ---
    auto appearance_page = create_page("Appearance", "preferences-desktop-theme-symbolic");
    auto theme_section = create_section(appearance_page, "Theme");
    
    auto theme_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(theme_combo), "system", "System Default");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(theme_combo), "dark", "Dark Mode");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(theme_combo), "light", "Light Mode");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(theme_combo), 
        SettingsManager::instance().appearance().theme.c_str());
    g_signal_connect(theme_combo, "changed", G_CALLBACK(on_theme_changed), nullptr);
    
    add_row(theme_section, theme_combo, "Browser Theme", "Choose the visual style of the browser");
    gtk_stack_add_titled(GTK_STACK(stack), appearance_page, "appearance", "Appearance");

    // --- Privacy Page ---
    auto privacy_page = create_page("Privacy & Security", "security-high-symbolic");
    auto privacy_section = create_section(privacy_page, "Tracking Protection");
    
    auto tp_switch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(tp_switch), 
        SettingsManager::instance().privacy().tracking_protection != TrackingProtection::Off);
    g_signal_connect(tp_switch, "notify::active", G_CALLBACK(on_tracking_protection_toggled), nullptr);
    
    add_row(privacy_section, tp_switch, "Enhanced Tracking Protection", 
        "Block known trackers, fingerprinting, and third-party cookies");

    auto ua_section = create_section(privacy_page, "Browser Identification");
    auto ua_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ua_combo), "firefox", "Firefox (Recommended for Privacy)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ua_combo), "chrome", "Chrome (Best Compatibility)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(ua_combo), "safari", "Safari (Native WebKit)");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(ua_combo), 
        SettingsManager::instance().general().user_agent.c_str());
    g_signal_connect(ua_combo, "changed", G_CALLBACK(on_user_agent_changed), nullptr);
    
    add_row(ua_section, ua_combo, "User Agent Control", 
        "Change how websites identify your browser. Use Firefox to reduce fingerprinting and Captchas.");
        
    gtk_stack_add_titled(GTK_STACK(stack), privacy_page, "privacy", "Privacy");
    
    // --- About Page ---
    auto about_page = create_page("About Sea Browser", "help-about-symbolic");
    auto about_section = create_section(about_page, "Version Information");
    
    auto version_label = gtk_label_new("Sea Browser v1.0.0");
    gtk_label_set_selectable(GTK_LABEL(version_label), TRUE);
    add_row(about_section, version_label, "Version");
    
    auto engine_label = gtk_label_new("WebKitGTK");
    add_row(about_section, engine_label, "Engine");
    
    gtk_stack_add_titled(GTK_STACK(stack), about_page, "about", "About");
    
    gtk_widget_show_all(dialog);
}

GtkWidget* SettingsDialog::create_page(const char* title, const char* icon_name) {
    (void)icon_name; // Unused for now
    auto scroll = gtk_scrolled_window_new(nullptr, nullptr);
    auto vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(vbox, 24);
    gtk_widget_set_margin_bottom(vbox, 24);
    gtk_widget_set_margin_start(vbox, 24);
    gtk_widget_set_margin_end(vbox, 24);
    
    auto title_label = gtk_label_new(title);
    auto attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    pango_attr_list_insert(attrs, pango_attr_scale_new(1.4));
    gtk_label_set_attributes(GTK_LABEL(title_label), attrs);
    pango_attr_list_unref(attrs);
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(scroll), vbox);
    
    return scroll; // Return scroll window as the page
}

GtkWidget* SettingsDialog::create_section(GtkWidget* page, const char* title) {
    // Helper extracting vbox from scrolled window
    GtkWidget* vbox = gtk_bin_get_child(GTK_BIN(page)); // viewport
    if (GTK_IS_VIEWPORT(vbox)) vbox = gtk_bin_get_child(GTK_BIN(vbox)); // actual box

    auto frame = gtk_frame_new(nullptr);
    auto box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(frame), box);
    
    auto label = gtk_label_new(title);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_margin_bottom(label, 6);
    gtk_style_context_add_class(gtk_widget_get_style_context(label), "dim-label");
    
    auto attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(GTK_LABEL(label), attrs);
    pango_attr_list_unref(attrs);

    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
    
    return box;
}

void SettingsDialog::add_row(GtkWidget* section, GtkWidget* widget, const char* title, const char* subtitle) {
    auto row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_size_request(row, -1, 50);
    gtk_container_set_border_width(GTK_CONTAINER(row), 12);
    
    auto labels_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    auto title_label = gtk_label_new(title);
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(labels_box), title_label, FALSE, FALSE, 0);
    
    if (subtitle) {
        auto subtitle_label = gtk_label_new(subtitle);
        gtk_widget_set_halign(subtitle_label, GTK_ALIGN_START);
        gtk_style_context_add_class(gtk_widget_get_style_context(subtitle_label), "dim-label");
        gtk_label_set_attributes(GTK_LABEL(subtitle_label), pango_attr_list_new()); // Clear attrs
        // Manual formatting via markup
        char* markup = g_strdup_printf("<small>%s</small>", subtitle);
        gtk_label_set_markup(GTK_LABEL(subtitle_label), markup);
        g_free(markup);
        gtk_box_pack_start(GTK_BOX(labels_box), subtitle_label, FALSE, FALSE, 0);
    }
    
    gtk_box_pack_start(GTK_BOX(row), labels_box, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(row), widget, FALSE, FALSE, 0);
    
    auto list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
    gtk_container_add(GTK_CONTAINER(list), row);
    gtk_box_pack_start(GTK_BOX(section), list, FALSE, FALSE, 0); // Pack into section box
}

void SettingsDialog::on_theme_changed(GtkComboBoxText* combo, gpointer) {
    const char* id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    if (id) SettingsManager::instance().set_theme(id);
}

void SettingsDialog::on_user_agent_changed(GtkComboBoxText* combo, gpointer) {
    const char* id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    if (id) SettingsManager::instance().general().user_agent = id;
    SettingsManager::instance().save();
}

void SettingsDialog::on_tracking_protection_toggled(GtkSwitch* sw, gpointer) {
    bool active = gtk_switch_get_active(sw);
    SettingsManager::instance().set_tracking_protection(
        active ? TrackingProtection::Strict : TrackingProtection::Off);
}

} // namespace SeaBrowser
