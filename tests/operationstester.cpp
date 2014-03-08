#include "operationstester.h"

#include <assert.h>
#include <list>
#include <map>
#include <iostream>

#include "datafilereader.h"
#include "operation.h"

OperationsTester::OperationsTester()
{
}

void OperationsTester::test()
{
    DataFileReader reader;

    Operations ops;
    if (! reader.createOperationsFromFile("/home/krzys/Documents/University/ds/ds-assignment/tests/operations.txt", &ops)) {
        assert(false);
    }

    const std::map<unsigned int, std::list<Operation *> > &procsOperations = ops.operations();
    std::map<unsigned int, std::list<Operation *> > testOperations;
    std::list<Operation *> pOps;

    pOps.clear();
    pOps.push_back(new MutexOperation(Operation::OT_BeginMutex));
    pOps.push_back(new PrintOperation("aaaa"));
    pOps.push_back(new PrintOperation("bbbb"));
    pOps.push_back(new PrintOperation("cccc"));
    pOps.push_back(new PrintOperation("dddd"));
    pOps.push_back(new PrintOperation("eeee"));
    pOps.push_back(new PrintOperation("ffff"));
    pOps.push_back(new MutexOperation(Operation::OT_EndMutex));
    pOps.push_back(new PrintOperation("gggg"));
    testOperations[1] = pOps;

    pOps.clear();
    pOps.push_back(new SendOrRecvOperation(Operation::OT_Recv, 4, "****"));
    pOps.push_back(new PrintOperation("xxxx"));
    pOps.push_back(new PrintOperation("yyyy"));
    testOperations[2] = pOps;

    pOps.clear();
    pOps.push_back(new PrintOperation("1111"));
    pOps.push_back(new PrintOperation("2222"));
    pOps.push_back(new PrintOperation("3333"));
    testOperations[3] = pOps;

    pOps.clear();
    pOps.push_back(new PrintOperation("&&&&"));
    pOps.push_back(new PrintOperation("$$$$"));
    pOps.push_back(new SendOrRecvOperation(Operation::OT_Send, 2, "****"));
    pOps.push_back(new PrintOperation("^^^^"));
    testOperations[4] = pOps;

    std::map<unsigned int, std::list<Operation *> >::const_iterator mainItProcs = procsOperations.begin();
    std::map<unsigned int, std::list<Operation *> >::const_iterator mainItTest = testOperations.begin();

    for (; procsOperations.end() != mainItProcs && testOperations.end() != mainItTest; ++mainItProcs, ++mainItTest) {
        std::list<Operation *>::const_iterator itProcs = mainItProcs->second.begin();
        std::list<Operation *>::const_iterator itTests = mainItTest->second.begin();

        for (; mainItProcs->second.end() != itProcs && mainItTest->second.end() != itTests; ++itProcs, ++itTests) {
            Operation *op1 = *itTests;
            Operation *op2 = *itProcs;
            assert(*op1 == *op2);
        }

        assert(mainItProcs->second.end() == itProcs);
        assert(mainItTest->second.end() == itTests);
    }

    assert(procsOperations.end() == mainItProcs);
    assert(testOperations.end() == mainItTest);
}
