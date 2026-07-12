/// @file
/// @brief      tap.comb~ — IIR comb filter with a lowpass in the feedback path.
/// @details    A feedback comb filter: the delayed output is lowpass-filtered, optionally clipped,
///             scaled by the feedback coefficient, and summed with the input. Delay can be set in ms
///             (float or signal), and the feedback/decay/cutoff are controllable. Faithful port of
///             the ttblue tt_comb — DSP is portable C++ (no Jamoma).
/// @note       The original coupled feedback<->decay through conversions that were undefined when the
///             decay was unset; this port defaults feedback to the original's audible default (0.9)
///             and guards the conversions so delay changes preserve decay time without blowing up.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class comb : public object<comb>, public vector_operator<> {
  public:
    MIN_DESCRIPTION{"An IIR comb filter with a lowpass filter in the feedback loop. Delay time, "
                    "feedback (or decay time), feedback lowpass cutoff, and autoclipping are all "
                    "controllable."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"comb~, tap.svf~, delay~, teeth~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_delay{this, "(signal/float) delay time in ms"};
    inlet<>  m_in_decay{this, "(float) decay time in seconds"};
    outlet<> m_out{this, "(signal) filtered output", "signal"};

    attribute<number> buffersize{
        this, "buffersize", 200.0,
        description{"Size of the internal delay buffer in milliseconds (fixed at instantiation)."}};

    attribute<number> feedback{this, "feedback", 0.9, setter{MIN_FUNCTION{
                                   m_fb_coef = args[0];
                                   m_decay   = feedback_to_decay(m_fb_coef, m_delay_ms);
                                   return args;
                               }},
                               description{"Feedback coefficient."}};

    attribute<bool> autoclip{this, "autoclip", false, setter{MIN_FUNCTION{
                                 m_clip = args[0] ? 1.0 : 0.0;
                                 return args;
                             }},
                             description{"Clip the feedback signal to +/-1 to keep the filter from blowing up."}};

    attribute<number> delay{this, "delay", 50.0, setter{MIN_FUNCTION{
                                set_delay(args[0], true);
                                return {m_delay_ms};
                            }},
                            description{"Delay time in milliseconds."}};

    attribute<number> decay{this, "decay", 0.0, setter{MIN_FUNCTION{
                                m_decay         = args[0];
                                const double fb = decay_to_feedback(m_decay, m_delay_ms);
                                if (fb != 0.0) {
                                    m_fb_coef = fb;
                                }
                                return args;
                            }},
                            description{"Decay time in seconds (an alternative way to set the feedback)."}};

    attribute<number> lowpass{this, "lowpass", 20000.0, setter{MIN_FUNCTION{
                                  m_lp_hz = args[0];
                                  update_lowpass();
                                  return args;
                              }},
                              description{"Cutoff frequency (Hz) of the lowpass filter in the feedback path."}};

    message<> clear{this, "clear", "Clear the delay buffer and filter memory.",
                    MIN_FUNCTION{
                        reset();
                        return {};
                    }};

    message<> m_number{this, "number", "Set the delay (inlet 2) or decay (inlet 3).",
                       MIN_FUNCTION{
                           if (inlet == 1) {
                               delay = args[0];
                           }
                           else if (inlet == 2) {
                               decay = args[0];
                           }
                           return {};
                       }};

    message<> dspsetup{this, "dspsetup", "Allocate/clear and recompute when the DSP chain starts.",
                       MIN_FUNCTION{
                           allocate();
                           update_lowpass();
                           set_delay(m_delay_ms, false);
                           return {};
                       }};

    comb(const atoms& args = {}) {
        // First positional argument is the buffer size in ms (the others map to the named attributes).
        if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
            buffersize = args[0];
        }
        allocate();
    }

    void operator()(audio_bundle input, audio_bundle output) override {
        if (m_in_delay.has_signal_connection()) {
            set_delay(input.samples(1)[0], true); // per-vector signal-controlled delay
        }

        const long    n   = input.frame_count();
        const double* in  = input.samples(0);
        double*       out = output.samples(0);
        const long    N   = static_cast<long>(m_buffer.size());
        if (N < 1) {
            return;
        }

        for (long i = 0; i < n; ++i) {
            const long read = (m_write - m_delay_samples + N) % N;
            double     fb   = m_buffer[read];

            // lowpass filter in the feedback path
            fb            = anti_denormal(fb * m_lp_coef + m_lp_feedback * (1.0 - m_lp_coef));
            m_lp_feedback = fb;

            if (m_clip != 0.0) {
                fb = std::clamp(fb, -m_clip, m_clip);
            }

            const double y    = anti_denormal(in[i] + m_fb_coef * fb);
            m_buffer[m_write] = y;
            out[i]            = y;

            if (++m_write >= N) {
                m_write = 0;
            }
        }
    }

  private:
    std::vector<double> m_buffer;
    long                m_write{0};

    double m_delay_ms{50.0};
    long   m_delay_samples{1};
    double m_fb_coef{0.9};
    double m_decay{0.0};
    double m_clip{0.0};
    double m_lp_hz{20000.0};
    double m_lp_coef{0.1};
    double m_lp_feedback{0.0};

    static double anti_denormal(double x) { return (std::abs(x) < 1e-15) ? 0.0 : x; }

    // Decay time (seconds) -> feedback coefficient. Returns 0 when undefined (decay == 0).
    static double decay_to_feedback(double decay_time, double delay_ms) {
        const double delay_s = delay_ms * 0.001;
        if (decay_time < 0.0) {
            return -std::pow(10.0, ((delay_s / -decay_time) * -60.0) / 20.0);
        }
        if (decay_time > 0.0) {
            return std::pow(10.0, ((delay_s / decay_time) * -60.0) / 20.0);
        }
        return 0.0;
    }

    static double feedback_to_decay(double fb, double delay_ms) {
        if (fb > 0.0) {
            return (-60.0 / (20.0 * std::log10(fb))) * delay_ms;
        }
        if (fb < 0.0) {
            return (-60.0 / (20.0 * std::log10(std::abs(fb)))) * (-delay_ms);
        }
        return 0.0;
    }

    void allocate() {
        const double sr = samplerate();
        long         n  = static_cast<long>(static_cast<double>(buffersize) * (sr * 0.001));
        if (n < 24) {
            n = 24;
        }
        if (static_cast<long>(m_buffer.size()) != n) {
            m_buffer.assign(n, 0.0);
            m_write = 0;
        }
        // delay can't exceed the buffer
        m_delay_samples = std::clamp(m_delay_samples, 1L, n);
    }

    void update_lowpass() {
        // hertz_to_radians(hz)/pi == hz * 2 / sr
        m_lp_coef = std::clamp(m_lp_hz * 2.0 / samplerate(), 0.0, 1.0);
    }

    // Set the delay; when couple_feedback is true, recompute the feedback to hold the decay time.
    void set_delay(double ms, bool couple_feedback) {
        m_delay_ms      = ms;
        const long n    = std::max(1L, static_cast<long>(m_buffer.size()));
        m_delay_samples = std::clamp(static_cast<long>(m_delay_ms * (samplerate() * 0.001)), 1L, n);
        if (couple_feedback) {
            const double fb = decay_to_feedback(m_decay, m_delay_ms);
            if (fb != 0.0) {
                m_fb_coef = fb;
            }
        }
    }

    void reset() {
        std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
        m_lp_feedback = 0.0;
    }
};

MIN_EXTERNAL(comb);
