#include "mainwindow.h"
#include <QApplication>

#include "opencv2/highgui.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    int result = a.exec();
    cvDestroyAllWindows();
    return result;
}
