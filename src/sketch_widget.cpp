#include "sketch_widget.h"
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include <QDebug>
#include <QRectF>

SketchWidget::SketchWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(600, 400);
    setAutoFillBackground(true);
    setMouseTracking(true);
}

void SketchWidget::setShowGrid(bool show) {
    m_showGrid = show;
    update();
}

void SketchWidget::setGridStep(qreal stepX, qreal stepY) {
    m_gridStepX = std::max(0.1, stepX); // Минимальный шаг 0.1
    m_gridStepY = std::max(0.1, stepY);
    update();
}

void SketchWidget::setGridColor(const QColor &color) {
    m_gridColor = color;
    update();
}

void SketchWidget::setMajorGridStep(int majorStep) {
    m_majorGridStep = std::max(1, majorStep);
    update();
}

void SketchWidget::clear() {
    m_points.clear();
    m_lines.clear();
    m_dimensionLines.clear();
    update();
}

void drawArrow(QPainter *painter, QPointF start, QPointF end, double arrowSize = 10) {
    // Вычисляем угол наклона линии
    double angle = std::atan2(end.y() - start.y(), end.x() - start.x());

    // Создаем три точки для наконечника (треугольника)
    QPointF arrowP1 = end - QPointF(sin(angle - M_PI / 12) * arrowSize,
                                    cos(angle - M_PI / 12) * arrowSize);
    QPointF arrowP2 = end - QPointF(sin(angle + M_PI / 12) * arrowSize,
                                    cos(angle + M_PI / 12) * arrowSize);
    QPointF arrowP3 = end - QPointF(sin(angle) * arrowSize * 0.8,
                                    cos(angle) * arrowSize * 0.8);

    // Рисуем наконечник как закрашенный полигон
    painter->drawPolygon(QPolygonF() << end << arrowP1 << arrowP3 << arrowP2);
}

QPointF SketchWidget::screenToWorld(const QPointF &screenPos) const {
    return (screenPos - m_pan) / m_scale;
}

QPointF SketchWidget::worldToScreen(const QPointF &worldPos) const {
    return worldPos * m_scale + m_pan;
}

void SketchWidget::addPoint(const QPointF &point) {
    m_points.append(point);
    update();
}

void SketchWidget::addLine(const QLineF &line) {
    addPoint(line.p1());
    addPoint(line.p2());
    m_lines.append(line);
    update();
}

void SketchWidget::addDimensionLine(const QLineF &line) {
    m_dimensionLines.append({line, QString::number(line.length()) + "mm"});
    update();
}

void SketchWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        m_dragging = true;
        m_lastDragPos = event->position();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void SketchWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging) {
        QPointF delta = (event->position() - m_lastDragPos);
        m_pan += delta;
        m_lastDragPos = event->position();
        update();
        event->accept();
    } else {
        QWidget::mouseMoveEvent(event);
    }
}

void SketchWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void SketchWidget::wheelEvent(QWheelEvent *event) {
    const qreal zoomFactor = 1.15;
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;
    
    qreal step = 0;
    if (!numPixels.isNull()) {
        step = numPixels.y();
    } else if (!numDegrees.isNull()) {
        step = numDegrees.y() / 15;
    }
    
    if (step == 0) {
        event->accept();
        return;
    }
    
    qreal factor = step > 0 ? zoomFactor : 1.0 / zoomFactor;
    
    // Сохраняем позицию курсора в мировых координатах
    QPointF cursorScenePos = screenToWorld(event->position());
    
    // Применяем масштабирование
    m_scale *= factor;
    m_scale = qBound(0.05, m_scale, 20.0); // Ограничиваем диапазон масштаба
    
    // Пересчитываем сдвиг для сохранения позиции курсора
    m_pan = event->position() - cursorScenePos * m_scale;
    
    update();
    event->accept();
}

