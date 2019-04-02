#include "istrabgui.h"
#include "ui_istrabgui.h"
#include "camera.h"
#include "arduinoThread.h"
#include "reviewFrames.h"
#include "overlay.h"
#include <sys/stat.h>
#include <stdlib.h>
#include "audio.h"
#include "arduino.h"
#include "ImageAnalysis.h"
#include "LEDcontroller.h"
#include <QDateTime>

char* SERIAL_NUM = "1.0 v051612";

// File declarations
FILE * qt_log;

int vidCounter;
int NUM_ACCEPTED_VIDS = 2;
int* acceptedVids;
QString techID_1, techID_2;
int accepted_vidCounter;
char* audio_filename;
int currentTech;
bool led_initialize = FALSE;
bool cameraON = FALSE;


char* patient1;
char* tech1;

time_t t1, t2, t_begin;

void wait(int delay);

iStrabGui::iStrabGui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::iStrabGui)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->showFullScreen();

    // Load an application style
    QFile styleFile( "/home/istrab/Qt Code/iStrabGUI/stylesheet/qmc2-black-0.10.qss" );
    styleFile.open( QFile::ReadOnly );

    // Apply the loaded stylesheet
    QString style( styleFile.readAll() );
    this->setStyleSheet( style );

    QGraphicsScene* logoScene = new QGraphicsScene();
    QPixmap logoPixmap("/home/istrab/Qt Code/iStrabGUI/iStrabLogo.png");
    logoScene->addPixmap(logoPixmap);
    ui->logoView->setScene(logoScene);

    ui->warningLabel->hide();
    ui->techNumber->hide();

    currentTech = 0;

    connect(ui->multipleTechs,SIGNAL(stateChanged(int)),this,SLOT(toggleMultipleTechs(int)));

    overlay = new Overlay(ui->cameraView);
    overlay->resize(ui->cameraView->size());

    t1 = time(NULL);
    t_begin = time(NULL);
}

iStrabGui::~iStrabGui()
{
    delete ui;
}

