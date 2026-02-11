#pragma once

#include <QFrame>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <functional>

namespace Tsunami {

class CustomMenu : public QFrame {
    Q_OBJECT
public:
    explicit CustomMenu(QWidget* parent = nullptr);
    void showAt(const QPoint& pos);
    void addAction(const QString& text, const std::function<void()>& callback);
    void addSeparator();
    void addSection(const QString& title);

protected:
    void paintEvent(QPaintEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void setupUi();
    
    QWidget* contentWidget_;
    QVBoxLayout* contentLayout_;
    QGraphicsDropShadowEffect* shadowEffect_;
};

} // namespace Tsunami