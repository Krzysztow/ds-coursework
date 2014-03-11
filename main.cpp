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

    klogger::setVerbosity(klogger::Normal);
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
    //read input file
    if (reader.createOperationsFromFile(fileName, &ops)) {
        //create all processes
        RoundRobinScheduler scheduler;
        RoundRobinMediumDispatcher rrDispatcher;
        ScheduledMediumDispatcher mediumDispatcher(&rrDispatcher);

        //medium dispatcher is also a process to be scheduled
        scheduler.registerObject(&mediumDispatcher);
        MultiProcessesOperations mProcsOptions = ops.operations();
        MultiProcessesOperations::const_iterator mProcsIt = mProcsOptions.begin();

        //create processes according to the configuration in the input file
        for (; mProcsOptions.end() != mProcsIt; ++ mProcsIt) {
            MediumParticipantImpl *medParticipant = new MediumParticipantImpl(mProcsIt->first, &rrDispatcher);
            ProcessObject *process = new ProcessObject(medParticipant, mProcsOptions.size(), mProcsIt->first);
            //build execution plan for each process
            process->buildPlan(&(mProcsIt->second));

            rrDispatcher.registerParticipant(medParticipant);
            scheduler.registerObject(process);
        }

        //add network printer
        MediumParticipantImpl printerParticipant(255, &rrDispatcher);
        NetworkPrinter printer(&printerParticipant);

        //start  simulation
        scheduler.exec();

        //todo: clean-up
    }
    else {
        std::cerr << "Cannot parse the file " << fileName << std::endl;
    }

    return 0;
}
