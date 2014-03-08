#ifndef DATAFILEREADER_H
#define DATAFILEREADER_H

#include <string>
#include <map>
#include <list>

#include "opertions.h"

class DataFileReader
{
public:
    DataFileReader();
    bool createOperationsFromFile(const std::string &filePath, Operations *operations);
};

#endif // DATAFILEREADER_H
