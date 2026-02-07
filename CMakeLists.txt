cmake_minimum_required(VERSION 3.20)
project(SeaBrowser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages - Using GTK3 for webkit2gtk compatibility
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.1)
pkg_check_modules(SQLITE REQUIRED sqlite3)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)

# Source files
set(SOURCES
    src/main.cpp
    src/application.cpp
    src/browser_window.cpp
    src/web_view.cpp
    src/tab_manager.cpp
    src/extension_manager.cpp
    src/settings/settings_manager.cpp
    src/settings/settings_dialog.cpp
    src/privacy/content_blocker.cpp
    src/privacy/cookie_manager.cpp
    src/history/history_manager.cpp
    src/bookmarks/bookmarks_manager.cpp
    src/downloads/downloads_manager.cpp
)

# Create executable
add_executable(seabrowser ${SOURCES})

# Include directories
target_include_directories(seabrowser PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${GTK3_INCLUDE_DIRS}
    ${WEBKIT_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(seabrowser PRIVATE
    ${GTK3_LIBRARIES}
    ${WEBKIT_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${X11_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
)

# Define source directory for resource loading
target_compile_definitions(seabrowser PRIVATE PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Compiler flags
target_compile_options(seabrowser PRIVATE
    ${GTK3_CFLAGS_OTHER}
    ${WEBKIT_CFLAGS_OTHER}
    -Wall -Wextra
)



# Install targets
install(TARGETS seabrowser DESTINATION bin)
install(FILES data/io.seabrowser.SeaBrowser.desktop DESTINATION share/applications)
install(FILES data/io.seabrowser.SeaBrowser.svg DESTINATION share/icons/hicolor/scalable/apps)

# CPack configuration
set(CPACK_PACKAGE_NAME "seabrowser")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sea Browser - Privacy-focused web browser")
include(CPack)
