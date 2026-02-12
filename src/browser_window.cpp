/*
 * Tsunami Browser - Privacy-focused web browser
 * browser_window.cpp - Main browser window implementation (Qt6)
 */

#include "browser_window.h"
#include "web_view.h"
#include "tab_manager.h"
#include "history/history_manager.h"
#include "settings/settings.h"
#include "application.h"
#include "settings/settings_dialog.h"
#include "ui/onboarding_dialog.h"
#include "ui/downloads_window.h"
#include "ui/bookmarks_window.h"
#include "ui/history_window.h"
#include "ui/extensions_window.h"
#include "ui/custom_menu.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineHistory>
#include <QWebEngineProfile>
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QMenuBar>
#include <QMenu>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QMimeData>
#include <iostream>
#include <algorithm>

using namespace Tsunami;

namespace Tsunami {

BrowserWindow::BrowserWindow(QWidget* parent)
    : QMainWindow(parent)
    , tab_widget_(nullptr)
    , url_bar_(nullptr)
    , title_bar_(nullptr)
    , progress_bar_(nullptr)
    , menu_btn_(nullptr)
    , back_btn_(nullptr)
    , forward_btn_(nullptr)
    , reload_btn_(nullptr)
    , home_btn_(nullptr)
    , bookmark_btn_(nullptr)
    , security_btn_(nullptr)
    , min_btn_(nullptr)
    , max_btn_(nullptr)
    , close_btn_(nullptr)
    , is_dragging_(false)
{
    setWindowTitle("Tsunami");
    resize(1400, 900);
    setAcceptDrops(true);
    
    // Enable client-side decorations (CSD) - custom title bar
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    setupUi();
    applyTheme();
    restoreSession();
}

BrowserWindow::~BrowserWindow() {
    saveSession();
}

void BrowserWindow::show() {
    QMainWindow::show();
}

void BrowserWindow::setupUi() {
    // Set window icon for all platforms (Wayland, X11, Windows)
    QStringList iconPaths;
    iconPaths << QCoreApplication::applicationDirPath() + "/data/logo.svg"
              << QCoreApplication::applicationDirPath() + "/logo.svg"
              << "data/logo.svg";
    
    for (const QString& iconPath : iconPaths) {
        if (QFile::exists(iconPath)) {
            QIcon windowIcon(iconPath);
            if (!windowIcon.isNull()) {
                setWindowIcon(windowIcon);
                break;
            }
        }
    }
    
    // Check if first run - show onboarding
    if (Settings::instance().isFirstRun()) {
        showOnboarding();
    }
    
    QWidget* central_widget = new QWidget(this);
    central_widget->setObjectName("centralWidget");
    setCentralWidget(central_widget);

    QVBoxLayout* main_layout = new QVBoxLayout(central_widget);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);

    setupTitleBar();
    main_layout->addWidget(title_bar_);

    tab_widget_ = new QTabWidget(this);
    tab_widget_->setObjectName("tabWidget");
    tab_widget_->setDocumentMode(true);
    tab_widget_->setTabsClosable(true);
    tab_widget_->setMovable(true);
    tab_widget_->setElideMode(Qt::ElideRight);
    connect(tab_widget_, &QTabWidget::tabCloseRequested, this, &BrowserWindow::onCloseTab);
    connect(tab_widget_, &QTabWidget::currentChanged, this, &BrowserWindow::onTabChanged);
    main_layout->addWidget(tab_widget_);

    progress_bar_ = new QProgressBar(this);
    progress_bar_->setObjectName("progressBar");
    progress_bar_->setMaximumHeight(3);
    progress_bar_->setTextVisible(false);
    progress_bar_->setValue(0);
    main_layout->addWidget(progress_bar_);

    // Connect to settings changes for instant updates
    qDebug() << "Connecting settingsChanged signal..." << Qt::endl;
    connect(&Settings::instance(), &Settings::settingsChanged, this, &BrowserWindow::onSettingsChanged);

    // Don't create tab here - restoreSession will handle it
}

