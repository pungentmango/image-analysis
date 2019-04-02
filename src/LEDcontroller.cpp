#include "LEDcontroller.h"
#include <iostream>
using namespace std;

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <cstring>
#include <sstream>

#include "hidapi.h"

int res;
hid_device *handle;
#define MAX_STR 255
wchar_t wstr[MAX_STR];

unsigned char LEDcontroller::buf[65] = { 0x01, 0x0a, 0x4d, 0x4f, 0x44, 0x45, 0x20,
                                         0x31, 0x20, /*replace with value*/ 0x00 , 0x0a, 0x0d};

unsigned char LEDcontroller::buf_brightness[65] = { 0x01, 0x0f, 0x43, 0x55, 0x52, 0x52, 0x45,
                                         0x4e, 0x54, 0x20, 0x31, 0x20, /* char0 */ 0x00,
                                        /* char1 */ 0x00, /* char2 */ 0x00, 0x0a, 0x0d};

LEDcontroller::LEDcontroller() {

}

bool LEDcontroller::initialize() {

    bool initialized = false;

    // Enumerate and print the HID devices on the system
    struct hid_device_info *devs, *cur_dev;

    devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs;
    while (cur_dev) {
        printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls",
               cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
        printf("\n");
        printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
        printf("  Product:      %ls\n", cur_dev->product_string);
        printf("\n");

        char* compareChar;
        compareChar = new char[255];
        sprintf(compareChar,"%ls",cur_dev->product_string);

        char* compareChar2 = (char*)"Sirius SLC-MA01-U";
        if (strncmp ( compareChar, compareChar2, 10) == 0) {
            initialized = true;
            break;
        }
        else {
            cur_dev = cur_dev->next;
        }
    }
    hid_free_enumeration(devs);

    if (initialized) {
        // Open the device using the VID, PID,
        // and optionally the Serial number.
        handle = hid_open(cur_dev->vendor_id, cur_dev->product_id, NULL);

        // Read the Manufacturer String
        res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
        printf("Manufacturer String: %ls\n", wstr);

        // Read the Product String
        res = hid_get_product_string(handle, wstr, MAX_STR);
        printf("Product String: %ls\n", wstr);

        // Read the Serial Number String
        res = hid_get_serial_number_string(handle, wstr, MAX_STR);
        printf("Serial Number String: %ls", wstr);
        printf("\n");

        return true;
    }
    else {
        return false;
    }
}

void LEDcontroller::updateIntensity(int intensity) {
    std::ostringstream sin;

    if (intensity < 10) {
        sin << "0";
    }
    if (intensity < 100) {
        sin << "0";
    }

    sin << intensity;
    std::string val = sin.str();

    sin.str(" ");

    buf_brightness[12] = val[0];
    buf_brightness[13] = val[1];
    buf_brightness[14] = val[2];

    res = hid_send_feature_report(handle, buf_brightness, 18);
}

void LEDcontroller::lightOFF() {
    // Turn off the LED light
    buf[9] = 0x30; // sets the buffer to NORMAL
    res = hid_send_feature_report(handle, buf, 18);
}

void LEDcontroller::lightON() {
    // Turn on the LED light
    buf[9] = 0x31; // sets the buffer to NORMAL
    res = hid_send_feature_report(handle, buf, 18);
}
