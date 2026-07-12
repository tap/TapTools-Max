/// @file
/// @brief      tap.random~ — generate a random value on each signal trigger.
/// @details    A signal-rate sample-and-hold random generator: each time the input transitions from
///             zero to non-zero, a new random value in [low_bound, high_bound] is generated and held
///             at the output until the next trigger. The random value uses the same generator as the
///             original TapTools object.
/// @note       The original generated its trigger from a per-vector (rather than per-sample) edge
///             test, which only fired correctly at block boundaries; this port performs the edge
///             detection per sample so each zero->non-zero transition produces a fresh value.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <cstdlib>

#include "c74_min.h"

using namespace c74::min;

class random_generator : public object<random_generator>, public sample_operator<1, 1> {
  public:
    MIN_DESCRIPTION{"Generate a random value on each signal trigger. When the input goes from zero "
                    "to non-zero, a new random value between low_bound and high_bound is generated "
                    "and held until the next trigger."};
    MIN_TAGS{"generators"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.random, random, rand~, sah~"};

    inlet<>  m_in{this, "(signal) trigger (zero to non-zero generates a new value)"};
    outlet<> m_out{this, "(signal) the held random value", "signal"};

    attribute<number> low_bound{this, "low_bound", -1.0, description{"Lower bound of the generated random values."}};

    attribute<number> high_bound{this, "high_bound", 1.0, description{"Upper bound of the generated random values."}};

    random_generator(const atoms& = {}) { generate(); }

    sample operator()(sample x) {
        const bool current_non_zero = (x != 0.0);
        if (m_prev_was_zero && current_non_zero) {
            generate();
        }
        m_prev_was_zero = (x == 0.0);
        return m_value;
    }

  private:
    double m_value{0.0};
    bool   m_prev_was_zero{true};

    void generate() {
        long r       = std::rand();
        r            = r * 1103515245 + 12345;
        const long i = (r >> 16) & 077777; // 0..32767
        double     t = static_cast<double>(i) / 32768.0;
        m_value =
            t * (static_cast<double>(high_bound) - static_cast<double>(low_bound)) + static_cast<double>(low_bound);
    }
};

MIN_EXTERNAL(random_generator);
