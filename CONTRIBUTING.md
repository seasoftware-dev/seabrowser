# Contributing to Tsunami Browser

Thank you for your interest in contributing to Tsunami Browser! This document provides guidelines and instructions for contributing.

## Ways to Contribute

- **Reporting Bugs**: Found a bug? Let us know!
- **Suggesting Features**: Have an idea? We'd love to hear it.
- **Code Contributions**: Fix bugs, implement features, or improve existing code.
- **Documentation**: Improve docs, README, or this guide.
- **Translations**: Help translate Tsunami into other languages.
- **Packaging**: Create packages for other distributions or platforms.

## Getting Started

### Prerequisites

- C++20 compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- Qt 6.4+ with Qt WebEngine
- CMake 3.20+
- SQLite 3.36+

### Setting Up Development Environment

1. Fork the repository on GitHub
2. Clone your fork locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/Tsunami.git
   cd Tsunami
   ```
3. Create a branch for your changes:
   ```bash
   git checkout -b my-feature-branch
   ```
4. Build the project:
   ```bash
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ```
5. Test your changes:
   ```bash
   ./Tsunami
   ```

## Coding Standards

### C++ Style Guide

- Follow C++20 standards
- Use `snake_case` for variables and functions
- Use `PascalCase` for class names
- Use `SCREAMING_SNAKE_CASE` for constants
- Prefer `const` where possible
- Use `Qtn` naming convention for Qt properties
- Follow Qt's signal/slot patterns

### Code Formatting

We use clang-format for code formatting. Before submitting:

```bash
# Format your code
clang-format -i src/**/*.cpp src/**/*.h
```

### Documentation

- Document public APIs with Doxygen-style comments
- Update README.md for user-facing changes
- Add inline comments for complex logic
- Keep comments up to date with code

## Pull Request Process

1. **Ensure your branch is up to date** with main
2. **Write clear commit messages** describing your changes
3. **Test your changes** thoroughly
4. **Update documentation** as needed
5. **Submit a Pull Request** with a clear description

### Pull Request Guidelines

- Keep PRs focused and small (one feature or fix per PR)
- Include screenshots for UI changes
- Add tests for new functionality
- Ensure CI passes
- Link related issues

## Project Structure

```
src/
├── application.cpp/h      # Application lifecycle
├── browser_window.cpp/h   # Main browser window
├── web_view.cpp/h        # Web engine view wrapper
├── tab_manager.cpp/h     # Tab management
├── update_manager.cpp/h  # Auto-updater
├── download_manager.cpp/h # Download handling
├── bookmark_manager.cpp/h # Bookmark storage
├── settings/
│   ├── settings.cpp/h    # Settings singleton
│   └── settings_dialog.cpp/h # Settings UI
└── ui/
    ├── onboarding_dialog.cpp/h # First-run wizard
    ├── downloads_window.cpp/h  # Downloads panel
    ├── bookmarks_window.cpp/h  # Bookmarks panel
    ├── history_window.cpp/h    # History panel
    ├── extensions_window.cpp/h # Extensions panel
    └── custom_menu.cpp/h      # Custom context menu
```

## Issue Labels

- `bug`: Something isn't working
- `feature`: New feature request
- `enhancement`: Improvement to existing feature
- `documentation`: Documentation changes
- `good first issue`: Good for newcomers
- `help wanted`: Needs assistance
- `priority`: High priority items

## Communication

- **GitHub Issues**: For bug reports and feature requests
- **GitHub Discussions**: For questions and ideas
- **Discord**: [Join our server](https://discord.gg/tsunami)

## Related Documentation

- [README.md](README.md) - Project overview and features
- [BUILDING.md](BUILDING.md) - Build instructions
- [SECURITY.md](SECURITY.md) - Security policy
- [CHANGELOG.md](CHANGELOG.md) - Version history

## License

By contributing to Tsunami Browser, you agree that your contributions will be licensed under the [MIT License](LICENSE).
