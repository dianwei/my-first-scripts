#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <stack>
#include <vector>
#include <regex>
#include <iomanip>
#include <boost/multiprecision/cpp_dec_float.hpp>

using BigFloat = boost::multiprecision::cpp_dec_float_100;

class BigScalar {
    public:
        BigScalar(double v = 0.0) : value(v) {}
        BigScalar(const std::string& s) : value(s) {}
        BigScalar(const BigFloat& v) : value(v) {}

        BigScalar operator+(const BigScalar& other) const { return BigScalar(value + other.value); }
        BigScalar operator-(const BigScalar& other) const { return BigScalar(value - other.value); }
        BigScalar operator*(const BigScalar& other) const { return BigScalar(value * other.value); }
        BigScalar operator/(const BigScalar& other) const { return BigScalar(value / other.value); }

        BigScalar sqrt() const { return BigScalar(boost::multiprecision::sqrt(value)); }
        const BigFloat& raw() const { return value; }

    private:
        BigFloat value;
};

class Complex {
    public:
        Complex(BigScalar r = 0.0, BigScalar i = 0.0, bool placeholder = false)
            : real(r), imag(i), isVal(placeholder) {}

        Complex operator+(const Complex& other) const {
            return Complex(real + other.real, imag + other.imag);
        }
        Complex operator-(const Complex& other) const {
            return Complex(real - other.real, imag - other.imag);
        }
        Complex operator*(const Complex& other) const {
            return Complex(real * other.real - imag * other.imag,
                           real * other.imag + imag * other.real);
        }
        Complex operator/(const Complex& other) const {
            BigScalar denom = other.real * other.real + other.imag * other.imag;
            return Complex((real * other.real + imag * other.imag) / denom,
                           (imag * other.real - real * other.imag) / denom);
        }

        Complex conjugate() const { return Complex(real, BigScalar(0.0) - imag); }
        BigScalar magnitude() const { return (real * real + imag * imag).sqrt(); }

        bool isVariablePlaceholder() const { return isVal; }

        friend std::ostream& operator<<(std::ostream& os, const Complex& c) {
            if (c.imag.raw() == 0) {
                os << c.real.raw();
            } else if (c.real.raw() == 0) {
                os << c.imag.raw() << "i";
            } else {
                os << c.real.raw() << (c.imag.raw() > 0 ? " + " : " - ")
                   << abs(c.imag.raw()) << "i";
            }
            return os;
        }

    private:
        BigScalar real;
        BigScalar imag;
        bool isVal = false;
};

enum class Op {
    Assign, Add, Sub, Mul, Div,
    LParen, RParen,
    ConCall, ModCall  // 表示 con( / mod( 的前缀符号
};

struct BindingPower {
    int left;
    int right;
};

// 由左至右：左 binding power > 右 binding power
static const std::unordered_map<Op, BindingPower> kBinding = {
    {Op::Assign, {5, 4}},    // '='
    {Op::Add,    {10, 9}},   // '+'
    {Op::Sub,    {10, 9}},   // '-'
    {Op::Mul,    {15, 14}},  // '*'
    {Op::Div,    {15, 14}},  // '/'
    {Op::LParen, {100, -1}}, // '(' 左括号左侧很大，右侧无效
    {Op::RParen, {-1, -1}},  // ')' 仅用于触发回退
    {Op::ConCall,{20, 19}},  // 'con('
    {Op::ModCall,{20, 19}},  // 'mod('
};

// 把 token 映射到 Op
Op toOp(const std::string& tok) {
    if (tok == "=")   return Op::Assign;
    if (tok == "+")   return Op::Add;
    if (tok == "-")   return Op::Sub;
    if (tok == "*")   return Op::Mul;
    if (tok == "/")   return Op::Div;
    if (tok == "(")   return Op::LParen;
    if (tok == ")")   return Op::RParen;
    if (tok == "con(")return Op::ConCall;
    if (tok == "mod(")return Op::ModCall;
    throw std::runtime_error("unknown op: " + tok);
}

// 根据“左权重大于右权重”的规则决定是否先处理栈顶运算符
bool shouldPop(const std::string& topTok, const std::string& incomingTok) {
    Op top = toOp(topTok);
    Op incoming = toOp(incomingTok);

    if (top == Op::LParen) return false;   // 左括号不参与比较
    if (incoming == Op::RParen) return true; // 遇到右括号时回退到 '('

    const auto& topBind = kBinding.at(top);
    const auto& inBind  = kBinding.at(incoming);
    return topBind.left >= inBind.right;
}

