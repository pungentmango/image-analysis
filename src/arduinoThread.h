#ifndef ARDUINOTHREAD_H
#define ARDUINOTHREAD_H

#include <QtGui>
#include <stdio.h>
#include "istrabgui.h"

class arduinoThread : public QThread
{
public:
    arduinoThread(QObject *parent);
    void run();
};

#endif // ARDUINOTHREAD_H
