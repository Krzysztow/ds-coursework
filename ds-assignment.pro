TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG -= release
CONFIG += debug

SOURCES += \
    RoundRobinMedium/roundrobinmessagescheduler.cpp \
    RoundRobinMedium/roundrobinmediumparticipant.cpp \
    main.cpp \
    rrschedulertester.cpp

HEADERS += \
    RoundRobinMedium/roundrobinmessagescheduler.h \
    RoundRobinMedium/roundrobinmediumparticipant.h \
    mediumparticipant.h \
    rrschedulertester.h

