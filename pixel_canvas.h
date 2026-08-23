#pragma once

#include <QWidget>
#include <QColor>
#include <QVector>

class PixelCanvas : public QWidget {
    Q_OBJECT

public:
    explicit PixelCanvas(QWidget *parent = nullptr);

    void setBrushColor(const QColor &color);
    void setEraserMode(bool enabled);
    void clearCanvas();

    const QVector<QVector<QColor>>& getGridData() const { return gridData; }
    void setPixelColor(int r, int c, const QColor &color);
    void refresh();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QColor activeColor;
    bool isEraserActive;
    
    QVector<QVector<QColor>> gridData;
    
    const int PIXEL_SIZE = 14;
    const int GRID_SIZE = 32;

    void drawPixelAtMouse(const QPoint &pos);
};
