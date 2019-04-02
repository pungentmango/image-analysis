#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

class LEDcontroller
{
public:
    LEDcontroller();
    static bool initialize();
    static void updateIntensity(int intensity);
    static void lightOFF();
    static void lightON();
private:
    static unsigned char buf[65];
    static unsigned char buf_brightness[65];
};

#endif // LEDCONTROLLER_H
