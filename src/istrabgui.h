#ifndef ISTRABGUI_H
#define ISTRABGUI_H

#include <QMainWindow>
#include <overlay.h>

namespace Ui {
    class iStrabGui;
}

class iStrabGui : public QMainWindow {
    Q_OBJECT
public:
    iStrabGui(QWidget *parent = 0);
    ~iStrabGui();

protected:
    void changeEvent(QEvent *e);

public:
    Ui::iStrabGui *ui;

private slots:
    void on_actionExit_triggered();
    void on_reconnectButton_released();
    void on_pushButton_released();
    void on_analyzeButton_released();
    void on_postReviewButton_released();
    void on_rejectVidButton_released();
    void on_acceptVidButton_released();
    void on_playButton_released();
    void on_recordButton_released();
    void on_startButton_released();
    void finishedRecording();
    void clearRejectionNotes();
    void toggleMultipleTechs(int state);
    void setupDevices();
private:
    Overlay *overlay;
};

#endif // ISTRABGUI_H
