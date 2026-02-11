#pragma once

#include <QWidget>
#include <QPoint>
#include <QSize>

namespace Tsunami {

class WindowManager {
public:
    static void enableFramelessWindow(QWidget* window);
    static void setTitleBarHeight(QWidget* window, int height);
    static void startDrag(QWidget* window, const QPoint& pos);
    static void startResize(QWidget* window, int edge, const QPoint& pos);

private:
    static int titleBarHeight_;
};

} // namespace Tsunami
