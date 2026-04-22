TEMPLATE = app
TARGET = graph-constructor
QT += core gui quick svg opengl
CONFIG += c++20

SOURCES += main.cpp \
           src/graphcanvas.cpp \
           src/tikz_exporter.cpp \
           src/graphexamples.cpp \
           src/graph_serializer.cpp \
           src/history_manager.cpp \
           src/graph_renderer.cpp

HEADERS += include/graphcanvas.h \
           include/tikz_exporter.h \
           include/graphexamples.h \
           include/components.h \
           include/graph_serializer.h \
           include/history_manager.h \
           include/graph_renderer.h

RESOURCES += resources/resources.qrc

MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/rcc
UI_DIR      = build/ui

INCLUDEPATH += include
