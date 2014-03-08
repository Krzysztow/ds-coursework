#include "opertions.h"

#include "operation.h"

Operations::Operations()
{
}

void Operations::destroyOperations(MultiProcessesOperations *operations)
{
    if (0 == operations)
        return;

    std::map<unsigned int, std::list<Operation *> >::iterator mainIt = operations->begin();
    for (; operations->end() != mainIt; ++mainIt) {
        std::list<Operation *> &l = mainIt->second;
        std::list<Operation *>::iterator it = l.begin();
        for (; l.end() != it; ++it) {
            delete (*it);
        }
    }

    delete operations;
}

Operations::~Operations()
{
    destroyOperations(_operations);
    _operations = 0;
}

void Operations::setOperations(MultiProcessesOperations *operations)
{
    _operations = operations;
}

const MultiProcessesOperations &Operations::operations()
{
    const std::map<unsigned int, std::list<Operation *> > *ptr = _operations;
    return *ptr;
}

