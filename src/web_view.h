/*
 * Sea Browser - Privacy-focused web browser
 * web_view.h - WebView utilities (GTK3)
 */

#pragma once

#include <webkit2/webkit2.h>
#include <string>

namespace SeaBrowser {

class WebView {
public:
    static std::string generate_internal_page(const std::string& page);
    
    // Internal page handler
    static void sea_scheme_request_handler(WebKitURISchemeRequest* request, gpointer user_data);
    static void setup_internal_page_bridge(WebKitWebView* web_view);
    static void on_script_message_received(WebKitUserContentManager* manager, WebKitJavascriptResult* result, gpointer user_data);
};

} // namespace SeaBrowser
