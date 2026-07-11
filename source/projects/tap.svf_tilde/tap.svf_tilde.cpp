/// @file
/// @brief      tap.svf~ — state-variable filter with LFO modulation and portamento.
/// @details    A stereo Chamberlin state-variable filter (lowpass / highpass / bandpass / notch /
///             peak), 2x oversampled per sample as in the original. The cutoff frequency is driven
///             by a portamento ramp and can be modulated by a built-in vector-rate LFO
///             (sine / triangle / square / ramp). Faithful port of the ttblue tt_svf + tt_lfo +
///             tt_ramp trio — all DSP is portable C++ (no Jamoma).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2004-2026 Timothy Place.

#include <algorithm>
#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class svf : public object<svf>, public vector_operator<> {
  private:
    static constexpr double k_pi{3.14159265358979323846};
    static constexpr double k_twopi{6.28318530717958647692};

    enum filter_mode { mode_lowpass = 0, mode_highpass, mode_bandpass, mode_notch, mode_peak };
    enum lfo_shape { shape_sine = 0, shape_triangle, shape_square, shape_ramp };

    // One Chamberlin SVF state (the four band outputs that carry over between samples).
    struct svf_state {
        double lp{0.0}, hp{0.0}, bp{0.0}, notch{0.0};
        void   clear() { lp = hp = bp = notch = 0.0; }
    };

  public:
    MIN_DESCRIPTION{"A state-variable filter (lowpass / highpass / bandpass / notch / peak) with a "
                    "built-in LFO modulating the cutoff frequency and portamento on frequency changes."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"svf~, biquad~, tap.biquadcalc, onepole~"};

    inlet<>  m_in_l{this, "(signal) left input / control messages"};
    inlet<>  m_in_r{this, "(signal) right input"};
    outlet<> m_out_l{this, "(signal) left filtered output", "signal"};
    outlet<> m_out_r{this, "(signal) right filtered output", "signal"};

    attribute<symbol> type{this,
                           "type",
                           "bandpass",
                           range{"lowpass", "highpass", "bandpass", "notch", "peak"},
                           setter{MIN_FUNCTION{
                               if (args[0] == "lowpass") {
                                   m_mode = mode_lowpass;
                               }
                               else if (args[0] == "highpass") {
                                   m_mode = mode_highpass;
                               }
                               else if (args[0] == "bandpass") {
                                   m_mode = mode_bandpass;
                               }
                               else if (args[0] == "notch") {
                                   m_mode = mode_notch;
                               }
                               else if (args[0] == "peak") {
                                   m_mode = mode_peak;
                               }
                               return args;
                           }},
                           description{"Filter type: lowpass, highpass, bandpass, notch, or peak."}};

    attribute<number> frequency{this, "frequency", 0.0, setter{MIN_FUNCTION{
                                    m_frequency = args[0];
                                    if (m_lfo_depth > (m_frequency - 1.0)) {
                                        m_lfo_depth = m_frequency - 1.0;
                                    }
                                    m_porta_dest = m_frequency;
                                    update_portamento_step();
                                    return {m_frequency};
                                }},
                                description{"Cutoff / center frequency in Hz (ramped via portamento)."}};

    attribute<number> resonance{this, "resonance", 1.0, setter{MIN_FUNCTION{
                                    m_res = static_cast<double>(args[0]) * 0.1;
                                    return args;
                                }},
                                description{"Filter resonance."}};

    attribute<number> lfo_freq_depth{
        this, "lfo.freq.depth", 1.0, setter{MIN_FUNCTION{
            m_lfo_depth = args[0];
            if (m_lfo_depth > (m_frequency - 1.0)) {
                m_lfo_depth = m_frequency - 1.0;
            }
            return {m_lfo_depth};
        }},
        description{"Depth (in Hz) of the LFO modulation applied to the cutoff frequency."}};

    attribute<number> lfo_freq_speed{this, "lfo.freq.speed", 0.0, setter{MIN_FUNCTION{
                                         m_lfo_speed = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 200.0);
                                         return {m_lfo_speed};
                                     }},
                                     description{"Speed (in Hz) of the cutoff-frequency LFO."}};

    attribute<symbol> lfo_freq_shape{this,
                                     "lfo.freq.shape",
                                     "sine",
                                     range{"sine", "triangle", "square", "ramp"},
                                     setter{MIN_FUNCTION{
                                         if (args[0] == "triangle") {
                                             m_lfo_shape = shape_triangle;
                                         }
                                         else if (args[0] == "square") {
                                             m_lfo_shape = shape_square;
                                         }
                                         else if (args[0] == "ramp") {
                                             m_lfo_shape = shape_ramp;
                                         }
                                         else {
                                             m_lfo_shape = shape_sine;
                                         }
                                         return args;
                                     }},
                                     description{"LFO waveform: sine, triangle, square, or ramp."}};

    attribute<bool> lfo_freq_toggle{this, "lfo.freq.toggle", false, description{"Enable the cutoff-frequency LFO."}};

    attribute<number> portamento_time{this, "portamento_time", 5.0, setter{MIN_FUNCTION{
                                          m_porta_ms = args[0];
                                          return args;
                                      }},
                                      description{"Glide time (ms) applied to frequency changes."}};

    message<> clear{this, "clear", "Reset the filter's internal state.",
                    MIN_FUNCTION{
                        m_l.clear();
                        m_r.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Recompute rates when the DSP chain starts.",
                       MIN_FUNCTION{
                           m_l.clear();
                           m_r.clear();
                           update_portamento_step();
                           return {};
                       }};

    void operator()(audio_bundle input, audio_bundle output) override {
        // Per-vector cutoff update: portamento ramp, then optional LFO modulation.
        update_portamento();
        double cf = m_porta_current;
        if (lfo_freq_toggle) {
            cf += lfo_next();
        }
        set_cutoff(cf);

        const long    n    = input.frame_count();
        const double* in_l = input.samples(0);
        const double* in_r = input.samples(1);
        double*       o_l  = output.samples(0);
        double*       o_r  = output.samples(1);

        for (long i = 0; i < n; ++i) {
            o_l[i] = filter_tick(m_l, in_l[i]);
            o_r[i] = filter_tick(m_r, in_r[i]);
        }
    }

  private:
    int    m_mode{mode_bandpass};
    double m_frequency{0.0};

    // Filter coefficients (recomputed once per vector from the current cutoff).
    double m_freq_coef{0.0};
    double m_res{0.1};
    double m_damp{0.0};

    svf_state m_l, m_r;

    // LFO.
    int    m_lfo_shape{shape_sine};
    double m_lfo_phase{0.0};
    double m_lfo_depth{1.0};
    double m_lfo_speed{0.0};

    // Portamento ramp.
    double m_porta_current{0.0};
    double m_porta_dest{0.0};
    double m_porta_step{0.0};
    double m_porta_ms{5.0};

    static double anti_denormal(double x) { return (std::abs(x) < 1e-15) ? 0.0 : x; }

    void set_cutoff(double cf) {
        const double sr   = samplerate();
        const double fc   = MIN_CLAMP(cf, 20.0, sr * 0.5);
        double       freq = 2.0 * std::sin(k_pi * fc / (sr * 2.0));
        if (freq > 0.25) {
            freq = 0.25;
        }
        m_freq_coef = freq;

        const double temp1 = (freq <= 0.0) ? 2.0 : std::min(2.0 / freq - freq * 0.5, 2.0);
        const double temp2 = 2.0 * (1.0 - std::pow(m_res, 0.25));
        m_damp             = std::min(temp1, temp2);
    }

    double filter_tick(svf_state& s, double value) {
        // Two passes (2x oversampling), unrolled as in the original.
        for (int i = 0; i < 2; ++i) {
            s.notch = anti_denormal(value - m_damp * s.bp);
            s.lp    = anti_denormal(s.lp + m_freq_coef * s.bp);
            s.hp    = anti_denormal(s.notch - s.lp);
            s.bp    = anti_denormal(m_freq_coef * s.hp + s.bp);
        }
        switch (m_mode) {
        case mode_highpass:
            return 0.5 * s.hp;
        case mode_bandpass:
            return 0.5 * s.bp;
        case mode_notch:
            return 0.5 * s.notch;
        case mode_peak:
            return 0.5 * (s.lp - s.hp);
        case mode_lowpass:
        default:
            return 0.5 * s.lp;
        }
    }

    double lfo_next() {
        m_lfo_phase += (m_lfo_speed / samplerate()) * vector_size();
        while (m_lfo_phase >= 1.0) {
            m_lfo_phase -= 1.0;
        }
        while (m_lfo_phase < 0.0) {
            m_lfo_phase += 1.0;
        }

        double w;
        switch (m_lfo_shape) {
        case shape_square:
            w = (m_lfo_phase < 0.5) ? 1.0 : -1.0;
            break;
        case shape_ramp:
            w = -1.0 + 2.0 * m_lfo_phase;
            break;
        case shape_triangle: // 0 → +1 → 0 → -1 → 0
            if (m_lfo_phase < 0.25) {
                w = 4.0 * m_lfo_phase;
            }
            else if (m_lfo_phase < 0.75) {
                w = 2.0 - 4.0 * m_lfo_phase;
            }
            else {
                w = 4.0 * m_lfo_phase - 4.0;
            }
            break;
        case shape_sine:
        default:
            w = std::sin(k_twopi * m_lfo_phase);
            break;
        }
        return w * m_lfo_depth;
    }

    void update_portamento_step() {
        const double ramp_samps = (m_porta_ms * 0.001) * samplerate();
        if (ramp_samps <= 0.0) {
            m_porta_current = m_porta_dest;
            m_porta_step    = 0.0;
        }
        else {
            m_porta_step = (m_porta_dest - m_porta_current) / ramp_samps;
        }
    }

    void update_portamento() {
        if (m_porta_step != 0.0) {
            m_porta_current += m_porta_step * vector_size();
            if ((m_porta_step > 0.0 && m_porta_current >= m_porta_dest)
                || (m_porta_step < 0.0 && m_porta_current <= m_porta_dest)) {
                m_porta_current = m_porta_dest;
                m_porta_step    = 0.0;
            }
        }
    }
};

MIN_EXTERNAL(svf);
