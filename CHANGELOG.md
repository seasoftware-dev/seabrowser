# Changelog

All notable changes to Tsunami Browser are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]

## [1.0.0] - 2024-02-11

### Added

- Initial release
- Qt6 WebEngine-based browser
- Privacy-focused features
- Dark/light theme support
- Customizable accent colors
- Tab management
- Bookmark management
- History management
- Download management
- Settings system
- Onboarding wizard
- Auto-updater via GitHub Releases
- Cross-platform support (Linux, Windows, macOS)

### Features

- **Browser Core**
  - Qt6 WebEngine integration
  - Tabbed browsing with drag-and-drop
  - Session restore
  - Private browsing mode

- **Privacy**
  - Tracker blocking
  - Third-party cookie control
  - HTTPS-only mode
  - Do Not Track support
  - Fingerprinting protection

- **UI/UX**
  - Modern dark/light themes
  - Customizable accent colors
  - Custom title bar
  - Keyboard shortcuts
  - Context menus

- **Internal Pages**
  - New Tab page
  - Settings page
  - History page
  - Bookmarks page
  - Downloads page
  - Extensions page

### Packages

- **Linux**
  - DEB package (Ubuntu/Debian)
  - RPM package (Fedora/openSUSE)
  - Flatpak bundle
  - AppImage

- **Windows**
  - NSIS installer
  - Portable ZIP

- **macOS**
  - DMG image (planned)

### Build System

- CMake-based build
- Cross-compilation support
- Multiple package formats

## Comparison with Previous Version

This is a complete rewrite from Sea Browser, featuring:

| Feature | Sea Browser | Tsunami |
|---------|-------------|---------|
| Engine | GTK/WebKitGTK | Qt6 WebEngine |
| UI | GTK | Qt Widgets |
| Settings | GSettings | JSON |
| Language | C | C++ |
| Platform | Linux only | Cross-platform |

## Security

- First release with standard security practices
- No known vulnerabilities at release

## Acknowledgments

See [README.md](README.md) for full acknowledgments.
