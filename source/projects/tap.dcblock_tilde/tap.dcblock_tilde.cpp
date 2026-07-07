/// @file
/// @brief      tap.dcblock~ — remove DC offset from an audio signal.
/// @details    A one-pole / one-zero high-pass (DC blocking) filter:
///                 y[n] = x[n] - x[n-1] + R * y[n-1]
///             The coefficient R = 0.9997 matches the original Jamoma TTDCBlock that the
///             classic tap.dcblock~ wrapped, so behavior is preserved. The DSP here is plain
///             portable C++ — Min is used only to wire the object into Max. For multichannel
///             use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2008-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

class dcblock : public object<dcblock>, public sample_operator<1, 1> {
  public:
    MIN_DESCRIPTION{"Filter out DC offset from a signal. A lightweight DC-blocking high-pass "
                    "filter. For multichannel signals, wrap this object in an mc. operator."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"onepole~, biquad~, filtercoeff~, hi~"};

    inlet<>  m_in{this, "(signal) audio input"};
    outlet<> m_out{this, "(signal) DC-blocked audio output", "signal"};

    attribute<bool> bypass{this, "bypass", false, description{"Pass the input through unprocessed."}};

    attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

    message<> clear{this, "clear",
                    "Reset the filter's internal memory. "
                    "Useful for resetting the filter if it has blown up.",
                    MIN_FUNCTION{m_x1 = 0.0;
    m_y1 = 0.0;
    return {};
}
}
;

sample operator()(sample x) {
    if (bypass) {
        return x;
    }

    const sample y = x - m_x1 + k_coefficient * m_y1;
    m_x1           = x;
    m_y1           = y;

    return mute ? 0.0 : y;
}

private:
static constexpr sample k_coefficient{0.9997}; ///< feedback coefficient (matches Jamoma TTDCBlock)
sample                  m_x1{0.0};             ///< previous input  (x[n-1])
sample                  m_y1{0.0};             ///< previous output (y[n-1])
}
;

MIN_EXTERNAL(dcblock);
