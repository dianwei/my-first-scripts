#pragma once

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <stdexcept>

namespace complex_eval {

using Big = boost::multiprecision::cpp_dec_float_100;

class Complex {
public:
    Complex(long double r = 0.0L, long double i = 0.0L, bool placeholder = false)
        : real(Big(r)), imag(Big(i)), isPlaceholder(placeholder) {}

    Complex(const Big& r, const Big& i, bool placeholder = false)
        : real(r), imag(i), isPlaceholder(placeholder) {}

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
        Big denom = other.real * other.real + other.imag * other.imag;
        if (denom == 0) {
            throw std::runtime_error("Division by zero");
        }
        return Complex((real * other.real + imag * other.imag) / denom,
                       (imag * other.real - real * other.imag) / denom);
    }

    Complex conjugate() const { return Complex(real, -imag); }
    Big magnitude() const { return boost::multiprecision::sqrt(real * real + imag * imag); }

    bool isVariablePlaceholder() const { return isPlaceholder; }

    const Big& realPart() const { return real; }
    const Big& imagPart() const { return imag; }

private:
    Big real;
    Big imag;
    bool isPlaceholder = false;
};

}  // namespace complex_eval