void BrowserWindow::setupTitleBar() {
    title_bar_ = new QWidget(this);
    title_bar_->setObjectName("titleBar");
    title_bar_->setFixedHeight(48);
    title_bar_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Enable dragging the window from the title bar
    title_bar_->setMouseTracking(true);
    title_bar_->installEventFilter(this);

    QHBoxLayout* title_layout = new QHBoxLayout(title_bar_);
    title_layout->setContentsMargins(12, 6, 12, 6);
    title_layout->setSpacing(8);

    QString iconPath = QCoreApplication::applicationDirPath() + "/data/icons/";
    bool isDark = Settings::instance().getDarkMode();
    QString iconSuffix = isDark ? "-white" : "";

    // Navigation buttons - Left side
    back_btn_ = new QToolButton(title_bar_);
    back_btn_->setObjectName("backButton");
    back_btn_->setFixedSize(32, 32);
    back_btn_->setToolTip("Back");
    back_btn_->setAutoRaise(true);
    back_btn_->setIcon(QIcon(iconPath + "back" + iconSuffix + ".svg"));
    back_btn_->setIconSize(QSize(16, 16));
    connect(back_btn_, &QToolButton::clicked, this, &BrowserWindow::onBack);
    title_layout->addWidget(back_btn_);

    forward_btn_ = new QToolButton(title_bar_);
    forward_btn_->setObjectName("forwardButton");
    forward_btn_->setFixedSize(32, 32);
    forward_btn_->setToolTip("Forward");
    forward_btn_->setAutoRaise(true);
    forward_btn_->setIcon(QIcon(iconPath + "forward" + iconSuffix + ".svg"));
    forward_btn_->setIconSize(QSize(16, 16));
    connect(forward_btn_, &QToolButton::clicked, this, &BrowserWindow::onForward);
    title_layout->addWidget(forward_btn_);

    reload_btn_ = new QToolButton(title_bar_);
    reload_btn_->setObjectName("reloadButton");
    reload_btn_->setFixedSize(32, 32);
    reload_btn_->setToolTip("Reload");
    reload_btn_->setAutoRaise(true);
    reload_btn_->setIcon(QIcon(iconPath + "reload" + iconSuffix + ".svg"));
    reload_btn_->setIconSize(QSize(16, 16));
    connect(reload_btn_, &QToolButton::clicked, this, &BrowserWindow::onReload);
    title_layout->addWidget(reload_btn_);

    home_btn_ = new QToolButton(title_bar_);
    home_btn_->setObjectName("homeButton");
    home_btn_->setFixedSize(32, 32);
    home_btn_->setToolTip("Home");
    home_btn_->setAutoRaise(true);
    home_btn_->setIcon(QIcon(iconPath + "home" + iconSuffix + ".svg"));
    home_btn_->setIconSize(QSize(16, 16));
    connect(home_btn_, &QToolButton::clicked, this, &BrowserWindow::onHome);
    title_layout->addWidget(home_btn_);

    // Spacer between nav buttons and URL bar
    title_layout->addSpacing(12);

    // URL Bar container with icons - Center
    QWidget* url_container = new QWidget(title_bar_);
    url_container->setObjectName("urlContainer");
    url_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QHBoxLayout* url_layout = new QHBoxLayout(url_container);
    url_layout->setContentsMargins(10, 0, 10, 0);
    url_layout->setSpacing(8);

    // Security icon (lock icon)
    security_btn_ = new QToolButton(url_container);
    security_btn_->setObjectName("securityButton");
    security_btn_->setFixedSize(18, 18);
    security_btn_->setAutoRaise(true);
    security_btn_->setIcon(QIcon(iconPath + "lock" + iconSuffix + ".svg"));
    security_btn_->setIconSize(QSize(14, 14));
    security_btn_->setToolTip("Secure Connection");
    connect(security_btn_, &QToolButton::clicked, this, &BrowserWindow::onSecurity);
    url_layout->addWidget(security_btn_);

    // URL input
    url_bar_ = new QLineEdit(url_container);
    url_bar_->setObjectName("urlBar");
    url_bar_->setPlaceholderText("Search or enter URL...");
    url_bar_->setFixedHeight(28);
    url_bar_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(url_bar_, &QLineEdit::returnPressed, this, &BrowserWindow::onUrlEntered);
    url_layout->addWidget(url_bar_, 1);

    // Bookmark star icon
    bookmark_btn_ = new QToolButton(url_container);
    bookmark_btn_->setObjectName("bookmarkButton");
    bookmark_btn_->setFixedSize(18, 18);
    bookmark_btn_->setAutoRaise(true);
    bookmark_btn_->setIcon(QIcon(iconPath + "star" + iconSuffix + ".svg"));
    bookmark_btn_->setIconSize(QSize(14, 14));
    bookmark_btn_->setToolTip("Bookmark this page");
    connect(bookmark_btn_, &QToolButton::clicked, this, &BrowserWindow::onBookmark);
    url_layout->addWidget(bookmark_btn_);

    title_layout->addWidget(url_container, 1);

    // Spacer between URL bar and right buttons
    title_layout->addStretch();

    // Menu button - before window controls
    menu_btn_ = new QToolButton(title_bar_);
    menu_btn_->setObjectName("menuButton");
    menu_btn_->setFixedSize(32, 32);
    menu_btn_->setToolTip("Menu");
    menu_btn_->setAutoRaise(true);
    menu_btn_->setIcon(QIcon(iconPath + "menu" + iconSuffix + ".svg"));
    menu_btn_->setIconSize(QSize(16, 16));
    connect(menu_btn_, &QToolButton::clicked, this, &BrowserWindow::onMenu);
    title_layout->addWidget(menu_btn_);

    title_layout->addSpacing(12);

    // Window control buttons - Right side
    min_btn_ = new QToolButton(title_bar_);
    min_btn_->setObjectName("minButton");
    min_btn_->setFixedSize(30, 30);
    min_btn_->setToolTip("Minimize");
    min_btn_->setAutoRaise(true);
    min_btn_->setIcon(QIcon(iconPath + "minimize" + iconSuffix + ".svg"));
    min_btn_->setIconSize(QSize(10, 10));
    connect(min_btn_, &QToolButton::clicked, this, &BrowserWindow::onMinimize);
    title_layout->addWidget(min_btn_);

    max_btn_ = new QToolButton(title_bar_);
    max_btn_->setObjectName("maxButton");
    max_btn_->setFixedSize(30, 30);
    max_btn_->setToolTip("Maximize");
    max_btn_->setAutoRaise(true);
    max_btn_->setIcon(QIcon(iconPath + "maximize" + iconSuffix + ".svg"));
    max_btn_->setIconSize(QSize(10, 10));
    connect(max_btn_, &QToolButton::clicked, this, &BrowserWindow::onMaximize);
    title_layout->addWidget(max_btn_);

    close_btn_ = new QToolButton(title_bar_);
    close_btn_->setObjectName("closeButton");
    close_btn_->setFixedSize(30, 30);
    close_btn_->setToolTip("Close");
    close_btn_->setAutoRaise(true);
    close_btn_->setIcon(QIcon(iconPath + "close" + iconSuffix + ".svg"));
    close_btn_->setIconSize(QSize(10, 10));
    connect(close_btn_, &QToolButton::clicked, this, &BrowserWindow::onClose);
    title_layout->addWidget(close_btn_);
}

