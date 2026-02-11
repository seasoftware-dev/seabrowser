# Tsunami Browser - Windows Build Script
# Run this in a MSYS2/MinGW environment or PowerShell with Qt installed

param(
    [string]$BuildDir = "build-windows",
    [string]$OutputDir = "output",
    [string]$Version = "1.0.0"
)

$ErrorActionPreference = "Stop"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Building Tsunami for Windows" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# Check for Qt
$QtPath = $null
$QtVersions = @(
    "C:\Qt\6.7.0\mingw64_64",
    "C:\Qt\6.6.0\mingw64_64",
    "C:\Qt\6.5.0\mingw64_64",
    "C:\Qt\Latest\mingw64_64"
)

foreach ($v in $QtVersions) {
    if (Test-Path "$v\bin\qmake.exe") {
        $QtPath = $v
        break
    }
}

if (-not $QtPath) {
    Write-Error "Qt not found. Please install Qt 6.x with MinGW toolchain."
    exit 1
}

Write-Host "[1/7] Using Qt from: $QtPath" -ForegroundColor Green

# Convert SVG to ICO
Write-Host "[2/7] Converting SVG to ICO..." -ForegroundColor Green
$IcoPath = "$PSScriptRoot\..\src\resources\windows"
$SvgPath = "$PSScriptRoot\..\data\logo.svg"

if (-not (Test-Path $SvgPath)) {
    Write-Error "SVG logo not found: $SvgPath"
    exit 1
}

# Try ImageMagick first
if (Get-Command "convert" -ErrorAction SilentlyContinue) {
    Write-Host "  Using ImageMagick..." -ForegroundColor Gray
    & convert -background none "$SvgPath" -resize 256x256 -extent 256x256 "$IcoPath\tsunami.ico"
    & convert -background none "$SvgPath" -resize 16x16 -extent 16x16 "$IcoPath\tsunami_sm.ico"
}
# Fallback: Create BMP-based ICO using PowerShell
else {
    Write-Host "  Creating BMP-based ICO (limited quality)..." -ForegroundColor Yellow
    
    Add-Type -AssemblyName System.Drawing
    
    function Create-Icon {
        param([int]$Size)
        $bmp = New-Object System.Drawing.Bitmap($Size, $Size)
        $graphics = [System.Drawing.Graphics]::FromImage($bmp)
        $graphics.Clear([System.Drawing.Color]::Transparent)
        
        $pen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(59, 130, 246), 2)
        $graphics.DrawEllipse($pen, [System.Drawing.Rectangle]::new(2, 2, $Size-4, $Size-4))
        
        $bmp.Save("$IcoPath\tsunami_${Size}.bmp", [System.Drawing.Imaging.ImageFormat]::Bmp)
        $bmp.Dispose()
        $graphics.Dispose()
    }
    
    Create-Icon -Size 256
    Create-Icon -Size 16
}

Write-Host "  Icon created at: $IcoPath" -ForegroundColor Gray

# Create directories
Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $BuildDir
New-Item -ItemType Directory -Force -Path "$OutputDir\windows"

