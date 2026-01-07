# -------------------------------------------------
# Project created by QtCreator 2010-03-29T13:28:53
# -------------------------------------------------
QT += core widgets gui xml svg qml charts
TARGET = TSVview
TEMPLATE = app
RC_FILE	 = icon.rc

INCLUDEPATH += Base
INCLUDEPATH += FileIO
INCLUDEPATH += Plots
INCLUDEPATH += Statistics
INCLUDEPATH += Signal

#include zlib library
LIBS += -lz

SOURCES += \
    GrepDialog.cpp \
    Main.cpp \
    MainWindow.cpp \
    Base/Parameters.cpp \
    Base/CustomExceptions.cpp \
    Base/DataSet.cpp \
    Base/ParameterEditor.cpp \
    FileIO/TextImportPreview.cpp \
    Plots/BasePlot.cpp \
    Plots/ScatterPlot.cpp \
    Plots/HistogramPlot.cpp \
    Plots/DataPlot.cpp \
    Base/DataGrid.cpp \
    Plots/BoxPlot.cpp \
    Plots/MyChartView.cpp \
    Signal/Smoothing.cpp \
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
    AddColumnDialog.cpp \
    TextItemEditDialog.cpp
    
HEADERS += \
    GrepDialog.h \
    MainWindow.h \
    Base/Parameters.h \
    Base/CustomExceptions.h \
    Base/DataSet.h \
    Base/ParameterEditor.h \
    FileIO/TextImportPreview.h \
    Plots/BasePlot.h \
    Plots/ScatterPlot.h \
    Plots/HistogramPlot.h \
    Plots/DataPlot.h \
    Base/DataGrid.h \
    Plots/BoxPlot.h \
    Plots/MyChartView.h \
    Signal/Smoothing.h \
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
    AddColumnDialog.h \
    TextItemEditDialog.h

FORMS += \
    GrepDialog.ui \
    MainWindow.ui \
    FileIO/TextImportPreview.ui \
    FileIO/FilePreview.ui \
    GoToDockWidget.ui \
    FindDockWidget.ui \
    FilterWidget.ui \
    FilterDialog.ui \
    Base/ReplacementDialog.ui \
    Base/MergeDialog.ui \
    Statistics/StatisticsSummaryWidget.ui \
    AddColumnDialog.ui \
    TextItemEditDialog.ui

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

DISTFILES += \
    ../../todo.txt
