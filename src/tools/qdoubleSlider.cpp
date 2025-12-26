#include "qdoubleSlider.h"
#include <QDebug>

QDoubleSlider::QDoubleSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent), m_precision(2)
{
}

QDoubleSlider::QDoubleSlider(QWidget *parent)
    : QSlider(Qt::Horizontal, parent), m_precision(2)
{
}

QDoubleSlider::~QDoubleSlider()
{
}

double QDoubleSlider::value() const
{
    return QSlider::value() / std::pow(10.0, m_precision);
}

void QDoubleSlider::setPrecision(int precision)
{
    m_precision = qMax(0, precision); // Убедимся, что точность >= 0
}

int QDoubleSlider::precision() const
{
    return m_precision;
}

void QDoubleSlider::setRange(double min, double max)
{
    int minInt = static_cast<int>(std::round(min * std::pow(10.0, m_precision)));
    int maxInt = static_cast<int>(std::round(max * std::pow(10.0, m_precision)));
    QSlider::setRange(minInt, maxInt);
}

double QDoubleSlider::minimum() const
{
    return QSlider::minimum() / std::pow(10.0, m_precision);
}

double QDoubleSlider::maximum() const
{
    return QSlider::maximum() / std::pow(10.0, m_precision);
}

void QDoubleSlider::setValue(double value)
{
    int intValue = static_cast<int>(std::round(value * std::pow(10.0, m_precision)));
    QSlider::setValue(intValue);
}

void QDoubleSlider::sliderChange(SliderChange change)
{
    QSlider::sliderChange(change);
    if (change == SliderValueChange) {
        emit valueChanged(this->value());
    }
}
