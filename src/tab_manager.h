#pragma once

#include <QTabWidget>
#include <QLineEdit>
#include <QObject>
#include <QWebEngineView>

namespace Tsunami {

class TabManager : public QTabWidget {
    Q_OBJECT
public:
    explicit TabManager(QWidget* parent = nullptr);
    ~TabManager();
    
    void createTab(const QString& url);
    void closeCurrentTab();
    QWebEngineView* getCurrentWebView();
    
signals:
    void urlChanged(const QString& url);
    void titleChanged(const QString& title);
    
private slots:
    void onTabClose(int index);
    void onUrlChanged(const QUrl& url);
    void onTitleChanged(const QString& title);
    void onLoadProgress(int progress);
};

} // namespace Tsunami
