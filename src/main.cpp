#include <QtGui/QApplication>
#include "istrabgui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    iStrabGui w;
    w.show();
    return a.exec();
}
