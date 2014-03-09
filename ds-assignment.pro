TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG -= release
CONFIG += debug

INCLUDEPATH += ./CommMedium \
    ./Scheduling

SOURCES += \
    main.cpp \
    tests/rrschedulertester.cpp \
    CommMedium/RoundRobinMedium/roundrobinmediumdispatcher.cpp \
    CommMedium/mediumdispatcher.cpp \
    CommMedium/mediumparticipantimpl.cpp \
    Scheduling/RoundRobinScheduler/roundrobinscheduler.cpp \
    processobject.cpp \
    Scheduling/scheduledmediumdispatcher.cpp \
    lamportclockhandler.cpp \
    datafilereader.cpp \
    opertions.cpp \
    tests/operationstester.cpp \
    lamportclock.cpp \
    mutexhandler.cpp

HEADERS += \
    mediumparticipant.h \
    tests/rrschedulertester.h \
    CommMedium/mediummessage.h \
    CommMedium/RoundRobinMedium/roundrobinmediumdispatcher.h \
    CommMedium/mediumdispatcher.h \
    CommMedium/mediumparticipantimpl.h \
    CommMedium/mediumparticipant.h \
    Scheduling/scheduler.h \
    Scheduling/scheduledobject.h \
    Scheduling/RoundRobinScheduler/roundrobinscheduler.h \
    processobject.h \
    Scheduling/scheduledmediumdispatcher.h \
    lamportclockhandler.h \
    datafilereader.h \
    opertions.h \
    tests/operationstester.h \
    operation.h \
    applicationmessages.h \
    lamportclock.h \
    mutexhandler.h

LIBS += -lrt
