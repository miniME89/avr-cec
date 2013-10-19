QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = avr-cec-client
TEMPLATE = app


SOURCES += src/main.cpp\
        src/windowmain.cpp

HEADERS  += includes/windowmain.h

FORMS    += ui/windowmain.ui

INCLUDEPATH = includes/

release: DESTDIR = ./build/release
debug:   DESTDIR = ./build/debug

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui
