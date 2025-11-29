#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

namespace complex_eval {

enum class Op { Assign, Add, Sub, Mul, Div, LParen, RParen, FnCon, FnMod };

struct BindingPower {
    int left;
    int right;
};

constexpr BindingPower binding(Op op) {
    switch (op) {
        case Op::Assign: return {4, 5};
        case Op::Add:
        case Op::Sub:   return {10, 9};
        case Op::Mul:
        case Op::Div:   return {15, 14};
        case Op::LParen:return {100, -1};
        case Op::RParen:return {-1, -1};
        case Op::FnCon:
        case Op::FnMod: return {20, 19};
        default:        return {-1, -1};
    }
}

inline bool shouldPop(Op top, Op incoming) {
    if (top == Op::LParen) return false;
    if (incoming == Op::RParen) return true;

    const auto tb = binding(top);
    const auto ib = binding(incoming);
    if (incoming == Op::Assign) {
        return tb.left > ib.right;
    }
    return tb.left >= ib.right;
}

enum class Kind { Number, Ident, OpTok };

struct Token {
    Kind kind{};
    Op op{};
    std::string lex;
    std::size_t pos = 0;
};

inline Op toOpChar(char c) {
    switch (c) {
        case '+': return Op::Add;
        case '-': return Op::Sub;
        case '*': return Op::Mul;
        case '/': return Op::Div;
        case '=': return Op::Assign;
        case '(': return Op::LParen;
        case ')': return Op::RParen;
        default: throw std::runtime_error(std::string("unknown op char: ") + c);
    }
}

}  // namespace complex_eval
