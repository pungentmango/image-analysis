#include "arduino.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

int fd1;

arduino::arduino(char* command, int length)
{
    write(fd1,command,length);
    printf("Written to Arduino");
}

bool arduino::wakeArduino() {
    fd1=open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd1 == -1 ) {
        perror("open_port: Unable to open /dev/ttyACM0 – , trying another port...");
        fd1=open("/dev/ttyACM1", O_RDWR | O_NOCTTY | O_NDELAY);

        if (fd1 == -1) {
            perror("open_port: Unable to open /dev/ttyACM1 – , trying another port...");
            fd1=open("/dev/ttyACM2",O_RDWR | O_NOCTTY | O_NDELAY);

            if (fd1 == -1) {
                perror("open_port: Unable to open /dev/ttyACM2 – :: Contact iStrab HQ at 706-iStrab8");
                return false;
            }
            else {
                fcntl(fd1, F_SETFL,0);
                printf("\nThe port has been successfully opened");
            }

        }
        else {
            fcntl(fd1, F_SETFL,0);
            printf("\nThe port has been successfully opened");
        }

    }
    else {
        fcntl(fd1, F_SETFL,0);
        printf("\nThe port has been successfully opened");
    }
    return true;

}

void arduino::closeArduino() {
    close(fd1);
}

