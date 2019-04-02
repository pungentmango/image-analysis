#ifndef ARDUINO_H
#define ARDUINO_H

#include "arduino.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

class arduino
{
public:
    arduino(char* command, int length);
    static bool wakeArduino();
    static void closeArduino();
};

#endif // ARDUINO_H
