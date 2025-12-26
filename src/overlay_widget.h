#pragma once
#include <QWidget>
#include <QEvent>
#include <QPushButton>
#include "tools/viewmodeswitch.h"

class OverlayWidget : public QWidget {
    Q_OBJECT
public:
    OverlayWidget(QWidget *parent = nullptr);
    ViewModeSwitch *modeSwitch;

protected:
    void setupUi();
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;
};
