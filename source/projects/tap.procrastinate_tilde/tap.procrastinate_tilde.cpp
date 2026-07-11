/// @file
/// @brief      tap.procrastinate~ — cascading idiosyncratic delay/pitch-shift unit.
/// @details    Four pitch-shifter voices (each a tap.shift~-style phasor + two windowed delay taps)
///             chained in cascade — every voice processes the previous voice's output. Each voice has
///             a randomly chosen pitch ratio, window/delay time, gain, and stereo pan, drawn from
///             configurable ranges (regenerated on bang). Faithful reconstruction of the ttblue
///             tt_procrastinate meta-object. The padded-Welch window is the original 256-point
///             half-table mirrored to 512 points; panning is equal-power. DSP is portable C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class procrastinate : public object<procrastinate>, public sample_operator<1, 2> {
  public:
    MIN_DESCRIPTION{"A cascade of four randomized pitch-shifting delay voices. Each voice processes "
                    "the previous one, with a random pitch ratio, delay, gain, and pan within "
                    "configurable ranges (regenerated on bang)."};
    MIN_TAGS{"processors"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.shift~, tap.multitap~, delay~"};

    inlet<>  m_in{this, "(signal) audio input"};
    outlet<> m_out_l{this, "(signal) left output", "signal"};
    outlet<> m_out_r{this, "(signal) right output", "signal"};

    attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

    message<> bang{this, "bang", "Generate a new random set of voice parameters.",
                   MIN_FUNCTION{
                       randomize();
                       return {};
                   }};

    message<> generate_parameters{this, "generate_parameters", "Generate new random parameters.",
                                  MIN_FUNCTION{
                                      randomize();
                                      return {};
                                  }};

    message<> shift_ratio{this, "shift_ratio", "[voice ratio] — set one voice's pitch ratio.",
                          MIN_FUNCTION{
                              if (args.size() >= 2) {
                                  const int v = vclamp(args[0]);
                                  m_shift[v]  = args[1];
                                  update_phasor(v);
                              }
                              return {};
                          }};

    message<> window{this, "window", "[voice ms] — set one voice's window/delay time.",
                     MIN_FUNCTION{
                         if (args.size() >= 2) {
                             const int v = vclamp(args[0]);
                             m_ws[v]     = std::clamp(static_cast<double>(args[1]), 1.0, k_buffersize_ms - 1.0);
                             update_phasor(v);
                         }
                         return {};
                     }};

    message<> gain_range{this, "gain_range", "[index low high] gain range in dB (index 0 = all).",
                         MIN_FUNCTION{
                             set_range(m_gain_low, m_gain_high, args);
                             return {};
                         }};
    message<> pan_range{this, "pan_range", "[index low high] pan range 0..1 (index 0 = all).",
                        MIN_FUNCTION{
                            set_range(m_pan_low, m_pan_high, args);
                            return {};
                        }};
    message<> shift_range{this, "shift_range", "[index low high] pitch ratio range (index 0 = all).",
                          MIN_FUNCTION{
                              set_range(m_shift_low, m_shift_high, args);
                              return {};
                          }};
    message<> delay_range{this, "delay_range", "[index low high] delay range in ms (index 0 = all).",
                          MIN_FUNCTION{
                              set_range(m_delay_low, m_delay_high, args);
                              return {};
                          }};

    message<> clear{this, "clear", "Clear the delay buffers.",
                    MIN_FUNCTION{
                        for (auto& b : m_buffer) {
                            std::fill(b.begin(), b.end(), 0.0);
                        }
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Allocate buffers and recompute rates.",
                       MIN_FUNCTION{
                           allocate();
                           for (int v = 0; v < k_voices; ++v) {
                               update_phasor(v);
                           }
                           return {};
                       }};

    procrastinate(const atoms& = {}) {
        build_window();
        for (int v = 0; v < k_voices; ++v) {
            m_gain_low[v]   = -6.0;
            m_gain_high[v]  = 0.0;
            m_pan_low[v]    = 0.25;
            m_pan_high[v]   = 0.75;
            m_shift_low[v]  = 0.8;
            m_shift_high[v] = 1.2;
            m_delay_low[v]  = 500.0;
            m_delay_high[v] = 2000.0;
            m_ws[v]         = 10000.0;
            m_shift[v]      = 1.0;
        }
        allocate();
        randomize();
    }

    samples<2> operator()(sample x) {
        if (mute) {
            return {0.0, 0.0};
        }

        double l_sum = 0.0, r_sum = 0.0;
        double voice_in = x;

        for (int v = 0; v < k_voices; ++v) {
            const long N = static_cast<long>(m_buffer[v].size());
            if (N < 2) {
                continue;
            }

            const double ph  = m_phase[v];
            double       ph2 = ph + 0.5;
            if (ph2 >= 1.0) {
                ph2 -= 1.0;
            }

            m_buffer[v][m_write[v]] = voice_in;

            const double sr_ms = samplerate() * 0.001;
            const double d1    = read_delay(v, ph * m_ws[v] * sr_ms, N);
            const double d2    = read_delay(v, ph2 * m_ws[v] * sr_ms, N);

            if (++m_write[v] >= N) {
                m_write[v] = 0;
            }
            m_phase[v] += m_phasor_inc[v];
            m_phase[v] -= std::floor(m_phase[v]);

            const double mix = m_gain[v] * (window_lookup(ph) * d1 + window_lookup(ph2) * d2);

            l_sum += mix * std::cos(m_pan[v] * k_half_pi); // equal-power pan
            r_sum += mix * std::sin(m_pan[v] * k_half_pi);
            voice_in = mix; // cascade into the next voice
        }
        return {l_sum, r_sum};
    }

  private:
    static constexpr int    k_voices{4};
    static constexpr double k_buffersize_ms{10000.0};
    static constexpr int    k_window_len{512};
    static constexpr double k_half_pi{1.57079632679489661923};

    std::array<std::vector<double>, k_voices> m_buffer;
    std::array<long, k_voices>                m_write{};
    std::array<double, k_voices>              m_phase{};
    std::array<double, k_voices>              m_phasor_inc{};
    std::array<double, k_voices>              m_ws{};
    std::array<double, k_voices>              m_shift{};
    std::array<double, k_voices>              m_gain{};
    std::array<double, k_voices>              m_pan{};

    std::array<double, k_voices> m_gain_low{}, m_gain_high{};
    std::array<double, k_voices> m_pan_low{}, m_pan_high{};
    std::array<double, k_voices> m_shift_low{}, m_shift_high{};
    std::array<double, k_voices> m_delay_low{}, m_delay_high{};

    std::array<double, k_window_len> m_window{};

    static constexpr double k_half_welch[256] = {
        0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
        0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.006989, 0.014008, 0.021027, 0.028046, 0.035034,
        0.042053, 0.049042, 0.056061, 0.063049, 0.070038, 0.077057, 0.084045, 0.091034, 0.097992, 0.104980, 0.111938,
        0.118927, 0.125885, 0.132812, 0.139771, 0.146698, 0.153656, 0.160583, 0.167480, 0.174408, 0.181305, 0.188202,
        0.195068, 0.201935, 0.208801, 0.215668, 0.222504, 0.229340, 0.236145, 0.242950, 0.249756, 0.256531, 0.263306,
        0.270081, 0.276825, 0.283539, 0.290253, 0.296967, 0.303650, 0.310333, 0.316986, 0.323639, 0.330261, 0.336853,
        0.343445, 0.350037, 0.356598, 0.363129, 0.369659, 0.376160, 0.382660, 0.389130, 0.395569, 0.402008, 0.408417,
        0.414795, 0.421173, 0.427521, 0.433868, 0.440155, 0.446442, 0.452698, 0.458954, 0.465179, 0.471375, 0.477539,
        0.483704, 0.489807, 0.495911, 0.501984, 0.508057, 0.514069, 0.520081, 0.526062, 0.532013, 0.537933, 0.543823,
        0.549683, 0.555542, 0.561340, 0.567139, 0.572906, 0.578644, 0.584351, 0.590027, 0.595673, 0.601288, 0.606873,
        0.612427, 0.617950, 0.623444, 0.628937, 0.634369, 0.639771, 0.645142, 0.650482, 0.655792, 0.661072, 0.666321,
        0.671509, 0.676697, 0.681854, 0.686951, 0.692047, 0.697083, 0.702087, 0.707062, 0.712006, 0.716919, 0.721802,
        0.726624, 0.731415, 0.736176, 0.740906, 0.745605, 0.750244, 0.754883, 0.759460, 0.764008, 0.768494, 0.772980,
        0.777405, 0.781799, 0.786133, 0.790466, 0.794739, 0.798981, 0.803162, 0.807312, 0.811432, 0.815521, 0.819550,
        0.823547, 0.827515, 0.831421, 0.835297, 0.839142, 0.842926, 0.846680, 0.850403, 0.854065, 0.857697, 0.861267,
        0.864807, 0.868317, 0.871765, 0.875183, 0.878540, 0.881866, 0.885162, 0.888397, 0.891602, 0.894745, 0.897858,
        0.900940, 0.903961, 0.906921, 0.909851, 0.912750, 0.915588, 0.918365, 0.921143, 0.923828, 0.926483, 0.929108,
        0.931671, 0.934204, 0.936676, 0.939117, 0.941498, 0.943848, 0.946136, 0.948364, 0.950592, 0.952728, 0.954834,
        0.956909, 0.958893, 0.960876, 0.962799, 0.964661, 0.966492, 0.968262, 0.970001, 0.971680, 0.973297, 0.974884,
        0.976410, 0.977905, 0.979340, 0.980743, 0.982086, 0.983368, 0.984619, 0.985840, 0.986969, 0.988068, 0.989136,
        0.990143, 0.991089, 0.992004, 0.992859, 0.993652, 0.994415, 0.995148, 0.995789, 0.996429, 0.996979, 0.997498,
        0.997955, 0.998383, 0.998749, 0.999084, 0.999329, 0.999573, 0.999725, 0.999847, 0.999939, 0.999969, 0.999969,
        0.999969, 0.999969, 0.999969, 0.999969, 0.999969, 0.999969, 0.999969, 0.999969, 0.999969, 0.999969, 0.999969,
        0.999969, 0.999969, 0.999969,
    };

    static int vclamp(double v) { return std::clamp(static_cast<int>(v), 0, k_voices - 1); }

    static double rand01() { return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX); }

    static double db_to_amp(double db) { return std::pow(10.0, db * 0.05); }

    void build_window() {
        for (int i = 0; i < 256; ++i) {
            m_window[i]       = k_half_welch[i];
            m_window[511 - i] = k_half_welch[i];
        }
    }

    void allocate() {
        const long n = std::max(2L, static_cast<long>(k_buffersize_ms * samplerate() * 0.001));
        for (int v = 0; v < k_voices; ++v) {
            if (static_cast<long>(m_buffer[v].size()) != n) {
                m_buffer[v].assign(n, 0.0);
                m_write[v] = 0;
            }
        }
    }

    void update_phasor(int v) {
        const double freq = -(m_shift[v] - 1.0) * (1.0 / (m_ws[v] * 0.001));
        m_phasor_inc[v]   = freq / samplerate();
    }

    double read_delay(int v, double delay_samples, long N) const {
        const long   di   = static_cast<long>(delay_samples);
        const double frac = delay_samples - static_cast<double>(di);
        const long   i0   = ((m_write[v] - di) % N + N) % N;
        const long   in1  = (i0 + 1) % N;
        return m_buffer[v][in1] * (1.0 - frac) + m_buffer[v][i0] * frac;
    }

    double window_lookup(double phase) const {
        const double index = phase * static_cast<double>(k_window_len);
        int          p1    = static_cast<int>(index);
        const int    p2    = (p1 + 1) & (k_window_len - 1);
        p1 &= (k_window_len - 1);
        const double diff = index - static_cast<double>(static_cast<int>(index));
        return m_window[p2] * diff + m_window[p1] * (1.0 - diff);
    }

    void set_range(std::array<double, k_voices>& lo, std::array<double, k_voices>& hi, const atoms& args) {
        if (args.size() >= 3) {
            const int    index = static_cast<int>(args[0]); // 0 = all voices, else index-1
            const double low = args[1], high = args[2];
            if (index != 0) {
                const int v = std::clamp(index - 1, 0, k_voices - 1);
                lo[v]       = low;
                hi[v]       = high;
            }
            else {
                for (int v = 0; v < k_voices; ++v) {
                    lo[v] = low;
                    hi[v] = high;
                }
            }
        }
    }

    void randomize() {
        for (int v = 0; v < k_voices; ++v) {
            m_gain[v]  = db_to_amp(m_gain_low[v] + (m_gain_high[v] - m_gain_low[v]) * rand01());
            m_pan[v]   = m_pan_low[v] + (m_pan_high[v] - m_pan_low[v]) * rand01();
            m_shift[v] = m_shift_low[v] + (m_shift_high[v] - m_shift_low[v]) * rand01();
            double w   = m_delay_low[v] + (m_delay_high[v] - m_delay_low[v]) * rand01();
            m_ws[v]    = std::clamp(w, 1.0, k_buffersize_ms - 1.0);
            update_phasor(v);
        }
    }
};

constexpr double procrastinate::k_half_welch[256];

MIN_EXTERNAL(procrastinate);