void SketchWidget::drawGrid(QPainter &painter) {
    if (!m_showGrid || m_gridStepX <= 0 || m_gridStepY <= 0 || m_scale < 0.25) return;
    
    painter.save();
    
    // Рассчитываем границы видимой области в мировых координатах
    QPointF topLeft = screenToWorld(QPointF(0, 0));
    QPointF bottomRight = screenToWorld(QPointF(width(), height()));
    
    // Определяем шаги для сетки в зависимости от масштаба
    qreal visibleWidth = bottomRight.x() - topLeft.x();
    qreal visibleHeight = bottomRight.y() - topLeft.y();
    
    // Адаптивное отображение сетки - не рисуем слишком густую сетку
    qreal minScreenStep = 5.0; // Минимальное расстояние между линиями в пикселях
    qreal adaptiveStepX = m_gridStepX;
    qreal adaptiveStepY = m_gridStepY;
    
    while (visibleWidth * m_scale / adaptiveStepX > width() / minScreenStep) {
        adaptiveStepX *= 2.0;
    }
    
    while (visibleHeight * m_scale / adaptiveStepY > height() / minScreenStep) {
        adaptiveStepY *= 2.0;
    }
    
    // Определение начальных точек сетки
    qreal startX = std::floor(topLeft.x() / adaptiveStepX) * adaptiveStepX;
    qreal startY = std::floor(topLeft.y() / adaptiveStepY) * adaptiveStepY;
    
    // Толщина линий сетки зависит от масштаба
    qreal thinLineThickness = 0.5 / m_scale;
    qreal thickLineThickness = 1.0 / m_scale;
    
    // Рисуем вертикальные линии
    for (qreal x = startX; x <= bottomRight.x(); x += adaptiveStepX) {
        bool isMajor = (static_cast<int>(std::round(x / m_gridStepX)) % m_majorGridStep == 0);
        QPen pen(isMajor ? m_gridMajorColor : m_gridColor, 
                isMajor ? thickLineThickness : thinLineThickness, 
                isMajor ? Qt::SolidLine : Qt::DotLine);
        painter.setPen(pen);
        painter.drawLine(QLineF(x, topLeft.y(), x, bottomRight.y()));
    }
    
    // Рисуем горизонтальные линии
    for (qreal y = startY; y <= bottomRight.y(); y += adaptiveStepY) {
        bool isMajor = (static_cast<int>(std::round(y / m_gridStepY)) % m_majorGridStep == 0);
        QPen pen(isMajor ? m_gridMajorColor : m_gridColor, 
                isMajor ? thickLineThickness : thinLineThickness, 
                isMajor ? Qt::SolidLine : Qt::DotLine);
        painter.setPen(pen);
        painter.drawLine(QLineF(topLeft.x(), y, bottomRight.x(), y));
    }
    
    painter.restore();
}

void SketchWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);\
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(20, 20, 20));
    
    // Применяем преобразования
    painter.translate(m_pan);
    painter.scale(m_scale, m_scale);
    
    // Рисуем сетку ПЕРЕД всеми элементами
    drawGrid(painter);
    
    // Рассчитываем размеры с учетом масштаба
    qreal basePointSize = 4.0 / m_scale;
    qreal baseLineThickness = 1.5 / m_scale;
    qreal dimLineThickness = 1.0 / m_scale;
    qreal tickLength = 8.0 / m_scale;
    qreal fontSize = 10.0 / m_scale;
    qreal sizeLineOffset = 18.0 * m_scale;
    qreal serifOffset = 20.0 * m_scale;
    qreal arrowSize = 20.0 / m_scale;

    painter.setPen(QPen(QColor(255, 116, 108), 0.5 * baseLineThickness));
    painter.drawLine(-200, 0, 200, 0);
    painter.setPen(QPen(QColor(128, 239, 128), 0.5 * baseLineThickness));
    painter.drawLine(0, -200, 0, 200);
    
    // Рисуем точки
    painter.setPen(QPen(QColor(76, 201, 255), 0.5 / m_scale));
    painter.setBrush(QColor(76, 201, 255));
    for (const auto &point : m_points) {
        painter.drawEllipse(point, basePointSize, basePointSize);
    }
    
    // Рисуем линии
    painter.setPen(QPen(QColor(76, 201, 255), baseLineThickness));
    for (const auto &line : m_lines) {
        painter.drawLine(line);
    }
    
    // Рисуем размерные линии
    painter.setPen(QPen(QColor(255, 76, 201), dimLineThickness));
    painter.setBrush(QColor(255, 76, 201));
    QFont font = painter.font();
    font.setPointSizeF(fontSize);
    painter.setFont(font);
    
    for (const auto &dim : m_dimensionLines) {
        QLineF line = dim.line;
        
        // Засечки
        QLineF normal = line.normalVector();
        normal.setLength(tickLength / 2);
        QPointF direction = normal.p2() - normal.p1();
        
        painter.drawLine(line.p1(), line.p1() - direction * serifOffset);
        drawArrow(&painter, line.p1() - direction * sizeLineOffset, line.p2() - direction * sizeLineOffset, arrowSize);
        drawArrow(&painter, line.p2() - direction * sizeLineOffset, line.p1() - direction * sizeLineOffset, arrowSize);
        painter.drawLine(line.p1() - direction * sizeLineOffset, line.p2() - direction * sizeLineOffset);
        painter.drawLine(line.p2() - direction * serifOffset, line.p2());
        
        // Текст
        QPointF midPoint = line.pointAt(0.5) - direction * serifOffset;
        qreal angle = line.angle();
        if (angle > 90 && angle <= 270) {
            angle -= 180;
        }
        
        painter.save();
        painter.translate(midPoint);
        painter.rotate(-angle);
        
        painter.setPen(QColor(255, 76, 201));
        QRectF textRect(-50, -fontSize, 100, fontSize * 2);
        painter.drawText(textRect, Qt::AlignCenter, dim.text);
        painter.restore();
    }
}

