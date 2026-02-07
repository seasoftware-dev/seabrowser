#!/bin/bash
# Sea Browser - RPM Build Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
VERSION="1.0.0"

echo "🌊 Sea Browser RPM Build"
echo "========================"

# Install rpm-build if needed
if ! command -v rpmbuild &> /dev/null; then
    echo "Installing rpm-build..."
    sudo dnf install -y rpm-build rpmdevtools
fi

# Setup RPM structure
rpmdev-setuptree
# Clean up previous builds in RPM tree to avoid confusion
rm -rf "$HOME/rpmbuild/BUILD/seabrowser-*"

# Create spec file
cat > "$HOME/rpmbuild/SPECS/seabrowser.spec" << EOF
Name:           seabrowser
Version:        ${VERSION}
Release:        1%{?dist}
Summary:        Privacy-focused web browser

License:        GPL-3.0
URL:            https://github.com/seabrowser/seabrowser

BuildRequires:  cmake gcc-c++
BuildRequires:  gtk3-devel webkit2gtk4.1-devel sqlite-devel
Requires:       gtk3 webkit2gtk4.1 sqlite

%description
Sea Browser is a privacy-focused web browser built with WebKitGTK.

%install
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/icons/hicolor/scalable/apps
# Copy artifacts from local build to buildroot (simplified for this script)
# In a real RPM build, we'd use %build section, but here we reuse the local build
cp $PROJECT_ROOT/build-output/seabrowser %{buildroot}/usr/bin/
cp $PROJECT_ROOT/data/io.seabrowser.SeaBrowser.desktop %{buildroot}/usr/share/applications/
cp $PROJECT_ROOT/data/io.seabrowser.SeaBrowser.svg %{buildroot}/usr/share/icons/hicolor/scalable/apps/

%files
/usr/bin/seabrowser
/usr/share/applications/io.seabrowser.SeaBrowser.desktop
/usr/share/icons/hicolor/scalable/apps/io.seabrowser.SeaBrowser.svg

%changelog
* $(date '+%a %b %d %Y') Sea Browser <seabrowser@example.com> - ${VERSION}-1
- Initial release
EOF

# Build locally first
"$SCRIPT_DIR/build.sh"

echo "Building RPM..."
rpmbuild -bb "$HOME/rpmbuild/SPECS/seabrowser.spec"

echo ""
echo "✅ RPM built!"
echo "📦 Find it in: ~/rpmbuild/RPMS/"
