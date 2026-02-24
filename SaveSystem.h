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

class SaveSystem {
public:
    static void saveToFile(const std::string& filename, const std::vector<BlockData>& blocks) {
        std::ofstream file(filename);
        if (file.is_open()) {
            for (const auto& b : blocks) {
                file << b.type << "," << b.opcode << "," << b.value << "\n";
            }
            file.close();
        }
    }

    static std::vector<BlockData> loadFromFile(const std::string& filename) {
        std::vector<BlockData> blocks;
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string item;
            BlockData b;
            
            if (std::getline(ss, item, ',')) b.type = std::stoi(item);
            if (std::getline(ss, item, ',')) b.opcode = item;
            if (std::getline(ss, item, ',')) b.value = item;
            
            blocks.push_back(b);
        }
        return blocks;
    }
};

#endif