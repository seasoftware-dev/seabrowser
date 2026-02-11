/*
 * Tsunami Browser - Tab Manager (Qt6)
 * tab_manager.cpp
 */

#include "tab_manager.h"
#include "web_view.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <iostream>

namespace Tsunami {

TabManager::TabManager(QWidget* parent)
    : QTabWidget(parent) {
    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);
}

TabManager::~TabManager() {
}

void TabManager::createTab(const QString& url) {
    auto web_view = new QWebEngineView();
    WebView::setupPage(web_view->page());
    
    auto tab_widget = new QWidget();
    auto layout = new QVBoxLayout(tab_widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(web_view);
    
    connect(web_view, &QWebEngineView::loadProgress, this, &TabManager::onLoadProgress);
    connect(web_view, &QWebEngineView::titleChanged, this, &TabManager::onTitleChanged);
    connect(web_view, &QWebEngineView::urlChanged, this, &TabManager::onUrlChanged);
    
    int index = addTab(tab_widget, "New Tab");
    setCurrentIndex(index);
    
    if (!url.isEmpty()) {
        web_view->load(QUrl(url));
    }
}

void TabManager::closeCurrentTab() {
    int index = currentIndex();
    if (index != -1) {
        QWidget* tab = widget(index);
        removeTab(index);
        delete tab;
    }
}

QWebEngineView* TabManager::getCurrentWebView() {
    int index = currentIndex();
    if (index == -1) return nullptr;
    
    QWidget* tab = widget(index);
    for (auto child : tab->findChildren<QWebEngineView*>()) {
        return child;
    }
    return nullptr;
}

void TabManager::onTabClose(int index) {
    if (index >= 0 && index < count()) {
        QWidget* tab = widget(index);
        removeTab(index);
        delete tab;
    }
}

void TabManager::onUrlChanged(const QUrl& url) {
    emit urlChanged(url.toString());
}

void TabManager::onTitleChanged(const QString& title) {
    QWebEngineView* view = qobject_cast<QWebEngineView*>(sender());
    if (view) {
        int index = indexOf(view->parentWidget());
        if (index >= 0) {
            setTabText(index, title.left(30));
            emit titleChanged(title);
        }
    }
}

void TabManager::onLoadProgress(int progress) {
    Q_UNUSED(progress)
}

} // namespace Tsunami
