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

double mean(std::vector<double> _values)
{
    double res = 0;
    for(const& val : _values)
        res += val;
    return res /= _values.size();
}

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

void MainWindow::findColorsOfConturs()
{
    vector<vector<Point>> contours = findConturs();
    vector<vector<Point>> aproximatedContours = aproximateConturs(contours);
    vector<vector<Point>> rectangleContours = filterConturs(aproximatedContours);

    std::vector<Scalar> colors = findColorOfShapes(rectangleContours);
    Q_ASSERT(rectangleContours.size() == colors.size());


}

std::vector<std::vector<Point> > MainWindow::findConturs()
{
    Mat canny_output = cvarrToMat(m_slices.edges);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

    Mat drawing = drawConturs(contours, true);
    showMat(drawing, "Contours");
    return contours;
}

std::vector<std::vector<Point> > MainWindow::aproximateConturs(std::vector<std::vector<Point> > &_contours)
{
    vector<vector<Point>> aproximatedContours;
    for(uint i = 0; i< _contours.size(); ++i)
    {
        double ACCURACY = 10;
        vector<Point> approximatedContur;
        approxPolyDP(Mat(_contours[i]), approximatedContur, ACCURACY, true);
        aproximatedContours.push_back(approximatedContur);
    }

    Mat drawingAprox = drawConturs(aproximatedContours);
    showMat(drawingAprox, "Aproximated");
    return aproximatedContours;
}

std::vector<std::vector<Point> > MainWindow::filterConturs(std::vector<std::vector<Point> >& _counturs)
{
    vector<vector<Point> > rectangleContours;
    vector<double> lengths, areas;
    for(uint i = 0; i< _counturs.size(); ++i)
    {
        const vector<Point>& contur = _counturs[i];
        if(contur.size() < 4 || contur.size() > 8)
            continue;

        double length = arcLength(contur, true);
        double area = contourArea(contur, false);
        double tolerance = length / 4048;

        if((length - 2 * area) < tolerance)
        {
            rectangleContours.push_back(contur);
            lengths.push_back(length);
            areas.push_back(area);
        }
    }

    if(rectangleContours.size() == 0)
    {
        qWarning() << "no rectangles found!";
        return rectangleContours;
    }

    qDebug() << "lengths: " << lengths;
    qDebug() << "areas: " << areas;
    qDebug() << "shapes found: " << areas.size();

    while(rectangleContours.size() > 27)
    {
        Q_ASSERT(areas.size() == rectangleContours.size());
        double arMean = mean(areas);

        int fluctuationAbs = 0;
        int fluctuationIndex = 0;
        for(uint i = 0; i < areas.size(); ++i)
        {
            double fluctuationValue = fabs(areas[i] - arMean);
            if( fluctuationValue > fluctuationAbs)
            {
                fluctuationAbs = fluctuationValue;
                fluctuationIndex = i;
            }
        }

        rectangleContours.erase(rectangleContours.begin() + fluctuationIndex);
        areas.erase(areas.begin() + fluctuationIndex);
    }

    Mat rectanglesDrawing = drawConturs(rectangleContours, true);
    showMat(rectanglesDrawing, "Rectangles");

    return rectangleContours;
}

std::vector<Scalar> MainWindow::findColorOfShapes(std::vector<std::vector<Point> > &_contours)
{
    Mat src = cvarrToMat(m_slices.original_rgb);
    Mat colored = Mat::zeros(pictureSize(), CV_8UC3 );

    Mat src_gray;
    cvtColor( src, src_gray, COLOR_BGR2GRAY );
    threshold( src_gray, src_gray, 50, 255, THRESH_BINARY );

    std::vector<Scalar> colors;

    for( size_t i = 0; i< _contours.size(); i++ )
    {
        Rect rect = boundingRect( _contours[i] );
        Scalar meanColor = mean( src( rect ), src_gray( rect ) );
        colors.push_back(meanColor);

        drawContours( colored, _contours, (int)i, meanColor, FILLED );
    }

    imshow( "colored", colored );
    return colors;
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

Mat MainWindow::drawConturs(std::vector<std::vector<Point> > _contours, bool _randomColors)
{
    RNG rng(12345);

    Mat drawing = Mat::zeros(pictureSize(), CV_8UC3 );
    for(uint i = 0; i< _contours.size(); i++ )
    {
        Scalar color = _randomColors?
                    Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) ) :
                    Scalar(255, 0, 0);
        drawContours( drawing, _contours, i, color, 2, 8);
    }
    return drawing;
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

    const QFileInfoList picturesFiles = dir.entryInfoList(QStringList {"*.jpg"}, QDir::Files | QDir::Readable);
    for(const QFileInfo& picturesFile : picturesFiles)
    {
        qDebug() << picturesFile.fileName();
        fillRbgSlices(picturesFile);
        fillEdges();
        findColorsOfConturs();

        showSlices();
    }
}

void MainWindow::clear()
{
    cvDestroyAllWindows();
    m_slices.clear();
}

Size MainWindow::pictureSize() const
{
    return Size(m_slices.original_rgb->width, m_slices.original_rgb->height);
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
    static const std::list<const char*> pic_names {"original", "R", "G", "B", "edges"};
    return pic_names;
}

const std::list<IplImage *> MainWindow::Slices::slices()
{
    return std::list<IplImage *> {original_rgb, r_plane, g_plane, b_plane, edges};
}
