/// @file
/// @brief      tap.multitap~ — a self-contained multitap delay line on the portable delay kernel.
/// @details    Records the input into one shared circular buffer and sums up to 99 taps, each with
///             its own delay time (ms), gain (dB), and equal-power pan (-1..1), to a stereo bus.
///             Rebuilt on tap::tools::delay::multitap (taptools/delay.h): fractional Hermite reads
///             by default (@interp 1) with the legacy bit-compatible integer-sample truncation at
///             @interp 0, and every per-tap parameter riding a per-sample kernel slew (no zippers).
///
///             The object now has TWO signal outlets (left/right). A center-panned tap (@pan 0,
///             the default) contributes equally to both — so the old mono output is simply the
///             mix of the two busses. Gain is set in dB (converted to linear for the kernel, as
///             the old wrapper did); pure feedforward, no dry path, no master gain — gain staging
///             is the patch's job (see delay.h "Honest limits").
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <algorithm>
#include <cmath>

#include <taptools/delay.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::delay;

class multitap : public object<multitap>, public sample_operator<1, 2> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::multitap m_engine;

  public:
    MIN_DESCRIPTION{"A self-contained multitap delay. Records the input into a buffer and sums up "
                    "to 99 taps, each with its own delay time (ms), gain (dB), and equal-power pan "
                    "(-1..1), to a stereo output pair. A center-panned tap feeds both outlets "
                    "equally; the old mono output is the mix of the two."};
    MIN_TAGS{"delays"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tapin~, tapout~, delay~, tap.delay~, tap.procrastinate~"};

    inlet<>  m_in{this, "(signal) audio input"};
    outlet<> m_out_left{this, "(signal) summed delay taps, left", "signal"};
    outlet<> m_out_right{this, "(signal) summed delay taps, right", "signal"};

    attribute<number> buffersize{
        this, "buffersize", 1000.0,
        description{"Size of the delay buffer in milliseconds (set by the first object argument; "
                    "applied when the DSP chain starts). This bounds the maximum delay time."}};

    attribute<int> taps{this, "taps", 1, setter{MIN_FUNCTION{
                            const int n = std::clamp(static_cast<int>(args[0]), 1, kernel::k_max_taps - 1);
                            m_engine.set_taps(n);
                            return {n};
                        }},
                        description{"Number of active delay taps (1..99)."}};

    attribute<std::vector<number>> delay{this,
                                         "delay",
                                         {0.0},
                                         setter{MIN_FUNCTION{
                                             for (size_t i = 0;
                                                  i < args.size() && i < static_cast<size_t>(kernel::k_max_taps); ++i) {
                                                 m_engine.set_time_ms(static_cast<int>(i), args[i]);
                                             }
                                             return args;
                                         }},
                                         description{"Delay time (ms) for each tap, slewed by the kernel."}};

    attribute<std::vector<number>> gain{
        this,
        "gain",
        {0.0},
        setter{MIN_FUNCTION{
            for (size_t i = 0; i < args.size() && i < static_cast<size_t>(kernel::k_max_taps); ++i) {
                m_engine.set_gain(static_cast<int>(i),
                                  std::pow(10.0, static_cast<double>(args[i]) * 0.05)); // dB -> linear
            }
            return args;
        }},
        description{"Gain (dB) for each tap, slewed by the kernel."}};

    attribute<std::vector<number>> pan{
        this,
        "pan",
        {0.0},
        setter{MIN_FUNCTION{
            for (size_t i = 0; i < args.size() && i < static_cast<size_t>(kernel::k_max_taps); ++i) {
                m_engine.set_pan(static_cast<int>(i), std::clamp(static_cast<double>(args[i]), -1.0, 1.0));
            }
            return args;
        }},
        description{"Equal-power pan (-1 hard left .. 1 hard right, default 0 center) for each "
                    "tap, slewed by the kernel. A center-panned tap feeds both outlets equally; "
                    "the endpoints are exact (a hard-panned tap is absent from the far outlet)."}};

    attribute<int> interp{this, "interp", kernel::interp_hermite, setter{MIN_FUNCTION{
                              const int v = (static_cast<int>(args[0]) == kernel::interp_trunc)
                                                ? kernel::interp_trunc
                                                : kernel::interp_hermite;
                              m_engine.set_interp(v);
                              return {v};
                          }},
                          description{"Interpolation mode for all taps: 1 (default) reads fractional delays with a "
                                      "4-point Hermite; 0 restores the legacy bit-compatible integer-sample "
                                      "truncation."}};

    message<> clear{this, "clear", "Clear the delay buffer.",
                    MIN_FUNCTION{
                        m_engine.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare for the sample rate and buffer size when the DSP chain starts.",
                       MIN_FUNCTION{
                           m_engine.prepare(samplerate(), buffersize);
                           return {};
                       }};

    multitap(const atoms& args = {}) {
        if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
            buffersize = args[0];
        }
        m_engine.prepare(samplerate(), buffersize);
    }

    samples<2> operator()(sample x) {
        double left  = 0.0;
        double right = 0.0;
        m_engine.process(x, left, right);
        return {left, right};
    }
};

MIN_EXTERNAL(multitap);
