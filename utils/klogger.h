#ifndef KLOGGER_H
#define KLOGGER_H

#include <iostream>
#include <sstream>

class klogger
{
public:
    class end {
    };

public:
    enum Verbosity {
        Normal,
        Errors,
        Warnings,
        Info,
        Tests
    };

    klogger(Verbosity verb = Normal);

    template <class T>
    klogger &operator<<(const T &t) {
        if (_verb <= _globalVerb)
            _stream << t;
        return *this;
    }

    std::string verbosityPrefix();

    klogger &operator<<(const klogger::end &end) {
        (void)end;
        if (_verb <= _globalVerb) {
            if (Errors == _verb)
                std::cerr << verbosityPrefix() << _stream.str() << std::endl;
            else
                std::cout << verbosityPrefix() << _stream.str() << std::endl;
        }
        return *this;
    }

    static void setVerbosity(Verbosity globVerb) {
        _globalVerb = globVerb;
    }

private:
    static Verbosity _globalVerb;
    Verbosity _verb;
    std::stringstream _stream;
};
#endif // KLOGGER_H
