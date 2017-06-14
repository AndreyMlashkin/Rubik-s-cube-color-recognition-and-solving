#-------------------------------------------------
#
# Project created by QtCreator 2017-06-10T12:52:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = open_cv_try
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += "C:\Users\Andrey\Downloads\opencv\build-sources-Desktop_Qt_5_8_0_MinGW_32bit-Debug\bin" \
               "C:\Users\Andrey\Downloads\opencv\build\include"

LIBS +=       -LC:\Users\Andrey\Downloads\opencv\build-sources-Desktop_Qt_5_8_0_MinGW_32bit-Debug\bin \
-llibopencv_core320d        \
-llibopencv_calib3d320d     \
-llibopencv_imgcodecs320d   \
-llibopencv_imgproc320d     \
-llibopencv_features2d320d  \
-llibopencv_flann320d       \
-llibopencv_highgui320d     \
-llibopencv_ml320d          \
-llibopencv_objdetect320d

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
