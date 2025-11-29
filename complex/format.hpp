#pragma once

#include <string>
#include <ios>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include "big_complex.hpp"

namespace complex_eval {

struct FormatConfig {
    bool sci = false;
    int precision = 30;
};

inline std::string to_string_big(const Big& value, const FormatConfig& cfg) {
    using std::ios_base;
    using boost::multiprecision::floor;

    auto is_int = [&](const Big& v) {
        return floor(v) == v;
    };

    if (cfg.sci) {
        return value.str(cfg.precision, ios_base::scientific);
    }

    if (is_int(value)) {
        return value.str(0, ios_base::fmtflags(0));
    }

    return value.str(cfg.precision, ios_base::fixed);
}

inline std::string formatComplex(const Complex& c, const FormatConfig& cfg) {
    using boost::multiprecision::abs;

    const Big& rr = c.realPart();
    const Big& ii = c.imagPart();

    if (ii == 0) {
        return to_string_big(rr, cfg);
    }

    if (rr == 0) {
        if (ii == 1) return "i";
        if (ii == -1) return "-i";
        return to_string_big(ii, cfg) + "i";
    }

    std::string s = to_string_big(rr, cfg);
    s += (ii > 0 ? " + " : " - ");
    Big absImag = abs(ii);
    if (absImag == 1) {
        s += "i";
    } else {
        s += to_string_big(absImag, cfg) + "i";
    }
    return s;
}

}  // namespace complex_eval
