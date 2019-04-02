#include "audio.h"

#include <QApplication>
#include <QtGui>
#include <QtCore>

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

#include "istrabgui.h"
#include <iostream>

using namespace std;

GstElement *audioPipeline, *fileSRC;
QString audioFile;

audio::audio(char *target, float volumeLevel)
{
    // GStreamer stuff...
    GError *error = NULL;

    gchar pipeline1_str[256];

    // Initializing GStreamer
    g_print("Initializing GStreamer.\n");
    gst_init(NULL, NULL);

    //configuring pipeline parameters string
    int res = 0;

    QByteArray ba = audioFile.toLatin1();
    char* audioFile_text = ba.data();

    res = sprintf(pipeline1_str, "filesrc name=inputFile ! wavparse ! audioconvert ! audioresample ! volume volume=%f ! alsasink",volumeLevel);
    //creating pipeline1

    audioPipeline = gst_parse_launch(pipeline1_str, &error);
    fileSRC = gst_bin_get_by_name (GST_BIN(audioPipeline), "inputFile" );
    g_assert (fileSRC);

    char *audioLocation = 0;
    audioLocation = (char*)calloc(255,1);
    sprintf(audioLocation, "/home/istrab/Qt Code/iStrabGUI/audio/%s/%s.wav", target, audioFile_text);
    fprintf(stderr,"\nLocation: %s",audioLocation);

    g_object_set (G_OBJECT (fileSRC), "location", audioLocation, NULL);

    //debugging
    g_print("%s\n",pipeline1_str);

    gst_element_set_state (audioPipeline, GST_STATE_PLAYING);
}

void audio::getAudioFile(QString filename) {
    audioFile = filename;
}
