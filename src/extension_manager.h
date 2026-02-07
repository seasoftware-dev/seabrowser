#pragma once
#include <string>
#include <vector>
#include <webkit2/webkit2.h>

namespace SeaBrowser {

enum class ExtensionType { JavaScript, Chrome, Firefox };

struct Extension {
    std::string name;
    std::string description;
    std::string version;
    std::string author;
    ExtensionType type;
    std::string script;
    std::string manifest;
    bool enabled;
};

class ExtensionManager {
public:
    static ExtensionManager& instance();

    void init();
    void inject_extensions(WebKitWebView* web_view);
    void reload_extensions();

    std::vector<Extension>& get_extensions() { return extensions_; }

    bool install_extension(const std::string& path);
    void uninstall_extension(const std::string& name);

private:
    ExtensionManager() = default;
    std::vector<Extension> extensions_;

    void load_from_dir();
    bool load_chrome_extension(const std::string& path);
    bool load_firefox_extension(const std::string& path);

public:
    void toggle_extension(const std::string& name, bool enabled);
};

} // namespace SeaBrowser
