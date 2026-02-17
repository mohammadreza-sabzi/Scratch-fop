//
// Created by Domim on 2/18/2026.
//

#ifndef SCRATCH_FOP_UTILS_H
#define SCRATCH_FOP_UTILS_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

// تابع باز کردن پنجره انتخاب فایل (Open Dialog)
string open_file_dialog() {
#ifdef _WIN32
    OPENFILENAME ofn;
    char szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Scratch Files\0*.SCR\0Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        return string(ofn.lpstrFile);
    }
#endif
    return "";
}

// تابع باز کردن پنجره ذخیره فایل
std::string save_file_dialog() {
#ifdef _WIN32
    OPENFILENAME ofn;
    char szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Scratch Files\0*.SCR\0Text Files\0*.TXT\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }
#endif
    return "";
}

// ذخیره متن در فایل
void save_content_to_file(const std::string& filename, const std::string& content) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
        std::cout << "File saved successfully: " << filename << std::endl;
    } else {
        std::cerr << "Error saving file!" << std::endl;
    }
}

// خواندن متن از فایل
string read_content_from_file(const std::string& filename) {
    ifstream file(filename);
    string content = "";
    string line;
    if (file.is_open()) {
        while (getline(file, line)) {
            content += line + "\n";
        }
        file.close();
    }
    return content;
}

#endif
