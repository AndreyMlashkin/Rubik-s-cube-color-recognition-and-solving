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
    void fillEdges();

    void showSlices();

private:
    struct Slices
    {
        Slices();
        ~Slices();
        void clear();

        static const std::list<const char *> &slices_names();
        const std::list<IplImage*> slices();

        IplImage* original_rgb = nullptr;
        // rbg channels:
        IplImage* r_plane = nullptr;
        IplImage* g_plane = nullptr;
        IplImage* b_plane = nullptr;

        IplImage* edges = nullptr;
    } m_slices;

private:
    Ui::MainWindow *ui;
    QSize m_screenSize;
};

#endif // MAINWINDOW_H
