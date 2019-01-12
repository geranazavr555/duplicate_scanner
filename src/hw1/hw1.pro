#-------------------------------------------------
#
# Project created by QtCreator 2018-12-08T02:05:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hw1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../directory_scanner/file_comparator.cpp \
    ../directory_scanner/scanner.cpp

HEADERS  += mainwindow.h \
    ../directory_scanner/exceptions.h \
    ../directory_scanner/file_comparator.h \
    ../directory_scanner/scanner.h

FORMS    += mainwindow.ui