void iStrabGui::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void iStrabGui::on_startButton_released()
{
    if (ui->techText->text() != NULL && ui->patientText->text() != NULL) {

        char* parentDir = 0;
        parentDir = new char[255];
        sprintf(parentDir,"/home/istrab/Desktop/iStrabGUI Output");
        mkdir(parentDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        delete[] parentDir;

        char* patientDir = 0;
        patientDir = new char[255];
        sprintf(patientDir,"/home/istrab/Desktop/iStrabGUI Output/%s",(const char*)ui->patientText->text().toLatin1());
        mkdir(patientDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        delete[] patientDir;

        char* techDir = 0;
        techDir = new char[255];
        sprintf(techDir,"/home/istrab/Desktop/iStrabGUI Output/%s/%s", (const char*)ui->patientText->text().toLatin1(), (const char*)ui->techText->text().toLatin1());

        struct stat st;
        if(stat(techDir,&st) == 0) {
            fprintf(stderr,"Operator already exists for current patient, generating new name...\n",(const char*)ui->techText->text().toLatin1());
            int counter = 1;
            char* newTech;
            techID_1 = ui->techText->text();

            while (stat(techDir,&st) == 0) {
                fprintf(stderr,"Operator %s already exists for current patient, generating new tech ID...\n",(const char*)ui->techText->text().toLatin1());

                newTech = new char[255];
                sprintf(newTech,"%s_%i",(const char*)techID_1.toLatin1(),counter);
                ui->techText->setText(newTech);

                techDir = new char[255];
                sprintf(techDir,"/home/istrab/Desktop/iStrabGUI Output/%s/%s", (const char*)ui->patientText->text().toLatin1(), (const char*)ui->techText->text().toLatin1());

                counter++;
            }
            mkdir(techDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            delete[] techDir;

            // Display pop-up window notifying operator new tech ID
            QMessageBox *msgbox = new QMessageBox();

            char* operatorChange;
            operatorChange = new char[255];
            sprintf(operatorChange,"Operator already exists for current patient ID.\nName has been changed: %s",(const char*)ui->techText->text().toLatin1());

            msgbox->setText(operatorChange);
            msgbox->setDefaultButton(QMessageBox::Close);
            msgbox->setGeometry(440,400,400,200);
            msgbox->setStyleSheet("QLabel { font : bold 'Bitstream Charter'; font-size : 14px }");

            int actions = msgbox->exec();

            // stores actions into an integer returned by exec and defines the different cases
            switch(actions)
            {
                case QMessageBox::Close:
                    // Closes the box
                    break;
                default:
                    // should never be reached
                    break;
            }
        }
        else {
            mkdir(techDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            delete[] techDir;
        }


        if (ui->multipleTechs->isChecked() && currentTech == 0) {
            acceptedVids = new int[NUM_ACCEPTED_VIDS*2];
            techID_1 = ui->techText->text();

            char *qt_log_path = 0;
            qt_log_path = new char[255];
            sprintf(qt_log_path,"/home/istrab/Desktop/iStrabGUI Output/%s/Qt Log.txt", (const char*)ui->patientText->text().toLatin1());

            qt_log = fopen (qt_log_path,"w");
            fprintf(qt_log,"Multiple operators selected\n");
            fprintf(qt_log,"Operator 1 of 2: %s\n\n",(const char*)ui->techText->text().toLatin1());
        }
        else if (!ui->multipleTechs->isChecked()) {
            acceptedVids = new int[NUM_ACCEPTED_VIDS];
            techID_1 = ui->techText->text();

            QDate qt_log_date = QDate::currentDate();
            QTime qt_log_time = QTime::currentTime();

            char *qt_log_month = 0;
            qt_log_month = new char[255];
            if (qt_log_date.month() < 10) {
                sprintf(qt_log_month,"0%i",qt_log_date.month());
            }
            else {
                sprintf(qt_log_month,"%i",qt_log_date.month());
            }
            fprintf(stderr,"\n %s",qt_log_month);

            char *qt_log_day = 0;
            qt_log_day = new char[255];
            if (qt_log_date.day() < 10) {
                sprintf(qt_log_day,"0%i",qt_log_date.day());
            }
            else {
                sprintf(qt_log_day,"%i",qt_log_date.day());
            }
            fprintf(stderr,"\n %s",qt_log_day);

            char *qt_log_path = 0;
            qt_log_path = new char[255];
            sprintf(qt_log_path,"/home/istrab/Desktop/iStrabGUI Output/%s/Qt Log %s%s%i %i.txt", (const char*)ui->patientText->text().toLatin1(),(char*)qt_log_month,(char*)qt_log_day,qt_log_date.year(),(60*60*qt_log_time.hour() + 60*qt_log_time.minute() + qt_log_time.second()));
            fprintf(stderr,"\n %s",qt_log_path);

            qt_log = fopen (qt_log_path,"w");
            fprintf(qt_log,"Single Operator Mode: %s\n\n",(const char*)ui->techText->text().toLatin1());
        }
        else if (currentTech == 1) {
            techID_2 = ui->techText->text();
            fprintf(qt_log,"Operator 2 of 2: %s\n\n",(const char*)ui->techText->text().toLatin1());
        }

        t2 = time(NULL);
        fprintf(qt_log,"Page 1 - Data Entry: %f sec\n",(double)(t2-t1));
        t1 = time(NULL);

        currentTech++;

        ui->stackedWidget->setCurrentIndex(1);
        ui->recordButton->setDisabled(true);

        QGraphicsScene* demoScene = new QGraphicsScene();
        QPixmap demoPixmap("/home/istrab/Qt Code/iStrabGUI/iStrabDemo.png");
        demoScene->addPixmap(demoPixmap);
        ui->demoView->setScene(demoScene);

        qApp->processEvents();

        vidCounter = 0;
        accepted_vidCounter = 0;

        setupDevices();
    }
    else {
        ui->warningLabel->show();
        ui->warningLabel->setStyleSheet("QLabel { color : red; }");
    }
}

void iStrabGui::setupDevices() {
    int initializeAttempts = 0;


    if (currentTech == 1 && led_initialize == FALSE) {
        led_initialize = LEDcontroller::initialize();
    }
    else {
        led_initialize = true;
    }

    if (cameraON == FALSE){

        fprintf(stderr,"Entered cameraON Loop............\n");

        cameraON = camera::initializeCamera();

        while(cameraON == false && initializeAttempts < 3) {
            cameraON = camera::initializeCamera();
            initializeAttempts++;
            fprintf(stderr,"Attempt # %i",initializeAttempts);
        }
    }
    else {
        fprintf(stderr,"Did not enter cameraON Loop\n");
    }


    bool arduinoON = arduino::wakeArduino();

    if (cameraON && arduinoON && led_initialize) {
        ui->setupLabel->setText("Please set up the patient for the test");
        ui->setupLabel->setStyleSheet("QLabel { color : lightgrey; }");

        overlay->show();

        ui->recordButton->show();
        ui->recordButton->setEnabled(true);

        ui->reconnectButton->hide();

        camera::camera(ui->cameraView,ui->imageContrastSlider,ui->LEDintensitySlider,ui->techText->text(),ui->patientText->text(),vidCounter);
    }
    else if (cameraON == false && arduinoON == false && led_initialize == false) {
        // Display error message, prevent GUI from advancing
        ui->recordButton->hide();
        ui->setupLabel->setText("Camera, Light Source and Arduino Failed to Initialize");
        ui->setupLabel->setStyleSheet("QLabel { color : red }");

        ui->reconnectButton->show();
    }
    else if (led_initialize == false && arduinoON == false) {
        // Display error message, prevent GUI from advancing
        ui->recordButton->hide();
        ui->setupLabel->setText("Light Source and Arduino Failed to Initialize");
        ui->setupLabel->setStyleSheet("QLabel { color : red }");

        ui->reconnectButton->show();

        // Close out the camera to prevent future errors
       // camera::exitCamera();
    }
    else if (cameraON == false && arduinoON == false) {
        // Display error message, prevent GUI from advancing
        ui->recordButton->hide();
        ui->setupLabel->setText("Camera and Arduino Failed to Initialize");
        ui->setupLabel->setStyleSheet("QLabel { color : red }");

        ui->reconnectButton->show();

        // Turn off the LED light source
        LEDcontroller::lightOFF();
    }
    else if (cameraON == false && led_initialize == false) {
        // Display error message, prevent GUI from advancing
        ui->recordButton->hide();
        ui->setupLabel->setText("Camera and Light Source Failed to Initialize");
        ui->setupLabel->setStyleSheet("QLabel { color : red }");

        ui->reconnectButton->show();
    }
    else if (cameraON == false) {
        // Display error message, prevent GUI from advancing
        ui->recordButton->hide();
        ui->setupLabel->setText("Camera Failed to Initialize");
        ui->setupLabel->setStyleSheet("QLabel { color : red }");

        ui->reconnectButton->show();

        // Turn off the LED light source
        LEDcontroller::lightOFF();
    }
    else if (led_initialize == false) {
        // Display error message, prevent GUI from advancing
        ui->recordButton->hide();
        ui->setupLabel->setText("Light Source Failed to Initialize");
        ui->setupLabel->setStyleSheet("QLabel { color : red }");

        ui->reconnectButton->show();

        // Close out the camera to prevent future errors
       // camera::exitCamera();
    }
    else { // arduinoON == false
        // Display error message, prevent GUI from advancing
        ui->recordButton->hide();
        ui->setupLabel->setText("Arduino Failed to Initialize");
        ui->setupLabel->setStyleSheet("QLabel { color : red }");

        ui->reconnectButton->show();

        // Turn off the LED light source
        LEDcontroller::lightOFF();

        // Close out the camera to prevent future errors
       // camera::exitCamera();
    }
}

void iStrabGui::on_recordButton_released()
{
    // Wake the Arduino from sleep mode
    //arduino::wakeArduino();
    //wait(1); // 1 sec pause to prevent serial communication errors
    overlay->hide();

    ui->setupLabel->setText("Recording...");
    ui->setupLabel->setStyleSheet("QLabel { color : red; }");

    ui->recordButton->setDisabled(true);

    audio::getAudioFile(ui->selectAudio->itemText(ui->selectAudio->currentIndex()));

    camera::changeMode(true);

    QThread *newThread = new arduinoThread(this);
    connect(newThread, SIGNAL(finished()), this, SLOT(finishedRecording()));

    newThread->start();

    fprintf(stderr,"Parent thread \n");
}

void iStrabGui::finishedRecording() {
    ui->setupLabel->setText("Finished Recording!");
    camera::exitCamera();

    cameraON = FALSE;

    // Turn off LED controller
    LEDcontroller::lightOFF();

    t2 = time(NULL);
    fprintf(qt_log,"Page 2 - Setup and Video Recording: %f sec\n",(double)(t2-t1));
    t1 = time(NULL);

    ui->stackedWidget->setCurrentIndex(2);
    ui->rejectionNotes->hide();
    int frameNum = 0;
    reviewFrames::initialFrame(ui->playView,ui->techText->text(),ui->patientText->text(),vidCounter, frameNum);
    connect(ui->rejectionNotes,SIGNAL(selectionChanged()),this,SLOT(clearRejectionNotes()));
    ui->rejectionWarningLabel->hide();

    if (ui->acceptVidButton->isChecked()) {
        ui->acceptVidButton->setAutoExclusive(false);
        ui->acceptVidButton->setChecked(false);
        ui->acceptVidButton->setAutoExclusive(true);
    }
    if (ui->rejectVidButton->isChecked()) {
        ui->rejectVidButton->setAutoExclusive(false);
        ui->rejectVidButton->setChecked(false);
        ui->rejectVidButton->setAutoExclusive(true);
    }
}

void iStrabGui::on_playButton_released()
{
    reviewFrames::getFrameCount(ui->techText->text(),ui->patientText->text(),vidCounter);
    ui->playButton->setDisabled(true);
    reviewFrames::reviewFrames(ui->playView,ui->techText->text(),ui->patientText->text(),vidCounter,ui->playButton);
}

void iStrabGui::on_acceptVidButton_released()
{
    ui->rejectionNotes->hide();
}

void iStrabGui::on_rejectVidButton_released()
{
    ui->rejectionNotes->show();
    ui->rejectionNotes->setText("Include reasons for rejection");
    ui->rejectionNotes->setStyleSheet("QTextEdit { color : grey; }");
    ui->rejectionNotes->blockSignals(false);
}

void iStrabGui::clearRejectionNotes()
{
    ui->rejectionNotes->clear();
    ui->rejectionNotes->setStyleSheet("QTextEdit { color : white; }");
    ui->rejectionNotes->blockSignals(true);
}

void iStrabGui::on_postReviewButton_released()
{
    if(ui->acceptVidButton->isChecked()) {
        if (accepted_vidCounter < NUM_ACCEPTED_VIDS - 1) { // If still less than desired number of successful videos, send back to record more

            int initializeAttempts = 0;
            bool cameraON = camera::initializeCamera();

            while(cameraON == false && initializeAttempts < 3) {
                cameraON = camera::initializeCamera();
                initializeAttempts++;
                fprintf(stderr,"Attempt # %i",initializeAttempts);
            }

            bool arduinoON = arduino::wakeArduino();

            if (cameraON && arduinoON) {
                ui->stackedWidget->setCurrentIndex(1);

                acceptedVids[accepted_vidCounter * currentTech] = vidCounter;
                vidCounter++;
                accepted_vidCounter++;

                ui->setupLabel->setText("Please set up the patient for the test");
                ui->setupLabel->setStyleSheet("QLabel { color : lightgrey; }");

                overlay->show();

                ui->recordButton->setEnabled(true);

                camera::camera(ui->cameraView,ui->imageContrastSlider,ui->LEDintensitySlider,ui->techText->text(),ui->patientText->text(),vidCounter);
            }
            else if (cameraON == false && arduinoON == false) {
                // Display error message, prevent GUI from advancing
                ui->postReviewButton->hide();
                ui->rejectionWarningLabel->show();
                ui->rejectionWarningLabel->setText("Camera and Arduino Failed to Initialize");
                ui->rejectionWarningLabel->setStyleSheet("QLabel { color : red }");
            }
            else if (cameraON == false) {
                // Display error message, prevent GUI from advancing
                ui->postReviewButton->hide();
                ui->rejectionWarningLabel->show();
                ui->rejectionWarningLabel->setText("Camera Failed to Initialize");
                ui->rejectionWarningLabel->setStyleSheet("QLabel { color : red }");
            }
            else { // arduinoON == false
                // Display error message, prevent GUI from advancing
                ui->postReviewButton->hide();
                ui->rejectionWarningLabel->show();
                ui->rejectionWarningLabel->setText("Arduino Failed to Initialize");
                ui->rejectionWarningLabel->setStyleSheet("QLabel { color : red }");

                // Close out the camera to prevent future errors
                camera::exitCamera();
            }
        }
        else if (accepted_vidCounter == NUM_ACCEPTED_VIDS - 1) { // If reached desired number of successful videos, advance to ImageAnalysis part of GUI
            if (ui->multipleTechs->isChecked() && techID_2==NULL) {
                ui->stackedWidget->setCurrentIndex(0);

                //ui->multipleTechs->setChecked(false);
                ui->multipleTechs->hide();
                ui->patientText->setReadOnly(true);
                ui->techText->setText("");

                ui->techNumber->show();
                ui->techNumber->setText("Technician #2");
                ui->warningLabel->hide();
            }
            else {
                acceptedVids[accepted_vidCounter * currentTech] = vidCounter;
                ui->stackedWidget->setCurrentIndex(3);
            }
        }

        t2 = time(NULL);
        fprintf(qt_log,"Page 3 - Reviewing Video: %f sec\n",(double)(t2-t1));
        fprintf(qt_log,"User image contrast setting: %i\n",ui->imageContrastSlider->value());
        fprintf(qt_log,"User LED intensity setting: %i\n",ui->LEDintensitySlider->value());
        t1 = time(NULL);
    }

    else if(ui->rejectVidButton->isChecked() && (ui->rejectionNotes->toPlainText() == NULL ||  ui->rejectionNotes->toPlainText() == "Include reasons for rejection")) {
        ui->rejectionWarningLabel->setText("Please include reasons for rejection");
        ui->rejectionWarningLabel->show();
        ui->rejectionWarningLabel->setStyleSheet("QLabel { color : red; }");
    }

    else if(ui->rejectVidButton->isChecked()) {
        /* save rejection notes */

        char *saveRejectionNotes = 0;
        saveRejectionNotes = (char*)calloc(255,1);
        sprintf(saveRejectionNotes,"/home/istrab/Desktop/iStrabGUI Output/%s/%s/%i/rejectionNotes.txt", (const char*)ui->patientText->text().toLatin1(), (const char*)ui->techText->text().toLatin1(), vidCounter);

        fprintf(stderr,"\n%s",saveRejectionNotes);

        FILE * pFile;
        pFile = fopen (saveRejectionNotes,"w");
        fprintf(pFile, "%s",(const char*)ui->rejectionNotes->toPlainText().toLatin1());
        fclose(pFile);

        free(saveRejectionNotes);

        int initializeAttempts = 0;
        bool cameraON = camera::initializeCamera();

        while(cameraON == false && initializeAttempts < 3) {
            cameraON = camera::initializeCamera();
            initializeAttempts++;
            fprintf(stderr,"Attempt # %i",initializeAttempts);
        }

        bool arduinoON = arduino::wakeArduino();

        if (cameraON && arduinoON) {
            ui->stackedWidget->setCurrentIndex(1);

            vidCounter++;

            ui->setupLabel->setText("Please set up the patient for the test");
            ui->setupLabel->setStyleSheet("QLabel { color : lightgrey; }");

            overlay->show();

            ui->recordButton->setEnabled(true);

            camera::camera(ui->cameraView,ui->imageContrastSlider,ui->LEDintensitySlider,ui->techText->text(),ui->patientText->text(),vidCounter);
        }
        else if (cameraON == false && arduinoON == false) {
            // Display error message, prevent GUI from advancing
            ui->postReviewButton->hide();
            ui->rejectionWarningLabel->show();
            ui->rejectionWarningLabel->setText("Camera and Arduino Failed to Initialize");
            ui->rejectionWarningLabel->setStyleSheet("QLabel { color : red }");
        }
        else if (cameraON == false) {
            // Display error message, prevent GUI from advancing
            ui->postReviewButton->hide();
            ui->rejectionWarningLabel->show();
            ui->rejectionWarningLabel->setText("Camera Failed to Initialize");
            ui->rejectionWarningLabel->setStyleSheet("QLabel { color : red }");
        }
        else { // arduinoON == false
            // Display error message, prevent GUI from advancing
            ui->postReviewButton->hide();
            ui->rejectionWarningLabel->show();
            ui->rejectionWarningLabel->setText("Arduino Failed to Initialize");
            ui->rejectionWarningLabel->setStyleSheet("QLabel { color : red }");

            // Close out the camera to prevent future errors
            camera::exitCamera();
        }
    }

    else {
        // Display warning that no selection was given
        ui->rejectionWarningLabel->setText("Please accept or reject video");
        ui->rejectionWarningLabel->show();
        ui->rejectionWarningLabel->setStyleSheet("QLabel { color : red; }");
    }


}

void wait(int delay)
{
    time_t now=time(NULL);
    time_t later=now+delay;
    while(now<=later)now=time(NULL);
}

void iStrabGui::on_analyzeButton_released()
{
    ui->analyzeButton->hide();
    this->setEnabled(false);

    if (techID_2 != NULL) {
        ui->imageAnalysisLabel->setText("Operator #1 of 2");
        ui->imageAnalysisLabel->setStyleSheet("QLabel { color : red; }");
    }

    double* aod_results_1 = NULL;
    double* aod_results_2 = NULL;
    double* aod_results_3 = NULL;
    double* aod_results_4 = NULL;

    ui->beginAnalysisLabel->setText("Currently Analyzing Video 1 of 2:");
    //aod_results_1 = ImageAnalysis("0005 050612_1","rvp",0,ui->analysisProgressBar,ui->analysisProgressLabel);
    aod_results_1 = ImageAnalysis(ui->patientText->text(),techID_1,acceptedVids[0],ui->analysisProgressBar,ui->analysisProgressLabel);

    char* output;
    if (aod_results_1 == NULL) {
        ui->eval_test1_left_text->setText("ERROR");
        ui->eval_test1_center_text->setText("ERROR");
        ui->eval_test1_right_text->setText("ERROR");
    }
    else {
        output = new char[255];
        sprintf(output,"%4.3f",aod_results_1[0]);
        ui->eval_test1_left_text->setText(output);
        delete[] output;

        output = new char[255];
        sprintf(output,"%4.3f",aod_results_1[1]);
        ui->eval_test1_center_text->setText(output);
        delete[] output;

        output = new char[255];
        sprintf(output,"%4.3f",aod_results_1[2]);
        ui->eval_test1_right_text->setText(output);
        delete[] output;
    }

    ui->beginAnalysisLabel->setText("Currently Analyzing Video 2 of 2:");
    //aod_results_2 = ImageAnalysis("0005 050612_1","rvp",1,ui->analysisProgressBar,ui->analysisProgressLabel);
    aod_results_2 = ImageAnalysis(ui->patientText->text(),techID_1,acceptedVids[1],ui->analysisProgressBar,ui->analysisProgressLabel);

    if (aod_results_2 == NULL) {
        ui->eval_test2_left_text->setText("ERROR");
        ui->eval_test2_center_text->setText("ERROR");
        ui->eval_test2_right_text->setText("ERROR");
    }
    else {
        output = new char[255];
        sprintf(output,"%4.3f",aod_results_2[0]);
        ui->eval_test2_left_text->setText(output);
        delete[] output;

        output = new char[255];
        sprintf(output,"%4.3f",aod_results_2[1]);
        ui->eval_test2_center_text->setText(output);
        delete[] output;

        output = new char[255];
        sprintf(output,"%4.3f",aod_results_2[2]);
        ui->eval_test2_right_text->setText(output);
        delete[] output;
    }

    ui->eval_operator1_text->setText(techID_1);

    output = new char[255];
    sprintf(output,"%i",acceptedVids[1]+1);
    ui->eval_numTests1_text->setText(output);
    delete[] output;

    // Analysis and output for multiple operators
    if (techID_2 != NULL) {
        ui->imageAnalysisLabel->setText("Operator #2 of 2");
        ui->imageAnalysisLabel->setStyleSheet("QLabel { color : red; }");

        ui->beginAnalysisLabel->setText("Currently Analyzing Video 1 of 2:");
        aod_results_3 = ImageAnalysis(ui->patientText->text(),techID_2,acceptedVids[2],ui->analysisProgressBar,ui->analysisProgressLabel);

        if (aod_results_3 == NULL) {
            ui->eval_test1_left_text_2->setText("ERROR");
            ui->eval_test1_center_text_2->setText("ERROR");
            ui->eval_test1_right_text_2->setText("ERROR");
        }
        else {
            output = new char[255];
            sprintf(output,"%4.3f",aod_results_3[0]);
            ui->eval_test1_left_text_2->setText(output);
            delete[] output;

            output = new char[255];
            sprintf(output,"%4.3f",aod_results_3[1]);
            ui->eval_test1_center_text_2->setText(output);
            delete[] output;

            output = new char[255];
            sprintf(output,"%4.3f",aod_results_3[2]);
            ui->eval_test1_right_text_2->setText(output);
            delete[] output;
        }

        ui->beginAnalysisLabel->setText("Currently Analyzing Video 2 of 2:");
        aod_results_4 = ImageAnalysis(ui->patientText->text(),techID_2,acceptedVids[3],ui->analysisProgressBar,ui->analysisProgressLabel);

        if (aod_results_4 == NULL) {
            ui->eval_test2_left_text_2->setText("ERROR");
            ui->eval_test2_center_text_2->setText("ERROR");
            ui->eval_test2_right_text_2->setText("ERROR");
        }
        else {
            output = new char[255];
            sprintf(output,"%4.3f",aod_results_4[0]);
            ui->eval_test2_left_text_2->setText(output);
            delete[] output;

            output = new char[255];
            sprintf(output,"%4.3f",aod_results_4[1]);
            ui->eval_test2_center_text_2->setText(output);
            delete[] output;

            output = new char[255];
            sprintf(output,"%4.3f",aod_results_4[2]);
            ui->eval_test2_right_text_2->setText(output);
            delete[] output;
        }

        ui->eval_operator2_text->setText(techID_2);

        output = new char[255];
        sprintf(output,"%i",acceptedVids[3]+1);
        ui->eval_numTests2_text->setText(output);
        delete[] output;
    }
    else {
        ui->eval_operator2->hide();
        ui->eval_operator2_text->hide();
        ui->eval_numTests2->hide();
        ui->eval_numTests2_text->hide();

        ui->eval_test1_2->hide();
        ui->eval_test1_left_2->hide();
        ui->eval_test1_left_text_2->hide();
        ui->eval_test1_center_2->hide();
        ui->eval_test1_center_text_2->hide();
        ui->eval_test1_right_2->hide();
        ui->eval_test1_right_text_2->hide();

        ui->eval_test2_2->hide();
        ui->eval_test2_left_2->hide();
        ui->eval_test2_left_text_2->hide();
        ui->eval_test2_center_2->hide();
        ui->eval_test2_center_text_2->hide();
        ui->eval_test2_right_2->hide();
        ui->eval_test2_right_text_2->hide();
    }

    ui->eval_serialNum_text->setText(SERIAL_NUM);
    ui->eval_patientID_text->setText(ui->patientText->text());

    t2 = time(NULL);
    fprintf(qt_log,"Page 4 - Video Analysis: %f sec\n",(double)(t2-t1));

    time_t t_end = time(NULL);
    fprintf(qt_log,"\nTotal program time: %f sec\n", (double)(t_end-t_begin));

    QDate date = QDate::currentDate();
    QString dateString = date.toString();
    ui->eval_testDate_text->setText(dateString);

    QTime time = QTime::currentTime();
    QString timeString = time.toString();
    ui->eval_testTime_text->setText(timeString);

    this->setEnabled(true);
    ui->stackedWidget->setCurrentIndex(4);

    fclose(qt_log);
}

void iStrabGui::on_pushButton_released()
{
    exit(true);
}

void iStrabGui::on_reconnectButton_released()
{
    setupDevices();
}

void iStrabGui::toggleMultipleTechs(int state)
{
    if(state) {
        ui->techNumber->show();
    }
    else {
        ui->techNumber->hide();
    }
}

void iStrabGui::on_actionExit_triggered()
{
    LEDcontroller::lightOFF();
    exit(true);
}
