#ifndef OPERATORLOGIC_H
#define OPERATORLOGIC_H
#include <string>
#include <cmath>
#include <algorithm>
using namespace std;

inline double calculate(double a, double b, string op) {
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "*") return a * b;
    if (op == "/") return (b != 0) ? a / b : 0;
    if (op == "mod") return fmod(a, b);
    return 0;
}

inline bool compare(double a, double b, string op) {
    if (op == ">") return a > b;
    if (op == "<") return a < b;
    if (op == "=") return a == b;
    return false;
}

inline bool logic(bool a, bool b, string op) {
    if (op == "and") return a && b;
    if (op == "or") return a || b;
    return false;
}

inline bool logicNot(bool a) {
    return !a;
}

inline string join(string a, string b) {
    return a + b;
}

inline string letterOf(string str, int index) {
    index -= 1;
    if (index >= 0 && index < (int)str.length()) {
        return string(1, str[index]);
    }
    return "";
}

inline int lengthOf(string str) {
    return (int)str.length();
}

inline bool contains(string str, string substr) {
    return str.find(substr) != string::npos;
}

#endif