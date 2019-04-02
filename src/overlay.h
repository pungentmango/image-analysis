#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>

class Overlay : public QWidget
{
public:
    Overlay(QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // OVERLAY_H
