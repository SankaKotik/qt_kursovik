#pragma once

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QLineF>
#include <QString>
#include <QTransform>
#include <QColor>

class SketchWidget : public QWidget {
    Q_OBJECT
public:
    explicit SketchWidget(QWidget *parent = nullptr);

    void addPoint(const QPointF &point);
    void addLine(const QLineF &line);
    void addDimensionLine(const QLineF &line);

    QVector<QVector<QPointF>> findClosedContours(qreal tolerance = 0.1) const;

    // Управление сеткой
    void setShowGrid(bool show);
    void setGridStep(qreal stepX, qreal stepY);
    void setGridColor(const QColor &color);
    void setMajorGridStep(int majorStep);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    struct DimensionLine {
        QLineF line;
        QString text;
    };

    QVector<QPointF> m_points;
    QVector<QLineF> m_lines;
    QVector<DimensionLine> m_dimensionLines;
    
    // Параметры навигации
    QPointF m_pan;
    qreal m_scale = 1.0;
    bool m_dragging = false;
    QPointF m_lastDragPos;
    
    // Параметры сетки
    bool m_showGrid = false;
    qreal m_gridStepX = 10.0;
    qreal m_gridStepY = 10.0;
    QColor m_gridColor = QColor(200, 200, 200, 64); // Полупрозрачная сетка
    QColor m_gridMajorColor = QColor(150, 150, 150, 64);
    int m_majorGridStep = 5; // Каждая 5-я линия - основная

    // Вспомогательные методы
    QPointF screenToWorld(const QPointF &screenPos) const;
    QPointF worldToScreen(const QPointF &worldPos) const;
    void drawGrid(QPainter &painter);
};