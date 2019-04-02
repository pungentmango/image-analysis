#include "reviewFrames.h"

#include <QtGui>
#include "istrabgui.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include "camera.h"
using namespace std;

int frameCount;
double contrast_ratio;
QVector<QRgb> color_table;

reviewFrames::reviewFrames(QGraphicsView *playView, QString technician, QString patient, int vidCounter, QPushButton *playButton) {

    char* displayFrame;
    QGraphicsScene* scene = new QGraphicsScene();

    for (int i=0; i<frameCount; i++) {
        displayFrame = new char[255];
        sprintf(displayFrame,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Frames/%i.bmp", (const char*)patient.toLatin1(), (const char*)technician.toLatin1(), vidCounter, i);

        QImage image;
        image.load(displayFrame,0);
        image.setColorTable(color_table);
        QPixmap pixmap = QPixmap::fromImage(image);

        pixmap = pixmap.scaledToWidth(playView->width(),Qt::SmoothTransformation);

        scene->addPixmap(pixmap);
        playView->setScene(scene);
        qApp->processEvents();

        double FRAMES_PER_SEC = camera::getFPS();
        usleep(pow(10,6) * 1 / FRAMES_PER_SEC);

        if (i != frameCount-1) {
            delete scene;
            scene = new QGraphicsScene();
        }
        delete[] displayFrame;
    }
    playButton->setEnabled(true);
}

void reviewFrames::initialFrame(QGraphicsView *playView, QString technician, QString patient, int vidCounter, int frameNum) {
    double contrastVal = camera::getContrastRatio();
    contrast_ratio = contrastVal;

    QVector<QRgb> colorTable = reviewFrames::getColorTable();
    color_table = colorTable;

    char* initialFrame;
    initialFrame = new char[255];
    sprintf(initialFrame,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Frames/%i.bmp", (const char*)patient.toLatin1(), (const char*)technician.toLatin1(), vidCounter, frameNum);

    QGraphicsScene* scene = new QGraphicsScene();

    QImage image;
    image.load(initialFrame,0);
    image.setColorTable(color_table);
    QPixmap pixmap = QPixmap::fromImage(image);

    pixmap = pixmap.scaledToWidth(playView->width(),Qt::SmoothTransformation);

    scene->addPixmap(pixmap);
    playView->setScene(scene);
    qApp->processEvents();

    delete[] initialFrame;
}

void reviewFrames::getFrameCount(QString technician, QString patient, int vidCounter) {
    char* getFrameCount;
    getFrameCount = new char[255];
    sprintf(getFrameCount,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Frames/frameCount.txt", (const char*)patient.toLatin1(), (const char*)technician.toLatin1(), vidCounter);

    frameCount = 0;
    int x;

    ifstream inFile;
    inFile.open(getFrameCount);
    if (!inFile) {
        cout << "Unable to open file";
        exit(1); // terminate with error
    }

    while (inFile >> x) {
        frameCount = frameCount + x;
    }

    inFile.close();

    delete[] getFrameCount;
}

QVector<QRgb> reviewFrames::getColorTable() {
    double ratio = -((contrast_ratio / 10) / 100) * 255;

    QVector<QRgb> colorTable(256);
    double contrastFactor = (259*(ratio + 255)) / (255*(259 - ratio));

    for(int i=0; i<256; i++) {
        double newVal = contrastFactor*(i-128) + 128;
        if (newVal < 0) {
            newVal = 0;
        }
        else if (newVal > 255) {
            newVal = 255;
        }
        colorTable[i] = qRgb(newVal, newVal, newVal);
    }
    return colorTable;
}
