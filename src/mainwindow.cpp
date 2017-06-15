#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QDebug>
#include <QDir>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

// для хранения каналов RGB
IplImage* rgb = 0;
IplImage* r_plane = 0;
IplImage* g_plane = 0;
IplImage* b_plane = 0;
// для хранения каналов RGB после преобразования
IplImage* r_range = 0;
IplImage* g_range = 0;
IplImage* b_range = 0;
// для хранения суммарной картинки
IplImage* rgb_and = 0;

int Rmin = 0;
int Rmax = 256;

int Gmin = 0;
int Gmax = 256;

int Bmin = 0;
int Bmax = 256;

int RGBmax = 256;

void myTrackbarRmin(int pos) {
        Rmin = pos;
        cvInRangeS(r_plane, cvScalar(Rmin), cvScalar(Rmax), r_range);
}

void myTrackbarRmax(int pos) {
        Rmax = pos;
        cvInRangeS(r_plane, cvScalar(Rmin), cvScalar(Rmax), r_range);
}

void myTrackbarGmin(int pos) {
        Gmin = pos;
        cvInRangeS(g_plane, cvScalar(Gmin), cvScalar(Gmax), g_range);
}

void myTrackbarGmax(int pos) {
        Gmax = pos;
        cvInRangeS(g_plane, cvScalar(Gmin), cvScalar(Gmax), g_range);
}

void myTrackbarBmin(int pos) {
        Bmin = pos;
        cvInRangeS(b_plane, cvScalar(Bmin), cvScalar(Bmax), b_range);
}

void myTrackbarBmax(int pos) {
        Bmax = pos;
        cvInRangeS(b_plane, cvScalar(Bmin), cvScalar(Bmax), b_range);
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QDir dir = QDir::current();
    Q_ASSERT(dir.cdUp());
    Q_ASSERT(dir.cd("bin"));

    qDebug() << dir.absolutePath();
    const QFileInfoList picturesFiles = dir.entryInfoList(QStringList {"*.jpg"}, QDir::Files | QDir::Readable);
    for(const QFileInfo& picturesFile : picturesFiles)
    {
        //showFullScreenImage(picturesFile);
        showProcessedImages(picturesFile);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showFullScreenImage(const QFileInfo &_file)
{
    std::string filePath = _file.absoluteFilePath().toStdString();
    QSize screenSize = QApplication::desktop()->screenGeometry().size();

    // read an image
    cv::Mat image = cv::imread(filePath, 1);

    Size size;
    size.height = screenSize.height();
    size.width = screenSize.width();
    cv::resize(image, image, size);

    cv::namedWindow(filePath);
    cv::imshow(filePath, image);
}

void MainWindow::showProcessedImages(const QFileInfo &_file)
{
    std::string tmp_filename = _file.absoluteFilePath().toStdString();
    const char* filename = tmp_filename.c_str();

    IplImage* image = 0;
    IplImage* dst = 0;

    image = cvLoadImage(filename,1);

    printf("[i] image: %s\n", filename);
    assert( image != 0 );

    // создаём картинки
    rgb = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 3 );
    r_plane = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
    g_plane = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
    b_plane = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
    r_range = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
    g_range = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
    b_range = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
    rgb_and = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
    //  копируем
    cvCopy(image, rgb);
    // разбиваем на отельные каналы
    cvSplit( rgb, b_plane, g_plane, r_plane, 0 );

    //
    // определяем минимальное и максимальное значение
    // у каналов HSV
    double framemin=0;
    double framemax=0;

    cvMinMaxLoc(r_plane, &framemin, &framemax);
    printf("[R] %f x %f\n", framemin, framemax );
    Rmin = framemin;
    Rmax = framemax;
    cvMinMaxLoc(g_plane, &framemin, &framemax);
    printf("[G] %f x %f\n", framemin, framemax );
    Gmin = framemin;
    Gmax = framemax;
    cvMinMaxLoc(b_plane, &framemin, &framemax);
    printf("[B] %f x %f\n", framemin, framemax );
    Bmin = framemin;
    Bmax = framemax;

    // окна для отображения картинки
    cvNamedWindow("original",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("R",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("G",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("B",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("R range",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("G range",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("B range",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("rgb and",CV_WINDOW_AUTOSIZE);

    cvCreateTrackbar("Rmin", "R range", &Rmin, RGBmax, myTrackbarRmin);
    cvCreateTrackbar("Rmax", "R range", &Rmax, RGBmax, myTrackbarRmax);
    cvCreateTrackbar("Gmin", "G range", &Gmin, RGBmax, myTrackbarGmin);
    cvCreateTrackbar("Gmax", "G range", &Gmax, RGBmax, myTrackbarGmax);
    cvCreateTrackbar("Bmin", "B range", &Gmin, RGBmax, myTrackbarBmin);
    cvCreateTrackbar("Bmax", "B range", &Gmax, RGBmax, myTrackbarBmax);

    //
    // разместим окна по рабочему столу
    //
    if(image->width <1920/4 && image->height<1080/2)
    {
        cvMoveWindow("original", 0, 0);
        cvMoveWindow("R", image->width+10, 0);
        cvMoveWindow("G", (image->width+10)*2, 0);
        cvMoveWindow("B", (image->width+10)*3, 0);
        cvMoveWindow("rgb and", 0, image->height+30);
        cvMoveWindow("R range", image->width+10, image->height+30);
        cvMoveWindow("G range", (image->width+10)*2, image->height+30);
        cvMoveWindow("B range", (image->width+10)*3, image->height+30);
    }

    while(true)
    {

        // показываем картинку
        cvShowImage("original",image);

        // показываем слои
        cvShowImage( "R", r_plane );
        cvShowImage( "G", g_plane );
        cvShowImage( "B", b_plane );

        // показываем результат порогового преобразования
        cvShowImage( "R range", r_range );
        cvShowImage( "G range", g_range );
        cvShowImage( "B range", b_range );

        // складываем
        cvAnd(r_range, g_range, rgb_and);
        cvAnd(rgb_and, b_range, rgb_and);

        // показываем результат
        cvShowImage( "rgb and", rgb_and );

        char c = cvWaitKey(33);
        if (c == 27) { // если нажата ESC - выходим
            break;
        }
    }
}