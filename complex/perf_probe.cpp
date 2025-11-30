#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>

#include "big_complex.hpp"
#include "calculator.hpp"
#include "scanner.hpp"

int main() {
    using namespace complex_eval;

    std::unordered_map<std::string, Complex> vars;
    Complex result;

    auto assignTokens = scan("a = 3 + 4i");
    evaluate(assignTokens, vars, result);

    auto exprTokens = scan("mod(con(a) * (1 - 2i))");
    const std::size_t iterations = 1000;

    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        evaluate(exprTokens, vars, result);
    }
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double, std::micro> per = (end - start) / iterations;
    std::cout << "Result: " << result.realPart() << "+" << result.imagPart() << "i\n";
    std::cout << "Average time per eval: " << per.count() << " us\n";
    return 0;
}
