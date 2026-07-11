/// @file
/// @brief      tap.zerox~ — count zero crossings.
/// @details    Over a sliding window of `size` samples, counts the number of zero crossings and
///             outputs that count (normalized by the window size) out the first outlet, while the
///             second outlet emits a 1 on each sample where a crossing occurred. Faithful port of
///             Jamoma's TTZerocross analysis unit — DSP is plain portable C++ (no Jamoma).
/// @author     Timothy Place, Trond Lossius
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2008-2026 Timothy Place.

#include <algorithm>

#include "c74_min.h"

using namespace c74::min;

class zerox : public object<zerox>, public sample_operator<1, 2> {
  public:
    MIN_DESCRIPTION{"Count zero crossings. Reports the number of zero crossings over a window of "
                    "samples (first outlet), and triggers on each crossing (second outlet)."};
    MIN_TAGS{"analysis"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.width~, zerox~, edge~"};

    inlet<>  m_in{this, "(signal) input to analyze"};
    outlet<> m_out_count{this, "(signal) number of zero crossings in the window", "signal"};
    outlet<> m_out_trig{this, "(signal) 1 on each sample where a crossing occurred", "signal"};

    attribute<int> size{this, "size", 2000, setter{MIN_FUNCTION{
                            const int s = std::max(1, static_cast<int>(args[0]));
                            m_rsize     = 1.0 / static_cast<double>(s);
                            return {s};
                        }},
                        description{"Number of samples in the analysis window."}};

    message<> clear{this, "clear", "Reset the analysis state.",
                    MIN_FUNCTION{
                        m_last_over_zero = false;
                        m_counter        = 0;
                        m_final          = 0.0;
                        m_location       = 0;
                        return {};
                    }};

    samples<2> operator()(sample x) {
        const bool over_zero = (0.0 < x);
        const bool crossing  = (m_last_over_zero != over_zero);
        m_last_over_zero     = over_zero;

        m_counter += crossing ? 1 : 0;
        m_location++;

        if (m_location >= size) {
            m_final    = m_counter * m_rsize;
            m_location = 0;
            m_counter  = 0;
        }

        return {m_final, crossing ? 1.0 : 0.0};
    }

  private:
    bool   m_last_over_zero{false};
    long   m_counter{0};
    long   m_location{0};
    double m_final{0.0};
    double m_rsize{1.0 / 2000.0}; // reciprocal of the default window size
};

MIN_EXTERNAL(zerox);
