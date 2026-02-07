/*
 * Sea Browser - Privacy-focused web browser
 * browser_window.cpp - Main browser window implementation (GTK3)
 */

#include "browser_window.h"
#include "web_view.h"
#include "tab_manager.h"
#include "history/history_manager.h"
#include "settings/settings_manager.h"
#include "settings/settings_dialog.h"
#include "extension_manager.h"
#include "privacy/content_blocker.h"
#include "downloads/downloads_manager.h"
#include <filesystem>
#include <regex>
#include <iostream>
#include <algorithm>
#include <gdk/gdk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

namespace SeaBrowser {

static TabManager* global_tab_manager = nullptr;

// Recently closed tabs stack
static std::vector<std::pair<std::string, std::string>> closed_tabs_stack;
static const size_t MAX_CLOSED_TABS = 10;

static const char* FIREFOX_USER_AGENT = 
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:134.0) Gecko/20100101 Firefox/134.0";
static const char* CHROME_USER_AGENT = 
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36";
static const char* SAFARI_USER_AGENT = 
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 14_7_4) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/18.3 Safari/605.1.15";

// X11 HACK: Forcefully remove decorations using MOTIF hints
#ifdef GDK_WINDOWING_X11
static void set_motif_hints_no_decorations(GtkWidget* window) {
    if (!window || !gtk_widget_get_realized(window)) return;
    GdkWindow* gdk_window = gtk_widget_get_window(window);
    if (!gdk_window || !GDK_IS_X11_WINDOW(gdk_window)) return;
    
    Display* display = GDK_DISPLAY_XDISPLAY(gdk_window_get_display(gdk_window));
    Window xwindow = GDK_WINDOW_XID(gdk_window);
    
    struct {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    } hints = {2, 0, 0, 0, 0};
    
    Atom motif_hints = XInternAtom(display, "_MOTIF_WM_HINTS", False);
    XChangeProperty(display, xwindow, motif_hints, motif_hints, 32,
                    PropModeReplace, (unsigned char*)&hints, 5);
    XFlush(display);
    std::cout << "[SeaBrowser] Applied MOTIF hints to hide decorations" << std::endl;
}

// Set additional KDE-specific properties
static void set_kde_properties(GtkWidget* window) {
    if (!window || !gtk_widget_get_realized(window)) return;
    GdkWindow* gdk_window = gtk_widget_get_window(window);
    if (!gdk_window || !GDK_IS_X11_WINDOW(gdk_window)) return;
    
    Display* display = GDK_DISPLAY_XDISPLAY(gdk_window_get_display(gdk_window));
    Window xwindow = GDK_WINDOW_XID(gdk_window);
    
    // Set _KDE_NET_WM_FORCE_CSD to force CSD on KDE
    Atom kde_csd = XInternAtom(display, "_KDE_NET_WM_FORCE_CSD", False);
    unsigned long force_csd = 1;
    XChangeProperty(display, xwindow, kde_csd, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*)&force_csd, 1);
    
    // Set _NET_WM_WINDOW_TYPE to NORMAL (not DIALOG)
    Atom wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom wm_window_type_normal = XInternAtom(display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    XChangeProperty(display, xwindow, wm_window_type, XA_ATOM, 32,
                    PropModeReplace, (unsigned char*)&wm_window_type_normal, 1);
    
    XFlush(display);
    std::cout << "[SeaBrowser] Applied KDE-specific window properties" << std::endl;
}
#endif

