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

    QDir dir = QDir::current();
    Q_ASSERT(dir.cdUp());
    Q_ASSERT(dir.cd("bin"));

    qDebug() << dir.absolutePath();
    const QFileInfoList picturesFiles = dir.entryInfoList(QStringList {"*.jpg"}, QDir::Files | QDir::Readable);
    for(const QFileInfo& picturesFile : picturesFiles)
    {
        showFullScreenImage(picturesFile);
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

    // create image window named "My Image"
    cv::namedWindow(filePath);
    // show the image on window
    cv::imshow(filePath, image);
}
