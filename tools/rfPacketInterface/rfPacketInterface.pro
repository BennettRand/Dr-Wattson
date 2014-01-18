#-------------------------------------------------
#
# Project created by QtCreator 2013-12-23T19:33:45
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = rfPacketInterface
TEMPLATE = app


SOURCES += main.cpp\
        Interface.cpp \
    packettablemodel.cpp

HEADERS  += Interface.h \
    packettablemodel.h \
    ../../hardware/firmware/remoteMonitorFirmware/protocol.h \
    ../../hardware/firmware/baseStationFirmware/protocol.h

FORMS    += widget.ui
