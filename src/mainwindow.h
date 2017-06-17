#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>

#include "opencv2/core.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showFullScreenImage(const QFileInfo& _file);
    void showProcessedImages(const QFileInfo& _file);

    void fillRbgSlices(const QFileInfo& _file);
    void fillRbgSlices(IplImage* source_image);
    void showSlices();

private:
    struct Slices
    {
        IplImage* original_rgb = nullptr;
        // rbg channels:
        IplImage* r_plane = nullptr;
        IplImage* g_plane = nullptr;
        IplImage* b_plane = nullptr;
        static const std::list<const char *> &slices_names();
    } m_slices;

private:
    Ui::MainWindow *ui;
    QSize m_screenSize;
};

#endif // MAINWINDOW_H
