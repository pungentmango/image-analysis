#ifndef REVIEWFRAMES_H
#define REVIEWFRAMES_H

#include <QtGui>

class reviewFrames
{
public:
    reviewFrames(QGraphicsView *playView, QString technician, QString patient, int vidCounter, QPushButton *playButton);
    static void initialFrame(QGraphicsView *playView, QString technician, QString patient, int vidCounter, int frameNum);
    static void getFrameCount(QString technician, QString patient, int vidCounter);
    static QVector<QRgb> getColorTable();
};

#endif // REVIEWFRAMES_H
