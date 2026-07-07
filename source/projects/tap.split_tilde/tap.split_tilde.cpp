/// @file
/// @brief      tap.split~ — route a signal to one of two outlets based on a value range.
/// @details    When the input sample falls within [low, high] it passes out the first outlet (and
///             the second outlet is silent); otherwise it passes out the second outlet (and the
///             first is silent). A third outlet reports the comparison result (1 or 0). The low and
///             high limits may be set by float, by attribute/argument, or driven by signals in the
///             second and third inlets. Faithful port — DSP is plain portable C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

class split : public object<split>, public sample_operator<3, 3> {
  public:
    MIN_DESCRIPTION{"Send a signal to one of two outlets depending on whether it falls within a "
                    "low/high range. A third outlet reports the comparison result (1 or 0)."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"split, thresh~, <~, >~"};

    inlet<>  m_in{this, "(signal) value to be routed"};
    inlet<>  m_in_low{this, "(signal/float) lower limit for the left outlet"};
    inlet<>  m_in_high{this, "(signal/float) upper limit for the left outlet"};
    outlet<> m_out_in{this, "(signal) input if within the limits", "signal"};
    outlet<> m_out_out{this, "(signal) input if outside the limits", "signal"};
    outlet<> m_out_cmp{this, "(signal) comparison result (1 or 0)", "signal"};

    attribute<number> low{this, "low", 0.0, description{"Lower limit of the in-range region."}};

    attribute<number> high{this, "high", 1.0, description{"Upper limit of the in-range region."}};

    argument<number> low_arg{this, "low", "Initial lower limit (same as the low attribute).",
                             MIN_ARGUMENT_FUNCTION{low = arg;
}
}
;

argument<number> high_arg{this, "high", "Initial upper limit (same as the high attribute).",
                          MIN_ARGUMENT_FUNCTION{high = arg;
}
}
;

// Float to the 2nd inlet sets the low limit; to the 3rd inlet sets the high limit.
message<> m_number{this, "number", "Set the low (inlet 2) or high (inlet 3) limit.",
                   MIN_FUNCTION{if (inlet == 1){low = args[0];
}
else if (inlet == 2) {
    high = args[0];
}
return {};
}
}
;

samples<3> operator()(sample x, sample low_sig, sample high_sig) {
    const double lo = m_in_low.has_signal_connection() ? low_sig : static_cast<double>(low);
    const double hi = m_in_high.has_signal_connection() ? high_sig : static_cast<double>(high);

    if (x >= lo && x <= hi) {
        return {x, 0.0, 1.0};
    }
    return {0.0, x, 0.0};
}
}
;

MIN_EXTERNAL(split);