// Show find bar for current tab
static void show_find_bar(GtkWindow* window, WebKitWebView* web_view) {
    static GtkWidget* find_bar = nullptr;
    static GtkWidget* find_entry = nullptr;
    static WebKitWebView* current_web_view = nullptr;
    
    if (!find_bar) {
        find_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_style_context_add_class(gtk_widget_get_style_context(find_bar), "find-bar");
        gtk_widget_set_margin_start(find_bar, 10);
        gtk_widget_set_margin_end(find_bar, 10);
        gtk_widget_set_margin_top(find_bar, 6);
        gtk_widget_set_margin_bottom(find_bar, 6);
        
        find_entry = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(find_entry), "Find in page...");
        gtk_widget_set_hexpand(find_entry, TRUE);
        
        auto prev_btn = gtk_button_new_from_icon_name("go-up-symbolic", GTK_ICON_SIZE_BUTTON);
        auto next_btn = gtk_button_new_from_icon_name("go-down-symbolic", GTK_ICON_SIZE_BUTTON);
        auto close_btn = gtk_button_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_BUTTON);
        
        gtk_box_pack_start(GTK_BOX(find_bar), gtk_label_new("Find:"), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(find_bar), find_entry, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(find_bar), prev_btn, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(find_bar), next_btn, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(find_bar), close_btn, FALSE, FALSE, 0);
        
        // Find next
        g_signal_connect(next_btn, "clicked", G_CALLBACK(+[](GtkButton*, gpointer) {
            if (current_web_view && find_entry) {
                webkit_find_controller_search_next(
                    webkit_web_view_get_find_controller(current_web_view));
            }
        }), nullptr);
        
        // Find previous
        g_signal_connect(prev_btn, "clicked", G_CALLBACK(+[](GtkButton*, gpointer) {
            if (current_web_view && find_entry) {
                webkit_find_controller_search_previous(
                    webkit_web_view_get_find_controller(current_web_view));
            }
        }), nullptr);
        
        // Close find bar
        g_signal_connect(close_btn, "clicked", G_CALLBACK(+[](GtkButton*, gpointer win) {
            if (find_bar && current_web_view) {
                webkit_find_controller_search_finish(
                    webkit_web_view_get_find_controller(current_web_view));
                gtk_widget_hide(find_bar);
            }
        }), window);
        
        // Real-time search
        g_signal_connect(find_entry, "changed", G_CALLBACK(+[](GtkEditable*, gpointer) {
            if (current_web_view && find_entry) {
                const char* text = gtk_entry_get_text(GTK_ENTRY(find_entry));
                if (text && strlen(text) > 0) {
                    webkit_find_controller_search(
                        webkit_web_view_get_find_controller(current_web_view),
                        text, WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE, G_MAXUINT);
                } else {
                    webkit_find_controller_search_finish(
                        webkit_web_view_get_find_controller(current_web_view));
                }
            }
        }), nullptr);
        
        // Escape to close
        g_signal_connect(find_entry, "key-press-event", G_CALLBACK(+[](GtkWidget*, GdkEventKey* event, gpointer) -> gboolean {
            if (event->keyval == GDK_KEY_Escape && find_bar) {
                gtk_widget_hide(find_bar);
                return TRUE;
            }
            return FALSE;
        }), nullptr);
        
        gtk_widget_show_all(find_bar);
    }
    
    current_web_view = web_view;
    gtk_widget_show(find_bar);
    gtk_widget_grab_focus(find_entry);
    
    // Add to window if not already added
    auto vbox = gtk_bin_get_child(GTK_BIN(window));
    if (vbox && GTK_IS_BOX(vbox)) {
        // Check if already packed
        GList* children = gtk_container_get_children(GTK_CONTAINER(vbox));
        bool already_packed = false;
        for (GList* l = children; l != nullptr; l = l->next) {
            if (GTK_WIDGET(l->data) == find_bar) {
                already_packed = true;
                break;
            }
        }
        g_list_free(children);
        
        if (!already_packed) {
            gtk_box_pack_start(GTK_BOX(vbox), find_bar, FALSE, FALSE, 0);
            gtk_box_reorder_child(GTK_BOX(vbox), find_bar, 1); // After header, before notebook
        }
    }
}

