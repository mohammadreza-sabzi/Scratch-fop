#ifndef OPERATORLOGIC_H
#define OPERATORLOGIC_H

#include <string>
#include <cmath>
#include <algorithm>

class OperatorLogic {
public:
    static double calculate(double a, double b, std::string op) {
        if (op == "+") return a + b;
        if (op == "-") return a - b;
        if (op == "*") return a * b;
        if (op == "/") return (b != 0) ? a / b : 0;
        if (op == "mod") return fmod(a, b);
        return 0;
    }

    static bool compare(double a, double b, std::string op) {
        if (op == ">") return a > b;
        if (op == "<") return a < b;
        if (op == "=") return a == b;
        return false;
    }

    static bool logic(bool a, bool b, std::string op) {
        if (op == "and") return a && b;
        if (op == "or") return a || b;
        return false;
    }

    static bool logicNot(bool a) {
        return !a;
    }

    static std::string join(std::string a, std::string b) {
        return a + b;
    }

    static std::string letterOf(std::string str, int index) {
        if (index >= 0 && index < str.length()) {
            return std::string(1, str[index]);
        }
        return "";
    }

    static int lengthOf(std::string str) {
        return str.length();
    }

    static bool contains(std::string str, std::string substr) {
        return str.find(substr) != std::string::npos;
    }
};

#endif