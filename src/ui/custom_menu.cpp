#include "custom_menu.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QPropertyAnimation>
#include <QApplication>
#include <QScreen>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

namespace Tsunami {

CustomMenu::CustomMenu(QWidget* parent)
    : QFrame(parent)
    , contentWidget_(nullptr)
    , contentLayout_(nullptr)
    , shadowEffect_(nullptr)
{
    setupUi();
}

void CustomMenu::setupUi() {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    
    setFixedWidth(220);
    
    // Main container
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Content widget with dark background
    contentWidget_ = new QWidget(this);
    contentWidget_->setObjectName("menuContent");
    contentWidget_->setStyleSheet(R"(
        QWidget#menuContent {
            background-color: #0f172a;
            border: 1px solid #1e293b;
            border-radius: 8px;
        }
    )");
    
    contentLayout_ = new QVBoxLayout(contentWidget_);
    contentLayout_->setContentsMargins(6, 6, 6, 6);
    contentLayout_->setSpacing(2);
    
    mainLayout->addWidget(contentWidget_);
    
    // Add shadow effect
    shadowEffect_ = new QGraphicsDropShadowEffect(this);
    shadowEffect_->setBlurRadius(20);
    shadowEffect_->setColor(QColor(0, 0, 0, 80));
    shadowEffect_->setOffset(0, 4);
    contentWidget_->setGraphicsEffect(shadowEffect_);
    
    // Install event filter to close on focus out
    qApp->installEventFilter(this);
}

void CustomMenu::showAt(const QPoint& pos) {
    // Ensure menu stays on screen
    QPoint adjustedPos = pos;
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        if (pos.x() + width() > screenGeometry.right()) {
            adjustedPos.setX(screenGeometry.right() - width() - 10);
        }
        if (pos.y() + height() > screenGeometry.bottom()) {
            adjustedPos.setY(pos.y() - height());
        }
    }
    
    move(adjustedPos);
    show();
    raise();
    activateWindow();
    
    // Animate in
    setWindowOpacity(0.0);
    QPropertyAnimation* fadeIn = new QPropertyAnimation(this, "windowOpacity");
    fadeIn->setDuration(150);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

void CustomMenu::addAction(const QString& text, const std::function<void()>& callback) {
    QPushButton* button = new QPushButton(text, contentWidget_);
    button->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #e2e8f0;
            border: none;
            border-radius: 6px;
            padding: 6px 12px;
            text-align: left;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #1e293b;
        }
        QPushButton:pressed {
            background-color: #334155;
        }
    )");
    
    connect(button, &QPushButton::clicked, this, [this, callback]() {
        callback();
        close();
    });
    
    contentLayout_->addWidget(button);
}

void CustomMenu::addSeparator() {
    QFrame* line = new QFrame(contentWidget_);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #1e293b;");
    line->setFixedHeight(1);
    contentLayout_->addWidget(line);
}

void CustomMenu::addSection(const QString& title) {
    QLabel* label = new QLabel(title, contentWidget_);
    label->setStyleSheet(R"(
        color: #64748b;
        font-size: 10px;
        font-weight: 600;
        padding: 6px 12px 2px 12px;
    )");
    contentLayout_->addWidget(label);
}

void CustomMenu::paintEvent(QPaintEvent* event) {
    QFrame::paintEvent(event);
}

void CustomMenu::focusOutEvent(QFocusEvent* event) {
    QFrame::focusOutEvent(event);
    close();
}

bool CustomMenu::eventFilter(QObject* obj, QEvent* event) {
    // Close menu when clicking outside
    if (event->type() == QEvent::MouseButtonPress) {
        if (!this->isAncestorOf(qobject_cast<QWidget*>(obj))) {
            close();
        }
    }
    return QFrame::eventFilter(obj, event);
}

} // namespace Tsunami