QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = avr-cec-client
TEMPLATE = app


SOURCES += src/main.cpp \
    src/windowmain.cpp \
    src/dialogaction.cpp

HEADERS  += inc/windowmain.h \
    inc/dialogaction.h

FORMS    += ui/windowmain.ui \
    ui/dialogaction.ui

INCLUDEPATH += inc
INCLUDEPATH += ..
INCLUDEPATH += ../avr-cec-lib/inc

LIBS += -L$$PWD/../avr-cec-lib/bin
LIBS += -lavrcec -lusb -lpthread

RESOURCES += res/res.qrc

win32 {
    INCLUDEPATH += inc/windows
    LIBS += -L$$PWD/lib/windows
}

unix {
    INCLUDEPATH += inc/linux
    LIBS += -L$$PWD/lib/linux
}

release: DESTDIR = ./build/release
debug:   DESTDIR = ./build/debug

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui
