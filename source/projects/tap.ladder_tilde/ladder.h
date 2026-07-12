/// @file
/// @brief      Portable virtual-analog ladder-filter kernel for tap.ladder~ — no Max/Min dependency.
/// @details    A zero-delay-feedback (topology-preserving-transform) 4-stage transistor-ladder
///             lowpass with per-stage tanh saturation — the virtual-analog sibling of the linear
///             Stilson/Smith model in tap.fourpole~. Design points:
///
///             - Four TPT one-pole stages (g = tan(pi*fc/fs), G = g/(1+g)) with prewarped tuning,
///               so the cutoff stays accurate into the top octaves at any sample rate.
///             - Global feedback k = 4*resonance (resonance reaches 1.1, i.e. k = 4.4), so the
///               filter self-oscillates cleanly at the tuned frequency; the tanh stages bound the
///               amplitude, no clipper needed. (Give it a ping — an idle all-zero state is a fixed
///               point and produces no oscillation on its own.)
///             - Nonlinear ZDF, two solvers: `solver_fast` (default) predicts the feedback value
///               with the linear ZDF closed form and runs one corrective pass through the tanh
///               stages (Huovilainen-flavored); `solver_exact` iterates the true nonlinear loop
///               to convergence with Newton's method (Simper-style circuit-sim accuracy) — the
///               two are audibly identical until drive and resonance are both pushed hard.
///             - Saturation asymmetry (`asym`, 0..1): a small bias in every tanh stage models
///               transistor mismatch and adds the even harmonics of real hardware (symmetric at
///               0; may produce slight signal-dependent DC when engaged — that too is authentic;
///               follow with tap.dcblock~ if it matters downstream).
///             - drive is the input gain into the nonlinearity (the "hit it harder" control);
///               comp is the classic passband-loss compensation (0 = authentic passband droop as
///               resonance rises, 1 = fully compensated).
///             - Multimode via Oberheim-Xpander-style pole mixing over the taps [u, y1..y4]:
///               lp24 / lp12 / bp12 / bp24 / hp12 / hp24. Under saturation the mixes are
///               approximations — exactly as in the analog original.
///             - Oversampling 1x/2x/4x (default 2x): zero-stuff + 4th-order Butterworth
///               anti-imaging on the way up, matching anti-alias cascade before decimation
///               (the tap.verb~ pattern, self-contained here per house rule).
///
///             As in the other TapTools kernels: every parameter rides a per-sample linear ramp
///             (no zipper), a 16-slot preset-morph engine lives in the kernel, and the cutoff can
///             be overridden per sample for signal-rate modulation. Allocation-free after
///             prepare(); setters are safe while audio runs.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace taptools {
    namespace ladder {

        constexpr int    k_presets           = 16;
        constexpr double k_freq_min          = 10.0;
        constexpr double k_freq_max          = 20000.0;
        constexpr double k_res_max           = 1.1; // k = 4.4: comfortably past self-oscillation
        constexpr double k_drive_range_db    = 24.0;
        constexpr double k_default_smooth_ms = 20.0;
        constexpr double k_pi                = 3.14159265358979323846;

        enum param_index : int {
            p_gain = 0,  // output gain, dB
            p_frequency, // cutoff, Hz
            p_resonance, // 0..1.1 (1.0 ~= edge of self-oscillation)
            p_drive,     // input gain into the nonlinearity, dB
            p_comp,      // 0..1 passband-loss compensation
            p_asym,      // 0..1 saturation asymmetry (transistor mismatch -> even harmonics)
            k_num_params
        };

        enum solver_mode : int {
            solver_fast = 0, // linear ZDF prediction + one corrective nonlinear pass (default)
            solver_exact,    // Newton iteration to convergence; audibly different only at extremes
            k_num_solvers
        };

        enum filter_mode : int { mode_lp24 = 0, mode_lp12, mode_bp12, mode_bp24, mode_hp12, mode_hp24, k_num_modes };

        /// One full parameter snapshot — a preset slot, and the unit the morph engine interpolates.
        struct params {
            std::array<double, k_num_params> v{};

            static params defaults() {
                params p;
                p.v[p_gain]      = 0.0;
                p.v[p_frequency] = 1000.0;
                p.v[p_resonance] = 0.0;
                p.v[p_drive]     = 0.0;
                p.v[p_comp]      = 0.0;
                p.v[p_asym]      = 0.0;
                return p;
            }
        };

        /// Clamp a value to the legal range of a parameter. Gain (dB) is unclamped.
        inline double clamp_param(int index, double value) {
            switch (index) {
            case p_gain:
                return value;
            case p_frequency:
                return std::clamp(value, k_freq_min, k_freq_max);
            case p_resonance:
                return std::clamp(value, 0.0, k_res_max);
            case p_drive:
                return std::clamp(value, -k_drive_range_db, k_drive_range_db);
            case p_comp:
                return std::clamp(value, 0.0, 1.0);
            case p_asym:
                return std::clamp(value, 0.0, 1.0);
            default:
                return value;
            }
        }

        class ladder_filter {
          public:
            ladder_filter() {
                const params d = params::defaults();
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = d.v[i];
                }
                m_presets.fill(d);
            }

            // -- lifecycle -------------------------------------------------------------------------------

            /// Set the sample rate, (re)configure the oversampling chain, clear state, snap all ramps.
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                configure_resampler();
                clear();
                snap();
                m_prepared = true;
            }

            /// Zero all filter and resampler state; parameters untouched.
            void clear() {
                m_s1 = m_s2 = m_s3 = m_s4 = 0.0;
                m_up.reset();
                m_down.reset();
            }

            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = 0.0;
                    r.remaining = 0;
                }
                m_ramps_active  = 0;
                m_derived_dirty = true;
            }

            // -- modes (not part of the morphable parameter set) --------------------------------------------

            void set_mode(int mode) { m_mode = std::clamp(mode, 0, k_num_modes - 1); }
            int  mode() const { return m_mode; }

            /// Oversampling factor 1, 2, or 4. Takes effect immediately (state is cleared).
            void set_oversample(int os) {
                const int v = (os >= 4) ? 4 : (os >= 2) ? 2 : 1;
                if (v != m_os) {
                    m_os = v;
                    configure_resampler();
                    clear();
                    m_derived_dirty = true;
                }
            }
            int oversample() const { return m_os; }

            void   set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }
            double smooth_ms() const { return m_smooth_ms; }

            // -- parameter targets (click-free; safe while audio runs) --------------------------------------

            void set_param(int index, double value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param(index, value), static_cast<long>(m_smooth_ms * 0.001 * m_sr));
            }

            void set_gain(double db) { set_param(p_gain, db); }
            void set_frequency(double hz) { set_param(p_frequency, hz); }
            void set_resonance(double r) { set_param(p_resonance, r); }
            void set_drive(double db) { set_param(p_drive, db); }
            void set_comp(double c) { set_param(p_comp, c); }
            void set_asym(double a) { set_param(p_asym, a); }

            void set_solver(int s) { m_solver = std::clamp(s, 0, k_num_solvers - 1); }
            int  solver() const { return m_solver; }

            // -- presets / morph -----------------------------------------------------------------------------

            bool store_preset(int slot) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = snap_targets();
                return true;
            }

            bool recall_preset(int slot, double seconds) {
                if (!valid_slot(slot)) {
                    return false;
                }
                const long n = static_cast<long>(std::max(0.0, seconds) * m_sr);
                for (int i = 0; i < k_num_params; ++i) {
                    ramp_to(i, clamp_param(i, m_presets[slot].v[i]), n);
                }
                return true;
            }

            bool set_preset(int slot, const params& p) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = p;
                return true;
            }

            const params& preset(int slot) const {
                return m_presets[static_cast<size_t>(std::clamp(slot, 0, k_presets - 1))];
            }

            bool morphing() const { return m_ramps_active > 0; }

            params snap_targets() const {
                params p;
                for (int i = 0; i < k_num_params; ++i) {
                    p.v[i] = m_ramp[i].target;
                }
                return p;
            }

            params snap_current() const {
                params p;
                for (int i = 0; i < k_num_params; ++i) {
                    p.v[i] = m_ramp[i].current;
                }
                return p;
            }

            double samplerate() const { return m_sr; }

            // -- audio ---------------------------------------------------------------------------------------

            /// Process one sample with the (ramped) frequency parameter.
            double process(double x) {
                tick_ramps();
                if (m_derived_dirty) {
                    update_derived(m_ramp[p_frequency].current);
                }
                return run(x);
            }

            /// Process one sample with a signal-rate cutoff override (Hz). The coefficient is recomputed
            /// every call on this path; the frequency parameter/ramp is left untouched.
            double process(double x, double cutoff_hz) {
                tick_ramps();
                update_derived(clamp_param(p_frequency, cutoff_hz));
                m_derived_dirty = true; // the cached G belongs to the override, not the parameter
                return run(x);
            }

            void process(const double* in, double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

          private:
            struct ramp {
                double current{0.0};
                double target{0.0};
                double inc{0.0};
                long   remaining{0};
            };

            // RBJ lowpass biquad (Transposed Direct Form II) — two in series make the 4th-order
            // Butterworth anti-image/anti-alias filters for the oversampling chain.
            struct biquad {
                double b0{1.0}, b1{0.0}, b2{0.0}, a1{0.0}, a2{0.0};
                double z1{0.0}, z2{0.0};

                void design_lowpass(double fc_norm, double q) { // fc_norm = fc / fs
                    const double w     = 2.0 * k_pi * fc_norm;
                    const double alpha = std::sin(w) / (2.0 * q);
                    const double cw    = std::cos(w);
                    const double a0    = 1.0 + alpha;
                    b0                 = ((1.0 - cw) * 0.5) / a0;
                    b1                 = (1.0 - cw) / a0;
                    b2                 = b0;
                    a1                 = (-2.0 * cw) / a0;
                    a2                 = (1.0 - alpha) / a0;
                }
                double tick(double x) {
                    const double y = b0 * x + z1;
                    z1             = b1 * x - a1 * y + z2;
                    z2             = b2 * x - a2 * y;
                    return y;
                }
                void reset() { z1 = z2 = 0.0; }
            };

            struct butterworth4 { // 4th-order Butterworth: two biquads with the standard Q pair
                biquad s1, s2;
                void   design(double fc_norm) {
                    s1.design_lowpass(fc_norm, 0.54119610);
                    s2.design_lowpass(fc_norm, 1.30656296);
                }
                double tick(double x) { return s2.tick(s1.tick(x)); }
                void   reset() {
                    s1.reset();
                    s2.reset();
                }
            };

            static bool valid_slot(int s) { return s >= 0 && s < k_presets; }

            static double anti_denormal(double x) {
                return (std::abs(x) < 1e-15) ? 0.0 : x; // house idiom (tap.comb~)
            }

            void ramp_to(int index, double tgt, long nsamples) {
                ramp&      r   = m_ramp[index];
                const bool was = r.remaining > 0;
                if (nsamples < 1 || tgt == r.current) {
                    r.current   = tgt;
                    r.target    = tgt;
                    r.inc       = 0.0;
                    r.remaining = 0;
                }
                else {
                    r.target    = tgt;
                    r.inc       = (tgt - r.current) / static_cast<double>(nsamples);
                    r.remaining = nsamples;
                }
                m_ramps_active += static_cast<int>(r.remaining > 0) - static_cast<int>(was);
                m_derived_dirty = true;
            }

            void tick_ramps() {
                if (m_ramps_active <= 0) {
                    return;
                }
                for (auto& r : m_ramp) {
                    if (r.remaining > 0) {
                        r.current += r.inc;
                        if (--r.remaining == 0) {
                            r.current = r.target;
                            --m_ramps_active;
                        }
                    }
                }
                m_derived_dirty = true;
            }

            void configure_resampler() {
                if (m_os > 1) {
                    // cut just below the original Nyquist, normalized to the oversampled rate
                    const double fc_norm = 0.45 / m_os;
                    m_up.design(fc_norm);
                    m_down.design(fc_norm);
                }
            }

            // Recompute the DSP coefficients from the (smoothed) parameter values for a given cutoff.
            void update_derived(double cutoff_hz) {
                const double fs_os = m_sr * m_os;
                const double fc    = std::min(cutoff_hz, 0.49 * fs_os);
                const double g     = std::tan(k_pi * fc / fs_os); // prewarped
                m_g                = g / (1.0 + g);
                m_k                = 4.0 * m_ramp[p_resonance].current;
                m_in_gain       = std::pow(10.0, m_ramp[p_drive].current * 0.05) * (1.0 + m_ramp[p_comp].current * m_k);
                m_out_gain      = std::pow(10.0, m_ramp[p_gain].current * 0.05);
                m_sat_bias      = 0.3 * m_ramp[p_asym].current; // transistor-mismatch operating-point shift
                m_sat_dc        = std::tanh(m_sat_bias);        // subtracted so sat(0) == 0 exactly
                m_derived_dirty = (m_ramps_active > 0);
            }

            // Biased saturator: symmetric tanh at asym 0; a shifted operating point (real ladders'
            // transistor mismatch) adds even harmonics as asym rises.
            double sat(double v) const { return std::tanh(v + m_sat_bias) - m_sat_dc; }

            // One TPT one-pole step: y = G*(x - s) + s; s advances trapezoidally.
            static double tpt(double& s, double x, double G) {
                const double v = (x - s) * G;
                const double y = v + s;
                s              = anti_denormal(y + v);
                return y;
            }

            // Linear ZDF prediction of the feedback value (each linear stage: y = G*x + (1-G)*s).
            double predict_linear(double L) const {
                const double G  = m_g;
                const double G2 = G * G;
                const double B  = 1.0 - G;
                const double S  = G2 * G * B * m_s1 + G2 * B * m_s2 + G * B * m_s3 + B * m_s4;
                return (G2 * G2 * L + S) / (1.0 + m_k * G2 * G2);
            }

            // Trial evaluation of the saturating ladder for a guessed feedback value — no state mutation.
            double y4_trial(double L, double guess) const {
                const double G  = m_g;
                const double B  = 1.0 - G;
                const double u  = sat(L - m_k * guess);
                const double y1 = G * u + B * m_s1;
                const double y2 = G * sat(y1) + B * m_s2;
                const double y3 = G * sat(y2) + B * m_s3;
                return G * sat(y3) + B * m_s4;
            }

            // Newton iteration on F(g) = y4_trial(g) - g, seeded by the linear prediction. Falls back to
            // the seed if the derivative degenerates; the tanh-bounded system keeps |g| small.
            double solve_exact(double L) {
                const double seed = predict_linear(L);
                double       g    = std::clamp(seed, -3.0, 3.0);
                for (int it = 0; it < 12; ++it) {
                    const double F = y4_trial(L, g) - g;
                    if (std::abs(F) < 1e-12) {
                        break;
                    }
                    constexpr double eps = 1e-7;
                    const double     d_f = ((y4_trial(L, g + eps) - (g + eps)) - F) / eps;
                    if (std::abs(d_f) < 1e-9) {
                        return seed;
                    }
                    g = std::clamp(g - F / d_f, -3.0, 3.0);
                }
                return g;
            }

            // The nonlinear ladder core at the oversampled rate.
            double core(double x) {
                const double G = m_g;
                const double L = m_in_gain * x;

                const double y4_est = (m_solver == solver_exact) ? solve_exact(L) : predict_linear(L);

                // commit pass through the saturating stages (for solver_exact this reproduces the
                // converged trial values while advancing the integrator states)
                const double t0 = sat(L - m_k * y4_est);
                const double y1 = tpt(m_s1, t0, G);
                const double y2 = tpt(m_s2, sat(y1), G);
                const double y3 = tpt(m_s3, sat(y2), G);
                const double y4 = tpt(m_s4, sat(y3), G);

                // Xpander-style pole mixing over [t0, y1, y2, y3, y4]
                static constexpr double k_c_mix[k_num_modes][5] = {
                    {0.0, 0.0, 0.0, 0.0, 1.0},   // lp24
                    {0.0, 0.0, 1.0, 0.0, 0.0},   // lp12
                    {0.0, 2.0, -2.0, 0.0, 0.0},  // bp12
                    {0.0, 0.0, 4.0, -8.0, 4.0},  // bp24
                    {1.0, -2.0, 1.0, 0.0, 0.0},  // hp12
                    {1.0, -4.0, 6.0, -4.0, 1.0}, // hp24
                };
                const double* m = k_c_mix[m_mode];
                return m[0] * t0 + m[1] * y1 + m[2] * y2 + m[3] * y3 + m[4] * y4;
            }

            double run(double x) {
                double y;
                if (m_os == 1) {
                    y = core(x);
                }
                else {
                    // zero-stuff + anti-image filter up, core at the high rate, anti-alias + decimate down
                    y = 0.0;
                    for (int j = 0; j < m_os; ++j) {
                        const double up = m_up.tick(j == 0 ? x * m_os : 0.0);
                        y               = m_down.tick(core(up));
                    }
                }
                return y * m_out_gain;
            }

            // configuration
            double m_sr{48000.0};
            double m_smooth_ms{k_default_smooth_ms};
            int    m_mode{mode_lp24};
            int    m_os{2};
            int    m_solver{solver_fast};
            bool   m_prepared{false};

            // parameters
            std::array<ramp, k_num_params> m_ramp;
            std::array<params, k_presets>  m_presets;
            int                            m_ramps_active{0};
            bool                           m_derived_dirty{true};

            // derived
            double m_g{0.05};
            double m_k{0.0};
            double m_in_gain{1.0};
            double m_out_gain{1.0};
            double m_sat_bias{0.0};
            double m_sat_dc{0.0};

            // state
            double       m_s1{0.0}, m_s2{0.0}, m_s3{0.0}, m_s4{0.0};
            butterworth4 m_up, m_down;
        };

    } // namespace ladder
} // namespace taptools