Complex popOperator(std::stack<Complex>& values,
                    std::stack<std::string>& ops,
                    std::stack<std::string>& assignTargets,
                    std::unordered_map<std::string, Complex>& variables) {
    std::string opTok = ops.top();
    ops.pop();
    Op op = toOp(opTok);
    if (op == Op::Add || op == Op::Sub || op == Op::Mul || op == Op::Div) {
        bool isselfsub = false;
        Complex right = values.top(); values.pop();
        Complex left;
        if(op == Op::Sub && values.empty()) {
            isselfsub = true;
        } else {
            left = values.top();
            values.pop();
        }
        Complex result;
        switch (op) {
            case Op::Add: result = left + right; break;
            case Op::Sub: if(isselfsub) result = Complex(0,0) - right; 
                else result = left - right;break;
            case Op::Mul: result = left * right; break;
            case Op::Div: result = left / right; break;
            default: throw std::runtime_error("Invalid binary operator");
        }
        return result;
    } else if (op == Op::Assign) {
        if (values.empty()) {
            throw std::runtime_error("Missing right value for assignment");
        }
        Complex value = values.top(); values.pop();
        if (values.empty()) {
            throw std::runtime_error("Missing assignment target");
        }
        Complex placeholder = values.top(); values.pop();
        if (!placeholder.isVariablePlaceholder()) {
            throw std::runtime_error("Left operand of assignment must be a variable");
        }
        if (assignTargets.empty()) {
            throw std::runtime_error("Internal error: no variable recorded for assignment");
        }
        const std::string varName = assignTargets.top();// 增加变量栈保存变量名，占位符保存在值栈中
        assignTargets.pop();
        variables[varName] = value;
        return value;
    }
    else if (op == Op::ConCall) {
        Complex arg = values.top(); values.pop();
        return arg.conjugate();
    }
    else if (op == Op::ModCall) {
        Complex arg = values.top(); values.pop();
        return Complex(arg.magnitude(), 0);
    }
    throw std::runtime_error("Invalid operator");
}

bool isValidNumber(const std::string& s) {
    static const std::regex re(
        R"(^[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?(i)?|i$)"// 匹配实数和虚数形式（包括单独的 i）
    );
    return std::regex_match(s, re);
}

bool isvalidVariable(const std::string& s) {
    if(s=="i" || s=="con" || s=="mod") return false;// 保留字
    if(s.empty() || (!isalpha(s[0]) && s[0]!='_')) return false;
    for(char c : s) {
        if(!isalnum(c) && c!='_') return false;
    }
    return true;
}

bool Scanner(const std::string& s, std::vector<std::string>& tokens) {
    std::string current="";
    size_t pos=0;
    size_t lp=0, rp=0;// 检查括号匹配
    for(size_t i=0; i<s.size(); ++i) {
        char c = s[i];
        if(c==' ') continue;
        if(c=='+' || c=='-' || c=='*' || c=='/' || c=='(' || c==')' || c=='=') {
            if(c=='(') lp++;
            if(c==')') rp++;
            if(!current.empty()) {
                tokens.push_back(current);
                current="";
            }
            if(i == s.size()-1 && c=='-') {
                throw std::runtime_error("Invalid token: -");// 处理最后一个字符是 '-' 的情况
            }else tokens.push_back(std::string(1,c));
        } else {
            current+=c;
        }
    }
    if(!current.empty()) {
        if(current != "-")
        tokens.push_back(current);
        else throw std::runtime_error("Invalid token: " + current);
    }
    if(lp != rp) {
        throw std::runtime_error("Mismatched parentheses");
    }
    return true;
}

