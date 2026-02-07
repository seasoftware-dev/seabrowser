/*
 * Sea Browser - Cookie Manager
 * cookie_manager.cpp
 */

#include "cookie_manager.h"

namespace SeaBrowser {

CookieManager& CookieManager::instance() {
    static CookieManager instance;
    return instance;
}

void CookieManager::initialize(const std::string& data_path) {
    data_path_ = data_path;
}

void CookieManager::delete_all_cookies() {
    // Cookie deletion handled via WebKit API in browser_window
}

} // namespace SeaBrowser
