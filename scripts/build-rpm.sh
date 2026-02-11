#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="$PROJECT_DIR/output"
STAGING_DIR="$OUTPUT_DIR/linux"
VERSION="1.0.0"

echo "========================================="
echo "Building RPM Package for Tsunami"
echo "========================================="

# Check if binary exists
if [ ! -f "$STAGING_DIR/usr/bin/Tsunami" ]; then
    echo "Error: Binary not found. Run ./scripts/build.sh first."
    exit 1
fi

# Create RPM staging directory
RPM_DIR="$OUTPUT_DIR/rpm"
rm -rf "$RPM_DIR"
mkdir -p "$RPM_DIR/usr/bin"
mkdir -p "$RPM_DIR/usr/share/applications"
mkdir -p "$RPM_DIR/usr/share/pixmaps"
mkdir -p "$RPM_DIR/usr/share/tsunami/data/icons"
mkdir -p "$RPM_DIR/usr/share/tsunami/data/pages"
mkdir -p "$RPM_DIR/usr/lib/tsunami/plugins"
mkdir -p "$RPM_DIR"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# Copy binary
cp "$STAGING_DIR/usr/bin/Tsunami" "$RPM_DIR/usr/bin/"

# Copy data folder with all contents
echo "Copying data folder..."
cp -r "$STAGING_DIR/usr/share/tsunami/data/." "$RPM_DIR/usr/share/tsunami/data/"

# Copy icons to pixmaps
cp "$PROJECT_DIR/data/io.tsunami.Tsunami.svg" "$RPM_DIR/usr/share/pixmaps/"

# Copy desktop file
cp "$PROJECT_DIR/data/io.tsunami.Tsunami.desktop" "$RPM_DIR/usr/share/applications/"

# Copy Qt plugins if needed
if [ -d "$STAGING_DIR/usr/lib/tsunami/plugins" ]; then
    cp -r "$STAGING_DIR/usr/lib/tsunami/plugins"/* "$RPM_DIR/usr/lib/tsunami/plugins/" 2>/dev/null || true
fi

# Copy Qt libraries
mkdir -p "$RPM_DIR/usr/lib"
for lib in $(ldd "$RPM_DIR/usr/bin/Tsunami" 2>/dev/null | grep "=> /" | awk '{print $3}'); do
    if [ -f "$lib" ]; then
        cp -f "$lib" "$RPM_DIR/usr/lib/" 2>/dev/null || true
    fi
done

# Create tarball for source
cd "$OUTPUT_DIR"
TARBALL_NAME="tsunami-${VERSION}.tar.gz"
tar -czf "$TARBALL_NAME" --transform "s|linux/usr|usr|" linux/
mv "$TARBALL_NAME" "$RPM_DIR/SOURCES/"

# Create SPEC file
cat > "$RPM_DIR/SPECS/tsunami.spec" << SPECFILE
Name:           tsunami
Version:        ${VERSION}
Release:        1%{?dist}
Summary:        A fast, private, and beautiful web browser
License:        MIT
URL:            https://tsunami.io
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  qt6-qtbase-devel
BuildRequires:  qt6-qtwebengine-devel
BuildRequires:  sqlite-devel

Requires:       qt6-qtbase-core >= 6.4
Requires:       qt6-qtwebengine-core >= 6.4
Requires:       sqlite-libs
Requires:       libxcb

%description
Tsunami is a privacy-focused web browser built with Qt6 WebEngine.
It features a clean interface, dark/light themes, and robust privacy protections.

%prep
%setup -q

%build
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%{_usr} ..
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/applications
mkdir -p %{buildroot}%{_datadir}/pixmaps
mkdir -p %{buildroot}%{_datadir}/%{name}
mkdir -p %{buildroot}%{_libdir}/%{name}

cp %{_bindir}/Tsunami %{buildroot}%{_bindir}/
cp -r %{_datadir}/%{name}/data %{buildroot}%{_datadir}/%{name}/
cp %{_datadir}/applications/io.tsunami.Tsunami.desktop %{buildroot}%{_datadir}/applications/
cp %{_datadir}/pixmaps/io.tsunami.Tsunami.svg %{buildroot}%{_datadir}/pixmaps/

%files
%{_bindir}/Tsunami
%{_datadir}/applications/io.tsunami.Tsunami.desktop
%{_datadir}/pixmaps/io.tsunami.Tsunami.svg
%{_datadir}/%{name}/

%post
# Update desktop database
if [ -f %{_datadir}/applications/io.tsunami.Tsunami.desktop ]; then
    update-desktop-database -q 2>/dev/null || true
fi

# Register MIME types
if [ -f %{_datadir}/applications/io.tsunami.Tsunami.desktop ]; then
    xdg-mime default io.tsunami.Tsunami.desktop x-scheme-handler/http 2>/dev/null || true
    xdg-mime default io.tsunami.Tsunami.desktop x-scheme-handler/https 2>/dev/null || true
    xdg-mime default io.tsunami.Tsunami.desktop text/html 2>/dev/null || true
fi

# Update icon cache
if [ -f %{_datadir}/pixmaps/io.tsunami.Tsunami.svg ]; then
    gtk-update-icon-cache -f -t %{_datadir}/pixmaps 2>/dev/null || true
fi

%postun
if [ \$1 -eq 0 ]; then
    update-desktop-database -q 2>/dev/null || true
fi

%changelog
* Wed Feb 11 2026 Tsunami Developers <dev@tsunami.io> - ${VERSION}-1
- Initial package release
SPECFILE

# Build RPM
cd "$RPM_DIR"
rpmbuild --define "_topdir $RPM_DIR" \
          -bb SPECS/tsunami.spec

echo ""
echo "RPM package created: $RPM_DIR/RPMS/x86_64/"
ls -la "$RPM_DIR/RPMS/x86_64/"

echo ""
echo "To install:"
echo "  sudo dnf install $RPM_DIR/RPMS/x86_64/tsunami-${VERSION}-1.x86_64.rpm"
