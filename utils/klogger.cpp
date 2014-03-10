#include "klogger.h"

klogger::Verbosity klogger::_globalVerb = klogger::Always;

klogger::klogger(Verbosity verb):
    _verb(verb)
{
}
