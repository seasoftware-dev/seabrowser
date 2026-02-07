# Building Sea Browser

This document outlines the steps to build Sea Browser from source.

## Prerequisites

Ensure you have the following dependencies installed on your system:

*   **C++ Compiler**: GCC 10+ or Clang 12+ (supports C++20)
*   **Build System**: CMake 3.16+
*   **GTK3**: Development headers (`libgtk-3-dev`)
*   **WebKitGTK**: Development headers (`libwebkit2gtk-4.1-dev`)
*   **SQLite3**: Development headers (`libsqlite3-dev`)

### Installing Dependencies

**Debian/Ubuntu:**
```bash
sudo apt install build-essential cmake libgtk-3-dev libwebkit2gtk-4.1-dev libsqlite3-dev
```

**Fedora:**
```bash
sudo dnf install gcc-c++ cmake gtk3-devel webkit2gtk4.1-devel sqlite-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake gtk3 webkit2gtk-4.1 sqlite
```

## Compilation

1.  **Clone the repository** (if you haven't already).

2.  **Run the build script**:
    
    The project includes a convenience script for Linux:
    
    ```bash
    ./build/linux/build.sh
    ```

    Alternatively, you can manually configure and build using CMake:
    
    ```bash
    mkdir build-output
    cd build-output
    cmake ..
    make -j$(nproc)
    ```

## Windows Build

1.  **Install Dependencies**:
    *   Install **Visual Studio 2022** with C++ Desktop Development.
    *   Install **vcpkg**: `git clone https://github.com/microsoft/vcpkg && .\vcpkg\bootstrap-vcpkg.bat`
    *   Set `VCPKG_ROOT` environment variable.

2.  **Install Libraries via vcpkg**:
    ```cmd
    vcpkg install gtk:x64-windows webkit2gtk:x64-windows sqlite3:x64-windows
    ```

3.  **Build**:
    Run `build.bat` in the project root to automatically configure and build using CMake.
    ```cmd
    .\build.bat
    ```

4.  **Run**:
    For windows, The executable will be in `build-win\Release\seabrowser.exe`.
    
    After a successful build, the binary will be located in the build directory:
    
    ```bash
    ./build-output/seabrowser
    ```

## Troubleshooting

*   **Missing Dependencies**: If CMake fails to find a package, ensure the `-dev` or `-devel` packages are installed.
*   **Compiler Errors**: Ensure your compiler supports C++20.

