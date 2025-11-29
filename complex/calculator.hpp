#pragma once

#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "big_complex.hpp"
#include "token.hpp"

namespace complex_eval {

inline Big parseBig(const std::string& text) {
    try {
        return text.empty() ? Big(0) : Big(text);
    } catch (...) {
        throw std::runtime_error("Invalid number: " + text);
    }
}

inline Complex popOperator(std::stack<Complex>& values,
                           std::stack<Op>& ops,
                           std::stack<std::string>& assignTargets,
                           std::unordered_map<std::string, Complex>& variables) {
    Op op = ops.top();
    ops.pop();

    switch (op) {
        case Op::Add: {
            Complex right = values.top(); values.pop();
            Complex left = values.top(); values.pop();
            return left + right;
        }
        case Op::Sub: {
            Complex right = values.top(); values.pop();
            Complex left = values.top(); values.pop();
            return left - right;
        }
        case Op::Mul: {
            Complex right = values.top(); values.pop();
            Complex left = values.top(); values.pop();
            return left * right;
        }
        case Op::Div: {
            Complex right = values.top(); values.pop();
            Complex left = values.top(); values.pop();
            return left / right;
        }
        case Op::Assign: {
            if (values.empty()) throw std::runtime_error("Missing right value for assignment");
            Complex value = values.top(); values.pop();
            if (values.empty()) throw std::runtime_error("Missing assignment target");
            Complex placeholder = values.top(); values.pop();
            if (!placeholder.isVariablePlaceholder()) {
                throw std::runtime_error("Left operand of assignment must be a variable");
            }
            if (assignTargets.empty()) {
                throw std::runtime_error("Internal error: no variable recorded for assignment");
            }
            std::string varName = assignTargets.top();
            assignTargets.pop();
            variables[varName] = value;
            return value;
        }
        case Op::FnCon: {
            Complex arg = values.top(); values.pop();
            return arg.conjugate();
        }
        case Op::FnMod: {
            Complex arg = values.top(); values.pop();
            return Complex(arg.magnitude(), Big(0));
        }
        default:
            throw std::runtime_error("Invalid operator");
    }
}

inline bool evaluate(const std::vector<Token>& tokens,
                     std::unordered_map<std::string, Complex>& variables,
                     Complex& result) {
    std::stack<Complex> values;
    std::stack<Op> ops;
    std::stack<std::string> assignTargets;

    bool expectOperand = true;
    bool hadAssignment = false;

    auto pushNumberFromLex = [&](const std::string& lex) {
        if (lex == "i") {
            values.push(Complex(Big(0), Big(1)));
            return;
        }
        if (!lex.empty() && lex.back() == 'i') {
            std::string imagPart = lex.substr(0, lex.size() - 1);
            if (imagPart.empty() || imagPart == "+" || imagPart == "-") {
                values.push(Complex(Big(0), (imagPart == "-") ? Big(-1) : Big(1)));
            } else {
                values.push(Complex(Big(0), parseBig(imagPart)));
            }
            return;
        }
        values.push(Complex(parseBig(lex), Big(0)));
    };

    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const Token& tk = tokens[i];

        if (tk.kind == Kind::Number) {
            pushNumberFromLex(tk.lex);
            expectOperand = false;
            continue;
        }

        if (tk.kind == Kind::Ident) {
            bool nextIsAssign = (i + 1 < tokens.size() &&
                                 tokens[i + 1].kind == Kind::OpTok &&
                                 tokens[i + 1].op == Op::Assign);
            if (nextIsAssign) {
                assignTargets.push(tk.lex);
                values.push(Complex(Big(0), Big(0), true));
                hadAssignment = true;
            } else {
                auto it = variables.find(tk.lex);
                if (it == variables.end()) {
                    throw std::runtime_error("Undefined variable: " + tk.lex);
                }
                values.push(it->second);
            }
            expectOperand = false;
            continue;
        }

        if (tk.kind == Kind::OpTok) {
            Op op = tk.op;

            if (op == Op::FnCon || op == Op::FnMod) {
                if (!expectOperand) {
                    throw std::runtime_error("Missing operator before function call");
                }
                ops.push(op);
                expectOperand = true;
                continue;
            }

            if (op == Op::LParen) {
                if (!expectOperand) {
                    throw std::runtime_error("Missing operator before '('");
                }
                ops.push(op);
                expectOperand = true;
                continue;
            }

            if (op == Op::RParen) {
                if (expectOperand) {
                    throw std::runtime_error("Missing operand before ')'");
                }
                while (!ops.empty() && ops.top() != Op::LParen) {
                    Complex value = popOperator(values, ops, assignTargets, variables);
                    values.push(value);
                }
                if (ops.empty() || ops.top() != Op::LParen) {
                    throw std::runtime_error("Mismatched parentheses");
                }
                ops.pop();
                if (!ops.empty() && (ops.top() == Op::FnCon || ops.top() == Op::FnMod)) {
                    Complex value = popOperator(values, ops, assignTargets, variables);
                    values.push(value);
                }
                expectOperand = false;
                continue;
            }

            if (op == Op::Add || op == Op::Sub || op == Op::Mul || op == Op::Div || op == Op::Assign) {
                if (expectOperand) {
                    if (op == Op::Add) {
                        continue;
                    }
                    if (op == Op::Sub) {
                        values.push(Complex(Big(0), Big(0)));
                    } else {
                        throw std::runtime_error("Missing operand before operator");
                    }
                }
                while (!ops.empty() && shouldPop(ops.top(), op)) {
                    Complex value = popOperator(values, ops, assignTargets, variables);
                    values.push(value);
                }
                ops.push(op);
                expectOperand = true;
                continue;
            }

            throw std::runtime_error("Unexpected operator");
        }

        throw std::runtime_error("Unknown token kind");
    }

    if (expectOperand) {
        throw std::runtime_error("Expression ends with an operator");
    }

    while (!ops.empty()) {
        Complex value = popOperator(values, ops, assignTargets, variables);
        values.push(value);
    }

    if (values.size() != 1) {
        throw std::runtime_error("Invalid expression");
    }

    result = values.top();
    return !hadAssignment;
}

}  // namespace complex_eval
