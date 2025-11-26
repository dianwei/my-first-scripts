#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <stack>
#include <vector>
#include <regex>


class Complex {
    private:
        double real;
        double imag;
    public:
        Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}

        // Overload the addition operator
        Complex operator+(const Complex& other) const {
            return Complex(real + other.real, imag + other.imag);
        }

        // Overload the subtraction operator
        Complex operator-(const Complex& other) const {
            return Complex(real - other.real, imag - other.imag);
        }

        // Overload the multiplication operator
        Complex operator*(const Complex& other) const {
            return Complex(real * other.real - imag * other.imag,
                           real * other.imag + imag * other.real);
        }

        // Overload the division operator
        Complex operator/(const Complex& other) const {
            double denom = other.real * other.real + other.imag * other.imag;
            return Complex((real * other.real + imag * other.imag) / denom,
                           (imag * other.real - real * other.imag) / denom);
        }

        // Overload the assignment operator
        Complex operator=(const Complex& other) {
            real = other.real;
            imag = other.imag;
            return *this;
        }

        // Method to compute the complex conjugate
        Complex conjugate() const {
            return Complex(real, -imag);
        }

        // Method to compute the magnitude of the complex number
        double magnitude() const {
            return sqrt(real * real + imag * imag);
        }

        // Overload the stream insertion operator for easy output
        friend std::ostream& operator<<(std::ostream& os, const Complex& c) {
            if(c.imag > 0){
                os << c.real << " + " << c.imag << "i";
            } else if(c.imag < 0){
                os << c.real << " - " << -c.imag << "i";
            } else {
                os << c.real;
            }
            return os;
        }
};

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
    for(auto c : s) {
        if(c==' ') continue;
        if(c=='+' || c=='-' || c=='*' || c=='/' || c=='(' || c==')' || c=='=') {
            if(!current.empty()) {
                tokens.push_back(current);
                current="";
            }
            tokens.push_back(std::string(1,c));
        } else {
            current+=c;
        }
    }
    if(!current.empty()) {
        tokens.push_back(current);
    }
    for(const auto& token : tokens) {
        std::cout << token << std::endl;
    }
    return true;
}

bool Calculator(const std::vector<std::string>& tokens, std::unordered_map<std::string, Complex>& variables, Complex& result) {
    // Placeholder for actual calculation logic
    // This function should parse the tokens and compute the result
    std::stack<Complex> values;
    std::stack<std::string> ops;
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
        } else if(isvalidVariable(token)) {
            if(variables.find(token) != variables.end()) {
                values.push(variables[token]);
            } else if(tokens[i+1] == "=") {
                variables[token] = Complex(); // Placeholder for assignment
                values.push(variables[token]);
            }
        }
        else if(token == "+" || token == "-" || token == "*" || token == "/" || token == "=" || token == "(" || token == ")") {
            ops.push(token);
        }
        else if((token == "con" || token == "mod") && tokens[i+1] == "(") {
            ops.push(token+"(");
            i++; // Skip the '('
        }
        else throw std::runtime_error("Invalid token: " + token);
    }
    // 打印栈的内容以调试
    while(!values.empty()) {
        Complex val = values.top();
        values.pop();
        std::cout << "Value on stack: " << val << std::endl;
    }
    while(!ops.empty()) {
        std::string op = ops.top();
        ops.pop();
        std::cout << "Operator on stack: " << op << std::endl;
    }
    return true;
}
int main(){
    std::unordered_map<std::string, Complex> variables;
    std::string line;
    while (std::getline(std::cin, line)) {
        try {
            std::vector<std::string> tokens;
            Scanner(line, tokens);
            Complex result;
            if (Calculator(tokens, variables, result)) {
                std::cout << "Result: " << result << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}