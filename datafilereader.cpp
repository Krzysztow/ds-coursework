#include "datafilereader.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <cstdio>

#include "operation.h"

const std::string BeginTag("begin");
const std::string ProcessTag("process");
const std::string EndTag("end");
const std::string MutexTag("mutex");
const std::string SendTag("send");
const std::string RecvTag("recv");
const std::string PrintTag("print");

DataFileReader::DataFileReader()
{
}

size_t findNextCharPos(const std::string &str, size_t startFrom) {
    return str.find_first_not_of(" \r\n", startFrom);
}

bool parseProcessId(const std::string &str, unsigned int *procId) {
    int ret = sscanf(str.c_str(), "p%ud", procId);
    if (1 != ret) {
        std::cerr << "Cannot get process id" << std::endl;
        return false;
    }
    return true;
}

bool createProcessOperations(std::istream &s, std::list<Operation *> *processOperations) {
    std::string token;
    bool isInMutex = false;

    while (! s.eof()) {
        s >> token;
        if (0 == token.compare(SendTag) ||
                0 == token.compare(RecvTag)) {
            Operation::OperationType type = (0 == token.compare(SendTag) ? Operation::OT_Send : Operation::OT_Recv);
            s >> token;
            unsigned int procId;
            if (parseProcessId(token, &procId)) {
                s >> token;
                Operation *op = new SendOrRecvOperation(type, procId, token);
                processOperations->push_back(op);
            }
        }
        else if (0 == token.compare(PrintTag)) {
            s >> token;
            Operation *op = new PrintOperation(token);
            processOperations->push_back(op);
        }
        else if (0 == token.compare(BeginTag)) {
            s >> token;
            if (isInMutex) {
                std::cerr << "Cannot nest mutexes!" << std::endl;
                break;
            }
            else if (0 == token.compare(MutexTag)) {
                isInMutex = true;
                processOperations->push_back(new MutexOperation(Operation::OT_BeginMutex));
            }
            else {
                std::cerr << "Unexpected tag: " << token << std::endl;
                break;
            }
        }
        else if (0 == token.compare(EndTag)) {
            s >> token;
            if (0 == token.compare(ProcessTag)) {
                //we are done
                return true;
            }
            else if (0 == token.compare(MutexTag)) {
                if (! isInMutex) {
                    std::cerr << "Not expected end of mutex" << std::endl;
                    break;
                }
                processOperations->push_back(new MutexOperation(Operation::OT_EndMutex));
            }
            else {
                std::cerr << "Unexpected tag " << token << std::endl;
                break;
            }
        }
    }

    std::list<Operation *>::iterator it = processOperations->begin();
    for (; processOperations->end() != it; ++it) {
        delete *it;
    }

    return false;
}

bool DataFileReader::createOperationsFromFile(const std::string &filePath, Operations *operations)
{
    assert(0 != operations);

    std::ifstream f(filePath.c_str());
    if (!f.is_open()) {
        std::cerr << "Cannot open " << filePath << std::endl;
        return false;
    }

    std::map<unsigned int, std::list<Operation *> > *procsOperations = new std::map<unsigned int, std::list<Operation *> >();

    std::string token1, token2;
    std::list<Operation *> pOperations;

    bool errorOccured = false;
    while (true) {
        f >> token1 >> token2;
        if (f.eof()) {
            break;
        }

        if (0 == token1.compare(BeginTag) &&
                0 == token2.compare(ProcessTag)) {

            f >> token1;
            unsigned int procId;
            if (! parseProcessId(token1, &procId)) {
                errorOccured = true;
                break;
            }

            if (procsOperations->end() != procsOperations->find(procId)) {
                std::cerr << "Duplicated processes ids" << std::endl;
                errorOccured = true;
                break;
            }

            pOperations.clear();
            if (createProcessOperations(f, &pOperations)) {
                (*procsOperations)[procId] = pOperations;
            }
            else {
                std::cerr << "Cannot parse process operations" << std::endl;
                errorOccured = true;
                break;
            }
        }
        else {
            std::cerr << "Problem parsing begin process" << std::endl;
        }
    }

    if (errorOccured) {
        //clear all elements in processOperations
        Operations::destroyOperations(procsOperations);
        procsOperations = 0;
    }

    operations->setOperations(procsOperations);
    return (! errorOccured);
}

