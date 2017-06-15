#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showFullScreenImage(const QFileInfo& _file);
    void showProcessedImages(const QFileInfo& _file);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
