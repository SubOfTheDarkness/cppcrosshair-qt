#include "pixel_canvas.h"
#include <QPainter>
#include <QMouseEvent>

PixelCanvas::PixelCanvas(QWidget *parent)
    : QWidget(parent)
    , activeColor(Qt::red)
    , isEraserActive(false)
{
    gridData.resize(GRID_SIZE, QVector<QColor>(GRID_SIZE, QColor()));
    
    int exactSize = GRID_SIZE * PIXEL_SIZE;
    setFixedSize(exactSize, exactSize);
    setCursor(Qt::CrossCursor);
}

void PixelCanvas::setBrushColor(const QColor &color) {
    activeColor = color;
    isEraserActive = false;
}

void PixelCanvas::setEraserMode(bool enabled) {
    isEraserActive = enabled;
}

void PixelCanvas::clearCanvas() {
    for (int r = 0; r < GRID_SIZE; ++r) {
        for (int c = 0; c < GRID_SIZE; ++c) {
            gridData[r][c] = QColor();
        }
    }
    update();
}

void PixelCanvas::setPixelColor(int r, int c, const QColor &color) {
    if (r >= 0 && r < GRID_SIZE && c >= 0 && c < GRID_SIZE) {
        gridData[r][c] = color;
    }
}

void PixelCanvas::refresh() {
    update();
}

void PixelCanvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);

    for (int r = 0; r < GRID_SIZE; ++r) {
        for (int c = 0; c < GRID_SIZE; ++c) {
            int px = c * PIXEL_SIZE;
            int py = r * PIXEL_SIZE;

            if (gridData[r][c].isValid()) {
                painter.fillRect(px, py, PIXEL_SIZE, PIXEL_SIZE, gridData[r][c]);
            } else {
                QColor chessColor = ((r + c) % 2 == 0) ? QColor("#2d3436") : QColor("#353b48");
                painter.fillRect(px, py, PIXEL_SIZE, PIXEL_SIZE, chessColor);
            }

            painter.setPen(QColor(128, 128, 128, 30));
            painter.drawRect(px, py, PIXEL_SIZE, PIXEL_SIZE);
        }
    }

    int centerPos = (GRID_SIZE / 2) * PIXEL_SIZE; 
    int totalLength = GRID_SIZE * PIXEL_SIZE;

    QPen centerPen(QColor(255, 255, 255, 180)); 
    centerPen.setWidth(1);
    painter.setPen(centerPen);

    painter.drawLine(centerPos, 0, centerPos, totalLength);

    painter.drawLine(0, centerPos, totalLength, centerPos);
}

void PixelCanvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        drawPixelAtMouse(event->pos());
    }
}

void PixelCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        drawPixelAtMouse(event->pos());
    }
}

void PixelCanvas::drawPixelAtMouse(const QPoint &pos) {
    int totalSize = GRID_SIZE * PIXEL_SIZE;

    if (pos.x() >= 0 && pos.x() < totalSize && pos.y() >= 0 && pos.y() < totalSize) {
        int c = pos.x() / PIXEL_SIZE;
        int r = pos.y() / PIXEL_SIZE;

        if (isEraserActive) {
            gridData[r][c] = QColor();
        } else {
            gridData[r][c] = activeColor;
        }
        update();
    }
}
