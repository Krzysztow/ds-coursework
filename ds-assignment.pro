TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG -= release
CONFIG += debug

SOURCES += \
    main.cpp \
    rrschedulertester.cpp \
    RoundRobinMedium/roundrobinmediumdispatcher.cpp \
    RoundRobinMedium/mediumdispatcher.cpp \
    RoundRobinMedium/mediumparticipantimpl.cpp

HEADERS += \
    mediumparticipant.h \
    rrschedulertester.h \
    RoundRobinMedium/mediummessage.h \
    RoundRobinMedium/roundrobinmediumdispatcher.h \
    RoundRobinMedium/mediumdispatcher.h \
    RoundRobinMedium/mediumparticipantimpl.h

LIBS += -lrt
