/********************************************************************************
** Form generated from reading UI file 'istrabgui.ui'
**
** Created: Thu May 17 08:28:08 2012
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ISTRABGUI_H
#define UI_ISTRABGUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGraphicsView>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QStackedWidget>
#include <QtGui/QStatusBar>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_iStrabGui
{
public:
    QAction *actionExit;
    QWidget *centralWidget;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QGraphicsView *logoView;
    QPushButton *startButton;
    QLabel *techLabel;
    QLabel *patientLabel;
    QLineEdit *techText;
    QLineEdit *patientText;
    QLabel *warningLabel;
    QCheckBox *multipleTechs;
    QLabel *techNumber;
    QWidget *page_2;
    QGraphicsView *cameraView;
    QGraphicsView *demoView;
    QLabel *setupLabel;
    QLabel *demoLabel;
    QLabel *feedLabel;
    QPushButton *recordButton;
    QLabel *audioSelectLabel;
    QComboBox *selectAudio;
    QPushButton *reconnectButton;
    QSlider *imageContrastSlider;
    QSlider *LEDintensitySlider;
    QLabel *imageContrastLabel;
    QLabel *LEDintensityLabel;
    QWidget *page_3;
    QGraphicsView *playView;
    QPushButton *playButton;
    QLabel *reviewLabel;
    QGroupBox *vidQualityBox;
    QRadioButton *acceptVidButton;
    QRadioButton *rejectVidButton;
    QTextEdit *rejectionNotes;
    QPushButton *postReviewButton;
    QLabel *rejectionWarningLabel;
    QWidget *page_4;
    QLabel *imageAnalysisLabel;
    QPushButton *analyzeButton;
    QProgressBar *analysisProgressBar;
    QLabel *analysisProgressLabel;
    QLabel *beginAnalysisLabel;
    QWidget *page_5;
    QLabel *completedLabel;
    QPushButton *pushButton;
    QLabel *eval_patientID;
    QLabel *eval_patientID_text;
    QLabel *eval_serialNum;
    QLabel *eval_serialNum_text;
    QLabel *eval_testDate_text;
    QLabel *eval_testDate;
    QLabel *eval_testTime;
    QLabel *eval_testTime_text;
    QLabel *eval_operator1_text;
    QLabel *eval_operator1;
    QLabel *eval_numTests1_text;
    QLabel *eval_numTests1;
    QLabel *eval_test1;
    QLabel *eval_test1_right;
    QLabel *eval_test1_center;
    QLabel *eval_test1_left;
    QLabel *eval_test1_right_text;
    QLabel *eval_test1_center_text;
    QLabel *eval_test1_left_text;
    QLabel *eval_test2_center;
    QLabel *eval_test2;
    QLabel *eval_test2_center_text;
    QLabel *eval_test2_right;
    QLabel *eval_test2_right_text;
    QLabel *eval_test2_left;
    QLabel *eval_test2_left_text;
    QLabel *eval_numTests2;
    QLabel *eval_test1_right_2;
    QLabel *eval_test1_right_text_2;
    QLabel *eval_operator2;
    QLabel *eval_test1_center_text_2;
    QLabel *eval_test2_center_2;
    QLabel *eval_test2_left_2;
    QLabel *eval_test1_left_text_2;
    QLabel *eval_test1_center_2;
    QLabel *eval_test2_left_text_2;
    QLabel *eval_test2_right_2;
    QLabel *eval_test1_2;
    QLabel *eval_test2_center_text_2;
    QLabel *eval_test2_right_text_2;
    QLabel *eval_operator2_text;
    QLabel *eval_test2_2;
    QLabel *eval_numTests2_text;
    QLabel *eval_test1_left_2;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *iStrabGui)
    {
        if (iStrabGui->objectName().isEmpty())
            iStrabGui->setObjectName(QString::fromUtf8("iStrabGui"));
        iStrabGui->resize(1280, 1024);
        actionExit = new QAction(iStrabGui);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        centralWidget = new QWidget(iStrabGui);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        stackedWidget = new QStackedWidget(centralWidget);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        stackedWidget->setGeometry(QRect(0, 0, 1280, 981));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        logoView = new QGraphicsView(page);
        logoView->setObjectName(QString::fromUtf8("logoView"));
        logoView->setGeometry(QRect(390, 70, 500, 330));
        logoView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        logoView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        startButton = new QPushButton(page);
        startButton->setObjectName(QString::fromUtf8("startButton"));
        startButton->setGeometry(QRect(540, 820, 200, 54));
        QFont font;
        font.setFamily(QString::fromUtf8("Bitstream Charter"));
        font.setPointSize(20);
        startButton->setFont(font);
        techLabel = new QLabel(page);
        techLabel->setObjectName(QString::fromUtf8("techLabel"));
        techLabel->setGeometry(QRect(445, 543, 300, 40));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Bitstream Charter"));
        font1.setPointSize(24);
        font1.setBold(true);
        font1.setWeight(75);
        techLabel->setFont(font1);
        patientLabel = new QLabel(page);
        patientLabel->setObjectName(QString::fromUtf8("patientLabel"));
        patientLabel->setGeometry(QRect(445, 460, 160, 40));
        patientLabel->setFont(font1);
        techText = new QLineEdit(page);
        techText->setObjectName(QString::fromUtf8("techText"));
        techText->setGeometry(QRect(600, 537, 230, 52));
        techText->setFont(font);
        patientText = new QLineEdit(page);
        patientText->setObjectName(QString::fromUtf8("patientText"));
        patientText->setGeometry(QRect(610, 450, 220, 52));
        patientText->setFont(font);
        warningLabel = new QLabel(page);
        warningLabel->setObjectName(QString::fromUtf8("warningLabel"));
        warningLabel->setGeometry(QRect(380, 750, 520, 42));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Bitstream Charter"));
        font2.setPointSize(24);
        font2.setBold(true);
        font2.setItalic(true);
        font2.setWeight(75);
        warningLabel->setFont(font2);
        warningLabel->setAlignment(Qt::AlignCenter);
        multipleTechs = new QCheckBox(page);
        multipleTechs->setObjectName(QString::fromUtf8("multipleTechs"));
        multipleTechs->setGeometry(QRect(470, 650, 342, 44));
        QFont font3;
        font3.setFamily(QString::fromUtf8("Bitstream Charter"));
        font3.setPointSize(24);
        font3.setBold(false);
        font3.setWeight(50);
        multipleTechs->setFont(font3);
        multipleTechs->setIconSize(QSize(56, 56));
        multipleTechs->setTristate(false);
        techNumber = new QLabel(page);
        techNumber->setObjectName(QString::fromUtf8("techNumber"));
        techNumber->setGeometry(QRect(440, 14, 400, 40));
        QFont font4;
        font4.setFamily(QString::fromUtf8("Bitstream Charter"));
        font4.setPointSize(36);
        font4.setBold(false);
        font4.setItalic(false);
        font4.setWeight(50);
        techNumber->setFont(font4);
        techNumber->setAlignment(Qt::AlignCenter);
        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        cameraView = new QGraphicsView(page_2);
        cameraView->setObjectName(QString::fromUtf8("cameraView"));
        cameraView->setGeometry(QRect(30, 130, 1000, 800));
        cameraView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        cameraView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        demoView = new QGraphicsView(page_2);
        demoView->setObjectName(QString::fromUtf8("demoView"));
        demoView->setGeometry(QRect(1060, 130, 200, 320));
        demoView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        demoView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setupLabel = new QLabel(page_2);
        setupLabel->setObjectName(QString::fromUtf8("setupLabel"));
        setupLabel->setGeometry(QRect(40, 15, 1200, 60));
        QFont font5;
        font5.setFamily(QString::fromUtf8("Bitstream Charter"));
        font5.setPointSize(36);
        setupLabel->setFont(font5);
        setupLabel->setAlignment(Qt::AlignCenter);
        demoLabel = new QLabel(page_2);
        demoLabel->setObjectName(QString::fromUtf8("demoLabel"));
        demoLabel->setGeometry(QRect(1060, 80, 200, 34));
        QFont font6;
        font6.setFamily(QString::fromUtf8("Bitstream Charter"));
        font6.setPointSize(20);
        font6.setBold(true);
        font6.setWeight(75);
        demoLabel->setFont(font6);
        feedLabel = new QLabel(page_2);
        feedLabel->setObjectName(QString::fromUtf8("feedLabel"));
        feedLabel->setGeometry(QRect(30, 80, 140, 34));
        feedLabel->setFont(font6);
        recordButton = new QPushButton(page_2);
        recordButton->setObjectName(QString::fromUtf8("recordButton"));
        recordButton->setGeometry(QRect(1060, 910, 200, 54));
        recordButton->setFont(font);
        audioSelectLabel = new QLabel(page_2);
        audioSelectLabel->setObjectName(QString::fromUtf8("audioSelectLabel"));
        audioSelectLabel->setGeometry(QRect(1060, 470, 150, 28));
        QFont font7;
        font7.setFamily(QString::fromUtf8("Bitstream Charter"));
        font7.setPointSize(18);
        font7.setBold(true);
        font7.setWeight(75);
        audioSelectLabel->setFont(font7);
        selectAudio = new QComboBox(page_2);
        selectAudio->setObjectName(QString::fromUtf8("selectAudio"));
        selectAudio->setGeometry(QRect(1060, 510, 200, 45));
        QFont font8;
        font8.setFamily(QString::fromUtf8("Bitstream Charter"));
        font8.setPointSize(16);
        selectAudio->setFont(font8);
        selectAudio->setIconSize(QSize(16, 16));
        reconnectButton = new QPushButton(page_2);
        reconnectButton->setObjectName(QString::fromUtf8("reconnectButton"));
        reconnectButton->setGeometry(QRect(1060, 830, 200, 54));
        reconnectButton->setFont(font);
        imageContrastSlider = new QSlider(page_2);
        imageContrastSlider->setObjectName(QString::fromUtf8("imageContrastSlider"));
        imageContrastSlider->setGeometry(QRect(1090, 630, 20, 180));
        imageContrastSlider->setMaximum(50);
        imageContrastSlider->setOrientation(Qt::Vertical);
        LEDintensitySlider = new QSlider(page_2);
        LEDintensitySlider->setObjectName(QString::fromUtf8("LEDintensitySlider"));
        LEDintensitySlider->setGeometry(QRect(1190, 630, 20, 180));
        LEDintensitySlider->setMaximum(350);
        LEDintensitySlider->setValue(175);
        LEDintensitySlider->setOrientation(Qt::Vertical);
        imageContrastLabel = new QLabel(page_2);
        imageContrastLabel->setObjectName(QString::fromUtf8("imageContrastLabel"));
        imageContrastLabel->setGeometry(QRect(1050, 575, 100, 50));
        QFont font9;
        font9.setFamily(QString::fromUtf8("Bitstream Charter"));
        font9.setPointSize(12);
        font9.setBold(true);
        font9.setWeight(75);
        imageContrastLabel->setFont(font9);
        imageContrastLabel->setAlignment(Qt::AlignCenter);
        imageContrastLabel->setWordWrap(true);
        LEDintensityLabel = new QLabel(page_2);
        LEDintensityLabel->setObjectName(QString::fromUtf8("LEDintensityLabel"));
        LEDintensityLabel->setGeometry(QRect(1150, 575, 100, 50));
        LEDintensityLabel->setFont(font9);
        LEDintensityLabel->setAlignment(Qt::AlignCenter);
        LEDintensityLabel->setWordWrap(true);
        stackedWidget->addWidget(page_2);
        page_3 = new QWidget();
        page_3->setObjectName(QString::fromUtf8("page_3"));
        playView = new QGraphicsView(page_3);
        playView->setObjectName(QString::fromUtf8("playView"));
        playView->setGeometry(QRect(240, 80, 800, 640));
        playView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        playView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        playButton = new QPushButton(page_3);
        playButton->setObjectName(QString::fromUtf8("playButton"));
        playButton->setGeometry(QRect(540, 730, 200, 54));
        playButton->setFont(font);
        reviewLabel = new QLabel(page_3);
        reviewLabel->setObjectName(QString::fromUtf8("reviewLabel"));
        reviewLabel->setGeometry(QRect(340, 20, 600, 34));
        reviewLabel->setFont(font5);
        reviewLabel->setAlignment(Qt::AlignCenter);
        vidQualityBox = new QGroupBox(page_3);
        vidQualityBox->setObjectName(QString::fromUtf8("vidQualityBox"));
        vidQualityBox->setGeometry(QRect(380, 815, 200, 160));
        vidQualityBox->setFont(font);
        acceptVidButton = new QRadioButton(vidQualityBox);
        acceptVidButton->setObjectName(QString::fromUtf8("acceptVidButton"));
        acceptVidButton->setGeometry(QRect(60, 50, 130, 44));
        acceptVidButton->setFont(font);
        acceptVidButton->setIconSize(QSize(48, 48));
        rejectVidButton = new QRadioButton(vidQualityBox);
        rejectVidButton->setObjectName(QString::fromUtf8("rejectVidButton"));
        rejectVidButton->setGeometry(QRect(60, 110, 130, 44));
        rejectVidButton->setFont(font);
        rejectVidButton->setIconSize(QSize(48, 48));
        rejectionNotes = new QTextEdit(page_3);
        rejectionNotes->setObjectName(QString::fromUtf8("rejectionNotes"));
        rejectionNotes->setGeometry(QRect(590, 815, 360, 160));
        rejectionNotes->setFont(font);
        postReviewButton = new QPushButton(page_3);
        postReviewButton->setObjectName(QString::fromUtf8("postReviewButton"));
        postReviewButton->setGeometry(QRect(1067, 900, 180, 54));
        postReviewButton->setFont(font);
        rejectionWarningLabel = new QLabel(page_3);
        rejectionWarningLabel->setObjectName(QString::fromUtf8("rejectionWarningLabel"));
        rejectionWarningLabel->setGeometry(QRect(1010, 730, 280, 160));
        rejectionWarningLabel->setFont(font2);
        rejectionWarningLabel->setAlignment(Qt::AlignCenter);
        rejectionWarningLabel->setWordWrap(true);
        stackedWidget->addWidget(page_3);
        page_4 = new QWidget();
        page_4->setObjectName(QString::fromUtf8("page_4"));
        imageAnalysisLabel = new QLabel(page_4);
        imageAnalysisLabel->setObjectName(QString::fromUtf8("imageAnalysisLabel"));
        imageAnalysisLabel->setGeometry(QRect(340, 15, 600, 60));
        imageAnalysisLabel->setFont(font5);
        imageAnalysisLabel->setAlignment(Qt::AlignCenter);
        analyzeButton = new QPushButton(page_4);
        analyzeButton->setObjectName(QString::fromUtf8("analyzeButton"));
        analyzeButton->setGeometry(QRect(540, 600, 200, 54));
        analyzeButton->setFont(font);
        analysisProgressBar = new QProgressBar(page_4);
        analysisProgressBar->setObjectName(QString::fromUtf8("analysisProgressBar"));
        analysisProgressBar->setGeometry(QRect(240, 400, 800, 60));
        analysisProgressBar->setFont(font);
        analysisProgressBar->setValue(0);
        analysisProgressLabel = new QLabel(page_4);
        analysisProgressLabel->setObjectName(QString::fromUtf8("analysisProgressLabel"));
        analysisProgressLabel->setGeometry(QRect(640, 350, 800, 34));
        analysisProgressLabel->setFont(font);
        beginAnalysisLabel = new QLabel(page_4);
        beginAnalysisLabel->setObjectName(QString::fromUtf8("beginAnalysisLabel"));
        beginAnalysisLabel->setGeometry(QRect(240, 350, 500, 34));
        beginAnalysisLabel->setFont(font);
        stackedWidget->addWidget(page_4);
        page_5 = new QWidget();
        page_5->setObjectName(QString::fromUtf8("page_5"));
        completedLabel = new QLabel(page_5);
        completedLabel->setObjectName(QString::fromUtf8("completedLabel"));
        completedLabel->setGeometry(QRect(380, 15, 520, 60));
        completedLabel->setFont(font5);
        completedLabel->setAlignment(Qt::AlignCenter);
        completedLabel->setWordWrap(true);
        pushButton = new QPushButton(page_5);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(540, 900, 200, 54));
        pushButton->setFont(font);
        eval_patientID = new QLabel(page_5);
        eval_patientID->setObjectName(QString::fromUtf8("eval_patientID"));
        eval_patientID->setGeometry(QRect(100, 260, 160, 40));
        eval_patientID->setFont(font1);
        eval_patientID_text = new QLabel(page_5);
        eval_patientID_text->setObjectName(QString::fromUtf8("eval_patientID_text"));
        eval_patientID_text->setGeometry(QRect(270, 260, 300, 40));
        eval_patientID_text->setFont(font3);
        eval_serialNum = new QLabel(page_5);
        eval_serialNum->setObjectName(QString::fromUtf8("eval_serialNum"));
        eval_serialNum->setGeometry(QRect(100, 120, 340, 40));
        eval_serialNum->setFont(font1);
        eval_serialNum_text = new QLabel(page_5);
        eval_serialNum_text->setObjectName(QString::fromUtf8("eval_serialNum_text"));
        eval_serialNum_text->setGeometry(QRect(440, 120, 300, 40));
        eval_serialNum_text->setFont(font3);
        eval_testDate_text = new QLabel(page_5);
        eval_testDate_text->setObjectName(QString::fromUtf8("eval_testDate_text"));
        eval_testDate_text->setGeometry(QRect(260, 170, 300, 40));
        eval_testDate_text->setFont(font3);
        eval_testDate = new QLabel(page_5);
        eval_testDate->setObjectName(QString::fromUtf8("eval_testDate"));
        eval_testDate->setGeometry(QRect(100, 170, 150, 40));
        eval_testDate->setFont(font1);
        eval_testTime = new QLabel(page_5);
        eval_testTime->setObjectName(QString::fromUtf8("eval_testTime"));
        eval_testTime->setGeometry(QRect(580, 170, 100, 40));
        eval_testTime->setFont(font1);
        eval_testTime_text = new QLabel(page_5);
        eval_testTime_text->setObjectName(QString::fromUtf8("eval_testTime_text"));
        eval_testTime_text->setGeometry(QRect(680, 170, 300, 40));
        eval_testTime_text->setFont(font3);
        eval_operator1_text = new QLabel(page_5);
        eval_operator1_text->setObjectName(QString::fromUtf8("eval_operator1_text"));
        eval_operator1_text->setGeometry(QRect(260, 370, 300, 40));
        eval_operator1_text->setFont(font3);
        eval_operator1 = new QLabel(page_5);
        eval_operator1->setObjectName(QString::fromUtf8("eval_operator1"));
        eval_operator1->setGeometry(QRect(100, 370, 160, 40));
        eval_operator1->setFont(font1);
        eval_numTests1_text = new QLabel(page_5);
        eval_numTests1_text->setObjectName(QString::fromUtf8("eval_numTests1_text"));
        eval_numTests1_text->setGeometry(QRect(930, 370, 80, 40));
        eval_numTests1_text->setFont(font3);
        eval_numTests1 = new QLabel(page_5);
        eval_numTests1->setObjectName(QString::fromUtf8("eval_numTests1"));
        eval_numTests1->setGeometry(QRect(580, 370, 350, 40));
        eval_numTests1->setFont(font1);
        eval_test1 = new QLabel(page_5);
        eval_test1->setObjectName(QString::fromUtf8("eval_test1"));
        eval_test1->setGeometry(QRect(190, 440, 250, 40));
        eval_test1->setFont(font1);
        eval_test1->setAlignment(Qt::AlignCenter);
        eval_test1_right = new QLabel(page_5);
        eval_test1_right->setObjectName(QString::fromUtf8("eval_test1_right"));
        eval_test1_right->setGeometry(QRect(100, 490, 100, 40));
        QFont font10;
        font10.setFamily(QString::fromUtf8("Bitstream Charter"));
        font10.setPointSize(24);
        font10.setBold(false);
        font10.setUnderline(true);
        font10.setWeight(50);
        eval_test1_right->setFont(font10);
        eval_test1_right->setAlignment(Qt::AlignCenter);
        eval_test1_center = new QLabel(page_5);
        eval_test1_center->setObjectName(QString::fromUtf8("eval_test1_center"));
        eval_test1_center->setGeometry(QRect(270, 490, 100, 40));
        eval_test1_center->setFont(font10);
        eval_test1_center->setAlignment(Qt::AlignCenter);
        eval_test1_left = new QLabel(page_5);
        eval_test1_left->setObjectName(QString::fromUtf8("eval_test1_left"));
        eval_test1_left->setGeometry(QRect(440, 490, 100, 40));
        eval_test1_left->setFont(font10);
        eval_test1_left->setAlignment(Qt::AlignCenter);
        eval_test1_right_text = new QLabel(page_5);
        eval_test1_right_text->setObjectName(QString::fromUtf8("eval_test1_right_text"));
        eval_test1_right_text->setGeometry(QRect(80, 540, 140, 40));
        QFont font11;
        font11.setFamily(QString::fromUtf8("Bitstream Charter"));
        font11.setPointSize(24);
        font11.setBold(false);
        font11.setUnderline(false);
        font11.setWeight(50);
        eval_test1_right_text->setFont(font11);
        eval_test1_right_text->setAlignment(Qt::AlignCenter);
        eval_test1_center_text = new QLabel(page_5);
        eval_test1_center_text->setObjectName(QString::fromUtf8("eval_test1_center_text"));
        eval_test1_center_text->setGeometry(QRect(250, 540, 140, 40));
        eval_test1_center_text->setFont(font11);
        eval_test1_center_text->setAlignment(Qt::AlignCenter);
        eval_test1_left_text = new QLabel(page_5);
        eval_test1_left_text->setObjectName(QString::fromUtf8("eval_test1_left_text"));
        eval_test1_left_text->setGeometry(QRect(420, 540, 140, 40));
        eval_test1_left_text->setFont(font11);
        eval_test1_left_text->setAlignment(Qt::AlignCenter);
        eval_test2_center = new QLabel(page_5);
        eval_test2_center->setObjectName(QString::fromUtf8("eval_test2_center"));
        eval_test2_center->setGeometry(QRect(890, 490, 100, 40));
        eval_test2_center->setFont(font10);
        eval_test2_center->setAlignment(Qt::AlignCenter);
        eval_test2 = new QLabel(page_5);
        eval_test2->setObjectName(QString::fromUtf8("eval_test2"));
        eval_test2->setGeometry(QRect(810, 440, 250, 40));
        eval_test2->setFont(font1);
        eval_test2->setAlignment(Qt::AlignCenter);
        eval_test2_center_text = new QLabel(page_5);
        eval_test2_center_text->setObjectName(QString::fromUtf8("eval_test2_center_text"));
        eval_test2_center_text->setGeometry(QRect(870, 540, 140, 40));
        eval_test2_center_text->setFont(font11);
        eval_test2_center_text->setAlignment(Qt::AlignCenter);
        eval_test2_right = new QLabel(page_5);
        eval_test2_right->setObjectName(QString::fromUtf8("eval_test2_right"));
        eval_test2_right->setGeometry(QRect(720, 490, 100, 40));
        eval_test2_right->setFont(font10);
        eval_test2_right->setAlignment(Qt::AlignCenter);
        eval_test2_right_text = new QLabel(page_5);
        eval_test2_right_text->setObjectName(QString::fromUtf8("eval_test2_right_text"));
        eval_test2_right_text->setGeometry(QRect(700, 540, 140, 40));
        eval_test2_right_text->setFont(font11);
        eval_test2_right_text->setAlignment(Qt::AlignCenter);
        eval_test2_left = new QLabel(page_5);
        eval_test2_left->setObjectName(QString::fromUtf8("eval_test2_left"));
        eval_test2_left->setGeometry(QRect(1060, 490, 100, 40));
        eval_test2_left->setFont(font10);
        eval_test2_left->setAlignment(Qt::AlignCenter);
        eval_test2_left_text = new QLabel(page_5);
        eval_test2_left_text->setObjectName(QString::fromUtf8("eval_test2_left_text"));
        eval_test2_left_text->setGeometry(QRect(1040, 540, 140, 40));
        eval_test2_left_text->setFont(font11);
        eval_test2_left_text->setAlignment(Qt::AlignCenter);
        eval_numTests2 = new QLabel(page_5);
        eval_numTests2->setObjectName(QString::fromUtf8("eval_numTests2"));
        eval_numTests2->setGeometry(QRect(580, 650, 350, 40));
        eval_numTests2->setFont(font1);
        eval_test1_right_2 = new QLabel(page_5);
        eval_test1_right_2->setObjectName(QString::fromUtf8("eval_test1_right_2"));
        eval_test1_right_2->setGeometry(QRect(100, 770, 100, 40));
        eval_test1_right_2->setFont(font10);
        eval_test1_right_2->setAlignment(Qt::AlignCenter);
        eval_test1_right_text_2 = new QLabel(page_5);
        eval_test1_right_text_2->setObjectName(QString::fromUtf8("eval_test1_right_text_2"));
        eval_test1_right_text_2->setGeometry(QRect(80, 820, 140, 40));
        eval_test1_right_text_2->setFont(font11);
        eval_test1_right_text_2->setAlignment(Qt::AlignCenter);
        eval_operator2 = new QLabel(page_5);
        eval_operator2->setObjectName(QString::fromUtf8("eval_operator2"));
        eval_operator2->setGeometry(QRect(100, 650, 160, 40));
        eval_operator2->setFont(font1);
        eval_test1_center_text_2 = new QLabel(page_5);
        eval_test1_center_text_2->setObjectName(QString::fromUtf8("eval_test1_center_text_2"));
        eval_test1_center_text_2->setGeometry(QRect(250, 820, 140, 40));
        eval_test1_center_text_2->setFont(font11);
        eval_test1_center_text_2->setAlignment(Qt::AlignCenter);
        eval_test2_center_2 = new QLabel(page_5);
        eval_test2_center_2->setObjectName(QString::fromUtf8("eval_test2_center_2"));
        eval_test2_center_2->setGeometry(QRect(890, 770, 100, 40));
        eval_test2_center_2->setFont(font10);
        eval_test2_center_2->setAlignment(Qt::AlignCenter);
        eval_test2_left_2 = new QLabel(page_5);
        eval_test2_left_2->setObjectName(QString::fromUtf8("eval_test2_left_2"));
        eval_test2_left_2->setGeometry(QRect(1060, 770, 100, 40));
        eval_test2_left_2->setFont(font10);
        eval_test2_left_2->setAlignment(Qt::AlignCenter);
        eval_test1_left_text_2 = new QLabel(page_5);
        eval_test1_left_text_2->setObjectName(QString::fromUtf8("eval_test1_left_text_2"));
        eval_test1_left_text_2->setGeometry(QRect(420, 820, 140, 40));
        eval_test1_left_text_2->setFont(font11);
        eval_test1_left_text_2->setAlignment(Qt::AlignCenter);
        eval_test1_center_2 = new QLabel(page_5);
        eval_test1_center_2->setObjectName(QString::fromUtf8("eval_test1_center_2"));
        eval_test1_center_2->setGeometry(QRect(270, 770, 100, 40));
        eval_test1_center_2->setFont(font10);
        eval_test1_center_2->setAlignment(Qt::AlignCenter);
        eval_test2_left_text_2 = new QLabel(page_5);
        eval_test2_left_text_2->setObjectName(QString::fromUtf8("eval_test2_left_text_2"));
        eval_test2_left_text_2->setGeometry(QRect(1040, 820, 140, 40));
        eval_test2_left_text_2->setFont(font11);
        eval_test2_left_text_2->setAlignment(Qt::AlignCenter);
        eval_test2_right_2 = new QLabel(page_5);
        eval_test2_right_2->setObjectName(QString::fromUtf8("eval_test2_right_2"));
        eval_test2_right_2->setGeometry(QRect(720, 770, 100, 40));
        eval_test2_right_2->setFont(font10);
        eval_test2_right_2->setAlignment(Qt::AlignCenter);
        eval_test1_2 = new QLabel(page_5);
        eval_test1_2->setObjectName(QString::fromUtf8("eval_test1_2"));
        eval_test1_2->setGeometry(QRect(190, 720, 250, 40));
        eval_test1_2->setFont(font1);
        eval_test1_2->setAlignment(Qt::AlignCenter);
        eval_test2_center_text_2 = new QLabel(page_5);
        eval_test2_center_text_2->setObjectName(QString::fromUtf8("eval_test2_center_text_2"));
        eval_test2_center_text_2->setGeometry(QRect(870, 820, 140, 40));
        eval_test2_center_text_2->setFont(font11);
        eval_test2_center_text_2->setAlignment(Qt::AlignCenter);
        eval_test2_right_text_2 = new QLabel(page_5);
        eval_test2_right_text_2->setObjectName(QString::fromUtf8("eval_test2_right_text_2"));
        eval_test2_right_text_2->setGeometry(QRect(700, 820, 140, 40));
        eval_test2_right_text_2->setFont(font11);
        eval_test2_right_text_2->setAlignment(Qt::AlignCenter);
        eval_operator2_text = new QLabel(page_5);
        eval_operator2_text->setObjectName(QString::fromUtf8("eval_operator2_text"));
        eval_operator2_text->setGeometry(QRect(260, 650, 300, 40));
        eval_operator2_text->setFont(font3);
        eval_test2_2 = new QLabel(page_5);
        eval_test2_2->setObjectName(QString::fromUtf8("eval_test2_2"));
        eval_test2_2->setGeometry(QRect(810, 720, 250, 40));
        eval_test2_2->setFont(font1);
        eval_test2_2->setAlignment(Qt::AlignCenter);
        eval_numTests2_text = new QLabel(page_5);
        eval_numTests2_text->setObjectName(QString::fromUtf8("eval_numTests2_text"));
        eval_numTests2_text->setGeometry(QRect(930, 650, 80, 40));
        eval_numTests2_text->setFont(font3);
        eval_test1_left_2 = new QLabel(page_5);
        eval_test1_left_2->setObjectName(QString::fromUtf8("eval_test1_left_2"));
        eval_test1_left_2->setGeometry(QRect(440, 770, 100, 40));
        eval_test1_left_2->setFont(font10);
        eval_test1_left_2->setAlignment(Qt::AlignCenter);
        stackedWidget->addWidget(page_5);
        iStrabGui->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(iStrabGui);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1280, 25));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        iStrabGui->setMenuBar(menuBar);
        statusBar = new QStatusBar(iStrabGui);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        iStrabGui->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionExit);

        retranslateUi(iStrabGui);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(iStrabGui);
    } // setupUi

    void retranslateUi(QMainWindow *iStrabGui)
    {
        iStrabGui->setWindowTitle(QApplication::translate("iStrabGui", "iStrabGui", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("iStrabGui", "Exit", 0, QApplication::UnicodeUTF8));
        startButton->setText(QApplication::translate("iStrabGui", "Start Test", 0, QApplication::UnicodeUTF8));
        techLabel->setText(QApplication::translate("iStrabGui", "Operator:", 0, QApplication::UnicodeUTF8));
        patientLabel->setText(QApplication::translate("iStrabGui", "Patient ID:", 0, QApplication::UnicodeUTF8));
        warningLabel->setText(QApplication::translate("iStrabGui", "Please enter the information above", 0, QApplication::UnicodeUTF8));
        multipleTechs->setText(QApplication::translate("iStrabGui", "  Multiple Technicians", 0, QApplication::UnicodeUTF8));
        techNumber->setText(QApplication::translate("iStrabGui", "Technician #1", 0, QApplication::UnicodeUTF8));
        setupLabel->setText(QApplication::translate("iStrabGui", "Please set up the patient for the test", 0, QApplication::UnicodeUTF8));
        demoLabel->setText(QApplication::translate("iStrabGui", "Optimal Setup:", 0, QApplication::UnicodeUTF8));
        feedLabel->setText(QApplication::translate("iStrabGui", "Live Feed:", 0, QApplication::UnicodeUTF8));
        recordButton->setText(QApplication::translate("iStrabGui", "Record", 0, QApplication::UnicodeUTF8));
        audioSelectLabel->setText(QApplication::translate("iStrabGui", "Select Audio:", 0, QApplication::UnicodeUTF8));
        selectAudio->clear();
        selectAudio->insertItems(0, QStringList()
         << QApplication::translate("iStrabGui", "Bike Horn", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("iStrabGui", "Computer Magic", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("iStrabGui", "Cow Mooing", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("iStrabGui", "Doorbell", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("iStrabGui", "Electrical Sweep", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("iStrabGui", "Magic Wand", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("iStrabGui", "Music Box", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("iStrabGui", "Ship Bell", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("iStrabGui", "Sleigh Bells Ringing", 0, QApplication::UnicodeUTF8)
        );
        reconnectButton->setText(QApplication::translate("iStrabGui", "Reconnect", 0, QApplication::UnicodeUTF8));
        imageContrastLabel->setText(QApplication::translate("iStrabGui", "Image Contrast", 0, QApplication::UnicodeUTF8));
        LEDintensityLabel->setText(QApplication::translate("iStrabGui", "LED Intensity", 0, QApplication::UnicodeUTF8));
        playButton->setText(QApplication::translate("iStrabGui", "Play Video", 0, QApplication::UnicodeUTF8));
        reviewLabel->setText(QApplication::translate("iStrabGui", "Please Review the Video:", 0, QApplication::UnicodeUTF8));
        vidQualityBox->setTitle(QApplication::translate("iStrabGui", "Video Review:", 0, QApplication::UnicodeUTF8));
        acceptVidButton->setText(QApplication::translate("iStrabGui", "Accept", 0, QApplication::UnicodeUTF8));
        rejectVidButton->setText(QApplication::translate("iStrabGui", "Reject", 0, QApplication::UnicodeUTF8));
        postReviewButton->setText(QApplication::translate("iStrabGui", "Next", 0, QApplication::UnicodeUTF8));
        rejectionWarningLabel->setText(QApplication::translate("iStrabGui", "Please include reasons for rejection", 0, QApplication::UnicodeUTF8));
        imageAnalysisLabel->setText(QApplication::translate("iStrabGui", "Image Analysis", 0, QApplication::UnicodeUTF8));
        analyzeButton->setText(QApplication::translate("iStrabGui", "Analyze", 0, QApplication::UnicodeUTF8));
        analysisProgressLabel->setText(QString());
        beginAnalysisLabel->setText(QApplication::translate("iStrabGui", "Click analyze to begin", 0, QApplication::UnicodeUTF8));
        completedLabel->setText(QApplication::translate("iStrabGui", "Evaluation Results", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("iStrabGui", "Exit Program", 0, QApplication::UnicodeUTF8));
        eval_patientID->setText(QApplication::translate("iStrabGui", "Patient ID:", 0, QApplication::UnicodeUTF8));
        eval_patientID_text->setText(QApplication::translate("iStrabGui", "<Insert ID>", 0, QApplication::UnicodeUTF8));
        eval_serialNum->setText(QApplication::translate("iStrabGui", "Device Serial Number:", 0, QApplication::UnicodeUTF8));
        eval_serialNum_text->setText(QApplication::translate("iStrabGui", "<Insert Serial Num>", 0, QApplication::UnicodeUTF8));
        eval_testDate_text->setText(QApplication::translate("iStrabGui", "<Insert Visit Date>", 0, QApplication::UnicodeUTF8));
        eval_testDate->setText(QApplication::translate("iStrabGui", "Visit Date:", 0, QApplication::UnicodeUTF8));
        eval_testTime->setText(QApplication::translate("iStrabGui", "Time:", 0, QApplication::UnicodeUTF8));
        eval_testTime_text->setText(QApplication::translate("iStrabGui", "<Insert Time>", 0, QApplication::UnicodeUTF8));
        eval_operator1_text->setText(QApplication::translate("iStrabGui", "<Operator>", 0, QApplication::UnicodeUTF8));
        eval_operator1->setText(QApplication::translate("iStrabGui", "Operator:", 0, QApplication::UnicodeUTF8));
        eval_numTests1_text->setText(QApplication::translate("iStrabGui", "<#>", 0, QApplication::UnicodeUTF8));
        eval_numTests1->setText(QApplication::translate("iStrabGui", "Total Number of Tests:", 0, QApplication::UnicodeUTF8));
        eval_test1->setText(QApplication::translate("iStrabGui", "Test 1 Results", 0, QApplication::UnicodeUTF8));
        eval_test1_right->setText(QApplication::translate("iStrabGui", "Right", 0, QApplication::UnicodeUTF8));
        eval_test1_center->setText(QApplication::translate("iStrabGui", "Center", 0, QApplication::UnicodeUTF8));
        eval_test1_left->setText(QApplication::translate("iStrabGui", "Left", 0, QApplication::UnicodeUTF8));
        eval_test1_right_text->setText(QApplication::translate("iStrabGui", "<Right>", 0, QApplication::UnicodeUTF8));
        eval_test1_center_text->setText(QApplication::translate("iStrabGui", "<Cntr>", 0, QApplication::UnicodeUTF8));
        eval_test1_left_text->setText(QApplication::translate("iStrabGui", "<Left>", 0, QApplication::UnicodeUTF8));
        eval_test2_center->setText(QApplication::translate("iStrabGui", "Center", 0, QApplication::UnicodeUTF8));
        eval_test2->setText(QApplication::translate("iStrabGui", "Test 2 Results", 0, QApplication::UnicodeUTF8));
        eval_test2_center_text->setText(QApplication::translate("iStrabGui", "<Cntr>", 0, QApplication::UnicodeUTF8));
        eval_test2_right->setText(QApplication::translate("iStrabGui", "Right", 0, QApplication::UnicodeUTF8));
        eval_test2_right_text->setText(QApplication::translate("iStrabGui", "<Right>", 0, QApplication::UnicodeUTF8));
        eval_test2_left->setText(QApplication::translate("iStrabGui", "Left", 0, QApplication::UnicodeUTF8));
        eval_test2_left_text->setText(QApplication::translate("iStrabGui", "<Left>", 0, QApplication::UnicodeUTF8));
        eval_numTests2->setText(QApplication::translate("iStrabGui", "Total Number of Tests:", 0, QApplication::UnicodeUTF8));
        eval_test1_right_2->setText(QApplication::translate("iStrabGui", "Right", 0, QApplication::UnicodeUTF8));
        eval_test1_right_text_2->setText(QApplication::translate("iStrabGui", "<Right>", 0, QApplication::UnicodeUTF8));
        eval_operator2->setText(QApplication::translate("iStrabGui", "Operator:", 0, QApplication::UnicodeUTF8));
        eval_test1_center_text_2->setText(QApplication::translate("iStrabGui", "<Cntr>", 0, QApplication::UnicodeUTF8));
        eval_test2_center_2->setText(QApplication::translate("iStrabGui", "Center", 0, QApplication::UnicodeUTF8));
        eval_test2_left_2->setText(QApplication::translate("iStrabGui", "Left", 0, QApplication::UnicodeUTF8));
        eval_test1_left_text_2->setText(QApplication::translate("iStrabGui", "<Left>", 0, QApplication::UnicodeUTF8));
        eval_test1_center_2->setText(QApplication::translate("iStrabGui", "Center", 0, QApplication::UnicodeUTF8));
        eval_test2_left_text_2->setText(QApplication::translate("iStrabGui", "<Left>", 0, QApplication::UnicodeUTF8));
        eval_test2_right_2->setText(QApplication::translate("iStrabGui", "Right", 0, QApplication::UnicodeUTF8));
        eval_test1_2->setText(QApplication::translate("iStrabGui", "Test 1 Results", 0, QApplication::UnicodeUTF8));
        eval_test2_center_text_2->setText(QApplication::translate("iStrabGui", "<Cntr>", 0, QApplication::UnicodeUTF8));
        eval_test2_right_text_2->setText(QApplication::translate("iStrabGui", "<Right>", 0, QApplication::UnicodeUTF8));
        eval_operator2_text->setText(QApplication::translate("iStrabGui", "<Operator>", 0, QApplication::UnicodeUTF8));
        eval_test2_2->setText(QApplication::translate("iStrabGui", "Test 2 Results", 0, QApplication::UnicodeUTF8));
        eval_numTests2_text->setText(QApplication::translate("iStrabGui", "<#>", 0, QApplication::UnicodeUTF8));
        eval_test1_left_2->setText(QApplication::translate("iStrabGui", "Left", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("iStrabGui", "File", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class iStrabGui: public Ui_iStrabGui {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ISTRABGUI_H
