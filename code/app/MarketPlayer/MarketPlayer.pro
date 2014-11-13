#-------------------------------------------------
#
# Project created by QtCreator 2014-08-17T00:02:09
#
#-------------------------------------------------

CONFIG += c++11
QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MarketPlayer
TEMPLATE = app

INCLUDEPATH += model

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    model/model.cpp \
    datatimeplot.cpp \
    plotutils.cpp \
    datatimeplotwithpercentage.cpp \
    distributionplot.cpp \
    plot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    model/model.h \
    datatimeplot.h \
    plotutils.h \
    datatimeplotwithpercentage.h \
    distributionplot.h \
    plot.h

FORMS    += mainwindow.ui