bool Calculator(const std::vector<std::string>& tokens, std::unordered_map<std::string, Complex>& variables, Complex& result) {
    std::stack<Complex> values;
    std::stack<std::string> ops;
    std::stack<std::string> assignTargets;
    bool expectOperand = true;
    bool hadAssignment = false;
    for(size_t i=0; i<tokens.size(); ++i) {
        const std::string& token = tokens[i];
        if(isValidNumber(token)) {
            double val = 0.0;
            if(token.back()=='i') {
                std::string imagPart = token.substr(0, token.size()-1);
                val = imagPart.empty() ? 1.0 : std::stod(imagPart);
                values.push(Complex(0.0, val));
            }
            else {
                val = std::stod(token);
                values.push(Complex(val, 0.0));
            }
            expectOperand = false;
        } else if(isvalidVariable(token)) {
            bool nextIsAssign = (i + 1 < tokens.size() && tokens[i+1] == "=");
            if(nextIsAssign) {
                assignTargets.push(token);
                values.push(Complex(0.0, 0.0, true));
                hadAssignment = true;
            } else {
                auto it = variables.find(token);
                if(it == variables.end()) {
                    throw std::runtime_error("Undefined variable: " + token);
                }
                values.push(it->second);
            }
            expectOperand = false;
        }
        else if(token == "con" || token == "mod") {
            if(!expectOperand) {
                throw std::runtime_error("Missing operator before function call: " + token);
            }
            if(i + 1 >= tokens.size() || tokens[i+1] != "(") {
                throw std::runtime_error("Function call must be followed by '('");
            }
            std::string opToken = token + "(";
            while(!ops.empty() && shouldPop(ops.top(), opToken)) {
                Complex val = popOperator(values, ops, assignTargets, variables);
                values.push(val);
            }
            ops.push(opToken);
            expectOperand = true;
        }
        else if(token == "+" || token == "-" || token == "*" || token == "/" || token == "=") {
            if(expectOperand) {
                if(token == "-") {
                    values.push(Complex(0.0, 0.0));
                } else {
                    throw std::runtime_error("Missing operand before operator: " + token);
                }
            }
            std::string opToken = token;
            while(!ops.empty() && shouldPop(ops.top(), opToken)) {
                Complex val = popOperator(values, ops, assignTargets, variables);
                values.push(val);
            }
            ops.push(opToken);
            expectOperand = true;
        } else if(token == "(") {
            if(!expectOperand) {
                throw std::runtime_error("Missing operator before '('");
            }
            ops.push("(");
            expectOperand = true;
        } else if(token == ")") {
            if(expectOperand) {
                throw std::runtime_error("Missing operand before ')'");
            }
            while(!ops.empty() && ops.top() != "(") {
                Complex val = popOperator(values, ops, assignTargets, variables);
                values.push(val);
            }
            if(!ops.empty() && ops.top() == "(") {
                ops.pop(); // 弹出左括号
            } else {
                throw std::runtime_error("Mismatched parentheses");
            }
            if(!ops.empty() && (ops.top() == "con(" || ops.top() == "mod(")) {
                Complex val = popOperator(values, ops, assignTargets, variables);
                values.push(val);
            }
            expectOperand = false;
        }
        else throw std::runtime_error("Invalid token: " + token);
    }
    if(expectOperand) {
        throw std::runtime_error("Expression ends with an operator");
    }
    while(!ops.empty()) {
        Complex val = popOperator(values, ops, assignTargets, variables);
        values.push(val);
    }
    if(values.size() != 1) {
        throw std::runtime_error("Invalid expression");
    }
    result = values.top();
    if(hadAssignment) {
        return false; // 表示这是一个赋值表达式，没有输出结果
    }
    return true;
}
std::string trim(const std::string& s) {
    const std::string whitespace = " \t\n\r";
    const auto start = s.find_first_not_of(whitespace);
    if(start == std::string::npos) return "";
    const auto end = s.find_last_not_of(whitespace);
    return s.substr(start, end - start + 1);
}

int main(){
    std::unordered_map<std::string, Complex> variables;
    std::string line;
    bool useScientific = false;
    constexpr int kOutputPrecision = 20;
    std::cout << ">>> ";
    while (std::getline(std::cin, line)) {
        try {
            const std::string command = trim(line);// 去除前后空白
            // 
            if(command == "format sci") {
                useScientific = true;
                std::cout << "已切换到科学计数法输出" << std::endl;
                std::cout << ">>> ";
                continue;
            } else if(command == "format fixed") {
                useScientific = false;
                std::cout << "已切换到普通小数输出" << std::endl;
                std::cout << ">>> ";
                continue;
            } else if(command == "exit" || command == "quit") {
                break;
            } else if(command.empty()) {
                std::cout << ">>> ";
                continue;
            } else if(command == "help") {
                std::cout << "支持的命令:\n"
                             "  format sci   切换到科学计数法输出\n"
                             "  format fixed 切换到普通小数输出\n"
                             "  exit 或 quit 退出程序\n"
                             "支持的操作:\n"
                             "  复数运算: +, -, *, /\n"
                             "  赋值: =\n"
                             "  共轭: con(z)\n"
                             "  模长: mod(z)\n"
                             "变量名规则: 以字母或下划线开头，后续可包含字母、数字和下划线，且不能使用保留字 i, con, mod\n";
                std::cout << ">>> ";
                continue;
            }
            std::vector<std::string> tokens;
            Scanner(line, tokens);
            Complex result;
            if (Calculator(tokens, variables, result)) {
                std::ios oldState(nullptr);
                oldState.copyfmt(std::cout);// 保存当前格式状态
                if(useScientific) {
                    std::cout.setf(std::ios::scientific, std::ios::floatfield);
                } else {
                    std::cout.setf(std::ios::fixed, std::ios::floatfield);
                }
                std::cout << std::setprecision(kOutputPrecision) << result << std::endl;
                std::cout.copyfmt(oldState);// 恢复格式状态
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        std::cout << ">>> ";
    }
    return 0;
}