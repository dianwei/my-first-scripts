#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

#include "big_complex.hpp"
#include "calculator.hpp"
#include "format.hpp"
#include "scanner.hpp"

namespace ce = complex_eval;

namespace {

ce::FormatConfig gFormat;

std::string trim(const std::string& s) {
    const std::string ws = " \t\n\r";
    const std::size_t begin = s.find_first_not_of(ws);
    if (begin == std::string::npos) {
        return "";
    }
    const std::size_t end = s.find_last_not_of(ws);
    return s.substr(begin, end - begin + 1);
}

void printHelp() {
    std::cout
        << "命令:\n"
        << "  help              显示帮助\n"
        << "  format sci        使用科学计数法输出\n"
        << "  format fixed      使用普通十进制输出（整数不带小数）\n"
        << "  precision N       设置小数位数（sci 为小数点后 N 位；fixed 为小数点后 N 位）\n"
        << "  quit / exit       退出\n"
        << "表达式:\n"
        << "  支持 + - * / ，赋值 = ，函数 con(z) 共轭、mod(z) 模长\n"
        << "  支持复数字面量如 3.14、.5、1e10、2.5i、-i、i\n";
}

bool handleCommand(const std::string& cmd) {
    if (cmd == "help") {
        printHelp();
        return true;
    }
    if (cmd == "format sci") {
        gFormat.sci = true;
        std::cout << "已切换到科学计数法输出\n";
        return true;
    }
    if (cmd == "format fixed") {
        gFormat.sci = false;
        std::cout << "已切换到普通十进制输出\n";
        return true;
    }
    if (cmd.rfind("precision ", 0) == 0) {
        const std::string value = trim(cmd.substr(10));
        const int p = std::max(0, std::stoi(value));
        gFormat.precision = p;
        std::cout << "已设置小数位数为 " << p << '\n';
        return true;
    }
    return false;
}

}  // namespace

int main() {
    std::unordered_map<std::string, ce::Complex> variables;
    std::string line;

    std::cout << ">>> ";
    while (std::getline(std::cin, line)) {
        try {
            const std::string cmd = trim(line);
            if (cmd.empty()) {
                std::cout << ">>> ";
                continue;
            }
            if (cmd == "quit" || cmd == "exit") {
                break;
            }
            if (handleCommand(cmd)) {
                std::cout << ">>> ";
                continue;
            }

            const auto tokens = ce::scan(line);
            ce::Complex result;
            if (ce::evaluate(tokens, variables, result)) {
                std::cout << ce::formatComplex(result, gFormat) << '\n';
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
        }
        std::cout << ">>> ";
    }
    return 0;
}
