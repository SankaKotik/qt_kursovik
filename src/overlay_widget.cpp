#include "overlay_widget.h"
#include <qevent.h>

OverlayWidget::OverlayWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setupUi();
}

void OverlayWidget::setupUi() {
    modeSwitch = new ViewModeSwitch(this);
}

void OverlayWidget::paintEvent(QPaintEvent *event) {
    
}

void OverlayWidget::resizeEvent(QResizeEvent* event) {
    modeSwitch->setGeometry(width() * 0.5 - 50, height() * 0.9, 100, 36);
    QRegion mask;
    mask += modeSwitch->geometry();
    setMask(mask);
}
