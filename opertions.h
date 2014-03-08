#ifndef OPERTIONS_H
#define OPERTIONS_H

#include <assert.h>
#include <string>

class Operation {
public:
    enum OperationType {
        OT_Send,
        OT_Recv,
        OT_Print,
        OT_BeginMutex,
        OT_EndMutex
    };

    Operation(OperationType type):
        _type(type)
    {}

    //define to be sure class is virtual and no memory leak occurs
    virtual ~Operation() {}

    OperationType type() const {
        return _type;
    }

    virtual bool operator==(const Operation &other) const = 0;

private:
    OperationType _type;
};

class SendOrRecvOperation:
        public Operation {
public:
    SendOrRecvOperation(OperationType operation, unsigned int destProcId, const std::string &message):
        Operation(operation),
        _destProcId(destProcId),
        _msg(message)
    {
        assert(OT_Send == operation ||
               OT_Recv == operation);
    }

    unsigned int destProcId() const {
        return _destProcId;
    }

    const std::string &message() const {
        return _msg;
    }

    virtual bool operator==(const Operation &other) const {
        if (OT_Send == other.type() || OT_Recv == other.type()) {
            const SendOrRecvOperation *srOp = dynamic_cast<const SendOrRecvOperation*>(&other);
            return (0 == message().compare(srOp->message()) && destProcId() == srOp->destProcId());
        }
        else
            return false;
    }

private:
    unsigned int _destProcId;
    std::string _msg;
};

class PrintOperation:
        public Operation {
public:
    PrintOperation(const std::string &message):
        Operation(OT_Print),
        _msg(message)
    {}

    const std::string &message() const {
        return _msg;
    }

    virtual bool operator==(const Operation &other) const {
        if (OT_Print == other.type()) {
            const PrintOperation *op = dynamic_cast<const PrintOperation*>(&other);
            return (0 == message().compare(op->message()));
        }
        else
            return false;
    }

private:
    std::string _msg;
};

class MutexOperation:
        public Operation {
public:
    MutexOperation(OperationType operation):
        Operation(operation) {
        assert(Operation::OT_BeginMutex == operation ||
               OT_EndMutex == operation);
    }

    virtual bool operator==(const Operation &other) const {
        return (OT_BeginMutex == other.type() || OT_EndMutex == other.type());
    }
};

#endif // OPERTIONS_H
