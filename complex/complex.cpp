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
#include <boost/multiprecision/cpp_dec_float.hpp>

using Big = boost::multiprecision::cpp_dec_float_100;

// 输出配置
struct FormatConfig {
    bool sci = false;      // 是否使用科学计数法
    int precision = 30;    // 精度：科学计数法为小数点后位数；fixed 为小数点后位数
} gFmt;

// ============ Complex ============
class Complex {
public:
    Complex(long double r = 0.0L, long double i = 0.0L, bool placeholder = false)
        : real(Big(r)), imag(Big(i)), isVal(placeholder) {}
    Complex(const Big& r, const Big& i, bool placeholder = false)
        : real(r), imag(i), isVal(placeholder) {}

    Complex operator+(const Complex& o) const { return Complex(real + o.real, imag + o.imag); }
    Complex operator-(const Complex& o) const { return Complex(real - o.real, imag - o.imag); }
    Complex operator*(const Complex& o) const { return Complex(real*o.real - imag*o.imag, real*o.imag + imag*o.real); }
    Complex operator/(const Complex& o) const {
        Big denom = o.real*o.real + o.imag*o.imag;
        if (denom == 0) throw std::runtime_error("Division by zero");
        return Complex((real*o.real + imag*o.imag)/denom, (imag*o.real - real*o.imag)/denom);
    }

    Complex conjugate() const { return Complex(real, -imag); }
    Big magnitude() const { return boost::multiprecision::sqrt(real*real + imag*imag); }
    bool isVariablePlaceholder() const { return isVal; }

    const Big& realPart() const { return real; }
    const Big& imagPart() const { return imag; }

private:
    Big real;
    Big imag;
    bool isVal = false;
};

// 将 Big 按配置转字符串：整数在 fixed 模式下用普通十进制，不带小数/科学记数
static std::string to_string_big(const Big& x, const FormatConfig& cfg) {
    using std::ios_base;
    using boost::multiprecision::floor;

    auto is_int = [&](const Big& v){ return floor(v) == v; };

    if (cfg.sci) {
        // 科学计数法，保留 cfg.precision 位（小数点后位数）
        return x.str(cfg.precision, ios_base::scientific);
    } else {
        if (is_int(x)) {
            // 整数：用最短十进制，不受流状态影响
            return x.str(0, ios_base::fmtflags(0));
        } else {
            // 非整数：fixed 小数点后 cfg.precision 位
            return x.str(cfg.precision, ios_base::fixed);
        }
    }
}

static std::string formatComplex(const Complex& c, const FormatConfig& cfg) {
    using boost::multiprecision::abs;
    const Big& rr = c.realPart();
    const Big& ii = c.imagPart();

    if (ii == 0) {
        return to_string_big(rr, cfg);
    }
    if (rr == 0) {
        if (ii == 1)  return "i";
        if (ii == -1) return "-i";
        return to_string_big(ii, cfg) + "i";
    }
    std::string s = to_string_big(rr, cfg);
    s += (ii > 0 ? " + " : " - ");
    Big a = abs(ii);
    if (a == 1) s += "i";
    else        s += to_string_big(a, cfg) + "i";
    return s;
}

// ============ 运算符/Token ============
enum class Op { Assign, Add, Sub, Mul, Div, LParen, RParen, FnCon, FnMod };
struct BindingPower { int left; int right; };
constexpr BindingPower binding(Op op) {
    switch (op) {
        case Op::Assign:  return {4, 5};   // 右结合
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
    if (incoming == Op::Assign) return tb.left > ib.right; // 右结合：严格大于
    return tb.left >= ib.right;
}

enum class Kind { Number, Ident, OpTok };
struct Token {
    Kind kind{};
    Op op{};                 // kind == OpTok
    std::string lex;         // kind == Number/Ident：原字面量（数字字符串或变量名）
    size_t pos = 0;          // 源位置（用于错误提示）
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

// 数字正则：支持实数、科学计数法、带 i 的虚数字面量、以及单独的 i
static const std::regex kNumRe(
    R"(^(?:[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?i?|i)$)"
);

// ============ Scanner：把字符串转 Tokens ============
static std::vector<Token> Scanner(const std::string& s) {
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
            if (cur == "i") {
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

        if (cur.empty()) curPos = i;
        cur.push_back(c);
    }
    flushCurrent();

    if (!par.empty()) throw std::runtime_error("Mismatched parentheses");
    return toks;
}

// ============ 执行一个运算符 ============
static Complex popOperator(std::stack<Complex>& values,
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
            return Complex(a.magnitude(), Big(0));
        }
        default:
            throw std::runtime_error("Invalid operator");
    }
}

// ============ Calculator ============
static Big parseBig(const std::string& s) {
    try {
        return s.empty() ? Big(0) : Big(s);
    } catch (...) {
        throw std::runtime_error("Invalid number: " + s);
    }
}

static bool Calculator(const std::vector<Token>& tokens,
                       std::unordered_map<std::string, Complex>& variables,
                       Complex& result) {
    std::stack<Complex> values;
    std::stack<Op> ops;
    std::stack<std::string> assignTargets;

    bool expectOperand = true;
    bool hadAssignment = false;

    auto pushNumberFromLex = [&](const std::string& lex) {
        if (lex == "i") { values.push(Complex(Big(0), Big(1))); return; }
        if (!lex.empty() && lex.back() == 'i') {
            std::string t = lex.substr(0, lex.size()-1);
            if (t.empty() || t == "+" || t == "-") {
                values.push(Complex(Big(0), (t == "-") ? Big(-1) : Big(1)));
            } else {
                values.push(Complex(Big(0), parseBig(t)));
            }
        } else {
            values.push(Complex(parseBig(lex), Big(0)));
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
                values.push(Complex(Big(0), Big(0), true)); // 占位符
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

            if (op == Op::Add || op == Op::Sub || op == Op::Mul || op == Op::Div || op == Op::Assign) {
                if (expectOperand) {
                    if (op == Op::Add) {
                        // 一元正号：忽略
                        continue;
                    } else if (op == Op::Sub) {
                        // 一元负号：0 - x
                        values.push(Complex(Big(0), Big(0)));
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

// ============ 工具 ============
static std::string trim(const std::string& s) {
    const std::string ws = " \t\n\r";
    size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

// ============ main (REPL) ============
int main() {
    std::unordered_map<std::string, Complex> variables;
    std::string line;

    std::cout << ">>> ";
    while (std::getline(std::cin, line)) {
        try {
            std::string cmd = trim(line);
            if (cmd.empty()) {
                std::cout << ">>> ";
                continue;
            }
            // 帮助命令
            if (cmd == "help") {
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
                std::cout << ">>> ";
                continue;
            }
            if (cmd == "quit" || cmd == "exit") break;

            if (cmd == "format sci") {
                gFmt.sci = true;
                std::cout << "已切换到科学计数法输出\n>>> ";
                continue;
            }
            if (cmd == "format fixed") {
                gFmt.sci = false;
                std::cout << "已切换到普通十进制输出\n>>> ";
                continue;
            }
            if (cmd.rfind("precision ", 0) == 0) {
                std::string nstr = trim(cmd.substr(10));
                int p = std::max(0, std::stoi(nstr));
                gFmt.precision = p;
                std::cout << "已设置小数位数为 " << p << "\n>>> ";
                continue;
            }

            // 表达式求值
            auto tokens = Scanner(line);
            Complex result;
            if (Calculator(tokens, variables, result)) {
                std::cout << formatComplex(result, gFmt) << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
        std::cout << ">>> ";
    }
    return 0;
}
