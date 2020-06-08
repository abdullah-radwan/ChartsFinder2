#-------------------------------------------------
#
# Project created by QtCreator 2019-03-05T12:43:32
#
#-------------------------------------------------

TEMPLATE = app

TARGET = ChartsFinder2

VERSION = 2.3.1

QT += core gui widgets autoupdaterwidgets

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17

SOURCES += \
    configeditor.cpp \
    main.cpp \
    mainwindow.cpp \
    downloader.cpp \
    adddialog.cpp \
    settingsdialog.cpp

HEADERS += \
    configeditor.h \
    mainwindow.h \
    downloader.h \
    adddialog.h \
    settingsdialog.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    adddialog.ui

RESOURCES += ../res/res.qrc

INCLUDEPATH += ../lib/include

LIBS += -L$$PWD/../lib -lQGumboParser

win32: LIBS += -L$$PWD/../lib -lcurl.dll
else: LIBS += -lcurl

win32:RC_ICONS = ../res/icon.ico
