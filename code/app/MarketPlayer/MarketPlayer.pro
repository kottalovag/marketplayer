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
INCLUDEPATH += plot

SOURCES += main.cpp\
        mainwindow.cpp \
    model/model.cpp \
    plot/qcustomplot.cpp \
    plot/datatimeplot.cpp \
    plot/plotutils.cpp \
    plot/datatimeplotwithpercentage.cpp \
    plot/distributionplot.cpp \
    plot/plot.cpp \
    model/modelutils.cpp \
    simulationcase.cpp \
    colormanager.cpp \
    casenamemanager.cpp \
    model/strategy.cpp \
    strategymapper.cpp

HEADERS  += mainwindow.h \
    model/model.h \
    plot/qcustomplot.h \
    plot/datatimeplot.h \
    plot/plotutils.h \
    plot/datatimeplotwithpercentage.h \
    plot/distributionplot.h \
    plot/plot.h \
    model/modelutils.h \
    simulationcase.h \
    colormanager.h \
    casenamemanager.h \
    model/strategy.h \
    strategymapper.h

FORMS    += mainwindow.ui
