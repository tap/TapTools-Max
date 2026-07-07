/// @file
/// @brief      tap.bits — convert between integers and lists of bits.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <algorithm>

#include "c74_min.h"

using namespace c74::min;

class bits : public object<bits> {
  public:
    MIN_DESCRIPTION{"Convert between integers and lists of bits. In ints2bits mode an integer is "
                    "exploded into a 32-bit list (most-significant bit first); in bits2ints mode "
                    "a list of bits is packed into an integer. In ints2matrixctrl mode a list of "
                    "integers is formatted as [column row state] triples for a matrixctrl."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"&, |, >>, <<, matrixctrl"};

    inlet<>  m_in{this, "(int/list) input"};
    outlet<> m_out{this, "(int/list) output"};
    outlet<> m_dump{this, "(anything) dumpout / error reporting"};

    attribute<symbol> mode{this, "mode", "bits2ints",
                           range{"bits2ints", "ints2bits", "matrixctrl2ints", "ints2matrixctrl"},
                           description{"The conversion to perform."}};

    attribute<int> matrix_width{this, "matrix_width", 8, range{2, 31},
                                description{"Number of columns to use in the matrixctrl modes."}};

    message<> number{this, "int", "Explode an integer into a list of bits (ints2bits mode).",
                     MIN_FUNCTION{const symbol m = mode;
    if (m == "ints2bits") {
        long  value = static_cast<long>(args[0]);
        atoms out(32);
        for (int i = 0; i < 32; ++i) {
            out[31 - i] = static_cast<long>(value & 1);
            value >>= 1;
        }
        m_out.send(out);
    }
    else {
        m_dump.send("error", "wrong mode for int input");
    }
    return {};
}
}
;

message<>    list_msg{this, "list", "Pack a list of bits, or format integers for a matrixctrl.",
                   MIN_FUNCTION{const long argc = static_cast<long>(args.size());
const symbol m = mode;

if (m == "bits2ints") {
    long val = 0;
    for (long i = argc - 1; i >= 0; --i) {
        const long bit = static_cast<long>(args[i]);
        val |= bit << (argc - (i + 1));
    }
    m_out.send(val);
}
else if (m == "ints2matrixctrl") {
    const int width = std::clamp(static_cast<int>(matrix_width), 2, 31);
    for (long j = 0; j < argc; ++j) {
        long value = static_cast<long>(args[j]);
        for (int i = 0; i < width; ++i) {
            m_out.send(atoms{static_cast<long>(i), j, static_cast<long>(value & 1)});
            value >>= 1;
        }
    }
}
else if (m == "matrixctrl2ints") {
    m_dump.send("error", "matrixctrl2ints mode is not yet implemented");
}
else {
    m_dump.send("error", "wrong mode for list input");
}
return {};
}
}
;
}
;

MIN_EXTERNAL(bits);