void configure_web_view(WebKitWebView* view) {
    auto settings = webkit_web_view_get_settings(view);
    
    // Setup the JS bridge for settings/history communication
    WebView::setup_internal_page_bridge(view);
    
    // Inject user extensions
    ExtensionManager::instance().inject_extensions(view);
    std::string ua_setting = SettingsManager::instance().general().user_agent;
    if (ua_setting == "firefox") webkit_settings_set_user_agent(settings, FIREFOX_USER_AGENT);
    else if (ua_setting == "safari") webkit_settings_set_user_agent(settings, SAFARI_USER_AGENT);
    else webkit_settings_set_user_agent(settings, CHROME_USER_AGENT);
    
    webkit_settings_set_enable_javascript(settings, TRUE);
    webkit_settings_set_enable_developer_extras(settings, TRUE);
    webkit_settings_set_hardware_acceleration_policy(settings, WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS);
    
    // Privacy: ITP and Content Blocking
    g_object_set(settings, "enable-intelligent-tracking-prevention", TRUE, nullptr);
    g_object_set(settings, "enable-write-console-messages-to-stdout", TRUE, nullptr);
    
    // Smooth scrolling
    g_object_set(settings, "enable-smooth-scrolling", TRUE, nullptr);
    
    // Media playback
    g_object_set(settings, "enable-media-stream", TRUE, nullptr);
    g_object_set(settings, "enable-mediasource", TRUE, nullptr);
    
    // WebGL
    g_object_set(settings, "enable-webgl", TRUE, nullptr);
    
    // Initialize Content Blocker
    ContentBlocker::instance().load_filter_lists();
    
    g_signal_connect(view, "decide-policy", G_CALLBACK(+[](WebKitWebView* web_view, WebKitPolicyDecision* decision, WebKitPolicyDecisionType type, gpointer) -> gboolean {
        if (type != WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION) return FALSE;
        
        auto action = webkit_navigation_policy_decision_get_navigation_action(WEBKIT_NAVIGATION_POLICY_DECISION(decision));
        auto request = webkit_navigation_action_get_request(action);
        const char* uri = webkit_uri_request_get_uri(request);
        
        if (!uri) return FALSE;
        std::string url(uri);
        
        // 1. Content Blocking
        if (SettingsManager::instance().privacy().tracking_protection != TrackingProtection::Off) {
             if (ContentBlocker::instance().is_blocked(url)) {
                 std::cout << "[SeaBrowser] BLOCKED: " << url << std::endl;
                 webkit_policy_decision_ignore(decision);
                 return TRUE;
             }
        }
        
        // 2. HTTPS-Only Mode
        if (SettingsManager::instance().privacy().https_only && url.rfind("http://", 0) == 0) {
             std::cout << "[SeaBrowser] Upgrading to HTTPS: " << url << std::endl;
             std::string https_url = "https://" + url.substr(7);
             webkit_policy_decision_ignore(decision);
             webkit_web_view_load_uri(web_view, https_url.c_str());
             return TRUE;
        }
        
        return FALSE;
    }), nullptr);
    
    auto context = webkit_web_view_get_context(view);
    auto cookie_manager = webkit_web_context_get_cookie_manager(context);
    auto policy = SettingsManager::instance().privacy().cookie_policy;
    if (policy == CookiePolicy::BlockAll)
        webkit_cookie_manager_set_accept_policy(cookie_manager, WEBKIT_COOKIE_POLICY_ACCEPT_NEVER);
    else if (policy == CookiePolicy::AcceptAll)
        webkit_cookie_manager_set_accept_policy(cookie_manager, WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS);
    else
        webkit_cookie_manager_set_accept_policy(cookie_manager, WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY);
    
    // Handle downloads
    g_signal_connect(view, "download-started", G_CALLBACK(+[](WebKitWebView*, WebKitDownload* download, gpointer) {
        auto request = webkit_download_get_request(download);
        const char* uri = webkit_uri_request_get_uri(request);
        std::string url = uri ? uri : "";
        
        // Get suggested filename from response or URL
        const char* suggested_filename = nullptr;
        auto response = webkit_download_get_response(download);
        if (response) {
            suggested_filename = webkit_uri_response_get_suggested_filename(response);
        }
        std::string filename = suggested_filename ? suggested_filename : "";
        if (filename.empty()) {
            // Extract from URL
            size_t last_slash = url.find_last_of('/');
            if (last_slash != std::string::npos && last_slash < url.length() - 1) {
                filename = url.substr(last_slash + 1);
            }
        }
        
        std::cout << "[SeaBrowser] Download started: " << filename << " from " << url << std::endl;
        
        // Start tracking the download
        std::string download_id = DownloadsManager::instance().start_download(url, filename);
        
        // Set download destination
        Download* dl = DownloadsManager::instance().get_download(download_id);
        if (dl) {
            webkit_download_set_destination(download, ("file://" + dl->path).c_str());
        }
        
        // Connect to download progress
        g_signal_connect(download, "received-data", G_CALLBACK(+[](WebKitDownload* d, guint64 length, gpointer user_data) {
            std::string* id = static_cast<std::string*>(user_data);
            guint64 received = webkit_download_get_received_data_length(d);
            
            // Get total size from response
            guint64 total = 0;
            auto resp = webkit_download_get_response(d);
            if (resp) {
                total = webkit_uri_response_get_content_length(resp);
            }
            
            // Calculate speed (this is simplified)
            static guint64 last_received = 0;
            static GTimer* timer = nullptr;
            if (!timer) timer = g_timer_new();
            
            double elapsed = g_timer_elapsed(timer, nullptr);
            double speed = 0;
            if (elapsed > 0) {
                speed = (received - last_received) / elapsed;
                last_received = received;
                g_timer_start(timer);
            }
            
            DownloadsManager::instance().update_progress(*id, received, total, speed);
        }), new std::string(download_id));
        
        // Connect to download finished
        g_signal_connect(download, "finished", G_CALLBACK(+[](WebKitDownload*, gpointer user_data) {
            std::string* id = static_cast<std::string*>(user_data);
            DownloadsManager::instance().complete_download(*id);
            delete id;
        }), new std::string(download_id));
        
        // Connect to download failed
        g_signal_connect(download, "failed", G_CALLBACK(+[](WebKitDownload*, GError* error, gpointer user_data) {
            std::string* id = static_cast<std::string*>(user_data);
            std::string error_msg = error ? error->message : "Unknown error";
            DownloadsManager::instance().fail_download(*id, error_msg);
            delete id;
        }), new std::string(download_id));
    }), nullptr);
}

