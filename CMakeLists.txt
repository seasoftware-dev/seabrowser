cmake_minimum_required(VERSION 3.20)
project(Tsunami VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Generate version header
set(VERSION_HEADER "${CMAKE_SOURCE_DIR}/src/version.h")
configure_file(
    "${CMAKE_SOURCE_DIR}/src/version.h.in"
    "${VERSION_HEADER}"
    @ONLY
)

find_package(Qt6 COMPONENTS Widgets WebEngineWidgets REQUIRED)

set(SOURCES
    src/main.cpp
    src/application.cpp
    src/browser_window.cpp
    src/web_view.cpp
    src/tab_manager.cpp
    src/settings/settings.cpp
    src/settings/settings_dialog.cpp
    src/ui/downloads_window.cpp
    src/ui/bookmarks_window.cpp
    src/ui/history_window.cpp
    src/ui/extensions_window.cpp
    src/ui/custom_menu.cpp
    src/ui/onboarding_dialog.cpp
    src/update_manager.cpp
    src/download_manager.cpp
    src/bookmark_manager.cpp
    src/history/history_manager.cpp
    src/platform/window_manager.cpp
)

if(WIN32)
    set(SOURCES ${SOURCES} src/platform/win_main.cpp)
elseif(APPLE)
    set(SOURCES ${SOURCES} src/platform/mac_main.cpp)
endif()

# Add Windows resource file for icon
if(WIN32)
    set(RESOURCE_FILE "${CMAKE_SOURCE_DIR}/src/resources/windows/tsunami.rc")
    if(EXISTS ${RESOURCE_FILE})
        set(SOURCES ${SOURCES} ${RESOURCE_FILE})
    endif()
endif()

add_executable(Tsunami WIN32 MACOSX_BUNDLE ${SOURCES})

target_link_libraries(Tsunami PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
    sqlite3
)

if(WIN32)
    target_link_libraries(Tsunami PRIVATE dwmapi)
elseif(APPLE)
else()
    target_link_libraries(Tsunami PRIVATE X11)
endif()

target_include_directories(Tsunami PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(Tsunami PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# Copy data files to build directory
file(GLOB_RECURSE DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/data/*")
foreach(file ${DATA_FILES})
    if(IS_DIRECTORY ${file})
        get_filename_component(dir ${file} NAME)
        add_custom_command(TARGET Tsunami POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${file}
            $<TARGET_FILE_DIR:Tsunami>/data/${dir}
        )
    else()
        file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}/data" ${file})
        file(RELATIVE_PATH rel_dir "${CMAKE_CURRENT_SOURCE_DIR}/data" ${file})
        get_filename_component(parent_dir ${rel_path} DIRECTORY)
        add_custom_command(TARGET Tsunami POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:Tsunami>/data/${parent_dir}
        )
        add_custom_command(TARGET Tsunami POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${file}
            $<TARGET_FILE_DIR:Tsunami>/data/${rel_path}
        )
    endif()
endforeach()

# Install desktop file on Linux
if(UNIX AND NOT APPLE)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/data/io.tsunami.Tsunami.desktop"
            DESTINATION share/applications)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/data/io.tsunami.Tsunami.svg"
            DESTINATION share/pixmaps RENAME tsunami.svg)
endif()

# Cross-compilation for Windows using MXE
if(CROSS_COMPILE_WINDOWS)
    message(STATUS "Cross-compiling for Windows")
    set(CMAKE_SYSTEM_NAME Windows)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)

    # MXE paths
    set(CMAKE_C_COMPILER /usr/bin/x86_64-w64-mingw32.static-gcc)
    set(CMAKE_CXX_COMPILER /usr/bin/x86_64-w64-mingw32.static-g++)
    set(CMAKE_RC_COMPILER /usr/bin/x86_64-w64-mingw32.static-windres)
    set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32.static)

    set(CMAKE_FIND_ROOT_PATH_PROGRAM CROSS_COMPILE_BINUTILS_AR)
    set(CMAKE_FIND_ROOT_PATH_PROGRAM CROSS_COMPILE_BINUTILS_AS)

    set(CMAKE_C_FLAGS "-static-libgcc -static-libstdc++")
    set(CMAKE_CXX_FLAGS "-static-libgcc -static-libstdc++")
endif()

message("")
message("=============================================")
message("Tsunami - Qt WebEngine Browser")
message("=============================================")
message("Platform: ${CMAKE_SYSTEM_NAME}")
message("Qt Version: ${Qt6_VERSION}")
message("Engine: Qt WebEngine (Chromium-based)")
message("")
message("To build:")
message("  mkdir build && cd build")
message("  cmake ..")
message("  make -j4")
message("")
message("For Windows cross-compilation (MXE):")
message("  mkdir build && cd build")
message("  cmake -DCROSS_COMPILE_WINDOWS=ON ..")
message("  make -j4")
message("")
