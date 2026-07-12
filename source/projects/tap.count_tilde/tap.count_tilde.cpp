/// @file
/// @brief      tap.count~ — count samples while the input is non-zero.
/// @details    Increments a counter on every sample for which the input is non-zero, restarting from
///             low_bound each time the input returns to non-zero after a zero, and pausing while the
///             input is zero. When the count reaches high_bound it either stops (loop on) or clamps
///             (loop off). Faithful port — plain portable C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

class count : public object<count>, public sample_operator<1, 1> {
  public:
    MIN_DESCRIPTION{"Count samples while the input signal is non-zero. The count pauses while the "
                    "input is zero and restarts when it becomes non-zero again."};
    MIN_TAGS{"analysis"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.counter~, count~, edge~"};

    inlet<>  m_in{this, "(signal) audio to analyze"};
    outlet<> m_out{this, "(signal) the current sample count", "signal"};

    attribute<int> low_bound{this, "low_bound", 0, description{"Value the counter restarts from."}};

    attribute<int> high_bound{this, "high_bound", 32000,
                              description{"Value at which counting stops (loop on) or clamps (loop off)."}};

    attribute<bool> active{this, "active", true, setter{MIN_FUNCTION{
                               m_active = args[0];
                               return args;
                           }},
                           description{"Whether the counter is currently running."}};

    attribute<bool> autoreset{this, "autoreset", false,
                              description{"Reset the count to low_bound when the DSP chain starts."}};

    attribute<bool> loop{this, "loop", true,
                         description{"When the count reaches high_bound: on = stop counting, off = clamp."}};

    message<> reset{this, "reset", "Reset the count to low_bound.",
                    MIN_FUNCTION{
                        m_value = low_bound;
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Called when the DSP chain starts.",
                       MIN_FUNCTION{
                           if (autoreset) {
                               m_value = low_bound;
                           }
                           return {};
                       }};

    sample operator()(sample x) {
        const bool non_zero = (x != 0.0);

        if (non_zero) {
            if (!m_active) { // input just went non-zero after being inactive
                m_value  = low_bound;
                m_active = true;
            }
        }
        else {
            m_active = false;
        }

        if (m_active) {
            m_value++;
        }

        if (m_value >= high_bound) {
            if (loop) {
                m_active = false;
            }
            else {
                m_value = high_bound;
            }
        }

        return static_cast<sample>(m_value);
    }

  private:
    long m_value{0};
    bool m_active{true};
};

MIN_EXTERNAL(count);
