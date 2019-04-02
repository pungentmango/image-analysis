#include "camera.h"

#include <QtGui>
#include <QImage>
#include <uEye.h>
#include <sys/stat.h>
#include "overlay.h"
#include <dirent.h>
#include "istrabgui.h"
#include "LEDcontroller.h"

HIDS hCam;
int* pid;
int* m_nSeqNumId;
char** ppcImgMem;
SENSORINFO* pInfo;

INT nRet;

int MAX_BUFFER = 20;
double SENSOR_SIZE[2];

double SENSOR_WIDTH;
double SENSOR_HEIGHT;

double FRAMES_PER_SEC;

bool recordMode;
bool cameraActive;
bool LED_ON;

double contrastRatio;

int LED_intensity;

int frame;

QSlider *contrastSlider;
QSlider *intensitySlider;

camera::camera(QGraphicsView *cameraView, QSlider *imageContrastSlider, QSlider *LEDintensitySlider, QString technician, QString patient, int vidCounter) {

    contrastSlider = imageContrastSlider;
    intensitySlider = LEDintensitySlider;

    LEDcontroller::lightON();

    char* counterDir;
    counterDir = new char[255];
    sprintf(counterDir,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i", (const char*)patient.toLatin1(), (const char*)technician.toLatin1(), vidCounter);
    mkdir(counterDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    delete[] counterDir;

    char* frameDir;
    frameDir = new char[255];
    sprintf(frameDir,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Frames", (const char*)patient.toLatin1(), (const char*)technician.toLatin1(), vidCounter);
    mkdir(frameDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    delete[] frameDir;

    char* bufferDir;
    bufferDir = new char[255];
    sprintf(bufferDir,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Frames/Buffer", (const char*)patient.toLatin1(), (const char*)technician.toLatin1(), vidCounter);
    mkdir(bufferDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    QGraphicsScene* scene = new QGraphicsScene();

    if(cameraActive) {
        is_EnableEvent (hCam, IS_SET_EVENT_FRAME);

        if(is_CaptureVideo(hCam,IS_DONT_WAIT)==IS_SUCCESS) {
            fprintf(stderr,"Video streaming begun. \n");
        }

        // Set Pixel Clock (determined with ueyedemo)
        if(is_SetPixelClock (hCam, 40)==IS_NO_SUCCESS){
            fprintf(stderr,"Pixel Clock Error\n");
        }
        else {
        //Set and report frame rate
       double FPS = 15; //Desired frame rate
       double *newFPS; //Actual frame rate achieved


       if(is_SetFrameRate (hCam, FPS,newFPS)==IS_SUCCESS){
           fprintf(stderr,"Frame Rate Set: %f \n", FPS);

         if (newFPS == NULL){
             fprintf(stderr,"newFPS is NULL\n");
             FRAMES_PER_SEC = 15;
             fprintf(stderr,"Frame Rate is default");
         }
             else{
                  FRAMES_PER_SEC = *newFPS;
                 fprintf(stderr,"Frame Rate Achieved: %f\n",*newFPS);

         }
         }
       else{

           fprintf(stderr,"Error Setting Frame Rate!\n");
       }



//       if(is_Exposure (hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, 0, 8)==IS_SUCCESS){
//           fprintf(stderr,"Exposure Set to Max\n");
//       }
   }
        frame=0;
        int frameCount = 0;

        while (cameraActive) {


            int camHand= is_WaitEvent(hCam,IS_SET_EVENT_FRAME,1000);

            if( camHand == IS_TIMED_OUT){
                fprintf(stderr,"Signal Timed Out. \n");
            }
            else if (camHand == IS_SUCCESS){

                // find the latest image buffer
                INT nNum;
                int j = 0;
                char *pcMem, *pcMemLast;
                is_GetActSeqBuf(hCam, &nNum, &pcMem, &pcMemLast);

                for( j=0 ; j<MAX_BUFFER ; j++)
                {
                    if( pcMemLast == ppcImgMem[j] ) {
                        break;
                    }
                }

                // lock buffer for processing
                is_LockSeqBuf( hCam,m_nSeqNumId[j], ppcImgMem[j] );

                QImage image ((uchar*)pcMemLast,
                                      SENSOR_WIDTH,
                                      SENSOR_HEIGHT,
                                      8,
                                      NULL,
                                      0,
                                      QImage::IgnoreEndian);

                char* saveFrame;
                saveFrame = new char[255];

                if (recordMode) {
                    sprintf(saveFrame,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Frames/%i.bmp", (const char*)patient.toLatin1(), (const char*)technician.toLatin1(), vidCounter, frame);
                    frame++;
                    frameCount = frame;
                }
                else {
                    sprintf(saveFrame,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Frames/Buffer/%i.bmp", (const char*)patient.toLatin1(), (const char*)technician.toLatin1(), vidCounter, frame);
                    frame++;
                    if (frame>10) {
                        frame=0;
                    }
                }

                if(is_SaveImageMem(hCam,saveFrame,ppcImgMem[j],pid[j])==IS_SUCCESS){
                    // fprintf(stderr,"Image Successfully Saved. \n");
                }

                // TODO: Increase displayed frame rate by grabbing frame from buffer instead
                delete scene;
                scene = new QGraphicsScene();

                QVector<QRgb> colorTable = camera::adjustColorTable();

                image.setColorTable(colorTable);
                QPixmap pixmap = QPixmap::fromImage(image);
                pixmap = pixmap.scaledToWidth(cameraView->width(),Qt::SmoothTransformation);

                scene->addPixmap(pixmap);
                cameraView->setScene(scene);

                qApp->processEvents();
                delete[] saveFrame;

                // unlock buffer
                is_UnlockSeqBuf( hCam, pid[j], ppcImgMem[j] );
            }
        }

        /* save frame count */

        char *saveFrameCount;
        saveFrameCount = new char[255];
        sprintf(saveFrameCount,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/Frames/frameCount.txt", (const char*)patient.toLatin1(), (const char*)technician.toLatin1(), vidCounter);

        FILE * pFile;
        pFile = fopen (saveFrameCount,"w");
        fprintf(pFile, "%i",frameCount);
        fclose(pFile);

        delete[] saveFrameCount;

        // Delete image buffer
        if(removeDirectory(bufferDir)) {
            fprintf(stderr,"Deleted buffer directory: %s",bufferDir);
        }
        delete[] bufferDir;
    }
    else {
        fprintf(stderr,"Not properly initialized... \n");
    }
}

bool camera::initializeCamera() {
    recordMode = false;
    cameraActive = false;

    // Initialize Camera
    hCam = 0;
    nRet = is_InitCamera (&hCam, NULL);
    if (nRet == IS_SUCCESS){
        fprintf(stderr,"Camera Succesfully Initialized. \n");

        nRet = is_SetColorMode (hCam, IS_CM_MONO8 );
        if (nRet == IS_SUCCESS) {
            fprintf(stderr,"Color Mode Set. \n");
        }

        if (is_GetSensorInfo (hCam, pInfo) == IS_SUCCESS) {
            fprintf(stderr,"Found camera sensor info \n");
            SENSOR_WIDTH = (double)pInfo->nMaxWidth;
            SENSOR_HEIGHT = (double)pInfo->nMaxHeight;
        }
        else {
            fprintf(stderr,"Error obtaining camera sensor info, setting default size \n");
            SENSOR_WIDTH = 1280;
            SENSOR_HEIGHT = 1024;
        }

        //Prepare to Initalize Buffer and memory

        ppcImgMem = new char*[MAX_BUFFER];
        pid = new int[MAX_BUFFER];
        m_nSeqNumId = new int[MAX_BUFFER+1];   // store sequence buffer number Id

        if (nRet == IS_SUCCESS)
        {
            // Allocate Memory

            for (int i=0; i<MAX_BUFFER; i++) {
                if(is_AllocImageMem(hCam,SENSOR_WIDTH,SENSOR_HEIGHT,8,&ppcImgMem[i],&pid[i])==IS_SUCCESS)
                {
                    fprintf(stderr,"Memory Successfully Allocated. \n");
                }
                if(is_AddToSequence(hCam,ppcImgMem[i],pid[i])==IS_SUCCESS) {
                    fprintf(stderr,"Successfully added to sequence. \n");
                }
                m_nSeqNumId[i] = i+1;   // store sequence buffer number Id
            }
            cameraActive = true;
            return true;
        }
    }
    return false;
}

void camera::exitCamera() {
    if(is_StopLiveVideo(hCam,IS_WAIT)==IS_SUCCESS) {
        fprintf(stderr,"Live video stopped. \n");
    }

    if(is_ClearSequence(hCam)==IS_SUCCESS) {
        fprintf(stderr,"Sequence cleared. \n");
    }

    // Deallocate Memory
    for (int i=1; i<MAX_BUFFER-1; i++) {
        if(is_FreeImageMem (hCam,ppcImgMem[i],pid[i])==IS_SUCCESS)
        {
            fprintf(stderr,"Memory Successfully Deallocated. \n");
        }
    }

    if(is_ExitCamera(hCam)==IS_SUCCESS) {
        fprintf(stderr,"Camera closed. \n");
    }

    cameraActive = false;
    hCam = 0;


}

void camera::changeMode(bool record) {
    frame = 0;
    if(record) {
        recordMode = true;
        fprintf(stderr,"MODE SWITCHED = RECORD \n");
    }
    else {
        recordMode = false;
        fprintf(stderr,"MODE SWITCHED = STREAM \n");
    }
}

bool camera::removeDirectory(const char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    char path[PATH_MAX];

    if (path == NULL) {
        fprintf(stderr, "Out of memory error\n");
        return false;
    }
    dir = opendir(dirname);
    if (dir == NULL) {
        perror("Error opendir()");
        return false;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            snprintf(path, (size_t) PATH_MAX, "%s/%s", dirname, entry->d_name);
            if (entry->d_type == DT_DIR) {
                removeDirectory(path);
            }
            // Delete file
            remove(path);
        }
    }
    closedir(dir);

    // Remove directory
    rmdir(dirname);

    return true;
}

double camera::getFPS() {
    return FRAMES_PER_SEC;
}

double* camera::getSensorSize() {
    if (SENSOR_WIDTH == NULL || SENSOR_HEIGHT == NULL) {
        SENSOR_WIDTH = 1280;
        SENSOR_HEIGHT = 1024;
    }
    SENSOR_SIZE[0] = SENSOR_WIDTH;
    SENSOR_SIZE[1] = SENSOR_HEIGHT;
    return SENSOR_SIZE;
}

QVector<QRgb> camera::adjustColorTable() {
    contrastRatio = contrastSlider->value();
    double ratio = -((contrastRatio / 10) / 100) * 255;


    int LED_intensity_temp = intensitySlider->value();
    if (LED_intensity_temp != LED_intensity) {
        LED_intensity = LED_intensity_temp;
        LEDcontroller::updateIntensity(LED_intensity);
    }

    QVector<QRgb> colorTable(256);
    double contrastFactor = (259*(ratio + 255)) / (255*(259 - ratio));

    for(int i=0; i<256; i++) {
        //colorTable[i] = qRgb(i + brightnessRatio*(255-i), i + brightnessRatio*(255-i), i + brightnessRatio*(255-i));
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

double camera::getContrastRatio() {
    return contrastRatio;
}
