/*
 * Tsunami Browser - Qt6 WebEngine Browser
 * application.h - Qt6 Application
 */

#pragma once

#include <QApplication>
#include <QString>
#include <string>

namespace Tsunami {

class Application {
public:
    static int run(int argc, char* argv[]);
    
    static QString get_config_dir();
    static QString get_data_dir();
    static QString get_cache_dir();
    static QString get_resource_path(const QString& relative_path);

private:
    static void on_startup();
    static void on_activate();
};

} // namespace Tsunami
