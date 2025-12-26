#include "viewmodeswitch.h"
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

ViewModeSwitch::ViewModeSwitch(QWidget *parent)
    : QWidget(parent), m_currentMode(Mode2D)
{
    setObjectName("ViewModeSwitch");
    setStyleSheet(R"(
        #ViewModeSwitch {
            background-color: #202020;
            border-radius: 12px;
        }
    )");
    setAttribute(Qt::WA_StyledBackground, true);

    // Создаем подложку для активного режима
    m_activeBg = new QWidget(this);
    m_activeBg->setStyleSheet(R"(
        background-color: rgba(128, 128, 128, 20%);
        border-radius: 8px;
        border: 1px solid rgba(128, 128, 128, 10%);
    )");
    m_activeBg->setGeometry(4, 4, 42, 28);

    // Анимация перемещения подложки
    m_animation = new QPropertyAnimation(m_activeBg, "geometry");
    m_animation->setDuration(250);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);

    // Кнопки
    m_button2D = new QPushButton("2D", this);
    m_button3D = new QPushButton("3D", this);

    for (auto btn : {m_button2D, m_button3D}) {
        btn->setFlat(true);
        btn->setFixedSize(50, 36);
        btn->setStyleSheet(R"(
            QPushButton {
                color: #ffffff;
                background-color: transparent;
                border: none;
                border-radius: 8px;
                margin: 4px;
            }
            QPushButton:hover {
                background-color: rgba(128, 128, 128, 10%);
            }
        )");
        connect(btn, &QPushButton::clicked, this, [=]() {
            if (btn == m_button2D && m_currentMode != Mode2D) {
                m_currentMode = Mode2D;
                updateActiveBgPosition(true);
                emit modeChanged(m_currentMode);
            } else if (btn == m_button3D && m_currentMode != Mode3D) {
                m_currentMode = Mode3D;
                updateActiveBgPosition(true);
                emit modeChanged(m_currentMode);
            }
        });
    }

    // Макет
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_button2D);
    m_layout->addWidget(m_button3D);
    setLayout(m_layout);

    // Устанавливаем начальную позицию подложки
    updateActiveBgPosition(false);
}

ViewModeSwitch::~ViewModeSwitch()
{
}

ViewModeSwitch::Mode ViewModeSwitch::currentMode() const
{
    return m_currentMode;
}

void ViewModeSwitch::onButtonClicked(int mode) {
    
}

void ViewModeSwitch::updateActiveBgPosition(bool animated)
{
    // Вычисляем позицию подложки относительно активной кнопки
    QWidget *targetButton = (m_currentMode == Mode2D) ? m_button2D : m_button3D;
    QRect targetRect = targetButton->geometry();
    
    // Скорректируем позицию для плавного перехода
    int bgX = targetRect.x() + 4;
    int bgY = targetRect.y() + 4;
    int bgWidth = targetRect.width() - 8;
    int bgHeight = targetRect.height() - 8;

    if (animated && m_animation->state() == QAbstractAnimation::Running)
        m_animation->stop();

    if (animated) {
        m_animation->setStartValue(m_activeBg->geometry());
        m_animation->setEndValue(QRect(bgX, bgY, bgWidth, bgHeight));
        m_animation->start();
    } else {
        m_activeBg->setGeometry(bgX, bgY, bgWidth, bgHeight);
    }
}

// Обработка изменения размера
void ViewModeSwitch::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateActiveBgPosition(false);
}