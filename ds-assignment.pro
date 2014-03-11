TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG -= release
CONFIG += debug

QMAKE_CXXFLAGS += -std=c++0x

INCLUDEPATH += ./CommMedium \
    ./Scheduling \
    ./utils

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
    mutexhandler.cpp \
    tests/lamportclocktest.cpp \
    tests/tests.cpp \
    utils/klogger.cpp \
    networkprinter.cpp

HEADERS += \
    CommMedium/mediumparticipant.h \
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
    mutexhandler.h \
    tests/lamportclocktest.h \
    tests/tests.h \
    utils/klogger.h \
    networkprinter.h

LIBS += -lrt
