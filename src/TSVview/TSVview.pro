# -------------------------------------------------
# Project created by QtCreator 2010-03-29T13:28:53
# -------------------------------------------------
QT += core widgets gui xml svg qml
TARGET = TSVview
TEMPLATE = app
RC_FILE	 = icon.rc

CONFIG += c++11

INCLUDEPATH += Base
INCLUDEPATH += FileIO
INCLUDEPATH += Plots
INCLUDEPATH += Statistics
INCLUDEPATH += Signal

SOURCES += \
    Main.cpp \
    MainWindow.cpp \
    Base/Parameters.cpp \
    Base/CustomExceptions.cpp \
    Base/DataSet.cpp \
    Base/ParameterEditor.cpp \
    FileIO/XMLImportPreview.cpp \
    FileIO/XMLFile.cpp \
    FileIO/TextImportPreview.cpp \
    FileIO/TextFile.cpp \
    Plots/BasePlot.cpp \
    Plots/ScatterPlot.cpp \
    Plots/HistogramPlot.cpp \
    Plots/DataPlot.cpp \
    Base/DataGrid.cpp \
    Plots/BoxPlot.cpp \
    Signal/Smoothing.cpp \
    FileIO/ZXVFile.cpp \
    Base/BaseColumn.cpp \
    Base/NumericColumn.cpp \
    Base/StringColumn.cpp \
    FileIO/FilePreview.cpp \
    GoToDockWidget.cpp \
    FindDockWidget.cpp \
    FilterWidget.cpp \
    Base/Filter.cpp \
    FilterDialog.cpp \
    Base/ReplacementDialog.cpp \
    Base/MergeDialog.cpp \
    Statistics/StatisticsSummary.cpp \
    Statistics/StatisticsSummaryWidget.cpp \
    AddColumnDialog.cpp
    
HEADERS += \
    MainWindow.h \
    Base/Parameters.h \
    Base/CustomExceptions.h \
    Base/DataSet.h \
    Base/ParameterEditor.h \
    FileIO/XMLImportPreview.h \
    FileIO/XMLFile.h \
    FileIO/TextImportPreview.h \
    FileIO/TextFile.h \
    Plots/BasePlot.h \
    Plots/ScatterPlot.h \
    Plots/HistogramPlot.h \
    Plots/DataPlot.h \
    Base/DataGrid.h \
    Plots/BoxPlot.h \
    Signal/Smoothing.h \
    FileIO/ZXVFile.h \
    Base/BaseColumn.h \
    Base/NumericColumn.h \
    Base/StringColumn.h \
    FileIO/FilePreview.h \
    GoToDockWidget.h \
    FindDockWidget.h \
    FilterWidget.h \
    Base/Filter.h \ 
    FilterDialog.h \
    Base/ReplacementDialog.h \
    Base/MergeDialog.h \
    Statistics/StatisticsSummary.h \
    Statistics/StatisticsSummaryWidget.h \
    AddColumnDialog.h

FORMS += \
    MainWindow.ui \
    FileIO/XMLImportPreview.ui \
    FileIO/TextImportPreview.ui \
    FileIO/FilePreview.ui \
    GoToDockWidget.ui \
    FindDockWidget.ui \
    FilterWidget.ui \
    FilterDialog.ui \
    Base/ReplacementDialog.ui \
    Base/MergeDialog.ui \
    Statistics/StatisticsSummaryWidget.ui \
    AddColumnDialog.ui

RESOURCES += \
    Resources.qrc

#include cppCORE library
INCLUDEPATH += $$PWD/../cppCORE
LIBS += -L$$PWD/../../bin -lcppCORE

#include cppGUI library
INCLUDEPATH += $$PWD/../cppGUI
LIBS += -L$$PWD/../../bin -lcppGUI

#copy EXE to bin folder
DESTDIR = $$PWD/../../bin

# -------------------------------------------------
# include QWT
# -------------------------------------------------
INCLUDEPATH +=$$PWD\\..\\qwt\\include
LIBS += -L$$PWD\\..\\qwt\\lib -lqwt














