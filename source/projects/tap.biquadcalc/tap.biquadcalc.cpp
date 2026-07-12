/// @file
/// @brief      tap.biquadcalc — calculate coefficients for the biquad~ object.
/// @details    Generates the five filter coefficients (a0 a1 a2 b1 b2, in Max's biquad~ ordering)
///             from a filter type, cutoff/center frequency, gain, and Q, using the well-known
///             Robert Bristow-Johnson "Audio EQ Cookbook" formulas. This is control-rate math (the
///             original was nominally an MSP object only so it could observe the patch's sample
///             rate); here the sample rate is taken from the global setting and may be overridden.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2004-2026 Timothy Place.

#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class biquadcalc : public object<biquadcalc> {
  private:
    static constexpr double k_twopi{6.28318530717958647692};
    bool m_initialized{false}; // declared first: keeps default attribute setters from emitting at construction

  public:
    MIN_DESCRIPTION{"Calculate coefficients for biquad~. Generates filter coefficients from a "
                    "filter type, frequency, gain, and Q using the RBJ Audio EQ Cookbook formulas."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"biquad~, filtergraph~, filtercoeff~"};

    inlet<>  m_in{this, "(anything) set attribute values"};
    outlet<> m_out{this, "(list) the biquad~ coefficients"};
    outlet<> m_dump{this, "(anything) dumpout"};

    biquadcalc(const atoms& = {}) { m_initialized = true; }

    attribute<symbol> filter_type{this, "type", "lowpass",
                                  range{"lowpass", "highpass", "bandpass", "bandstop", "peaknotch", "lowshelf",
                                        "highshelf", "resonant", "allpass"},
                                  description{"The kind of filter algorithm to use."}};

    attribute<number> cf{this, "cf", 1000.0, setter{MIN_FUNCTION{
                             trigger_if(1);
                             return args;
                         }},
                         description{"The center or cutoff frequency of the filter."}};

    attribute<number> gain{this, "gain", 0.0, setter{MIN_FUNCTION{
                               trigger_if(2);
                               return args;
                           }},
                           description{"Gain of the filter in decibels (where applicable to the filter type)."}};

    attribute<number> q{this, "q", 1.0, setter{MIN_FUNCTION{
                            trigger_if(3);
                            return args;
                        }},
                        description{"The Q (narrowness/steepness) of the filter."}};

    attribute<int> sr{this, "sr", static_cast<int>(c74::max::sys_getsr()),
                      description{"Sample rate used for the coefficient calculation. Overridden by sr_override "
                                  "when that is non-zero."}};

    attribute<int> sr_override{this, "sr_override", 0,
                               description{"When non-zero, overrides the sample rate used for the calculation."}};

    attribute<int> trigger{this, "trigger", 4, range{0, 4},
                           description{"When the coefficients are recalculated: 0 = only on bang, 1 = cf, 2 = gain, "
                                       "3 = q, 4 = any of cf/gain/q."}};

    message<> bang{this, "bang", "Calculate the coefficients and send them out the left outlet.",
                   MIN_FUNCTION{
                       calculate();
                       return {};
                   }};

  private:
    void trigger_if(int which) {
        if (m_initialized && (trigger == which || trigger == 4)) {
            calculate();
        }
    }

    void calculate() {
        const double Fs      = (sr_override > 0) ? static_cast<double>(sr_override) : static_cast<double>(sr);
        const double f0      = cf;
        const double d_bgain = gain;
        const double Q       = q;
        const double S       = q; // the "q" attribute doubles as shelf slope

        const double A   = std::pow(10.0, d_bgain / 40.0);
        const double w0  = k_twopi * f0 / Fs;
        const double cw0 = std::cos(w0);
        const double sw0 = std::sin(w0);

        const symbol t = filter_type;

        double alpha;
        if (t == "lowshelf" || t == "highshelf") {
            alpha = sw0 / 2.0 * std::sqrt((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0);
        }
        else {
            alpha = sw0 / (2.0 * Q);
        }

        double a0, a1, a2, b1, b2; // Max biquad~ naming (opposite of the RBJ convention)
        double temp;

        if (t == "lowpass") {
            temp = 1.0 + alpha;
            a0   = ((1.0 - cw0) / 2.0) / temp;
            a1   = (1.0 - cw0) / temp;
            a2   = ((1.0 - cw0) / 2.0) / temp;
            b1   = (-2.0 * cw0) / temp;
            b2   = (1.0 - alpha) / temp;
        }
        else if (t == "highpass") {
            temp = 1.0 + alpha;
            a0   = ((1.0 + cw0) / 2.0) / temp;
            a1   = (-(1.0 + cw0)) / temp;
            a2   = ((1.0 + cw0) / 2.0) / temp;
            b1   = (-2.0 * cw0) / temp;
            b2   = (1.0 - alpha) / temp;
        }
        else if (t == "bandpass") {
            temp = 1.0 + alpha;
            a0   = alpha / temp;
            a1   = 0.0;
            a2   = -alpha / temp;
            b1   = (-2.0 * cw0) / temp;
            b2   = (1.0 - alpha) / temp;
        }
        else if (t == "bandstop") {
            temp = 1.0 + alpha;
            a0   = 1.0 / temp;
            a1   = (-2.0 * cw0) / temp;
            a2   = 1.0 / temp;
            b1   = (-2.0 * cw0) / temp;
            b2   = (1.0 - alpha) / temp;
        }
        else if (t == "peaknotch") {
            temp = 1.0 + alpha / A;
            a0   = (1.0 + alpha * A) / temp;
            a1   = (-2.0 * cw0) / temp;
            a2   = (1.0 - alpha * A) / temp;
            b1   = (-2.0 * cw0) / temp;
            b2   = (1.0 - alpha / A) / temp;
        }
        else if (t == "lowshelf") {
            temp = (A + 1.0) + (A - 1.0) * cw0 + 2.0 * std::sqrt(A) * alpha;
            a0   = (A * ((A + 1.0) - (A - 1.0) * cw0 + 2.0 * std::sqrt(A) * alpha)) / temp;
            a1   = (2.0 * A * ((A - 1.0) - (A + 1.0) * cw0)) / temp;
            a2   = (A * ((A + 1.0) - (A - 1.0) * cw0 - 2.0 * std::sqrt(A) * alpha)) / temp;
            b1   = (-2.0 * ((A - 1.0) + (A + 1.0) * cw0)) / temp;
            b2   = ((A + 1.0) + (A - 1.0) * cw0 - 2.0 * std::sqrt(A) * alpha) / temp;
        }
        else if (t == "highshelf") {
            temp = (A + 1.0) - (A - 1.0) * cw0 + 2.0 * std::sqrt(A) * alpha;
            a0   = (A * ((A + 1.0) + (A - 1.0) * cw0 + 2.0 * std::sqrt(A) * alpha)) / temp;
            a1   = (-2.0 * A * ((A - 1.0) + (A + 1.0) * cw0)) / temp;
            a2   = (A * ((A + 1.0) + (A - 1.0) * cw0 - 2.0 * std::sqrt(A) * alpha)) / temp;
            b1   = (2.0 * ((A - 1.0) - (A + 1.0) * cw0)) / temp;
            b2   = ((A + 1.0) - (A - 1.0) * cw0 - 2.0 * std::sqrt(A) * alpha) / temp;
        }
        else if (t == "allpass") {
            temp = 1.0 + alpha;
            a0   = (1.0 - alpha) / temp;
            a1   = (-2.0 * cw0) / temp;
            a2   = 1.0;
            b1   = (-2.0 * cw0) / temp;
            b2   = (1.0 - alpha) / temp;
        }
        else {
            cerr << "invalid filter type specified" << endl;
            return;
        }

        m_out.send({a0, a1, a2, b1, b2});
    }
};

MIN_EXTERNAL(biquadcalc);
