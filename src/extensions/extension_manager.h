#pragma once

#include <string>
#include <vector>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

namespace SeaBrowser {

struct Extension {
    std::string name;
    std::string js_source;
    bool enabled;
};

class ExtensionManager {
public:
    static ExtensionManager& instance() {
        static ExtensionManager instance;
        return instance;
    }

    void load_extensions() {
        // Placeholder: Load from user profile extensions/ directory using GFile/GDir
        // For now, add a sample "Dark Reader Lite" extension
        Extension dark_mode = {
            "Dark Reader Lite",
            "document.addEventListener('DOMContentLoaded', function() { "
            "  if (window.location.href.indexOf('sea://') === -1) {"
            "    let style = document.createElement('style');"
            "    style.innerHTML = 'html, body { filter: invert(1) hue-rotate(180deg); } img, video { filter: invert(1) hue-rotate(180deg); }';"
            "    document.head.appendChild(style);"
            "  }"
            "});",
            false // Disabled by default
        };
        extensions_.push_back(dark_mode);
    }

    void inject_extensions(WebKitWebView* web_view) {
        auto content_manager = webkit_web_view_get_user_content_manager(web_view);
        webkit_user_content_manager_remove_all_scripts(content_manager);

        for (const auto& ext : extensions_) {
            if (ext.enabled) {
                WebKitUserScript* script = webkit_user_script_new(
                    ext.js_source.c_str(),
                    WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
                    WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
                    nullptr, nullptr
                );
                webkit_user_content_manager_add_script(content_manager, script);
                webkit_user_script_unref(script);
            }
        }
    }

    std::vector<Extension>& get_extensions() { return extensions_; }

private:
    ExtensionManager() = default;
    std::vector<Extension> extensions_;
};

} // namespace SeaBrowser
