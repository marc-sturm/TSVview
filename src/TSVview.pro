TEMPLATE = subdirs
CONFIG += console

#Library targets and depdendencies
SUBDIRS = cppCORE\
        cppGUI\
        TSVview

cppGUI.depends = cppCORE
TSVview.depends = cppCORE cppGUI
