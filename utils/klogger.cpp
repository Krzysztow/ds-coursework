#include "klogger.h"

klogger::Verbosity klogger::_globalVerb = klogger::Normal;

klogger::klogger(Verbosity verb):
    _verb(verb)
{
}

std::string klogger::verbosityPrefix()
{
    switch (_verb) {
    case (Normal):  return "";
    case (Errors):  return "ERROR: ";
    case (Warnings):return "\tWARNING: ";
    case (Info):    return "\tINFO: ";
    case (Tests):   return "\tTEST: ";
    }

    return "";
}
