QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = avr-cec-client
TEMPLATE = app


SOURCES += src/main.cpp \
    src/windowmain.cpp \
    src/dialogaction.cpp \
    src/usbcontroller.cpp

HEADERS  += inc/windowmain.h \
    inc/dialogaction.h \
    inc/usbcontroller.h

FORMS    += ui/windowmain.ui \
    ui/dialogaction.ui

INCLUDEPATH += inc
INCLUDEPATH += ..

RESOURCES += res/res.qrc

win32 {
    INCLUDEPATH += inc/windows
    LIBS += -L$$PWD/lib/windows -llibusb
}

unix {

}

release: DESTDIR = ./build/release
debug:   DESTDIR = ./build/debug

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui
