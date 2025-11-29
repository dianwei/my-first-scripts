#include <iostream>
#include <string>
#include <unordered_map>
#include <stack>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <cmath>
#include <regex>
#include <iomanip>

// ================= Complex =================
class Complex {
public:
    Complex(long double r = 0.0L, long double i = 0.0L, bool placeholder = false)
        : real(r), imag(i), isVal(placeholder) {}

    Complex operator+(const Complex& o) const { return Complex(real + o.real, imag + o.imag); }
    Complex operator-(const Complex& o) const { return Complex(real - o.real, imag - o.imag); }
    Complex operator*(const Complex& o) const { return Complex(real*o.real - imag*o.imag, real*o.imag + imag*o.real); }
    Complex operator/(const Complex& o) const {
        long double denom = o.real*o.real + o.imag*o.imag;
        if (denom == 0.0L) throw std::runtime_error("Division by zero");
        return Complex((real*o.real + imag*o.imag)/denom, (imag*o.real - real*o.imag)/denom);
    }

    Complex conjugate() const { return Complex(real, -imag); }
    long double magnitude() const { return std::sqrtl(real*real + imag*imag); }
    bool isVariablePlaceholder() const { return isVal; }

    friend std::ostream& operator<<(std::ostream& os, const Complex& c) {
        const long double eps = 1e-15L;
        auto z = [&](long double x){ return std::fabsl(x) < eps ? 0.0L : x; };
        long double rr = z(c.real), ii = z(c.imag);
        if (ii == 0.0L) {
            os << rr;
        } else if (rr == 0.0L) {
            if (ii == 1.0L) os << "i";
            else if (ii == -1.0L) os << "-i";
            else os << ii << "i";
        } else {
            os << rr << (ii > 0 ? " + " : " - ");
            if (std::fabsl(ii) == 1.0L) os << "i";
            else os << std::fabsl(ii) << "i";
        }
        return os;
    }

private:
    long double real;
    long double imag;
    bool isVal = false;
};

// ================= Op / Token =================
enum class Op { Assign, Add, Sub, Mul, Div, LParen, RParen, FnCon, FnMod };

struct BindingPower { int left; int right; };
constexpr BindingPower binding(Op op) {
    switch (op) {
        case Op::Assign:  return {4, 5};   // 右结合：左<右
        case Op::Add:     return {10, 9};
        case Op::Sub:     return {10, 9};
        case Op::Mul:     return {15, 14};
        case Op::Div:     return {15, 14};
        case Op::LParen:  return {100, -1};
        case Op::RParen:  return {-1, -1};
        case Op::FnCon:   return {20, 19};
        case Op::FnMod:   return {20, 19};
        default:          return {-1, -1};
    }
}
inline bool shouldPop(Op top, Op incoming) {
    if (top == Op::LParen) return false;
    if (incoming == Op::RParen) return true;
    const auto tb = binding(top), ib = binding(incoming);
    // 赋值为右结合：严格大于；其他使用 >=
    if (incoming == Op::Assign) return tb.left > ib.right;
    return tb.left >= ib.right;
}

enum class Kind { Number, Ident, OpTok };
struct Token {
    Kind kind{};
    Op op{};                 // kind == OpTok
    std::string lex;         // kind == Number/Ident：原字面量
    size_t pos = 0;          // 源位置（可用于报错）
};

// 工具：一字符运算符到 Op
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

// 兜底数字正则（仅在 Scanner 验证字面量时用一次）
static const std::regex kNumRe(
    R"(^(?:[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?i?|i)$)"
);

