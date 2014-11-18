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
INCLUDEPATH += appstate

SOURCES += main.cpp\
        mainwindow.cpp \
    model/model.cpp \
    plot/qcustomplot.cpp \
    plot/datatimeplot.cpp \
    plot/plotutils.cpp \
    plot/datatimeratioplot.cpp \
    plot/distributionplot.cpp \
    plot/plot.cpp \
    model/modelutils.cpp \
    simulationcase.cpp \
    colormanager.cpp \
    casenamemanager.cpp \
    model/strategy.cpp \
    strategymapper.cpp \
    plot/plottablebundle.cpp \
    plot/datatimeplottablebundle.cpp \
    plot/distributionplottablebundle.cpp \
    plot/bundlehelper.cpp \
    casemanager.cpp \
    plot/datatimeratioplottablebundle.cpp \
    appstate/appstate.cpp \
    appstate/appwaitingforsimulationloaded.cpp \
    appstate/appinsimulationmode.cpp \
    appstate/appincomparisonmode.cpp

HEADERS  += mainwindow.h \
    model/model.h \
    plot/qcustomplot.h \
    plot/datatimeplot.h \
    plot/plotutils.h \
    plot/datatimeratioplot.h \
    plot/distributionplot.h \
    plot/plot.h \
    model/modelutils.h \
    simulationcase.h \
    colormanager.h \
    casenamemanager.h \
    model/strategy.h \
    strategymapper.h \
    plot/plottablebundle.h \
    plot/datatimeplottablebundle.h \
    plot/distributionplottablebundle.h \
    plot/bundlehelper.h \
    casemanager.h \
    plot/datatimeratioplottablebundle.h \
    appstate/appstate.h \
    appstate/appwaitingforsimulationloaded.h \
    appstate/appinsimulationmode.h \
    appstate/appincomparisonmode.h \
    appstate/apphavingsimulationloaded.h

FORMS    += mainwindow.ui