GtkWidget* BrowserWindow::create(GtkApplication* app) {
    auto window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Sea Browser");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    
    // Use CSD (Client Side Decorations) - We set our own titlebar so GTK handles the window manager
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    
    // Set class for CSS targeting
    gtk_style_context_add_class(gtk_widget_get_style_context(window), "sea-main-window");
    
    auto vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    auto url_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(url_entry), "Search or enter URL...");
    
    // Setup header bar via GtkHeaderBar
    auto header = setup_header_bar(window, nullptr, url_entry);
    
    // Set as the official titlebar
    gtk_window_set_titlebar(GTK_WINDOW(window), header);
    
    // Enable window dragging on the header bar
    g_signal_connect(header, "button-press-event", G_CALLBACK(+[](GtkWidget*, GdkEventButton* event, gpointer win) -> gboolean {
        if (event->button == 1) {
            gtk_window_begin_move_drag(GTK_WINDOW(win), event->button, event->x_root, event->y_root, event->time);
            return TRUE;
        }
        return FALSE;
    }), window);
    
    auto notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_widget_set_vexpand(notebook, TRUE);

    // Vertical tabs support
    bool vertical_tabs = SettingsManager::instance().appearance().vertical_tabs;
    if (vertical_tabs) {
        gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_LEFT);
        gtk_widget_set_hexpand(notebook, TRUE);
    } else {
        gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    }
    
    global_tab_manager = new TabManager(GTK_NOTEBOOK(notebook), GTK_ENTRY(url_entry), GTK_WINDOW(window));
    g_signal_connect(url_entry, "activate", G_CALLBACK(on_url_activate), global_tab_manager);
    
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
    
    const char* first_run_env = g_getenv("SEA_BROWSER_FIRST_RUN");
    if (first_run_env && strcmp(first_run_env, "1") == 0) {
        global_tab_manager->create_tab("sea://setup");
    } else {
        global_tab_manager->create_tab(SettingsManager::instance().general().homepage);
    }
    
    // Keyboard Shortcuts (Accelerators)
    auto accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    // Ctrl+T: New Tab
    auto new_tab_closure = g_cclosure_new(G_CALLBACK(on_new_tab_clicked), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_t, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, new_tab_closure);

    // Ctrl+W: Close Tab
    auto close_tab_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer) {
        if (global_tab_manager) {
            // Store tab info for undo before closing
            auto view = global_tab_manager->get_current_web_view();
            if (view) {
                const char* uri = webkit_web_view_get_uri(view);
                const char* title = webkit_web_view_get_title(view);
                if (uri) {
                    closed_tabs_stack.push_back({uri, title ? title : uri});
                    if (closed_tabs_stack.size() > MAX_CLOSED_TABS) {
                        closed_tabs_stack.erase(closed_tabs_stack.begin());
                    }
                }
            }
            global_tab_manager->close_current_tab();
        }
    }), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, close_tab_closure);
    
    // Ctrl+Shift+T: Reopen Closed Tab
    auto reopen_tab_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer) {
        if (global_tab_manager && !closed_tabs_stack.empty()) {
            auto last_tab = closed_tabs_stack.back();
            closed_tabs_stack.pop_back();
            global_tab_manager->create_tab(last_tab.first);
            std::cout << "[SeaBrowser] Reopened tab: " << last_tab.first << std::endl;
        }
    }), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_t, GdkModifierType(GDK_CONTROL_MASK | GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE, reopen_tab_closure);

    // Ctrl+R / F5: Reload
    auto reload_closure = g_cclosure_new(G_CALLBACK(on_reload_clicked), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, reload_closure);
    gtk_accel_group_connect(accel_group, GDK_KEY_F5, (GdkModifierType)0, GTK_ACCEL_VISIBLE, reload_closure);
    
    // Ctrl+Shift+R / Ctrl+F5: Hard reload
    auto hard_reload_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer) {
        if (global_tab_manager) {
            auto view = global_tab_manager->get_current_web_view();
            if (view) webkit_web_view_reload_bypass_cache(view);
        }
    }), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_r, GdkModifierType(GDK_CONTROL_MASK | GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE, hard_reload_closure);
    gtk_accel_group_connect(accel_group, GDK_KEY_F5, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, hard_reload_closure);

    // Alt+Left: Back
    auto back_closure = g_cclosure_new(G_CALLBACK(on_back_clicked), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_Left, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE, back_closure);

    // Alt+Right: Forward
    auto fwd_closure = g_cclosure_new(G_CALLBACK(on_forward_clicked), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_Right, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE, fwd_closure);

    // Ctrl+L: Focus URL bar
    auto focus_url_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer entry) {
        gtk_widget_grab_focus(GTK_WIDGET(entry));
    }), url_entry, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_l, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, focus_url_closure);
    
    // Ctrl+K: Focus URL bar for search
    auto focus_search_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer entry) {
        gtk_entry_set_text(GTK_ENTRY(entry), "");
        gtk_widget_grab_focus(GTK_WIDGET(entry));
    }), url_entry, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_k, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, focus_search_closure);

    // F12: Dev Tools
    auto dev_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer) {
        if (global_tab_manager) {
            auto web_view = global_tab_manager->get_current_web_view();
            if (web_view) {
                auto inspector = webkit_web_view_get_inspector(web_view);
                if (webkit_web_inspector_is_attached(inspector)) webkit_web_inspector_close(inspector);
                else webkit_web_inspector_show(inspector);
            }
        }
    }), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_F12, (GdkModifierType)0, GTK_ACCEL_VISIBLE, dev_closure);
    
    // F11: Fullscreen
    auto fullscreen_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer win) {
        GdkWindow* gdk_win = gtk_widget_get_window(GTK_WIDGET(win));
        if (gdk_win && gdk_window_get_state(gdk_win) & GDK_WINDOW_STATE_FULLSCREEN) {
            gtk_window_unfullscreen(GTK_WINDOW(win));
        } else {
            gtk_window_fullscreen(GTK_WINDOW(win));
        }
    }), window, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_F11, (GdkModifierType)0, GTK_ACCEL_VISIBLE, fullscreen_closure);
    
    // Ctrl+F: Find in page
    auto find_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer win) {
        if (global_tab_manager) {
            auto view = global_tab_manager->get_current_web_view();
            if (view) show_find_bar(GTK_WINDOW(win), view);
        }
    }), window, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_f, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, find_closure);
    
    // Ctrl++: Zoom in
    auto zoom_in_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer) {
        if (global_tab_manager) {
            auto view = global_tab_manager->get_current_web_view();
            if (view) {
                double zoom = webkit_web_view_get_zoom_level(view);
                webkit_web_view_set_zoom_level(view, zoom + 0.1);
            }
        }
    }), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_plus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, zoom_in_closure);
    gtk_accel_group_connect(accel_group, GDK_KEY_equal, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, zoom_in_closure);
    
    // Ctrl+-: Zoom out
    auto zoom_out_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer) {
        if (global_tab_manager) {
            auto view = global_tab_manager->get_current_web_view();
            if (view) {
                double zoom = webkit_web_view_get_zoom_level(view);
                webkit_web_view_set_zoom_level(view, std::max(0.25, zoom - 0.1));
            }
        }
    }), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_minus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, zoom_out_closure);
    
    // Ctrl+0: Reset zoom
    auto zoom_reset_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer) {
        if (global_tab_manager) {
            auto view = global_tab_manager->get_current_web_view();
            if (view) webkit_web_view_set_zoom_level(view, 1.0);
        }
    }), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_0, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, zoom_reset_closure);
    
    // Store notebook reference for shortcuts
    g_object_set_data(G_OBJECT(window), "notebook", notebook);
    
    // Ctrl+Tab: Next tab
    auto next_tab_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer win) {
        auto nb = GTK_WIDGET(g_object_get_data(G_OBJECT(win), "notebook"));
        if (nb && global_tab_manager) {
            int current = gtk_notebook_get_current_page(GTK_NOTEBOOK(nb));
            int total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb));
            if (current < total - 1) {
                gtk_notebook_set_current_page(GTK_NOTEBOOK(nb), current + 1);
            } else {
                gtk_notebook_set_current_page(GTK_NOTEBOOK(nb), 0);
            }
        }
    }), window, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_Tab, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, next_tab_closure);
    
    // Ctrl+Shift+Tab: Previous tab
    auto prev_tab_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer win) {
        auto nb = GTK_WIDGET(g_object_get_data(G_OBJECT(win), "notebook"));
        if (nb && global_tab_manager) {
            int current = gtk_notebook_get_current_page(GTK_NOTEBOOK(nb));
            if (current > 0) {
                gtk_notebook_set_current_page(GTK_NOTEBOOK(nb), current - 1);
            } else {
                int total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb));
                gtk_notebook_set_current_page(GTK_NOTEBOOK(nb), total - 1);
            }
        }
    }), window, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_ISO_Left_Tab, GdkModifierType(GDK_CONTROL_MASK | GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE, prev_tab_closure);
    
    // Ctrl+[1-8]: Go to tab N (Ctrl+9 goes to last tab)
    for (int i = 0; i < 8; i++) {
        // Create a struct to hold both window and tab number
        struct GotoTabData { GtkWidget* win; int tab; };
        auto* goto_data = new GotoTabData{window, i};
        auto goto_tab_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer data) {
            auto* gtd = static_cast<GotoTabData*>(data);
            if (gtd) {
                auto nb = GTK_WIDGET(g_object_get_data(G_OBJECT(gtd->win), "notebook"));
                if (nb && gtd->tab < gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb))) {
                    gtk_notebook_set_current_page(GTK_NOTEBOOK(nb), gtd->tab);
                }
            }
        }), goto_data, nullptr);
        gtk_accel_group_connect(accel_group, GDK_KEY_1 + i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, goto_tab_closure);
    }
    // Ctrl+9: Go to last tab
    auto goto_last_tab_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer win) {
        if (global_tab_manager) {
            auto nb = GTK_WIDGET(g_object_get_data(G_OBJECT(win), "notebook"));
            if (nb) {
                int total = gtk_notebook_get_n_pages(GTK_NOTEBOOK(nb));
                if (total > 0) gtk_notebook_set_current_page(GTK_NOTEBOOK(nb), total - 1);
            }
        }
    }), window, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_9, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE, goto_last_tab_closure);
    
    // Ctrl+Shift+N: New Window
    auto new_window_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer app) {
        BrowserWindow::create(GTK_APPLICATION(app));
    }), app, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_n, GdkModifierType(GDK_CONTROL_MASK | GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE, new_window_closure);
    
    // Ctrl+Shift+Delete: Clear browsing data
    auto clear_data_closure = g_cclosure_new(G_CALLBACK(+[](GtkWidget*, gpointer) {
        // Open settings to clear data page
        if (global_tab_manager) global_tab_manager->create_tab("sea://settings#privacy");
    }), nullptr, nullptr);
    gtk_accel_group_connect(accel_group, GDK_KEY_Delete, GdkModifierType(GDK_CONTROL_MASK | GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE, clear_data_closure);

#ifdef GDK_WINDOWING_X11
    // Apply MOTIF hints to remove decorations on X11
    auto apply_hints = +[](GtkWidget* win, GdkEvent*, gpointer) -> gboolean {
        set_motif_hints_no_decorations(win);
        set_kde_properties(win);
        return FALSE;
    };
    g_signal_connect(window, "realize", G_CALLBACK(+[](GtkWidget* win, gpointer) {
        set_motif_hints_no_decorations(win);
        set_kde_properties(win);
    }), nullptr);
    g_signal_connect(window, "map-event", G_CALLBACK(apply_hints), nullptr);
    g_signal_connect(window, "configure-event", G_CALLBACK(apply_hints), nullptr);
#endif
    HistoryManager::instance().cleanup_history();
    return window;
}