void BrowserWindow::applyTheme() {
    QString accentColor = "#3b82f6"; // Fixed blue accent color
    
    bool isDark = Settings::instance().getDarkMode();
    
    QString bgColor = isDark ? "#030712" : "#b8e0ff"; // Changed from #f8fafc to #ffffff
    QString titleBg = isDark ? "#0f172a" : "#ffffff";
    QString inputBg = isDark ? "#1e293b" : "#f1f5f9";
    QString borderColor = isDark ? "#334155" : "#e2e8f0";
    QString textColor = isDark ? "#e2e8f0" : "#1e293b";
    QString tabBg = isDark ? "#0f172a" : "#e2e8f0";
    QString tabSelected = isDark ? "#030712" : "#ffffff";
    QString tabHover = isDark ? "#1e293b" : "#cbd5e1";
    
    QString style = QString(R"(
        QWidget#centralWidget { 
            background-color: %1; 
        }
        QWidget#titleBar { 
            background-color: %2; 
            border-bottom: 1px solid %3;
        }
        QWidget#urlContainer {
            background-color: %4;
            border-radius: 16px;
            border: 1px solid %3;
        }
        QWidget#urlContainer:focus-within {
            border: 2px solid %5;
        }
        QToolButton { 
            background-color: transparent; 
            border: none; 
            border-radius: 6px; 
            padding: 4px; 
            margin: 2px;
        }
        QToolButton:hover { 
            background-color: rgba(0, 0, 0, 0.1); 
        }
        QToolButton:pressed {
            background-color: rgba(0, 0, 0, 0.15);
        }
        QToolButton#closeButton:hover {
            background-color: #e81123;
        }
        QToolButton#closeButton:pressed {
            background-color: #f1707a;
        }
        QToolButton#minButton:hover, QToolButton#maxButton:hover {
            background-color: rgba(0, 0, 0, 0.1);
        }
        QLineEdit#urlBar { 
            background-color: transparent; 
            color: %6; 
            border: none;
            padding: 4px 8px; 
            font-size: 13px; 
        }
        QLineEdit#urlBar:focus { 
            outline: none;
        }
        QProgressBar { 
            background-color: transparent; 
            border: none; 
        }
        QProgressBar::chunk { 
            background-color: %5; 
        }
        QTabWidget::pane { 
            background-color: %1; 
            border: none; 
        }
        QTabBar::tab { 
            background-color: %7; 
            color: %6; 
            padding: 8px 16px; 
            border: none; 
            border-radius: 8px 8px 0 0; 
            margin: 0 2px; 
            min-width: 100px; 
        }
        QTabBar::tab:selected { 
            background-color: %8; 
            color: %6; 
        }
        QTabBar::tab:hover:!selected { 
            background-color: %9; 
            color: %6;
        }
    )").arg(bgColor, titleBg, borderColor, inputBg, accentColor, textColor, tabBg, tabSelected, tabHover);
    
    setStyleSheet(style);
}

