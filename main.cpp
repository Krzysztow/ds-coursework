#include <iostream>

#include "datafilereader.h"
#include "operation.h"
#include "tests/rrschedulertester.h"
#include "tests/operationstester.h"

#include "RoundRobinScheduler/roundrobinscheduler.h"
#include "scheduledmediumdispatcher.h"
#include "RoundRobinMedium/roundrobinmediumdispatcher.h"
#include "processobject.h"
#include "mediumparticipantimpl.h"

int main(int argc, const char *argv[]) {
    RRSchedulerTester tester;
    tester.test();

    OperationsTester tester2;
    tester2.test();

    std::string fileName;
    if (2 == argc)
        fileName = std::string(argv[1]);
    else if (1 == argc)
        fileName = std::string("../ds-assignment/tests/operations.txt");
    else {
        std::cerr << "Usage: ";
        std::cerr << argv[0] << " <operations-file>" << std::endl;
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
            ProcessObject *process = new ProcessObject(medParticipant);
            process->setOperations(&(mProcsIt->second));

            rrDispatcher.registerParticipant(medParticipant);
            scheduler.registerObject(process);
        }

        scheduler.exec();
        //could make clean-up
    }
    else {
        std::cerr << "Cannot parse the file " << fileName;
    }

    return 0;
}
