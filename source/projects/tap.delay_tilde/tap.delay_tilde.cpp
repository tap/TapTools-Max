/// @file
/// @brief      tap.delay~ — a feedback delay line on the portable delay kernel.
/// @details    A single-channel delay rebuilt on tap::tools::delay::line (taptools/delay.h) — the
///             kernel-first rebuild of the 1999-lineage object (improvements plan §5, decision:
///             rebuild behind the same name). The input is written into a circular buffer sized at
///             instantiation by the optional buffersize argument (ms, applied when the DSP chain
///             starts) and read back @delay milliseconds later through a 4-point Hermite
///             fractional read (@interp 1, the default) or the legacy bit-compatible integer-sample
///             truncation (@interp 0). A @feedback path (DC-blocked, capped at 0.99) and an
///             equal-power @mix (default 100 = wet-only, matching the old object's output) ride
///             per-sample kernel slews, so parameter moves are click-free.
///
///             BREAKING CHANGE (by design): a signal connected to the right inlet now *always*
///             drives the delay time, including at 0.0. The old wrapper treated a 0.0 signal as
///             "use the @delay attribute", which made a genuine zero-time modulation unreachable
///             and turned silence into a mode switch — that trap dies here. With no signal
///             connected, the right inlet takes floats (or the @delay attribute), slewed by the
///             kernel. Note the kernel's structural floor: a zero-time tap is unreachable (1
///             sample in truncation mode, 2.5 samples Hermite — see delay.h "Honest limits").
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing. For
///             multichannel use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 1999-2026 Timothy Place.

#include <algorithm>

#include <taptools/delay.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::delay;

class delay : public object<delay>, public sample_operator<2, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::line m_line;

  public:
    MIN_DESCRIPTION{"A feedback delay line. The input is delayed by a number of milliseconds set "
                    "via the right inlet (signal or float) or the delay attribute, with feedback, "
                    "an equal-power dry/wet mix, and a choice of fractional (Hermite) or legacy "
                    "integer-sample interpolation. A signal in the right inlet always drives the "
                    "delay time, including at 0.0. The optional argument sets the size (ms) of the "
                    "delay buffer, which bounds the maximum delay. For multichannel signals, wrap "
                    "this object in an mc. operator."};
    MIN_TAGS{"delays"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"delay~, tapin~, tapout~, tap.multitap~, tap.comb~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_dt{this, "(signal/float) delay time in milliseconds"};
    outlet<> m_out{this, "(signal) delayed audio output", "signal"};

    attribute<number> buffersize{
        this, "buffersize", 1000.0,
        description{"Size of the delay buffer in milliseconds (set by the first object argument; "
                    "applied when the DSP chain starts). This bounds the maximum delay time."}};

    attribute<number> delaytime{this, "delay", 0.0, setter{MIN_FUNCTION{
                                    const double v = std::max(0.0, static_cast<double>(args[0]));
                                    m_line.set_time_ms(v);
                                    return {v};
                                }},
                                description{"Delay time in milliseconds, slewed by the kernel (click-free). Clamped "
                                            "to the buffer size. Ignored while a signal is connected to the right "
                                            "inlet — the signal always wins, including at 0.0."}};

    attribute<number> feedback{this, "feedback", 0.0, setter{MIN_FUNCTION{
                                   const double v = std::clamp(static_cast<double>(args[0]), 0.0, kernel::k_fb_max);
                                   m_line.set_feedback(v);
                                   return {v};
                               }},
                               description{"Feedback amount (0..0.99). The feedback path is DC-blocked and capped "
                                           "so the loop always decays."}};

    attribute<number> mix{this, "mix", 100.0, setter{MIN_FUNCTION{
                              const double v = std::clamp(static_cast<double>(args[0]), 0.0, 100.0);
                              m_line.set_mix(v);
                              return {v};
                          }},
                          description{"Dry/wet mix in percent (0..100), equal-power. 0 is bitwise dry, 100 (the "
                                      "default, matching the old wet-only object) bitwise wet."}};

    attribute<int> interp{this, "interp", kernel::interp_hermite, setter{MIN_FUNCTION{
                              const int v = (static_cast<int>(args[0]) == kernel::interp_trunc)
                                                ? kernel::interp_trunc
                                                : kernel::interp_hermite;
                              m_line.set_interp(v);
                              return {v};
                          }},
                          description{"Interpolation mode: 1 (default) reads fractional delays with a 4-point "
                                      "Hermite; 0 restores the legacy bit-compatible integer-sample truncation."}};

    message<> m_number{this, "number", "A float in the right inlet sets the delay time in milliseconds.",
                       MIN_FUNCTION{
                           if (inlet == 1) {
                               delaytime = args[0];
                           }
                           return {};
                       }};

    message<> clear{this, "clear", "Clear the delay buffer and the feedback-path filter state.",
                    MIN_FUNCTION{
                        m_line.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare for the sample rate and buffer size when the DSP chain starts.",
                       MIN_FUNCTION{
                           m_line.prepare(samplerate(), buffersize);
                           return {};
                       }};

    delay(const atoms& args = {}) {
        if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
            buffersize = args[0];
        }
        m_line.prepare(samplerate(), buffersize);
    }

    sample operator()(sample x, sample dt) {
        if (m_in_dt.has_signal_connection()) {
            return m_line.process(x, dt); // the time signal always wins, including at 0.0
        }
        return m_line.process(x);
    }
};

MIN_EXTERNAL(delay);