# Configure with CMake
Write-Host "[3/7] Configuring with CMake..." -ForegroundColor Green
cmake -B $BuildDir -G "MinGW Makefiles" `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_INSTALL_PREFIX="C:\Program Files\Tsunami" `
    -DCMAKE_PREFIX_PATH="$QtPath" `
    -DQt6_DIR="$QtPath\lib\cmake\Qt6"

# Build
Write-Host "[4/7] Building..." -ForegroundColor Green
cmake --build $BuildDir --config Release -- -j$(nproc)

# Install to staging
Write-Host "[5/7] Creating distribution package..." -ForegroundColor Green
$StagingDir = "$BuildDir\staging"
New-Item -ItemType Directory -Force -Path "$StagingDir"
New-Item -ItemType Directory -Force -Path "$StagingDir\data\icons"
New-Item -ItemType Directory -Force -Path "$StagingDir\data\pages"
cmake --install $BuildDir --prefix $StagingDir

# Copy Qt DLLs
Write-Host "Copying Qt dependencies..." -ForegroundColor Yellow
$QtBinDir = "$QtPath\bin"
$NeededDlls = @(
    "Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6WebEngineCore.dll",
    "Qt6WebEngineWidgets.dll", "Qt6OpenGL.dll", "Qt6Svg.dll", "Qt6Qml.dll",
    "Qt6QmlModels.dll", "Qt6Network.dll", "Qt6Positioning.dll"
)

foreach ($dll in $NeededDlls) {
    $src = "$QtBinDir\$dll"
    if (Test-Path $src) {
        Copy-Item $src "$StagingDir\"
        Write-Host "  Copied: $dll" -ForegroundColor Gray
    }
}

# Copy ICU files (needed for Qt WebEngine)
$Icudt = (Get-ChildItem -Path $QtPath -Filter "icudt*.dll" -Recurse | Select-Object -First 1).FullName
if ($Icudt) {
    Copy-Item $Icudt "$StagingDir\"
    Write-Host "  Copied: $(Split-Path $Icudt -Leaf)" -ForegroundColor Gray
}

# Copy libgcc_s_seh-1.dll and libstdc++-6.dll (MinGW runtime)
Copy-Item "$QtPath\bin\libgcc_s_seh-1.dll" "$StagingDir\" -ErrorAction SilentlyContinue
Copy-Item "$QtPath\bin\libstdc++-6.dll" "$StagingDir\" -ErrorAction SilentlyContinue
Copy-Item "$QtPath\bin\libwinpthread-1.dll" "$StagingDir\" -ErrorAction SilentlyContinue

# Copy data files
Write-Host "Copying data files..." -ForegroundColor Yellow
Copy-Item "$PSScriptRoot\..\data\icons\*.svg" "$StagingDir\data\icons\" -ErrorAction SilentlyContinue
Copy-Item "$PSScriptRoot\..\data\pages\*.html" "$StagingDir\data\pages\" -ErrorAction SilentlyContinue
Copy-Item "$PSScriptRoot\..\data\*.css" "$StagingDir\data\" -ErrorAction SilentlyContinue
Copy-Item "$PSScriptRoot\..\data\*.svg" "$StagingDir\" -ErrorAction SilentlyContinue
Copy-Item "$PSScriptRoot\..\data\logo.svg" "$StagingDir\" -ErrorAction SilentlyContinue

# Create Qt plugins directory
New-Item -ItemType Directory -Force -Path "$StagingDir\platforms"
Copy-Item "$QtPath\plugins\platforms\qwindows.dll" "$StagingDir\platforms\" -ErrorAction SilentlyContinue

# Copy ICO
Copy-Item "$IcoPath\tsunami.ico" "$StagingDir\" -ErrorAction SilentlyContinue

# Create NSIS installer script with proper bundling
Write-Host "[6/7] Creating NSIS installer..." -ForegroundColor Green
$NsisScript = @'
!include "MUI2.nsh"

Name "Tsunami Browser"
OutFile "Tsunami-Setup.exe"
InstallDir "$PROGRAMFILES\Tsunami"
InstallDirRegKey HKLM "Software\Tsunami" "InstallDir"

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_NOAUTOCLOSE

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "Install"
    SetOutPath "$INSTDIR"
    
    # Copy binary
    File ".\Tsunami.exe"
    
    # Copy data folder
    SetOutPath "$INSTDIR\data"
    CreateDirectory "$INSTDIR\data"
    SetOutPath "$INSTDIR\data"
    File /r ".\data\*.*"
    
    # Return to INSTDIR
    SetOutPath "$INSTDIR"
    
    # Copy icon
    File ".\tsunami.ico"
    
    # Create start menu shortcuts
    CreateDirectory "$SMPROGRAMS\Tsunami"
    CreateShortcut "$SMPROGRAMS\Tsunami\Tsunami.lnk" "$INSTDIR\Tsunami.exe" "" "$INSTDIR\tsunami.ico"
    CreateShortcut "$SMPROGRAMS\Tsunami\Uninstall.lnk" "$INSTDIR\uninstall.exe"
    
    # Create desktop shortcut
    CreateShortcut "$DESKTOP\Tsunami.lnk" "$INSTDIR\Tsunami.exe" "" "$INSTDIR\tsunami.ico"
    
    # Write uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"
    
    # Register MIME types
    WriteRegStr HKLM "Software\Classes\.html" "" "HTMLfile"
    WriteRegStr HKLM "Software\Classes\.htm" "" "HTMLfile"
    WriteRegStr HKLM "Software\Classes\Http\shell\open\command" "" "`"$INSTDIR\Tsunami.exe`" `"%1`""
    WriteRegStr HKLM "Software\Classes\Https\shell\open\command" "" "`"$INSTDIR\Tsunami.exe`" `"%1`""
    
    # Register as default browser
    WriteRegStr HKLM "Software\Clients\StartMenuInternet\Tsunami" "" "Tsunami"
    WriteRegStr HKLM "Software\Clients\StartMenuInternet\Tsunami\DefaultIcon" "" "$INSTDIR\Tsunami.exe,0"
    WriteRegStr HKLM "Software\Clients\StartMenuInternet\Tsunami\shell\open\command" "" "`"$INSTDIR\Tsunami.exe`" `"%1`""
    
    # Register for http/https protocols
    WriteRegStr HKLM "Software\RegisteredApplications" "Tsunami" "Software\Clients\StartMenuInternet\Tsunami"
    
    # Add uninstall info
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tsunami" `
                     "DisplayName" "Tsunami Browser"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tsunami" `
                     "UninstallString" "`"$INSTDIR\uninstall.exe`""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tsunami" `
                     "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tsunami" `
                     "Publisher" "Tsunami Developers"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tsunami" `
                     "DisplayVersion" "1.0.0"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tsunami" `
                       "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tsunami" `
                       "NoRepair" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tsunami" `
                       "EstimatedSize" 0
    
    SetShellVarContext all
