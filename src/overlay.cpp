#include <QPainter>
#include <QPen>
#include "overlay.h"

Overlay::Overlay(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: transparent;");
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void Overlay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::red));
    painter.drawLine(width()/2, 0, width()/2, height());
    painter.drawLine(0, height()/2, width(), height()/2);
}