// ================= Scanner =================
std::vector<Token> Scanner(const std::string& s) {
    std::vector<Token> toks;
    std::stack<char> par;
    std::string cur;
    size_t curPos = 0;

    auto flushCurrent = [&]() {
        if (cur.empty()) return;
        if (std::regex_match(cur, kNumRe)) {
            toks.push_back(Token{Kind::Number, Op{}, cur, curPos});
        } else {
            // 标识符/函数名
            if (!(std::isalpha((unsigned char)cur[0]) || cur[0] == '_'))
                throw std::runtime_error("Invalid token: " + cur);
            for (char ch : cur) {
                if (!(std::isalnum((unsigned char)ch) || ch == '_'))
                    throw std::runtime_error("Invalid ident: " + cur);
            }
            if (cur == "i") { // 纯虚单位
                toks.push_back(Token{Kind::Number, Op{}, cur, curPos});
            } else if (cur == "con") {
                toks.push_back(Token{Kind::OpTok, Op::FnCon, "", curPos});
            } else if (cur == "mod") {
                toks.push_back(Token{Kind::OpTok, Op::FnMod, "", curPos});
            } else {
                toks.push_back(Token{Kind::Ident, Op{}, cur, curPos});
            }
        }
        cur.clear();
    };

    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (std::isspace((unsigned char)c)) continue;

        // 运算符与括号
        if (c=='+' || c=='-' || c=='*' || c=='/' || c=='=' || c=='(' || c==')') {
            flushCurrent();
            if (c == '(') par.push('(');
            else if (c == ')') {
                if (par.empty()) throw std::runtime_error("Unmatched ')' at pos " + std::to_string(i));
                par.pop();
            }
            toks.push_back(Token{Kind::OpTok, toOpChar(c), "", i});
            continue;
        }

        // 累积为字面量/标识符
        if (cur.empty()) curPos = i;
        cur.push_back(c);
    }
    flushCurrent();

    if (!par.empty()) throw std::runtime_error("Mismatched parentheses");
    return toks;
}

// ================= 执行一个运算符 =================
Complex popOperator(std::stack<Complex>& values,
                    std::stack<Op>& ops,
                    std::stack<std::string>& assignTargets,
                    std::unordered_map<std::string, Complex>& variables) {
    Op op = ops.top(); ops.pop();
    switch (op) {
        case Op::Add: {
            Complex r = values.top(); values.pop();
            Complex l = values.top(); values.pop();
            return l + r;
        }
        case Op::Sub: {
            Complex r = values.top(); values.pop();
            Complex l = values.top(); values.pop();
            return l - r;
        }
        case Op::Mul: {
            Complex r = values.top(); values.pop();
            Complex l = values.top(); values.pop();
            return l * r;
        }
        case Op::Div: {
            Complex r = values.top(); values.pop();
            Complex l = values.top(); values.pop();
            return l / r;
        }
        case Op::Assign: {
            if (values.empty()) throw std::runtime_error("Missing right value for assignment");
            Complex value = values.top(); values.pop();
            if (values.empty()) throw std::runtime_error("Missing assignment target");
            Complex placeholder = values.top(); values.pop();
            if (!placeholder.isVariablePlaceholder())
                throw std::runtime_error("Left operand of assignment must be a variable");
            if (assignTargets.empty())
                throw std::runtime_error("Internal error: no variable recorded for assignment");
            std::string var = assignTargets.top(); assignTargets.pop();
            variables[var] = value;
            return value;
        }
        case Op::FnCon: {
            Complex a = values.top(); values.pop();
            return a.conjugate();
        }
        case Op::FnMod: {
            Complex a = values.top(); values.pop();
            return Complex(a.magnitude(), 0.0L);
        }
        default:
            throw std::runtime_error("Invalid operator");
    }
}

