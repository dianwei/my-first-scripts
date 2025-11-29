#pragma once

#include <cctype>
#include <regex>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

#include "token.hpp"

namespace complex_eval {

inline const std::regex& numberRegex() {
    static const std::regex re(
        R"(^(?:[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?i?|i)$)"
    );
    return re;
}

inline std::vector<Token> scan(const std::string& input) {
    std::vector<Token> tokens;
    std::stack<char> parens;
    std::string current;
    std::size_t currentPos = 0;

    auto flushCurrent = [&]() {
        if (current.empty()) return;
        const auto& re = numberRegex();
        if (std::regex_match(current, re)) {
            tokens.push_back(Token{Kind::Number, Op{}, current, currentPos});
        } else {
            if (!(std::isalpha(static_cast<unsigned char>(current[0])) || current[0] == '_')) {
                throw std::runtime_error("Invalid token: " + current);
            }
            for (char ch : current) {
                if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')) {
                    throw std::runtime_error("Invalid ident: " + current);
                }
            }
            if (current == "i") {
                tokens.push_back(Token{Kind::Number, Op{}, current, currentPos});
            } else if (current == "con") {
                tokens.push_back(Token{Kind::OpTok, Op::FnCon, "", currentPos});
            } else if (current == "mod") {
                tokens.push_back(Token{Kind::OpTok, Op::FnMod, "", currentPos});
            } else {
                tokens.push_back(Token{Kind::Ident, Op{}, current, currentPos});
            }
        }
        current.clear();
    };

    for (std::size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        if (std::isspace(static_cast<unsigned char>(c))) continue;

        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '(' || c == ')') {
            flushCurrent();
            if (c == '(') {
                parens.push('(');
            } else if (c == ')') {
                if (parens.empty()) {
                    throw std::runtime_error("Unmatched ')' at pos " + std::to_string(i));
                }
                parens.pop();
            }
            tokens.push_back(Token{Kind::OpTok, toOpChar(c), "", i});
            continue;
        }

        if (current.empty()) currentPos = i;
        current.push_back(c);
    }

    flushCurrent();

    if (!parens.empty()) {
        throw std::runtime_error("Mismatched parentheses");
    }

    return tokens;
}

}  // namespace complex_eval
