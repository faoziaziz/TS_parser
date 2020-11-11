QT += core sql
QT -= gui

CONFIG += c++11

TARGET = parser
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    parsing.cpp \
    aes.c \
    tsdata.cpp \
    parser.cpp

HEADERS += \
    parsing.h \
    aes.h \
    tsdata.h \
    config.h \
    parser.h

DISTFILES += \
    readme.txt
