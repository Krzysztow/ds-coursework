TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG -= release
CONFIG += debug

SOURCES += \
    main.cpp \
    tests/rrschedulertester.cpp \
    RoundRobinMedium/roundrobinmediumdispatcher.cpp \
    RoundRobinMedium/mediumdispatcher.cpp \
    RoundRobinMedium/mediumparticipantimpl.cpp \
    roundrobinscheduler.cpp \
    processobject.cpp \
    scheduledmediumdispatcher.cpp \
    lamportclockhandler.cpp \
    datafilereader.cpp \
    opertions.cpp \
    tests/operationstester.cpp

HEADERS += \
    mediumparticipant.h \
    tests/rrschedulertester.h \
    RoundRobinMedium/mediummessage.h \
    RoundRobinMedium/roundrobinmediumdispatcher.h \
    RoundRobinMedium/mediumdispatcher.h \
    RoundRobinMedium/mediumparticipantimpl.h \
    scheduler.h \
    scheduledobject.h \
    roundrobinscheduler.h \
    processobject.h \
    scheduledmediumdispatcher.h \
    lamportclockhandler.h \
    datafilereader.h \
    opertions.h \
    tests/operationstester.h \
    operation.h

LIBS += -lrt