// Custom Header Bar using GtkHeaderBar
GtkWidget* BrowserWindow::setup_header_bar(GtkWidget* window, GtkWidget*, GtkWidget* url_entry) {
    auto header = gtk_header_bar_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(header), "blue-titlebar");
    gtk_widget_set_name(header, "sea-header");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), FALSE);
    gtk_header_bar_set_custom_title(GTK_HEADER_BAR(header), url_entry);
    
    // Navigation Box (Left)
    auto nav_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_style_context_add_class(gtk_widget_get_style_context(nav_box), "linked");
    
    auto back_btn = gtk_button_new_from_icon_name("go-previous-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(back_btn), "nav-btn");
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_clicked), nullptr);
    gtk_box_pack_start(GTK_BOX(nav_box), back_btn, FALSE, FALSE, 0);
    
    auto forward_btn = gtk_button_new_from_icon_name("go-next-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(forward_btn), "nav-btn");
    g_signal_connect(forward_btn, "clicked", G_CALLBACK(on_forward_clicked), nullptr);
    gtk_box_pack_start(GTK_BOX(nav_box), forward_btn, FALSE, FALSE, 0);
    
    auto reload_btn = gtk_button_new_from_icon_name("view-refresh-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(reload_btn), "nav-btn");
    g_signal_connect(reload_btn, "clicked", G_CALLBACK(on_reload_clicked), nullptr);
    gtk_box_pack_start(GTK_BOX(nav_box), reload_btn, FALSE, FALSE, 0);
    
    auto home_btn = gtk_button_new_from_icon_name("go-home-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(home_btn), "nav-btn");
    g_signal_connect(home_btn, "clicked", G_CALLBACK(on_home_clicked), nullptr);
    gtk_box_pack_start(GTK_BOX(nav_box), home_btn, FALSE, FALSE, 0);

    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), nav_box);
    
    // Right Box (Menu + Controls)
    auto right_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    
    auto new_tab_btn = gtk_button_new_from_icon_name("tab-new-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(new_tab_btn), "nav-btn");
    g_signal_connect(new_tab_btn, "clicked", G_CALLBACK(on_new_tab_clicked), nullptr);
    gtk_box_pack_start(GTK_BOX(right_box), new_tab_btn, FALSE, FALSE, 0);

    // Menu Button with Popup
    auto m_btn = gtk_menu_button_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(m_btn), "nav-btn");
    auto menu_icon = gtk_image_new_from_icon_name("open-menu-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(m_btn), menu_icon);
    
    auto menu = gtk_menu_new();
    
    auto s_item = gtk_menu_item_new_with_label("Settings");
    g_signal_connect(s_item, "activate", G_CALLBACK(on_settings_clicked), nullptr);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), s_item);
    
    auto h_item = gtk_menu_item_new_with_label("History");
    g_signal_connect(h_item, "activate", G_CALLBACK(+[](GtkMenuItem*, gpointer) {
        if (global_tab_manager) global_tab_manager->create_tab("sea://history");
    }), nullptr);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), h_item);
    
    // Bookmarks menu item
    auto b_item = gtk_menu_item_new_with_label("Bookmarks");
    g_signal_connect(b_item, "activate", G_CALLBACK(+[](GtkMenuItem*, gpointer) {
        if (global_tab_manager) global_tab_manager->create_tab("sea://bookmarks");
    }), nullptr);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), b_item);
    
    // Downloads menu item
    auto d_item = gtk_menu_item_new_with_label("Downloads");
    g_signal_connect(d_item, "activate", G_CALLBACK(+[](GtkMenuItem*, gpointer) {
        if (global_tab_manager) global_tab_manager->create_tab("sea://downloads");
    }), nullptr);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), d_item);

    auto e_item = gtk_menu_item_new_with_label("Extensions");
    g_signal_connect(e_item, "activate", G_CALLBACK(+[](GtkMenuItem*, gpointer) {
        if (global_tab_manager) global_tab_manager->create_tab("sea://extensions");
    }), nullptr);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), e_item);
    
    // Separator
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    
    // Zoom submenu
    auto zoom_item = gtk_menu_item_new_with_label("Zoom");
    auto zoom_menu = gtk_menu_new();
    auto zoom_in = gtk_menu_item_new_with_label("Zoom In (+)");
    auto zoom_out = gtk_menu_item_new_with_label("Zoom Out (-)");
    auto zoom_reset = gtk_menu_item_new_with_label("Reset (0)");
    
    g_signal_connect(zoom_in, "activate", G_CALLBACK(+[](GtkMenuItem*, gpointer) {
        if (global_tab_manager) {
            auto view = global_tab_manager->get_current_web_view();
            if (view) {
                double zoom = webkit_web_view_get_zoom_level(view);
                webkit_web_view_set_zoom_level(view, zoom + 0.1);
            }
        }
    }), nullptr);
    
    g_signal_connect(zoom_out, "activate", G_CALLBACK(+[](GtkMenuItem*, gpointer) {
        if (global_tab_manager) {
            auto view = global_tab_manager->get_current_web_view();
            if (view) {
                double zoom = webkit_web_view_get_zoom_level(view);
                webkit_web_view_set_zoom_level(view, std::max(0.25, zoom - 0.1));
            }
        }
    }), nullptr);
    
    g_signal_connect(zoom_reset, "activate", G_CALLBACK(+[](GtkMenuItem*, gpointer) {
        if (global_tab_manager) {
            auto view = global_tab_manager->get_current_web_view();
            if (view) webkit_web_view_set_zoom_level(view, 1.0);
        }
    }), nullptr);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(zoom_menu), zoom_in);
    gtk_menu_shell_append(GTK_MENU_SHELL(zoom_menu), zoom_out);
    gtk_menu_shell_append(GTK_MENU_SHELL(zoom_menu), zoom_reset);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(zoom_item), zoom_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), zoom_item);
    
    // Fullscreen menu item
    auto fullscreen_item = gtk_menu_item_new_with_label("Fullscreen (F11)");
    g_signal_connect(fullscreen_item, "activate", G_CALLBACK(+[](GtkMenuItem*, gpointer win) {
        GdkWindow* gdk_win = gtk_widget_get_window(GTK_WIDGET(win));
        if (gdk_win && gdk_window_get_state(gdk_win) & GDK_WINDOW_STATE_FULLSCREEN) {
            gtk_window_unfullscreen(GTK_WINDOW(win));
        } else {
            gtk_window_fullscreen(GTK_WINDOW(win));
        }
    }), window);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), fullscreen_item);
    
    // Separator
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    
    // Quit menu item
    auto quit_item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(quit_item, "activate", G_CALLBACK(+[](GtkMenuItem*, gpointer win) {
        gtk_window_close(GTK_WINDOW(win));
    }), window);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);
    
    gtk_widget_show_all(menu);
    gtk_menu_button_set_popup(GTK_MENU_BUTTON(m_btn), menu);
    gtk_box_pack_start(GTK_BOX(right_box), m_btn, FALSE, FALSE, 0);
    
    // Window Controls
    auto win_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(win_controls), "window-controls");

    auto min_btn = gtk_button_new_from_icon_name("window-minimize-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(min_btn), "win-control-btn");
    g_signal_connect(min_btn, "clicked", G_CALLBACK(+[](GtkButton*, gpointer win) {
        gtk_window_iconify(GTK_WINDOW(win));
    }), window);
    gtk_box_pack_start(GTK_BOX(win_controls), min_btn, FALSE, FALSE, 0);

    auto max_btn = gtk_button_new_from_icon_name("window-maximize-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(max_btn), "win-control-btn");
    g_signal_connect(max_btn, "clicked", G_CALLBACK(+[](GtkButton*, gpointer win) {
        if (gtk_window_is_maximized(GTK_WINDOW(win))) gtk_window_unmaximize(GTK_WINDOW(win));
        else gtk_window_maximize(GTK_WINDOW(win));
    }), window);
    gtk_box_pack_start(GTK_BOX(win_controls), max_btn, FALSE, FALSE, 0);

    auto close_btn = gtk_button_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(close_btn), "win-control-btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(close_btn), "close-btn");
    g_signal_connect(close_btn, "clicked", G_CALLBACK(+[](GtkButton*, gpointer win) {
        gtk_window_close(GTK_WINDOW(win));
    }), window);
    gtk_box_pack_start(GTK_BOX(win_controls), close_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(right_box), win_controls, FALSE, FALSE, 10);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), right_box);
    
    g_signal_connect(window, "button-press-event", G_CALLBACK(+[](GtkWidget*, GdkEventButton* event, gpointer) -> gboolean {
        if (event->button == 8) { on_back_clicked(nullptr, nullptr); return TRUE; }
        else if (event->button == 9) { on_forward_clicked(nullptr, nullptr); return TRUE; }
        return FALSE;
    }), nullptr);

    gtk_widget_show_all(header);
    return header;
}

