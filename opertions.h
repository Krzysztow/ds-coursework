#ifndef OPERTIONS_H
#define OPERTIONS_H

#include <list>
#include <map>

class Operation;

class Operations {
public:
    Operations();
    ~Operations();

    /**
     *pass ownership of operations. When deleted, operations will be destroyed as well.
     */
    void setOperations(std::map<unsigned int, std::list<Operation *> > *operations);
    const std::map<unsigned int, std::list<Operation *> > &operations();

    static void destroyOperations(std::map<unsigned int, std::list<Operation *> > *operations);

private:
    //don't define -> prevent from copying
    Operations(const Operations &other);

private:
    std::map<unsigned int, std::list<Operation *> > *_operations;
};


#endif // OPERTIONS_H
