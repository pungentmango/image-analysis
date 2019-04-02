#ifndef CAMERA_H
#define CAMERA_H

#include <QtGui>
#include <uEye.h>
#include "overlay.h"

class camera
{
public:
    camera(QGraphicsView *cameraView, QSlider *brightnessSlider, QSlider *LEDintensitySlider, QString technician, QString patient, int vidCounter);
    static bool initializeCamera();
    static void exitCamera();
    static void changeMode(bool record);
    static double getFPS();
    static double* getSensorSize();
    static bool removeDirectory(const char *dirname);
    static QVector<QRgb> adjustColorTable();
    static double getContrastRatio();
private:
    Overlay *overlay;
};

#endif // CAMERA_H