void BrowserWindow::on_url_activate(GtkEntry* entry, gpointer user_data) {
    auto tab_manager = static_cast<TabManager*>(user_data);
    auto web_view = tab_manager->get_current_web_view();
    if (web_view) webkit_web_view_load_uri(web_view, process_url_input(gtk_entry_get_text(entry)).c_str());
}

void BrowserWindow::on_back_clicked(GtkButton*, gpointer) {
    if (global_tab_manager) {
        auto v = global_tab_manager->get_current_web_view();
        if (v && webkit_web_view_can_go_back(v)) webkit_web_view_go_back(v);
    }
}

void BrowserWindow::on_forward_clicked(GtkButton*, gpointer) {
    if (global_tab_manager) {
        auto v = global_tab_manager->get_current_web_view();
        if (v && webkit_web_view_can_go_forward(v)) webkit_web_view_go_forward(v);
    }
}

void BrowserWindow::on_reload_clicked(GtkButton*, gpointer) {
    if (global_tab_manager) {
        auto v = global_tab_manager->get_current_web_view();
        if (v) webkit_web_view_reload(v);
    }
}

void BrowserWindow::on_home_clicked(GtkButton*, gpointer) {
    if (global_tab_manager) {
        auto v = global_tab_manager->get_current_web_view();
        if (v) webkit_web_view_load_uri(v, SettingsManager::instance().general().homepage.c_str());
    }
}