SectionEnd

Section "Uninstall"
    # Delete files
    Delete "$INSTDIR\Tsunami.exe"
    Delete "$INSTDIR\tsunami.ico"
    Delete "$INSTDIR\uninstall.exe"
    
    # Delete data folder
    Delete "$INSTDIR\data\*.*"
    RMDir "$INSTDIR\data"
    
    # Delete INSTDIR
    RMDir "$INSTDIR"
    
    # Delete shortcuts
    Delete "$SMPROGRAMS\Tsunami\*.*"
    RMDir "$SMPROGRAMS\Tsunami"
    Delete "$DESKTOP\Tsunami.lnk"
    
    # Clean registry
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Tsunami"
    DeleteRegKey HKLM "Software\Classes\Http\shell\open\command"
    DeleteRegKey HKLM "Software\Classes\Https\shell\open\command"
    DeleteRegKey HKLM "Software\RegisteredApplications" "Tsunami"
    DeleteRegKey HKLM "Software\Clients\StartMenuInternet\Tsunami"
    DeleteRegKey HKLM "Software\Classes\.html"
    DeleteRegKey HKLM "Software\Classes\.htm"
    
    SetShellVarContext all
SectionEnd
'@

# Write NSIS script
$NsisScript | Out-File -Encoding ASCII "$BuildDir\installer.nsi"

# Build installer
Push-Location $BuildDir

# Copy required files for NSIS
Copy-Item "$StagingDir\Tsunami.exe" "$BuildDir\" -ErrorAction SilentlyContinue
Copy-Item "$StagingDir\tsunami.ico" "$BuildDir\" -ErrorAction SilentlyContinue
Copy-Item "$StagingDir\data" "$BuildDir\" -Recurse -ErrorAction SilentlyContinue

if (Get-Command "makensis" -ErrorAction SilentlyContinue) {
    makensis installer.nsi
    
    Move-Item "$BuildDir\Tsunami-Setup.exe" "$OutputDir\windows\" -Force
    Write-Host "Installer created: $OutputDir\windows\Tsunami-Setup.exe" -ForegroundColor Green
} else {
    Write-Host "NSIS not found - skipping installer" -ForegroundColor Yellow
}

Pop-Location

# Create portable ZIP
Write-Host "[7/7] Creating portable ZIP..." -ForegroundColor Green
Compress-Archive -Path "$StagingDir\*" -DestinationPath "$OutputDir\windows\Tsunami-$Version-windows.zip" -Force
Write-Host "Portable ZIP created: $OutputDir\windows\Tsunami-$Version-windows.zip" -ForegroundColor Green

Write-Host ""
Write-Host "=========================================" -ForegroundColor Green
Write-Host "Build complete!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Output files:" -ForegroundColor Cyan
Get-ChildItem -Path "$OutputDir\windows" | Select-Object Name | Format-Table

Write-Host ""
Write-Host "Bundled contents:" -ForegroundColor Cyan
Write-Host "  - Tsunami.exe (main binary)"
Write-Host "  - tsunami.ico (application icon)"
Write-Host "  - data/ (icons, pages, stylesheets)"
Write-Host "  - Qt DLLs and dependencies"
Write-Host "  - Qt platform plugins"
