/// @file
/// @brief      tap.counter~ — count signal transitions.
/// @details    Each time the input rises from zero to a non-zero value the count is advanced (up or
///             down, per the direction attribute), wrapping around between low_bound and high_bound.
///             The current count is output as a signal. Faithful port — plain portable C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

class counter : public object<counter>, public sample_operator<1, 1> {
  public:
    MIN_DESCRIPTION{"Count signal transitions. Advances a count (up or down, with wraparound) "
                    "each time the input goes from zero to non-zero, and outputs it as a signal."};
    MIN_TAGS{"analysis"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.count~, counter, edge~"};

    inlet<>  m_in{this, "(signal) audio to analyze"};
    outlet<> m_out{this, "(signal) the current count", "signal"};

    attribute<int> low_bound{this, "low_bound", 0,
                             description{"Lowest value of the count (wraps here when counting down)."}};

    attribute<int> high_bound{this, "high_bound", 2000000,
                              description{"Highest value of the count (wraps here when counting up)."}};

    attribute<symbol> direction{this, "direction", "up", range{"up", "down"}, description{"Count up or down."}};

    attribute<int> init_value{this, "init_value", 0, setter{MIN_FUNCTION{m_stored = args[0];
    return args;
}
}
, description {
    "Value the count is initialized and reset to."
}
}
;

message<> set{this, "set", "Set the current count.", MIN_FUNCTION{m_stored = static_cast<int>(args[0]);
return {};
}
}
;

message<> reset{this, "reset", "Reset the count to init_value.", MIN_FUNCTION{m_stored = init_value;
return {};
}
}
;

sample operator()(sample x) {
    if (x != 0.0 && m_last == 0.0) { // rising edge from zero
        const symbol dir = direction;
        if (dir == "up") {
            m_stored++;
            if (m_stored > high_bound) {
                m_stored = low_bound;
            }
        }
        else {
            m_stored--;
            if (m_stored < low_bound) {
                m_stored = high_bound;
            }
        }
    }
    m_last = x;
    return static_cast<sample>(m_stored);
}

private:
long   m_stored{0};
double m_last{0.0};
}
;

MIN_EXTERNAL(counter);
