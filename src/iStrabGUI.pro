# -------------------------------------------------
# Project created by QtCreator 2012-03-10T14:50:39
# -------------------------------------------------
TARGET = iStrabGUI
TEMPLATE = app
INCLUDEPATH += "/usr/local/include/opencv"
INCLUDEPATH += "/usr/include/gstreamer-0.10/"
INCLUDEPATH += "/usr/include/glib-2.0/"
INCLUDEPATH += "/usr/lib/glib-2.0/include/"
INCLUDEPATH += "/usr/include/libxml2/
INCLUDEPATH += /usr/include/libusb-1.0
LIBS += -lueye_api \
    -lgstapp-0.10 \
    -lgstinterfaces-0.10 \
    -lcv \
    -lhighgui \
    -lsqlite3 \
    -lusb-1.0
LIBS += -L"/usr/include/gstreamer-0.10/"
SOURCES += main.cpp \
    istrabgui.cpp \
    camera.cpp \
    arduinoThread.cpp \
    reviewFrames.cpp \
    overlay.cpp \
    audio.cpp \
    arduino.cpp \
    ImageAnalysis.cpp \
    hid-libusb.c \
    LEDcontroller.cpp
QT += core \
    gui \
    opengl \
    qt3support
HEADERS += istrabgui.h \
    uEye.h \
    camera.h \
    arduinoThread.h \
    reviewFrames.h \
    overlay.h \
    audio.h \
    arduino.h \
    ImageAnalysis.h \
    hidapi.h \
    LEDcontroller.h
FORMS += istrabgui.ui
OTHER_FILES += 
