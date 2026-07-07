/// @file
/// @brief      tap.width~ — measure the width of a pulse.
/// @details    Counts how many consecutive samples the input stays above zero, and when the input
///             falls back to zero (or below) reports that duration in milliseconds, holding the
///             value until the next pulse completes. Faithful port — plain portable C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

class width : public object<width>, public sample_operator<1, 1> {
  public:
    MIN_DESCRIPTION{"Measure pulse width. Reports the duration (in milliseconds) for which the "
                    "input signal remains above zero, updated each time the pulse ends."};
    MIN_TAGS{"analysis"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.zerox~, edge~, thresh~"};

    inlet<>  m_in{this, "(signal) input to be tested"};
    outlet<> m_out{this, "(signal) pulse width in milliseconds", "signal"};

    sample operator()(sample x) {
        if (x > 0.0) {
            m_counter++;
        }
        else if (x <= 0.0 && m_counter != 0.0) {
            m_width   = (1000.0 * m_counter) / samplerate();
            m_counter = 0.0;
        }
        return m_width;
    }

  private:
    double m_counter{0.0}; // number of samples the input has been above zero
    double m_width{0.0};   // last measured width, in milliseconds
};

MIN_EXTERNAL(width);
