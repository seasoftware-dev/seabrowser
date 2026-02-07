/*
 * Sea Browser - Privacy-focused web browser
 * application.h - GTK3 Application
 */

#pragma once

#include <gtk/gtk.h>
#include <string>

namespace SeaBrowser {

class Application {
public:
    static int run(int argc, char* argv[]);
    
    static std::string get_config_dir();
    static std::string get_data_dir();
    static std::string get_cache_dir();
    static std::string get_resource_path(const std::string& relative_path);

private:
    static void on_startup(GApplication* app, gpointer user_data);
    static void on_activate(GtkApplication* app, gpointer user_data);
};

} // namespace SeaBrowser