// ================= Calculator =================
bool Calculator(const std::vector<Token>& tokens,
                std::unordered_map<std::string, Complex>& variables,
                Complex& result) {
    std::stack<Complex> values;
    std::stack<Op> ops;
    std::stack<std::string> assignTargets;

    bool expectOperand = true;// 期待操作数
    bool hadAssignment = false;// 是否有赋值操作

    auto pushNumberFromLex = [&](const std::string& lex) {// stold 解析数字字面量
        if (lex == "i") { values.push(Complex(0.0L, 1.0L)); return; }
        if (!lex.empty() && lex.back() == 'i') {
            std::string t = lex.substr(0, lex.size()-1);
            long double v = t.empty() ? 1.0L : std::stold(t);
            values.push(Complex(0.0L, v));
        } else {
            long double v = std::stold(lex);
            values.push(Complex(v, 0.0L));
        }
    };

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& tk = tokens[i];

        if (tk.kind == Kind::Number) {
            pushNumberFromLex(tk.lex);
            expectOperand = false;
            continue;
        }

        if (tk.kind == Kind::Ident) {
            bool nextIsAssign = (i + 1 < tokens.size()
                                 && tokens[i+1].kind == Kind::OpTok
                                 && tokens[i+1].op == Op::Assign);
            if (nextIsAssign) {
                assignTargets.push(tk.lex);
                values.push(Complex(0.0L, 0.0L, true)); // 占位符
                hadAssignment = true;
            } else {
                auto it = variables.find(tk.lex);
                if (it == variables.end())
                    throw std::runtime_error("Undefined variable: " + tk.lex);
                values.push(it->second);
            }
            expectOperand = false;
            continue;
        }

        // 运算符/括号/函数
        if (tk.kind == Kind::OpTok) {
            Op op = tk.op;

            if (op == Op::FnCon || op == Op::FnMod) {
                if (!expectOperand) throw std::runtime_error("Missing operator before function call");
                ops.push(op);
                expectOperand = true;
                continue;
            }

            if (op == Op::LParen) {
                if (!expectOperand) throw std::runtime_error("Missing operator before '('");
                ops.push(op);
                expectOperand = true;
                continue;
            }

            if (op == Op::RParen) {
                if (expectOperand) throw std::runtime_error("Missing operand before ')'");
                while (!ops.empty() && ops.top() != Op::LParen) {
                    Complex v = popOperator(values, ops, assignTargets, variables);
                    values.push(v);
                }
                if (ops.empty() || ops.top() != Op::LParen)
                    throw std::runtime_error("Mismatched parentheses");
                ops.pop(); // 弹出 '('
                if (!ops.empty() && (ops.top() == Op::FnCon || ops.top() == Op::FnMod)) {
                    Complex v = popOperator(values, ops, assignTargets, variables);
                    values.push(v);
                }
                expectOperand = false;
                continue;
            }

            // 二元 + - * / =
            if (op == Op::Add || op == Op::Sub || op == Op::Mul || op == Op::Div || op == Op::Assign) {
                if (expectOperand) {
                    if (op == Op::Add) {
                        // 一元正号：忽略
                        continue;
                    } else if (op == Op::Sub) {
                        // 一元负号：0 - x
                        values.push(Complex(0.0L, 0.0L));
                    } else {
                        throw std::runtime_error("Missing operand before operator");
                    }
                }
                while (!ops.empty() && shouldPop(ops.top(), op)) {
                    Complex v = popOperator(values, ops, assignTargets, variables);
                    values.push(v);
                }
                ops.push(op);
                expectOperand = true;
                continue;
            }

            throw std::runtime_error("Unexpected operator");
        }

        throw std::runtime_error("Unknown token kind");
    }

    if (expectOperand) throw std::runtime_error("Expression ends with an operator");

    while (!ops.empty()) {
        Complex v = popOperator(values, ops, assignTargets, variables);
        values.push(v);
    }
    if (values.size() != 1) throw std::runtime_error("Invalid expression");
    result = values.top();
    return !hadAssignment;
}

// ================= main (REPL) =================
int main() {
    std::unordered_map<std::string, Complex> variables;
    std::string line;
    std::cout << ">>> ";
    while (std::getline(std::cin, line)) {
        try {
            auto tokens = Scanner(line);
            Complex result;
            if (Calculator(tokens, variables, result)) {
                std::cout << result << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        std::cout << ">>> ";
    }
    return 0;
}
