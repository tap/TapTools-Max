/// @file
/// @brief      tap.radians~ — convert between hertz, radians, and degrees.
/// @details    Works at signal rate (left/signal outlet) and on individual floats (right/float
///             outlet). The four modes match the original TapTools object, whose conversions came
///             from the ttblue tt_audio_base helpers:
///                 0: hertz   -> radians   ( hz  * pi / (sr/2) )
///                 1: radians -> hertz     ( rad * sr / 2pi )
///                 2: degrees -> radians   ( deg * pi / 180 )
///                 3: radians -> degrees   ( rad * 180 / pi )
///             DSP is plain portable C++ — Min is used only to wire the object into Max.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

class radians : public object<radians>, public sample_operator<1, 1> {
  private:
    static constexpr double k_pi{3.14159265358979323846};
    static constexpr double k_twopi{6.28318530717958647692};

    enum conversion { hz_to_rad = 0, rad_to_hz = 1, deg_to_rad = 2, rad_to_deg = 3 };

  public:
    MIN_DESCRIPTION{"Convert to/from radians. Converts between hertz, radians, and degrees at "
                    "either signal rate or for individual float values, selected by the mode "
                    "attribute."};
    MIN_TAGS{"math"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.biquadcalc, atodb~, mtof"};

    inlet<>  m_in{this, "(signal/float) input value to convert"};
    outlet<> m_out_sig{this, "(signal) converted output", "signal"};
    outlet<> m_out_float{this, "(float) converted output (for float input)"};

    attribute<int> mode{this, "mode", hz_to_rad, range{hz_to_rad, rad_to_deg},
                        description{"Conversion: 0 = hertz->radians, 1 = radians->hertz, 2 = degrees->radians, "
                                    "3 = radians->degrees."}};

    argument<int> mode_arg{this, "mode", "Initial conversion mode (same as the mode attribute).",
                           MIN_ARGUMENT_FUNCTION{mode = static_cast<int>(arg);
}
}
;

// Float input is converted and sent out the dedicated float outlet.
message<> number{this, "number", "Convert a single value and output it as a float.",
                 MIN_FUNCTION{m_out_float.send(convert(static_cast<double>(args[0])));
return {};
}
}
;

sample operator()(sample x) {
    return convert(x);
}

private:
double convert(double value) const {
    const double sr = samplerate();
    switch (mode) {
    case rad_to_hz:
        return (value * sr) / k_twopi;
    case deg_to_rad:
        return (value * k_pi) / 180.0;
    case rad_to_deg:
        return (value * 180.0) / k_pi;
    case hz_to_rad:
    default:
        return value * (k_pi / (sr * 0.5));
    }
}
}
;

MIN_EXTERNAL(radians);
