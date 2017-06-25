#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QDebug>
#include <QDir>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_screenSize = QApplication::desktop()->screenGeometry().size();

    connect(ui->m_spinBox, SIGNAL(valueChanged(int)), SLOT(updateThreshold(int)));
    loadFromFile();
}

MainWindow::~MainWindow()
{
    clear();
    delete ui;
}

void MainWindow::fillRbgSlices(const QFileInfo &_file)
{
    std::string tmp_filename = _file.absoluteFilePath().toStdString();
    const char* filename = tmp_filename.c_str();

    IplImage* image = cvLoadImage(filename,1);
    fillRbgSlices(image);
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

void MainWindow::fillEdges()
{
    int thresholdRatio = 3;

    cvCanny(m_slices.r_plane, m_slices.r_plane, m_threshold, thresholdRatio * m_threshold);
    cvCanny(m_slices.b_plane, m_slices.b_plane, m_threshold, thresholdRatio * m_threshold);
    cvCanny(m_slices.g_plane, m_slices.g_plane, m_threshold, thresholdRatio * m_threshold);

    m_slices.edges = cvCreateImage(cvGetSize(m_slices.original_rgb), IPL_DEPTH_8U, 1);
    cvAdd(m_slices.r_plane, m_slices.b_plane, m_slices.edges);
    cvAdd(m_slices.edges,   m_slices.g_plane, m_slices.edges);

    cvSmooth(m_slices.edges, m_slices.edges);
}

void MainWindow::findConturs()
{
    RNG rng(12345);

    Mat canny_output = cvarrToMat(m_slices.edges);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

    /// Draw contours
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for(uint i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
    }
    namedWindow( "Contours", CV_WINDOW_NORMAL );
    imshow( "Contours", drawing );
    cvResizeWindow("Contours", 100, 100);

    //--------- Apriximation:
    Mat drawingAprox = Mat::zeros( canny_output.size(), CV_8UC3 );
    vector<vector<Point> > aproximatedContours;
    for(uint i = 0; i< contours.size(); ++i)
    {
        vector<Point> approximatedContur;
        approxPolyDP(Mat(contours[i]), approximatedContur, arcLength(Mat(contours[i]), true)*0.02, true);
        aproximatedContours.push_back(approximatedContur);
        drawContours(drawingAprox, contours, i, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point() );
    }
    showMat(drawingAprox, "Aproximated");
    // ----------------------

    vector<vector<Point> > rectangleContours;
    for(uint i = 0; i< aproximatedContours.size(); ++i)
    {
        const vector<Point>& contur = aproximatedContours[i];
        if(contur.size() < 4)
            continue;

        double length = arcLength(contur, true);
        double area = contourArea(contur, false);
        double tolerance = length / 1024;

        if(length - 2 * area < tolerance)
        {
            rectangleContours.push_back(contur);
        }
    }

    if(rectangleContours.size() == 0)
        return;

    Mat rectangles = Mat::zeros( canny_output.size(), CV_8UC3 );
    for(uint i = 0; i< rectangleContours.size(); ++i)
    {
        drawContours(rectangles, rectangleContours, i, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point());
    }

    showMat(rectangles, "Rectangles");
}

void MainWindow::showSlices()
{
    auto pic_names = Slices::slicesNames();
    const int width = m_screenSize.width() / m_slices.cols();
    const int height = m_screenSize.height() / 2;

    for(const char *name : pic_names)
        cvNamedWindow(name, WINDOW_NORMAL);

    int i = 0;
    for(const char *name : pic_names)
    {
        int rowIndex    = i / m_slices.cols();
        int columnIndex = i % m_slices.cols();
        cvMoveWindow(name, width * columnIndex, height * rowIndex);
        ++i;
    }

    const std::list<IplImage *> slices = m_slices.slices();
    auto picIter = slices.begin();
    for(const char *name : pic_names)
    {
        Q_ASSERT(picIter != slices.end());
        IplImage* img = *picIter;
        cvShowImage(name, img);
        ++picIter;
    }

    for(const char* name : pic_names)
        cvResizeWindow(name, width, height);
}

void MainWindow::showMat(const Mat &_mat, const char* _windowName) const
{
    const int width = m_screenSize.width() / m_slices.cols();
    const int height = m_screenSize.height() / 2;

    namedWindow(_windowName, CV_WINDOW_NORMAL );
    imshow(_windowName, _mat);
    cvResizeWindow(_windowName, width, height);
}

void MainWindow::logContur(const vector<Point> &_contur)
{
    qDebug() << "==contur:==";
    for(const Point& point : _contur)
    {
        qDebug() << QString("[%1, %2]\n")
                    .arg(point.x)
                    .arg(point.y);
    }
}

void MainWindow::updateThreshold(int _newThreshold)
{
    m_threshold = _newThreshold;
    loadFromFile();
}

void MainWindow::loadFromFile()
{
    clear();
//    m_slices.clear();

    QDir dir = QDir::current();
    Q_ASSERT(dir.cdUp());
    Q_ASSERT(dir.cd("bin"));

    qDebug() << dir.absolutePath();
    const QFileInfoList picturesFiles = dir.entryInfoList(QStringList {"*.jpg"}, QDir::Files | QDir::Readable);
    for(const QFileInfo& picturesFile : picturesFiles)
    {
        fillRbgSlices(picturesFile);
        fillEdges();
        findConturs();

        showSlices();
    }
}

void MainWindow::clear()
{
    cvDestroyAllWindows();
    m_slices.clear();
}

MainWindow::Slices::Slices()
{}

MainWindow::Slices::~Slices()
{
    clear();
}

void MainWindow::Slices::clear()
{
    if(original_rgb) cvReleaseImage(&original_rgb);   original_rgb = nullptr;
    if(r_plane     ) cvReleaseImage(&r_plane     );   r_plane      = nullptr;
    if(g_plane     ) cvReleaseImage(&g_plane     );   g_plane      = nullptr;
    if(b_plane     ) cvReleaseImage(&b_plane     );   b_plane      = nullptr;
    if(edges       ) cvReleaseImage(&edges       );   edges        = nullptr;
}

const std::list<const char *>& MainWindow::Slices::slicesNames()
{
    static const std::list<const char*> pic_names {"original", "R", "G", "B", "edges", "borders"};
    return pic_names;
}

const std::list<IplImage *> MainWindow::Slices::slices()
{
    return std::list<IplImage *> {original_rgb, r_plane, g_plane, b_plane, edges, borders};
}
