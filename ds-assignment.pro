TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG -= release
CONFIG += debug

SOURCES += main.cpp \
    RoundRobinMedium/roundrobinmessagescheduler.cpp \
    RoundRobinMedium/roundrobinmediumparticipant.cpp

HEADERS += \
    RoundRobinMedium/roundrobinmessagescheduler.h \
    RoundRobinMedium/roundrobinmediumparticipant.h \
    mediumparticipant.h

