#ifndef QDOUBLESIDER_H
#define QDOUBLESIDER_H

#include <QSlider>
#include <cmath>
#include <QObject>

class QDoubleSlider : public QSlider
{
    Q_OBJECT

public:
    explicit QDoubleSlider(Qt::Orientation orientation, QWidget *parent = nullptr);
    explicit QDoubleSlider(QWidget *parent = nullptr);
    ~QDoubleSlider() override;

    double value() const;
    void setPrecision(int precision);
    int precision() const;

    void setRange(double min, double max);
    double minimum() const;
    double maximum() const;

    void setValue(double value);

signals:
    void valueChanged(double value);

protected:
    void sliderChange(SliderChange change) override;

private:
    int m_precision;
};

#endif // QDOUBLESIDER_H