void BrowserWindow::on_new_tab_clicked(GtkButton*, gpointer) {
    if (global_tab_manager) global_tab_manager->create_tab("sea://newtab");
}

void BrowserWindow::on_settings_clicked(GtkMenuItem*, gpointer) {
    if (global_tab_manager) global_tab_manager->create_tab("sea://settings");
}

std::string BrowserWindow::process_url_input(const std::string& input) {
    auto trimmed = input;
    while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t')) trimmed.erase(0, 1);
    while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t')) trimmed.pop_back();
    if (trimmed.empty()) return "sea://newtab";
    if (trimmed.starts_with("sea://") || trimmed.starts_with("http://") || trimmed.starts_with("https://")) return trimmed;
    if (is_url(trimmed)) return "https://" + trimmed;
    return get_search_url(trimmed);
}

bool BrowserWindow::is_url(const std::string& input) {
    if (input.find(' ') != std::string::npos) return false;
    static const std::vector<std::string> tlds = { ".com", ".org", ".net", ".io", ".co", ".dev", ".app", ".gov", ".edu", ".ai", ".tech", ".cloud", ".design", ".blog", ".info", ".biz", ".us", ".uk", ".eu", ".de", ".fr", ".jp", ".cn", ".ru", ".in", ".br" };
    for (const auto& tld : tlds) if (input.size() > tld.size() && input.substr(input.size() - tld.size()) == tld) return true;
    return input.find(".") != std::string::npos && input.find("..") == std::string::npos;
}

std::string BrowserWindow::get_search_url(const std::string& query) {
    std::string engine = SettingsManager::instance().search().default_engine;
    std::string prefix = "https://www.google.com/search?q=";
    if (engine == "duckduckgo") prefix = "https://duckduckgo.com/?q=";
    else if (engine == "bing") prefix = "https://www.bing.com/search?q=";
    else if (engine == "brave") prefix = "https://search.brave.com/search?q=";
    else if (engine == "ecosia") prefix = "https://www.ecosia.org/search?q=";
    else if (engine == "startpage") prefix = "https://www.startpage.com/do/dsearch?query=";
    std::string encoded;
    for (char c : query) {
        if (isalnum(c)) encoded += c;
        else if (c == ' ') encoded += '+';
        else {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", static_cast<unsigned char>(c));
            encoded += buf;
        }
    }
    return prefix + encoded;
}

} // namespace SeaBrowser
