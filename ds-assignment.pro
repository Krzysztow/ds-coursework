TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    RoundRobinMedium/roundrobinmessagescheduler.cpp \
    RoundRobinMedium/roundrobinmediumparticipant.cpp

HEADERS += \
    RoundRobinMedium/roundrobinmessagescheduler.h \
    RoundRobinMedium/roundrobinmediumparticipant.h \
    mediumparticipant.h

