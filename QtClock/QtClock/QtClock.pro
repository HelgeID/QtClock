QT       += core gui\
            xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtClock
TEMPLATE = app


SOURCES += main.cpp\
        clockwidget.cpp

HEADERS  += clockwidget.h

FORMS    += clockwidget.ui

RESOURCES += \
    reswidget.qrc
