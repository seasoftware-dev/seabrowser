/*
 * Sea Browser - Cookie Manager
 * cookie_manager.h
 */

#pragma once

#include <string>
#include <vector>

namespace SeaBrowser {

class CookieManager {
public:
    static CookieManager& instance();
    
    void initialize(const std::string& data_path);
    void delete_all_cookies();

private:
    CookieManager() = default;
    std::string data_path_;
};

} // namespace SeaBrowser
