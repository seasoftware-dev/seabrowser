/*
 * Sea Browser - Privacy-focused web browser
 * utils.h - Security and utility helpers
 */

#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <iomanip>

#ifdef __linux__
#include <unistd.h>
#include <limits.h>
#endif

namespace SeaBrowser {
namespace Utils {

/**
 * Escapes HTML entities to prevent XSS.
 */
inline std::string escape_html(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for (size_t pos = 0; pos != data.size(); ++pos) {
        switch (data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(1, data[pos]); break;
        }
    }
    return buffer;
}

/**
 * Escapes strings for JSON inclusion to prevent injection.
 */
inline std::string escape_json_string(const std::string& data) {
    std::ostringstream ss;
    for (auto iter = data.cbegin(); iter != data.cend(); ++iter) {
        switch (*iter) {
            case '\\': ss << "\\\\"; break;
            case '"':  ss << "\\\""; break;
            case '/':  ss << "\\/";  break;
            case '\b': ss << "\\b";  break;
            case '\f': ss << "\\f";  break;
            case '\n': ss << "\\n";  break;
            case '\r': ss << "\\r";  break;
            case '\t': ss << "\\t";  break;
            default:
                if (static_cast<unsigned char>(*iter) < 0x20) {
                    ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(*iter);
                } else {
                    ss << *iter;
                }
                break;
        }
    }
    return ss.str();
}

/**
 * Validates and sanitizes a resource path to prevent traversal.
 * Returns true if the path is safe (no .. or absolute paths).
 */
inline bool is_safe_resource_path(const std::string& path) {
    if (path.empty()) return false;
    
    // Block absolute paths
    if (path[0] == '/') return false;
    
    // Block path traversal tokens
    std::string lower_path = path;
    std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), ::tolower);
    
    if (lower_path.find("..") != std::string::npos) return false;
    if (lower_path.find(":") != std::string::npos) return false; // Block absolute Windows paths if cross-compiled
    
    return true;
}

/**
 * Robustly gets the executable directory.
 */
inline std::string get_exe_dir() {
#ifdef __linux__
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return std::filesystem::path(std::string(result, count)).parent_path().string();
    }
#endif
    return std::filesystem::current_path().string();
}

} // namespace Utils
} // namespace SeaBrowser
