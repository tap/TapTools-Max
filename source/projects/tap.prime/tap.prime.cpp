/// @file
/// @brief      tap.prime — generate prime numbers.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 1999-2026 Timothy Place.

#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class prime : public object<prime> {
  public:
    MIN_DESCRIPTION{"Generate prime numbers. A bang (or an integer in the left inlet) steps to "
                    "the next prime number greater than the current value."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"random, expr"};

    inlet<>  m_in_step{this, "(bang/int) step to the next prime number"};
    inlet<>  m_in_set{this, "(int) set the value to step from (no output)"};
    outlet<> m_out{this, "(int) prime number"};
    outlet<> m_dump{this, "(anything) dumpout"};

    argument<int> initial_arg{this, "initial-value", "Initial value from which the first bang will step.",
                              MIN_ARGUMENT_FUNCTION{m_value = static_cast<long>(arg);
}
}
;

message<> bang{this, "bang", "Step to the next prime number.", MIN_FUNCTION{m_value = next_prime(m_value);
m_out.send(m_value);
return {};
}
}
;

message<> number{this, "int",
                 "Left inlet: output the next prime greater than the input. "
                 "Right inlet: set the value to step from, without output.",
                 MIN_FUNCTION{const long value = static_cast<long>(args[0]);
if (inlet == 0) {
    m_value = next_prime(value);
    m_out.send(m_value);
}
else {
    m_value = value;
}
return {};
}
}
;

private:
long m_value{0};

// Faithful port of Jamoma's TTPrime: the smallest prime strictly greater than `value`.
static long next_prime(long value) {
    if (value < 2) {
        return 2;
    }
    if (value == 2) {
        return 3;
    }

    long candidate = value;
    if (candidate % 2 == 0) {
        --candidate; // test only odd numbers
    }

    bool is_prime;
    do {
        is_prime = true;
        candidate += 2;
        const long last = static_cast<long>(std::sqrt(static_cast<double>(candidate)));
        for (long i = 3; i <= last && is_prime; i += 2) {
            if (candidate % i == 0) {
                is_prime = false;
            }
        }
    } while (!is_prime);

    return candidate;
}
}
;

MIN_EXTERNAL(prime);