void BrowserWindow::refreshIcons() {
    QString iconPath = QCoreApplication::applicationDirPath() + "/data/icons/";
    if (!QDir(iconPath).exists()) {
        iconPath = QCoreApplication::applicationDirPath() + "/../share/tsunami/data/icons/";
    }
    if (!QDir(iconPath).exists()) {
        iconPath = "/usr/share/tsunami/data/icons/";
    }

    bool isDark = Settings::instance().getDarkMode();
    QString iconSuffix = isDark ? "-white" : "";

    QList<QToolButton*> buttons = {
        back_btn_, forward_btn_, reload_btn_, home_btn_,
        security_btn_, bookmark_btn_, menu_btn_,
        min_btn_, max_btn_, close_btn_
    };

    QStringList iconNames = {
        "back", "forward", "reload", "home",
        "lock", "star", "menu",
        "minimize", "maximize", "close"
    };

    for (int i = 0; i < buttons.size(); ++i) {
        if (buttons[i]) {
            QString fullPath = iconPath + iconNames[i] + iconSuffix + ".svg";
            if (QFile::exists(fullPath)) {
                buttons[i]->setIcon(QIcon(fullPath));
            } else {
                QString fallbackPath = iconPath + iconNames[i] + ".svg";
                if (QFile::exists(fallbackPath)) {
                    buttons[i]->setIcon(QIcon(fallbackPath));
                }
            }
        }
    }
}

void BrowserWindow::onSettingsChanged() {
    qDebug() << "Settings changed - applying theme..." << Qt::endl;
    qDebug() << "Current dark mode:" << Settings::instance().getDarkMode();
    qDebug() << "Current accent color:" << Settings::instance().getAccentColor();
    applyTheme();
    refreshIcons();
    qDebug() << "Theme applied successfully" << Qt::endl;
}

void BrowserWindow::showOnboarding() {
    OnboardingDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        applyTheme();
        refreshIcons();
    }
}

void BrowserWindow::loadUrl(const QUrl& url) {
    auto view = qobject_cast<QWebEngineView*>(tab_widget_->currentWidget());
    if (view) {
        view->load(url);
        view->setFocus();
    }
}

void BrowserWindow::createNewTabWithUrl(const QUrl& url) {
    createNewTab(url);
}

void BrowserWindow::onNewTab() {
    createNewTab(QUrl::fromLocalFile(getInternalPagePath("newtab.html")));
}

