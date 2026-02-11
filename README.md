# Tsunami Browser

<div align="center">

![Tsunami Logo](data/logo.svg)

**A fast, private, and beautiful web browser built with Qt6**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Qt6](https://img.shields.io/badge/Qt-6-blue.svg)](https://www.qt.io/)

</div>

## About

Tsunami Browser is a privacy-focused web browser built with C++ and Qt6 WebEngine. It aims to provide a fast, secure, and distraction-free browsing experience with a modern, beautiful interface.

## Features

- **Privacy First**: Built-in tracking protection, HTTPS-only mode, and robust privacy controls
- **Modern Interface**: Clean dark/light themed UI with customizable accent colors
- **Qt6 WebEngine**: Powered by Chromium-based Qt WebEngine for fast, standards-compliant rendering
- **Cross-Platform**: Native builds for Linux (DEB, RPM, Flatpak, AppImage), Windows, and macOS
- **Auto-Updater**: Automatic updates via GitHub Releases
- **Tsunami Pages**: Fast, local internal pages for Settings, History, Bookmarks, Downloads, and New Tabs
- **Tab Management**: Drag-and-drop tabs, undo close, and session restore

## Quick Start

### Building from Source

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install cmake build-essential qt6-base-dev qt6-webengine-dev libsqlite3-dev

# Clone and build
git clone https://github.com/yourusername/Tsunami.git
cd Tsunami
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./Tsunami
```

### Pre-built Packages

See [Releases](https://github.com/yourusername/Tsunami/releases) for:

- **Linux**: DEB, RPM, Flatpak, AppImage
- **Windows**: Installer (.exe) and Portable ZIP
- **macOS**: DMG image

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+T` | New Tab |
| `Ctrl+W` | Close Tab |
| `Ctrl+R` / `F5` | Reload Page |
| `Ctrl+L` / `Alt+D` | Focus Address Bar |
| `Ctrl+Shift+T` | Undo Close Tab |
| `Ctrl+Tab` | Next Tab |
| `Ctrl+Shift+Tab` | Previous Tab |
| `Alt+Left` | Go Back |
| `Alt+Right` | Go Forward |
| `Ctrl+N` | New Window |
| `Ctrl+Shift+N` | Private Window |
| `Ctrl+H` | History |
| `Ctrl+Shift+B` | Bookmarks |
| `Ctrl+J` | Downloads |

## Privacy Features

Tsunami Browser includes privacy protections enabled by default:

- **Tracker Blocking**: Blocks common web trackers and analytics
- **Cookie Control**: Block third-party cookies
- **HTTPS-Only Mode**: Force secure connections
- **Do Not Track**: Send DNT header to websites
- **Fingerprinting Protection**: Reduce browser fingerprinting

Configure these in Settings → Privacy.

## Project Structure

```
Tsunami/
├── src/                    # C++ source code
│   ├── browser_window.cpp # Main browser window
│   ├── web_view.cpp       # Web engine view
│   ├── tab_manager.cpp    # Tab management
│   ├── settings/          # Settings system
│   ├── ui/                # UI components
│   │   ├── onboarding_dialog.cpp
│   │   ├── downloads_window.cpp
│   │   ├── bookmarks_window.cpp
│   │   └── history_window.cpp
│   └── update_manager.cpp  # Auto-updater
├── data/                   # Application data
│   ├── icons/             # UI icons
│   ├── pages/             # Internal HTML pages
│   └── style.css          # Shared styles
├── scripts/               # Build scripts
│   ├── build.sh           # Main build script
│   ├── build-deb.sh       # DEB package builder
│   ├── build-rpm.sh       # RPM package builder
│   ├── build-flatpak.sh   # Flatpak builder
│   ├── build-appimage.sh  # AppImage builder
│   └── build-windows.ps1  # Windows build (PowerShell)
├── CMakeLists.txt         # CMake configuration
└── README.md              # This file
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

### Windows

```powershell
# In PowerShell with Qt installed
.\scripts\build-windows.ps1
```

Output: `output/windows/`

### Cross-Compilation

For Windows cross-compilation on Linux using MXE:

```bash
cmake -DCROSS_COMPILE_WINDOWS=ON ..
make -j$(nproc)
```

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Building

For detailed build instructions, see [BUILDING.md](BUILDING.md).

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history and changes.

## Security

See [SECURITY.md](SECURITY.md) for our security policy and reporting vulnerabilities.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Acknowledgments

- [Qt](https://www.qt.io/) - Cross-platform application framework
- [Qt WebEngine](https://doc.qt.io/qt-6/qtwebengine-index.html) - Chromium-based web engine
- [SQLite](https://www.sqlite.org/) - Embedded database
- [Icons](data/icons/) - Custom SVG icons

---

<div align="center">

**Built with ❤️ for privacy**

</div>
