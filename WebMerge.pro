#-------------------------------------------------
#
# Project created by QtCreator 2012-06-10T22:19:00
#
#-------------------------------------------------

QT       += core gui webkit network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WebMerge
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x
SOURCES += main.cpp\
        webview.cpp

HEADERS  += webview.h