void BrowserWindow::onCloseTab(int index) {
    if (tab_widget_->count() <= 1) {
        close();
        return;
    }
    auto widget = tab_widget_->widget(index);
    tab_widget_->removeTab(index);
    delete widget;
}

void BrowserWindow::onTabChanged(int index) {
    auto view = qobject_cast<QWebEngineView*>(tab_widget_->widget(index));
    if (view) {
        updateUrlDisplay(view->url());
    }
}

void BrowserWindow::updateUrlDisplay(const QUrl& url) {
    QString urlStr = url.toString();
    
    // Show friendly name for internal pages
    if (urlStr.contains("newtab.html")) {
        url_bar_->setText("");
        url_bar_->setPlaceholderText("Search or enter URL...");
    } else if (urlStr.contains("settings.html")) {
        url_bar_->setText("tsunami://settings");
    } else if (urlStr.contains("bookmarks.html")) {
        url_bar_->setText("tsunami://bookmarks");
    } else if (urlStr.contains("history.html")) {
        url_bar_->setText("tsunami://history");
    } else if (urlStr.contains("downloads.html")) {
        url_bar_->setText("tsunami://downloads");
    } else if (urlStr.contains("extensions.html")) {
        url_bar_->setText("tsunami://extensions");
    } else {
        url_bar_->setText(urlStr);
    }
}

void BrowserWindow::onTitleChanged(const QString& title) {
    int index = tab_widget_->indexOf(qobject_cast<QWidget*>(sender()));
    if (index >= 0) {
        tab_widget_->setTabText(index, title.left(32));
    }
}

void BrowserWindow::onLoadProgress(int progress) {
    progress_bar_->setValue(progress);
    if (progress == 100) {
        progress_bar_->hide();
    } else {
        progress_bar_->show();
    }
}

void BrowserWindow::onLoadFinished(bool ok) {
    if (!ok) {
        url_bar_->setStyleSheet("QLineEdit#urlBar { border: 1px solid #ef4444; }");
    } else {
        applyTheme();
    }
}

QWebEngineView* BrowserWindow::createNewTab(const QUrl& url) {
    QWebEngineView* view = new QWebEngineView();
    view->setUrl(url);

    connect(view, &QWebEngineView::urlChanged, this, &BrowserWindow::onUrlChanged);
    connect(view, &QWebEngineView::titleChanged, this, &BrowserWindow::onTitleChanged);
    connect(view, &QWebEngineView::loadProgress, this, &BrowserWindow::onLoadProgress);
    connect(view, &QWebEngineView::loadFinished, this, &BrowserWindow::onLoadFinished);

    WebView::setupPage(view->page());

    int index = tab_widget_->addTab(view, "New Tab");
    tab_widget_->setCurrentIndex(index);

    updateUrlDisplay(url);
    view->setFocus();

    return view;
}

void BrowserWindow::onUrlChanged(const QUrl& url) {
    updateUrlDisplay(url);
}

void BrowserWindow::onUrlEntered() {
    QString input = url_bar_->text();
    if (input.isEmpty()) return;

    QUrl url = QUrl::fromUserInput(input);
    if (!url.isValid() || (!input.contains(".") || input.contains(" "))) {
        // Treat as search query
        url = QUrl(QString("https://duckduckgo.com/?q=") + QUrl::toPercentEncoding(input));
    }

    loadUrl(url);
}

void BrowserWindow::onBack() {
    auto view = qobject_cast<QWebEngineView*>(tab_widget_->currentWidget());
    if (view) {
        QWebEngineHistory* history = view->history();
        if (history && history->canGoBack()) {
            view->back();
        }
    }
}

void BrowserWindow::onForward() {
    auto view = qobject_cast<QWebEngineView*>(tab_widget_->currentWidget());
    if (view) {
        QWebEngineHistory* history = view->history();
        if (history && history->canGoForward()) {
            view->forward();
        }
    }
}

void BrowserWindow::onReload() {
    auto view = qobject_cast<QWebEngineView*>(tab_widget_->currentWidget());
    if (view) {
        view->reload();
    }
}

