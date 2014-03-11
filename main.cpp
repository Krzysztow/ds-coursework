#include <iostream>

#include "datafilereader.h"
#include "klogger.h"
#include "operation.h"
#include "mediumparticipantimpl.h"
#include "networkprinter.h"
#include "processobject.h"
#include "scheduledmediumdispatcher.h"

#include "RoundRobinScheduler/roundrobinscheduler.h"
#include "RoundRobinMedium/roundrobinmediumdispatcher.h"

#include "tests/tests.h"

int main(int argc, const char *argv[]) {
    Tests t;
//    t.test();

    klogger::setVerbosity(klogger::Tests);
    std::string fileName;
    if (2 == argc)
        fileName = std::string(argv[1]);
    else if (1 == argc)
        fileName = std::string("../ds-assignment/tests/operations.txt");
    else {
        std::cout << "Usage: ";
        std::cout << argv[0] << " <operations-file>" << std::endl;
        return 1;
    }

    DataFileReader reader;
    Operations ops;
    if (reader.createOperationsFromFile(fileName, &ops)) {
        RoundRobinScheduler scheduler;
        RoundRobinMediumDispatcher rrDispatcher;
        ScheduledMediumDispatcher mediumDispatcher(&rrDispatcher);

        scheduler.registerObject(&mediumDispatcher);
        MultiProcessesOperations mProcsOptions = ops.operations();
        MultiProcessesOperations::const_iterator mProcsIt = mProcsOptions.begin();
        for (; mProcsOptions.end() != mProcsIt; ++ mProcsIt) {
            MediumParticipantImpl *medParticipant = new MediumParticipantImpl(mProcsIt->first, &rrDispatcher);
            ProcessObject *process = new ProcessObject(medParticipant, mProcsOptions.size(), mProcsIt->first);
            process->buildPlan(&(mProcsIt->second));

            rrDispatcher.registerParticipant(medParticipant);
            scheduler.registerObject(process);
        }

        MediumParticipantImpl printerParticipant(255, &rrDispatcher);
        NetworkPrinter printer(&printerParticipant);

        scheduler.exec();
        //could make clean-up
    }
    else {
        klogger(klogger::Errors) << "Cannot parse the file " << fileName;
    }

    return 0;
}
