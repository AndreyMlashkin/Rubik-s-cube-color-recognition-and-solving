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

    void fillRbgSlices(const QFileInfo& _file);
    void fillRbgSlices(IplImage* source_image);
    void fillEdges();
    void findConturs();

    void showSlices();

    void showMat(const cv::Mat& _mat, const char *_windowName) const;

    static void logContur(const std::vector<cv::Point>& _contur);

public slots:
    void updateThreshold(int _newThreshold);
    void loadFromFile();

private:
    void clear();

    struct Slices
    {
        Slices();
        ~Slices();
        void clear();

        int rows() const { return slicesNames().size() / cols(); }
        int cols() const { return 4; }

        static const std::list<const char *> &slicesNames();
        const std::list<IplImage*> slices();

        IplImage* original_rgb = nullptr;
        // rbg channels:
        IplImage* r_plane = nullptr;
        IplImage* g_plane = nullptr;
        IplImage* b_plane = nullptr;

        IplImage* edges   = nullptr;
        IplImage* borders = nullptr;
    } m_slices;

private:
    Ui::MainWindow *ui;
    QSize m_screenSize;
    int m_threshold = 50;
};

#endif // MAINWINDOW_H