void BrowserWindow::onMenu() {
    QMenu* menu = new QMenu(this);
    
    bool isDark = Settings::instance().getDarkMode();
    QString accentColor = "#3b82f6"; // Fixed blue accent color
    
    QString bgColor = isDark ? "#0f172a" : "#ffffff";
    QString textColor = isDark ? "#e2e8f0" : "#1e293b";
    QString borderColor = isDark ? "#1e293b" : "#e2e8f0";
    QString selectedBg = isDark ? "#1e293b" : "#f1f5f9";
    
    menu->setStyleSheet(QString(R"(
        QMenu {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 8px;
            padding: 8px;
        }
        QMenu::item {
            color: %3;
            padding: 8px 24px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background-color: %4;
        }
        QMenu::item:selected {
            background-color: %5;
        }
        QMenu::separator {
            background-color: %2;
            height: 1px;
            margin: 6px 0px;
        }
    )").arg(bgColor, borderColor, textColor, selectedBg, accentColor));
    
    menu->addAction("New Tab", this, &BrowserWindow::onNewTab);
    menu->addAction("Open File...", this, &BrowserWindow::onOpenFile);
    menu->addSeparator();
    menu->addAction("Bookmarks", this, &BrowserWindow::onBookmarks);
    menu->addAction("History", this, &BrowserWindow::onHistory);
    menu->addAction("Downloads", this, &BrowserWindow::onDownloads);
    menu->addAction("View Page Source", this, &BrowserWindow::onViewPageSource);
    menu->addSeparator();
    menu->addAction("Settings", this, &BrowserWindow::onSettings);
    menu->addSeparator();
    menu->addAction("Exit", this, &QWidget::close);

    QPoint pos = menu_btn_->mapToGlobal(QPoint(0, menu_btn_->height()));
    menu->exec(pos);
    delete menu;
}

void BrowserWindow::onOpenFile() {
    QString file = QFileDialog::getOpenFileName(this, "Open File", QString(),
        "Web Files (*.html *.htm *.txt);;All Files (*)");
    if (!file.isEmpty()) {
        createNewTab(QUrl::fromLocalFile(file));
    }
}

void BrowserWindow::onSavePage() {
    QString file = QFileDialog::getSaveFileName(this, "Save Page As", "page.html",
        "HTML Files (*.html);;All Files (*)");
    if (!file.isEmpty()) {
        auto view = qobject_cast<QWebEngineView*>(tab_widget_->currentWidget());
        if (view) {
            view->page()->save(file, QWebEngineDownloadRequest::CompleteHtmlSaveFormat);
        }
    }
}

void BrowserWindow::onClearHistory() {
    SeaBrowser::HistoryManager::instance().clear_history();
    QMessageBox::information(this, "Clear History", "Browsing history has been cleared.");
}

void BrowserWindow::onHistory() {
    Tsunami::HistoryWindow* historyWindow = new Tsunami::HistoryWindow(this);
    historyWindow->setAttribute(Qt::WA_DeleteOnClose);
    historyWindow->show();
}

void BrowserWindow::onBookmarks() {
    Tsunami::BookmarksWindow* bookmarksWindow = new Tsunami::BookmarksWindow(this);
    bookmarksWindow->setAttribute(Qt::WA_DeleteOnClose);
    bookmarksWindow->show();
}

void BrowserWindow::onDownloads() {
    Tsunami::DownloadsWindow* downloadsWindow = new Tsunami::DownloadsWindow(this);
    downloadsWindow->setAttribute(Qt::WA_DeleteOnClose);
    downloadsWindow->show();
}

void BrowserWindow::onExtensions() {
    Tsunami::ExtensionsWindow* extensionsWindow = new Tsunami::ExtensionsWindow(this);
    extensionsWindow->setAttribute(Qt::WA_DeleteOnClose);
    extensionsWindow->show();
}

void BrowserWindow::onSettings() {
    Tsunami::SettingsDialog::showDialog(this);
}

void BrowserWindow::onAbout() {
    QMessageBox::about(this, "About Tsunami",
        "<h2>Tsunami Browser</h2>"
        "<p>Version 1.0.0</p>"
        "<p>A privacy-focused web browser built with Qt6.</p>"
        "<p>Built by Sea Software</p>");
}

void BrowserWindow::onFullscreen() {
    if (this->isFullScreen()) {
        this->showNormal();
    } else {
        this->showFullScreen();
    }
}

void BrowserWindow::onHome() {
    QString homepage = Settings::instance().getHomepage();
    if (homepage.isEmpty() || homepage == "tsunami://newtab") {
        loadUrl(QUrl::fromLocalFile(getInternalPagePath("newtab.html")));
    } else {
        loadUrl(QUrl(homepage));
    }
}

void BrowserWindow::onBookmark() {
    auto view = qobject_cast<QWebEngineView*>(tab_widget_->currentWidget());
    if (view) {
        QString url = view->url().toString();
        QString title = view->title();
        // TODO: Add bookmark implementation
        QMessageBox::information(this, "Bookmark", "Bookmark added: " + title);
    }
}

void BrowserWindow::onSecurity() {
    auto view = qobject_cast<QWebEngineView*>(tab_widget_->currentWidget());
    if (view) {
        QUrl url = view->url();
        QString scheme = url.scheme();
        if (scheme == "https") {
            QMessageBox::information(this, "Security", "This is a secure HTTPS connection.");
        } else if (scheme == "http") {
            QMessageBox::warning(this, "Security", "This is an insecure HTTP connection.");
        } else {
            QMessageBox::information(this, "Security", "Connection information: " + scheme);
        }
    }
}

void BrowserWindow::onMinimize() {
    showMinimized();
}

void BrowserWindow::onMaximize() {
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void BrowserWindow::onClose() {
    close();
}

QString BrowserWindow::getInternalPagePath(const QString& page) {
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList searchPaths = {
        appDir + "/data/pages/" + page,
        appDir + "/../data/pages/" + page,
        "/usr/share/tsunami/data/pages/" + page
    };
    
    for (const QString& path : searchPaths) {
        if (QFile::exists(path)) {
            return QFileInfo(path).absoluteFilePath();
        }
    }
    
    return appDir + "/data/pages/" + page;
}

void BrowserWindow::saveSession() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Tsunami", "Browser");
    settings.beginGroup("Session");
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("tabCount", tab_widget_->count());

    QStringList urls;
    for (int i = 0; i < tab_widget_->count(); ++i) {
        auto view = qobject_cast<QWebEngineView*>(tab_widget_->widget(i));
        if (view) {
            urls.append(view->url().toString());
        }
    }
    settings.setValue("urls", urls);
    settings.setValue("currentIndex", tab_widget_->currentIndex());
    settings.endGroup();
}

void BrowserWindow::restoreSession() {
    // Only create one new tab on startup
    if (tab_widget_->count() == 0) {
        createNewTab(QUrl::fromLocalFile(getInternalPagePath("newtab.html")));
    }
}

void BrowserWindow::closeEvent(QCloseEvent* event) {
    saveSession();
    event->accept();
}

void BrowserWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_F11) {
        onFullscreen();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void BrowserWindow::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (event->pos().y() < title_bar_->height()) {
            if (isMaximized()) {
                showNormal();
            } else {
                showMaximized();
            }
        }
    }
}

void BrowserWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (event->pos().y() < title_bar_->height()) {
            is_dragging_ = true;
            drag_position_ = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void BrowserWindow::mouseMoveEvent(QMouseEvent* event) {
    if (is_dragging_ && (event->buttons() & Qt::LeftButton)) {
        if (isMaximized()) {
            showNormal();
        }
        move(event->globalPos() - drag_position_);
        event->accept();
    }
}

void BrowserWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        is_dragging_ = false;
    }
}

void BrowserWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void BrowserWindow::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls()) {
        for (const QUrl& url : event->mimeData()->urls()) {
            createNewTab(url);
        }
    }
}

bool BrowserWindow::eventFilter(QObject* obj, QEvent* event) {
    return QMainWindow::eventFilter(obj, event);
}

} // namespace Tsunami

void BrowserWindow::onViewPageSource() {
    auto view = qobject_cast<QWebEngineView*>(tab_widget_->currentWidget());
    if (view) {
        QString url = view->url().toString();
        if (!url.isEmpty()) {
            createNewTab(QUrl("view-source:" + url));
        }
    }
}
