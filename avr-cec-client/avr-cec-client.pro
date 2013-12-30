QT += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -Wno-unused-parameter

TARGET = avr-cec-client
TEMPLATE = app


SOURCES += src/main.cpp \
    src/windowmain.cpp \
    src/tabactions.cpp \
    src/tabconnection.cpp \
    src/tabsniffer.cpp \
    src/tabdebug.cpp \
    src/tabsettings.cpp \
    src/actions.cpp

HEADERS += inc/windowmain.h \
    inc/tabactions.h \
    inc/tabconnection.h \
    inc/tabsniffer.h \
    inc/tabdebug.h \
    inc/tabsettings.h \
    inc/actions.h

FORMS += ui/windowmain.ui

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