bool pointsAlmostEqual(const QPointF &p1, const QPointF &p2, qreal tolerance) {
    return std::abs(p1.x() - p2.x()) <= tolerance && 
           std::abs(p1.y() - p2.y()) <= tolerance;
}

QVector<QVector<QPointF>> SketchWidget::findClosedContours(qreal tolerance) const {
    QVector<QVector<QPointF>> contours;
    if (m_lines.isEmpty()) return contours;

    // Создаем копию линий для работы
    QVector<QLineF> remainingLines = m_lines;
    QVector<bool> used(remainingLines.size(), false);

    // Пытаемся найти контуры для всех непроверенных линий
    for (int startIdx = 0; startIdx < remainingLines.size(); ++startIdx) {
        if (used[startIdx]) continue;

        QVector<QPointF> currentContour;
        QVector<int> usedIndices;
        QLineF currentLine = remainingLines[startIdx];
        QPointF startPoint = currentLine.p1();
        QPointF currentPoint = currentLine.p2();
        
        currentContour.append(startPoint);
        currentContour.append(currentPoint);
        usedIndices.append(startIdx);
        used[startIdx] = true;
        bool foundLoop = false;

        // Продолжаем строить контур пока не замкнем его
        while (true) {
            bool foundNext = false;
            
            for (int i = 0; i < remainingLines.size(); ++i) {
                if (used[i]) continue;
                
                QLineF nextLine = remainingLines[i];
                bool directMatch = pointsAlmostEqual(currentPoint, nextLine.p1(), tolerance);
                bool reverseMatch = pointsAlmostEqual(currentPoint, nextLine.p2(), tolerance);
                
                if (directMatch || reverseMatch) {
                    QPointF nextPoint = directMatch ? nextLine.p2() : nextLine.p1();
                    
                    // Проверяем на замыкание контура
                    if (pointsAlmostEqual(nextPoint, startPoint, tolerance)) {
                        if (currentContour.size() > 2) {
                            contours.append(currentContour);
                            foundLoop = true;
                        }
                        used[i] = true;
                        foundNext = false;
                        break;
                    }
                    
                    // Добавляем точку в контур
                    currentContour.append(nextPoint);
                    currentPoint = nextPoint;
                    used[i] = true;
                    usedIndices.append(i);
                    foundNext = true;
                    break;
                }
            }
            
            if (!foundNext || foundLoop) break;
        }
        
        // Откатываем использование линий, если контур не замкнут
        if (!foundLoop) {
            for (int idx : usedIndices) {
                used[idx] = false;
            }
        }
    }
    
    return contours;
}