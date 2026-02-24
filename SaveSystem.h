#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

struct BlockData {
    int type;
    string opcode;
    string value;
};

<<<<<<< HEAD
inline void saveToFile(const string& filename, const vector<BlockData>& blocks) {
    ofstream file(filename);
    if (file.is_open()) {
        for (const auto& b : blocks) {
            file << b.type << "," << b.opcode << "," << b.value << "\n";
        }
        file.close();
    }
}

inline vector<BlockData> loadFromFile(const string& filename) {
    vector<BlockData> blocks;
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string item;
        BlockData b;

        if (getline(ss, item, ',')) b.type = stoi(item);
        if (getline(ss, item, ',')) b.opcode = item;
        if (getline(ss, item)) b.value = item;

        blocks.push_back(b);
    }
    return blocks;
}


#endif