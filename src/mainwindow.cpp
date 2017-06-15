#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QDebug>
#include <QDir>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

// для хранения каналов RGB после преобразования
IplImage* r_range = 0;
IplImage* g_range = 0;
IplImage* b_range = 0;
// для хранения суммарной картинки
IplImage* rgb_and = 0;

int Rmin = 1;
int Rmax = 255;

int Gmin = 1;
int Gmax = 255;

int Bmin = 1;
int Bmax = 255;

int RGBmax = 255;

//void myTrackbarRmin(int pos) {
//        Rmin = pos;
//        cvInRangeS(m_slices.r_plane, cvScalar(Rmin), cvScalar(Rmax), r_range);
//}

//void myTrackbarRmax(int pos) {
//        Rmax = pos;
//        cvInRangeS(m_slices.r_plane, cvScalar(Rmin), cvScalar(Rmax), r_range);
//}

//void myTrackbarGmin(int pos) {
//        Gmin = pos;
//        cvInRangeS(m_slices.g_plane, cvScalar(Gmin), cvScalar(Gmax), g_range);
//}

//void myTrackbarGmax(int pos) {
//        Gmax = pos;
//        cvInRangeS(m_slices.g_plane, cvScalar(Gmin), cvScalar(Gmax), g_range);
//}

//void myTrackbarBmin(int pos) {
//        Bmin = pos;
//        cvInRangeS(m_slices.b_plane, cvScalar(Bmin), cvScalar(Bmax), b_range);
//}

//void myTrackbarBmax(int pos) {
//        Bmax = pos;
//        cvInRangeS(m_slices.b_plane, cvScalar(Bmin), cvScalar(Bmax), b_range);
//}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_screenSize = QApplication::desktop()->screenGeometry().size();

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
    fillRbgSlices(image);
    r_range = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
    g_range = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
    b_range = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );
    rgb_and = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 );

    //
    // определяем минимальное и максимальное значение
    // у каналов HSV
    double framemin=0;
    double framemax=0;

    cvMinMaxLoc(m_slices.r_plane, &framemin, &framemax);
    printf("[R] %f x %f\n", framemin, framemax );
    Rmin = framemin;
    Rmax = framemax;
    cvMinMaxLoc(m_slices.g_plane, &framemin, &framemax);
    printf("[G] %f x %f\n", framemin, framemax );
    Gmin = framemin;
    Gmax = framemax;
    cvMinMaxLoc(m_slices.b_plane, &framemin, &framemax);
    printf("[B] %f x %f\n", framemin, framemax );
    Bmin = framemin;
    Bmax = framemax;

    // окна для отображения картинки
//    cvNamedWindow("original",CV_WINDOW_AUTOSIZE);
//    cvNamedWindow("R",CV_WINDOW_AUTOSIZE);
//    cvNamedWindow("G",CV_WINDOW_AUTOSIZE);
//    cvNamedWindow("B",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("R range",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("G range",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("B range",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("rgb and",CV_WINDOW_AUTOSIZE);

//    cvCreateTrackbar("Rmin", "R range", &Rmin, RGBmax, myTrackbarRmin);
//    cvCreateTrackbar("Rmax", "R range", &Rmax, RGBmax, myTrackbarRmax);
//    cvCreateTrackbar("Gmin", "G range", &Gmin, RGBmax, myTrackbarGmin);
//    cvCreateTrackbar("Gmax", "G range", &Gmax, RGBmax, myTrackbarGmax);
//    cvCreateTrackbar("Bmin", "B range", &Gmin, RGBmax, myTrackbarBmin);
//    cvCreateTrackbar("Bmax", "B range", &Gmax, RGBmax, myTrackbarBmax);

    // разместим окна по рабочему столу
    if(image->width <1920/4 && image->height<1080/2)
    {
//        cvMoveWindow("original", 0, 0);
//        cvMoveWindow("R", image->width+10, 0);
//        cvMoveWindow("G", (image->width+10)*2, 0);
//        cvMoveWindow("B", (image->width+10)*3, 0);
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
        cvShowImage( "R", m_slices.r_plane );
        cvShowImage( "G", m_slices.g_plane );
        cvShowImage( "B", m_slices.b_plane );

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

void MainWindow::fillRbgSlices(IplImage *source_image)
{
    m_slices.original_rgb = cvCreateImage( cvGetSize(source_image), IPL_DEPTH_8U, 3 );
    m_slices.r_plane      = cvCreateImage( cvGetSize(source_image), IPL_DEPTH_8U, 1 );
    m_slices.g_plane      = cvCreateImage( cvGetSize(source_image), IPL_DEPTH_8U, 1 );
    m_slices.b_plane      = cvCreateImage( cvGetSize(source_image), IPL_DEPTH_8U, 1 );
    //  копируем
    cvCopy(source_image, m_slices.original_rgb);
    // разбиваем на отельные каналы
    cvSplit( m_slices.original_rgb, m_slices.b_plane, m_slices.g_plane, m_slices.r_plane, 0 );

}

void MainWindow::showSlices()
{
    const int pic_count = 4;
    const int width = m_screenSize.width() / 4;
    const int height = m_screenSize.height() / 2;

    // окна для отображения картинки
    cvNamedWindow("original",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("R",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("G",CV_WINDOW_AUTOSIZE);
    cvNamedWindow("B",CV_WINDOW_AUTOSIZE);

    cvMoveWindow("original", 0,       0);
    cvMoveWindow("R",        width,   0);
    cvMoveWindow("G",        width*2, 0);
    cvMoveWindow("B",        width*3, 0);

    // показываем картинку
    cvShowImage("original",m_slices.original_rgb);
    // показываем слои
    cvShowImage( "R", m_slices.r_plane );
    cvShowImage( "G", m_slices.g_plane );
    cvShowImage( "B", m_slices.b_plane );
}
