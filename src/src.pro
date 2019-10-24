#-------------------------------------------------
#
# Project created by QtCreator 2019-03-05T12:43:32
#
#-------------------------------------------------

QT       += core gui widgets autoupdaterwidgets

TARGET = ChartsFinder2
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

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

RESOURCES += res.qrc

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib

win32:RC_ICONS = ../res/icons/icon.ico

win32: LIBS += $$PWD/../lib/curl/libcurl.dll.a
else: LIBS += -lcurl

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/QGumboParser/release/ -lQGumboParser
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/QGumboParser/debug/ -lQGumboParser
else:unix: LIBS += -L$$OUT_PWD/../lib/QGumboParser/ -lQGumboParser

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/QGumboParser/release/libQGumboParser.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/QGumboParser/debug/libQGumboParser.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/QGumboParser/release/QGumboParser.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/QGumboParser/debug/QGumboParser.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../lib/QGumboParser/libQGumboParser.a
