# Building Tsunami Browser

This guide covers building Tsunami Browser from source on various platforms.

## Prerequisites

### Common Dependencies

- **CMake** 3.20+
- **C++ Compiler** with C++20 support (GCC 11+, Clang 14+, MSVC 2022+)
- **Qt 6.4+** with the following modules:
  - Qt Base (Core, GUI, Widgets)
  - Qt WebEngine
  - Qt Sql (SQLite integration)
- **SQLite** 3.36+

### Platform-Specific

#### Linux (Ubuntu/Debian)

```bash
sudo apt install cmake build-essential qt6-base-dev qt6-webengine-dev libsqlite3-dev
```

#### Linux (Fedora)

```bash
sudo dnf install cmake gcc-c++ qt6-qtbase-devel qt6-qtwebengine-devel sqlite-devel
```

#### Linux (Arch Linux)

```bash
sudo pacman -S cmake base-devel qt6-base qt6-webengine sqlite
```

#### macOS

Install [Qt 6 for macOS](https://www.qt.io/download) or via Homebrew:

```bash
brew install qt6 sqlite
```

#### Windows

Install [Qt 6 for Windows](https://www.qt.io/download) with MinGW toolchain.

## Building from Source

### Linux/macOS

```bash
# Clone the repository
git clone https://github.com/yourusername/Tsunami.git
cd Tsunami

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
make -j$(nproc)

# Run
./Tsunami
```

### Windows

```powershell
# In PowerShell or Developer Command Prompt
git clone https://github.com/yourusername/Tsunami.git
cd Tsunami
mkdir build
cd build

# Configure with MinGW
cmake -G "MinGW Makefiles" ..
mingw32-make -j$(nproc)

# Run
.\Tsunami.exe
```

## Building Packages

### All Linux Packages

```bash
./scripts/build-all.sh
```

Output will be in `output/`:
- `output/deb/` - DEB package (Ubuntu/Debian)
- `output/rpm/` - RPM package (Fedora/openSUSE)
- `output/flatpak/` - Flatpak bundle
- `output/appimage/` - AppImage (portable)

### Individual Packages

```bash
# DEB package
./scripts/build-deb.sh

# RPM package
./scripts/build-rpm.sh

# AppImage
./scripts/build-appimage.sh

# Flatpak
./scripts/build-flatpak.sh
```

### Windows

```powershell
# In PowerShell with Qt installed
.\scripts\build-windows.ps1
```

Output: `output/windows/`

## Cross-Compilation

### Windows on Linux (MXE)

Requires [MXE (Mingw-builds)](https://mingw-w64.org/doku.php/download/mingw-builds):

```bash
# Install MXE dependencies
sudo apt install autoconf automake autopoint bash bison bzip2 \
    flex g++ gawk gcc gettext git gperf intltool libtool \
    make pkg-config mingw-w64

# Configure and build
cmake -DCROSS_COMPILE_WINDOWS=ON ..
make -j$(nproc)
```

## Build Options

CMake options can be configured during build:

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

| Option | Description | Default |
|--------|-------------|---------|
| `CMAKE_BUILD_TYPE` | Build type (Debug, Release, RelWithDebInfo) | Release |
| `CMAKE_INSTALL_PREFIX` | Installation prefix | /usr/local |
| `CROSS_COMPILE_WINDOWS` | Enable Windows cross-compilation | OFF |

## Troubleshooting

### Qt WebEngine Not Found

Ensure Qt WebEngine is installed:
- Ubuntu/Debian: `qt6-webengine-dev`
- Fedora: `qt6-qtwebengine-devel`
- Arch: `qt6-webengine`
- macOS/Windows: Install via Qt installer

### Build Errors

If you encounter build errors:

1. Clean the build directory:
   ```bash
   rm -rf build && mkdir build && cd build
   ```

2. Update submodules (if any):
   ```bash
   git submodule update --init --recursive
   ```

3. Ensure you have the latest Qt version:
   ```bash
   qmake --version
   ```

### Runtime Issues

- **Blank pages**: Ensure Qt WebEngine process can access GPU
- **Crashes**: Try running with `QT_QPA_PLATFORM=wayland` or `xcb`
- **Icons missing**: Check that `data/icons/` is copied to the build directory

## Dependencies List

### Runtime Dependencies

| Dependency | Minimum Version | Description |
|------------|----------------|-------------|
| Qt Core | 6.4 | Application framework |
| Qt WebEngine | 6.4 | Chromium-based browser engine |
| Qt Sql | 6.4 | SQLite integration |
| SQLite | 3.36 | Data storage |
| libxcb | 1.15 | X11 windowing (Linux) |
| OpenGL | 3.0+ | Hardware acceleration |

### Optional Dependencies

| Dependency | Description |
|------------|-------------|
| AppImageKit | For AppImage creation |
| flatpak-builder | For Flatpak creation |
| NSIS | For Windows installer |

## Development

### IDE Setup

#### Qt Creator

1. Open `CMakeLists.txt` in Qt Creator
2. Configure with default settings
3. Build and run

#### VS Code

Recommended extensions:
- C/C++ (Microsoft)
- CMake (Microsoft)
- CMake Tools (Microsoft)

#### CLion

Open the project directory directly - CLion will detect CMake and configure automatically.

## Related Documentation

- [README.md](README.md) - Project overview and features
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [SECURITY.md](SECURITY.md) - Security policy
- [CHANGELOG.md](CHANGELOG.md) - Version history

## Getting Help

- [GitHub Issues](https://github.com/yourusername/Tsunami/issues) - Report build problems
- [GitHub Discussions](https://github.com/yourusername/Tsunami/discussions) - Ask questions
- [Discord](https://discord.gg/tsunami) - Community chat

## Related Documentation

- [README.md](README.md) - Project overview and features
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [SECURITY.md](SECURITY.md) - Security policy
- [CHANGELOG.md](CHANGELOG.md) - Version history
