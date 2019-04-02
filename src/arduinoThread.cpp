#include "arduinoThread.h"
#include <stdio.h>
#include "istrabgui.h"
#include "camera.h"
#include "audio.h"
#include "arduino.h"

arduinoThread::arduinoThread(QObject *parent) : QThread(parent)
{

}

void arduinoThread::run() {

    // DO SOMETHING
    fprintf(stderr,"Currently in thread... \n");

    // Set Brightness
    char *brightness = 0;
    brightness = (char*)calloc(255,1);
    int brightLevel = 100;
    sprintf(brightness, "%i", brightLevel);

    arduino::arduino((char*)brightness,3);

    time_t t1 = time(NULL);
    time_t t2 = t1;

    while (t2 < t1 + 0.5) {
        // Keep looping until 1 sec has passed
        t2 = time(NULL);
    }

    audio::audio("Center",1);

    while (t2 < t1 + 4.5) {
        // Keep looping until 3 sec has passed
        t2 = time(NULL);
    }

    audio::audio("Left",1);

    while (t2 < t1 + 8.5) {
        // Keep looping until 1 sec has passed
        t2 = time(NULL);
    }

    audio::audio("Right",1);

    while (t2 < t1 + 12.5) {
        // Keep looping until 1 sec has passed
        t2 = time(NULL);
    }

    audio::audio("Center",1);

    while (t2 < t1 + 15) {
        // Keep looping until 2 sec has passed
        t2 = time(NULL);
    }

    camera::changeMode(false);

    while (t2 < t1 + 16) {
        // Keep looping until 2 sec has passed
        t2 = time(NULL);
    }

    emit finished();
    arduino::closeArduino();
}
