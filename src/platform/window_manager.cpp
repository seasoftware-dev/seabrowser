#include "window_manager.h"
#include <QWidget>
#include <QApplication>
#include <QDebug>
#include <QMouseEvent>

namespace Tsunami {

int WindowManager::titleBarHeight_ = 48;

void WindowManager::enableFramelessWindow(QWidget* window) {
    window->setWindowFlags(Qt::FramelessWindowHint);
    window->setAttribute(Qt::WA_TranslucentBackground);
}

void WindowManager::setTitleBarHeight(QWidget* window, int height) {
    Q_UNUSED(window);
    titleBarHeight_ = height;
}

void WindowManager::startDrag(QWidget* window, const QPoint& pos) {
    if (window->isMaximized()) return;
    window->move(window->pos() + pos - window->geometry().topLeft());
}

void WindowManager::startResize(QWidget* window, int edge, const QPoint& pos) {
    Q_UNUSED(window);
    Q_UNUSED(edge);
    Q_UNUSED(pos);
}

} // namespace Tsunami
