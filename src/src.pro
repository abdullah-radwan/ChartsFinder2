#-------------------------------------------------
#
# Project created by QtCreator 2019-03-05T12:43:32
#
#-------------------------------------------------

QT       += core gui widgets network

TARGET = ChartsFinder2
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    downloader.cpp \
    settingswindow.cpp \
    config.cpp \
    adddialog.cpp

HEADERS += \
        mainwindow.h \
    downloader.h \
    settingswindow.h \
    config.h \
    adddialog.h

FORMS += \
        mainwindow.ui \
    settingswindow.ui \
    adddialog.ui
		
RC_ICONS = ../res/icon.ico

RESOURCES += res.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/QGumboParser/release/ -lQGumboParser
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/QGumboParser/debug/ -lQGumboParser
else:unix: LIBS += -L$$OUT_PWD/../lib/QGumboParser/ -lQGumboParser

INCLUDEPATH += $$PWD/../lib/QGumboParser
DEPENDPATH += $$PWD/../lib/QGumboParser

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/QGumboParser/release/libQGumboParser.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/QGumboParser/debug/libQGumboParser.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/QGumboParser/release/QGumboParser.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/QGumboParser/debug/QGumboParser.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../lib/QGumboParser/libQGumboParser.a
