#ifndef AUDIO_H
#define AUDIO_H

#include <QApplication>
#include <QtGui>
#include <QtCore>

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

class audio
{
public:
    audio(char *target, float volumeLevel);
    static void getAudioFile(QString filename);
};

#endif // AUDIO_H
