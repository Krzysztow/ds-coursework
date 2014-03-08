#ifndef OPERTIONS_H
#define OPERTIONS_H

#include <list>
#include <map>

class Operation;

typedef std::list<Operation *> ProcessOperations;
typedef std::map<unsigned int, ProcessOperations> MultiProcessesOperations;

class Operations {
public:
    Operations();
    ~Operations();

    /**
     *pass ownership of operations. When deleted, operations will be destroyed as well.
     */
    void setOperations(MultiProcessesOperations *operations);
    const MultiProcessesOperations &operations();

    static void destroyOperations(MultiProcessesOperations *operations);

private:
    //don't define -> prevent from copying
    Operations(const Operations &other);

private:
    std::map<unsigned int, std::list<Operation *> > *_operations;
};


#endif // OPERTIONS_H
