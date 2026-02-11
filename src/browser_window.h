#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QProgressBar>
#include <QToolButton>
#include <QString>
#include <QUrl>
#include <QWebEngineView>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>

#include "settings/settings.h"

namespace Tsunami {

class BrowserWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit BrowserWindow(QWidget* parent = nullptr);
    ~BrowserWindow();
    
    void show();
    void loadUrl(const QUrl& url);
    void createNewTabWithUrl(const QUrl& url);
    
private slots:
    void onNewTab();
    void onCloseTab(int index);
    void onTabChanged(int index);
    void updateUrlDisplay(const QUrl& url);
    void onTitleChanged(const QString& title);
    void onLoadProgress(int progress);
    void onLoadFinished(bool ok);
    void onUrlChanged(const QUrl& url);
    void onUrlEntered();
    void onBack();
    void onForward();
    void onReload();
    void onMenu();
    void onOpenFile();
    void onSavePage();
    void onClearHistory();
    void onHistory();
    void onBookmarks();
    void onDownloads();
    void onExtensions();
    void onSettings();
    void onAbout();
    void onFullscreen();
    void onHome();
    void onBookmark();
    void onSecurity();
    void onMinimize();
    void onMaximize();
    void onClose();
    void onViewPageSource();
    void onSettingsChanged();
    
protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    
private:
    void setupUi();
    void setupTitleBar();
    void applyTheme();
    void refreshIcons();
    void showOnboarding();
    QWebEngineView* createNewTab(const QUrl& url);
    QString getInternalPagePath(const QString& page);
    void saveSession();
    void restoreSession();
    
    QWidget* central_widget_;
    QTabWidget* tab_widget_;
    QLineEdit* url_bar_;
    QWidget* title_bar_;
    QProgressBar* progress_bar_;
    QToolButton* menu_btn_;
    QToolButton* back_btn_;
    QToolButton* forward_btn_;
    QToolButton* reload_btn_;
    QToolButton* home_btn_;
    QToolButton* bookmark_btn_;
    QToolButton* security_btn_;
    QToolButton* min_btn_;
    QToolButton* max_btn_;
    QToolButton* close_btn_;
    
    bool is_dragging_;
    QPoint drag_position_;
};

} // namespace Tsunami
