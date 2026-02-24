#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

struct BlockData {
    int type;
    std::string opcode;
    std::string value;
};


#endif