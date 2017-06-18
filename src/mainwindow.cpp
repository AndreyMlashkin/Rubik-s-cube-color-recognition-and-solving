#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QDebug>
#include <QDir>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

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

    Mat edges  = cvarrToMat(m_slices.r_plane);
    edges     += cvarrToMat(m_slices.b_plane);
    edges     += cvarrToMat(m_slices.g_plane);

    m_slices.edges = new IplImage(edges);
}

void MainWindow::showSlices()
{
    auto pic_names = Slices::slices_names();
    const int width = m_screenSize.width() / pic_names.size();
    const int height = m_screenSize.height() / 2;   

    for(const char *name : pic_names)
        cvNamedWindow(name, WINDOW_NORMAL);

    int i = 0;
    for(const char *name : pic_names)
        cvMoveWindow(name, i++ * width, 0);

    const std::list<IplImage *> slices = m_slices.slices();
    auto picIter = slices.begin();
    for(const char *name : pic_names)
    {
        Q_ASSERT(picIter != slices.end());
        IplImage* img = *picIter;
        cvShowImage(name, img);
        ++picIter;
    }

    for(const QString& name : pic_names)
        cvResizeWindow(name.toStdString().c_str(), width, height);
}

void MainWindow::updateThreshold(int _newThreshold)
{
    m_threshold = _newThreshold;
    loadFromFile();
}

void MainWindow::loadFromFile()
{
//    clear();
    m_slices.clear();

    QDir dir = QDir::current();
    Q_ASSERT(dir.cdUp());
    Q_ASSERT(dir.cd("bin"));

    qDebug() << dir.absolutePath();
    const QFileInfoList picturesFiles = dir.entryInfoList(QStringList {"*.jpg"}, QDir::Files | QDir::Readable);
    for(const QFileInfo& picturesFile : picturesFiles)
    {
        fillRbgSlices(picturesFile);
        fillEdges();

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
    //if(edges       ) cvReleaseImage(&edges       );   edges        = nullptr;
}

const std::list<const char *>& MainWindow::Slices::slices_names()
{
    static const std::list<const char*> pic_names {"original", "R", "G", "B", "edges"};
    return pic_names;
}

const std::list<IplImage *> MainWindow::Slices::slices()
{
    return std::list<IplImage *> {original_rgb, r_plane, g_plane, b_plane, edges};
}
