TEMPLATE = app
TARGET = graph-constructor
QT += core gui quick svg opengl
CONFIG += c++20

SOURCES += main.cpp \
           src/graphcanvas.cpp \
           src/tikz_exporter.cpp

MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

HEADERS += include/graphcanvas.h \
           include/tikz_exporter.h

RESOURCES += resources/resources.qrc

INCLUDEPATH += include
