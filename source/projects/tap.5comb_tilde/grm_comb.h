/// @file
/// @brief      Portable comb-filter-bank kernel for tap.5comb~ — no Max/Min dependency.
/// @details    Five parallel feedback comb filters modeled on the GRM Tools Classic "Comb Filters"
///             plugin: per-voice frequency / resonance / feedback-loop lowpass, master FREQ/RES/LP
///             multipliers, equal-power dry/wet mix and output gain, plus the modern GRM Comb's
///             Warp (in-loop allpass -> inharmonic partial stretch) and Phase (pickup position ->
///             odd/even harmonic balance), both neutral by default.
///
///             Provenance and deliberate deviations from the legacy tap.5comb~ (an abstraction over
///             five tap.comb~ objects, recovered from git history at b62bba8^):
///             - The 20-name parameter surface (freq/res/lp masters, freq1..5/res1..5/lp1..5,
///               gain, mix) and the 5 Hz effective-frequency floor (its 200 ms buffer) are kept.
///             - Delays are FRACTIONAL (4-point Hermite) and every parameter rides a per-sample
///               linear ramp; the legacy integer-sample, control-rate stepping detuned the combs
///               and zippered on sweeps — the main reason it never sounded like the GRM original.
///             - Resonance maps to ring time (log curve, k_rt60_min..k_rt60_max seconds), not
///               linearly to feedback: the GRM manual describes res as a duration ("longest
///               possible resonance") and a decay map is musical across the whole 0-100 travel.
///             - The feedback loop uses an exact one-pole lowpass coefficient and a DC blocker
///               instead of tap.comb~'s hard +-1 autoclip; feedback is capped just under unity, so
///               res 100 rings effectively forever *cleanly* (the legacy clipper distorted there).
///
///             The preset-morph engine (16 slots, timed interpolation of all parameters, GRM's
///             hallmark) lives here in the kernel so any host gets it.
///
///             All processing is double-precision, per-sample, allocation-free after prepare().
///             Setters are safe to call while audio runs (they only retarget ramps).
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace taptools {
    namespace fivecomb {

        constexpr int    k_voices            = 5;
        constexpr int    k_presets           = 16;  // GRM Classic slot count
        constexpr double k_freq_floor_hz     = 5.0; // legacy 200 ms buffer implied this floor
        constexpr double k_freq_ceil_hz      = 20000.0;
        constexpr double k_fb_max            = 0.99999; // "longest possible resonance": clean, still BIBO-stable
        constexpr double k_min_delay_samples = 2.5;     // Hermite interpolation headroom
        constexpr double k_rt60_min          = 0.02;    // ring time at res -> 0+ (seconds)
        constexpr double k_rt60_max          = 100.0;   // ring time at res == 100 (fb cap makes it ~infinite)
        constexpr double k_warp_coef_max     = 0.85;    // |allpass coefficient| at warp == 100 (negative c:
                                                        // dispersion sits in the audible partial range and
                                                        // stretches upper partials sharp, stiff-string-like)
        constexpr double k_dc_block_r        = 0.999;   // in-loop DC blocker pole (~7 Hz corner @ 48k)
        constexpr double k_wet_norm          = 0.2;     // 1/k_voices wet-sum normalization
        constexpr double k_default_smooth_ms = 20.0;    // anti-zipper ramp for direct setters
        constexpr double k_pi                = 3.14159265358979323846;

        enum param_index : int {
            p_gain = 0,                        // output gain, dB
            p_mix,                             // 0..100 dry->wet, equal-power
            p_freq_master,                     // 0..2, multiplies freq1..5  (legacy name: "freq")
            p_res_master,                      // 0..2, multiplies res1..5   (legacy name: "res")
            p_lp_master,                       // 0..2, multiplies lp1..5    (legacy name: "lp")
            p_warp,                            // 0..100, in-loop allpass partial warp (0 = harmonic/Classic)
            p_phase,                           // 0..100, pickup position (100 = odd harmonics only)
            p_freq1,                           // .. p_freq1 + 4, per-voice frequency, 45..20000 Hz
            p_res1       = p_freq1 + k_voices, // .. p_res1 + 4, per-voice resonance, 0..100
            p_lp1        = p_res1 + k_voices,  // .. p_lp1 + 4, per-voice feedback lowpass cutoff, 0..20000 Hz
            k_num_params = p_lp1 + k_voices    // == 22
        };

        /// One full parameter snapshot — a preset slot, and the unit the morph engine interpolates.
        struct params {
            std::array<double, k_num_params> v{};

            static params defaults() {
                params p;
                p.v[p_gain]                      = 0.0;
                p.v[p_mix]                       = 100.0;
                p.v[p_freq_master]               = 1.0;
                p.v[p_res_master]                = 1.0;
                p.v[p_lp_master]                 = 1.0;
                p.v[p_warp]                      = 0.0;
                p.v[p_phase]                     = 0.0;
                constexpr double freqs[k_voices] = {80.0, 120.0, 160.0, 200.0, 102.0}; // legacy factory preset 1
                for (int i = 0; i < k_voices; ++i) {
                    p.v[p_freq1 + i] = freqs[i];
                    p.v[p_res1 + i]  = 50.0;
                    p.v[p_lp1 + i]   = 20000.0;
                }
                return p;
            }
        };

        /// Clamp a value to the legal range of a parameter. Gain (dB) is unclamped, like the legacy flonum.
        inline double clamp_param(int index, double value) {
            if (index == p_gain) {
                return value;
            }
            if (index == p_mix || index == p_warp || index == p_phase) {
                return std::clamp(value, 0.0, 100.0);
            }
            if (index == p_freq_master || index == p_res_master || index == p_lp_master) {
                return std::clamp(value, 0.0, 2.0);
            }
            if (index >= p_freq1 && index < p_freq1 + k_voices) {
                return std::clamp(value, 45.0, k_freq_ceil_hz);
            }
            if (index >= p_res1 && index < p_res1 + k_voices) {
                return std::clamp(value, 0.0, 100.0);
            }
            if (index >= p_lp1 && index < p_lp1 + k_voices) {
                return std::clamp(value, 0.0, k_freq_ceil_hz);
            }
            return value;
        }

        /// A single feedback comb: fractional-delay circular buffer with a one-pole lowpass, DC blocker,
        /// and first-order allpass (warp) in the feedback path, plus a half-loop pickup tap (phase).
        /// All per-sample parameters are passed in by the bank; the voice owns only signal state.
        class comb_voice {
          public:
            void prepare(double sr) {
                const size_t n = static_cast<size_t>(std::ceil(sr / k_freq_floor_hz)) + 8;
                m_buffer.assign(n, 0.0);
                m_write = 0;
                clear_filters();
            }

            void clear() {
                std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
                clear_filters();
            }

            bool prepared() const { return !m_buffer.empty(); }

            /// @param in         input sample
            /// @param d_read     main loop tap in samples (already warp-compensated), >= 2.5, fractional
            /// @param d_half     pickup tap in samples (half the nominal loop delay), >= 1
            /// @param fb         feedback coefficient, [0, k_fb_max]
            /// @param lp_a       one-pole lowpass coefficient a in state += a*(x - state), [0, 1]
            /// @param ap_c       warp allpass coefficient, [0, k_warp_coef_max]
            /// @param pickup     phase pickup amount, [0, 1]
            double process(double in, double d_read, double d_half, double fb, double lp_a, double ap_c,
                           double pickup) {
                // feedback path: delayed -> lowpass -> DC block -> warp allpass -> * fb
                const double delayed = read_hermite(d_read);

                m_lp_state = anti_denormal(m_lp_state + lp_a * (delayed - m_lp_state));

                const double dc_out = m_lp_state - m_dc_x1 + k_dc_block_r * m_dc_y1;
                m_dc_x1             = m_lp_state;
                m_dc_y1             = anti_denormal(dc_out);

                const double ap_out = ap_c * dc_out + m_ap_x1 - ap_c * m_ap_y1;
                m_ap_x1             = dc_out;
                m_ap_y1             = anti_denormal(ap_out);

                const double y    = anti_denormal(in + fb * ap_out);
                m_buffer[m_write] = y;

                // pickup: subtracting the half-loop tap cancels even harmonics as pickup -> 1
                const double out = y - pickup * read_linear(d_half);

                if (++m_write >= m_buffer.size()) {
                    m_write = 0;
                }
                return out;
            }

          private:
            void clear_filters() {
                m_lp_state = 0.0;
                m_dc_x1 = m_dc_y1 = 0.0;
                m_ap_x1 = m_ap_y1 = 0.0;
            }

            static double anti_denormal(double x) {
                return (std::abs(x) < 1e-15) ? 0.0 : x; // same guard as tap.comb~
            }

            size_t wrap(long i) const {
                const long n = static_cast<long>(m_buffer.size());
                return static_cast<size_t>(((i % n) + n) % n);
            }

            // 4-point, 3rd-order Hermite. Position D samples behind the current write index; needs D >= 2
            // strictly so the youngest point used is the most recently written sample.
            double read_hermite(double d) const {
                const double pos  = static_cast<double>(m_write) - d;
                const double fpos = std::floor(pos);
                const double frac = pos - fpos;
                const long   base = static_cast<long>(fpos);
                const double xm1  = m_buffer[wrap(base - 1)];
                const double x0   = m_buffer[wrap(base)];
                const double x1   = m_buffer[wrap(base + 1)];
                const double x2   = m_buffer[wrap(base + 2)];
                const double c    = (x1 - xm1) * 0.5;
                const double v    = x0 - x1;
                const double w    = c + v;
                const double a    = w + v + (x2 - x0) * 0.5;
                const double b    = w + a;
                return (((a * frac - b) * frac + c) * frac + x0);
            }

            double read_linear(double d) const {
                const double pos  = static_cast<double>(m_write) - d;
                const double fpos = std::floor(pos);
                const double frac = pos - fpos;
                const long   base = static_cast<long>(fpos);
                const double x0   = m_buffer[wrap(base)];
                const double x1   = m_buffer[wrap(base + 1)];
                return x0 + frac * (x1 - x0);
            }

            std::vector<double> m_buffer;
            size_t              m_write{0};
            double              m_lp_state{0.0};
            double              m_dc_x1{0.0}, m_dc_y1{0.0}; // DC blocker: y = x - x1 + R*y1
            double              m_ap_x1{0.0}, m_ap_y1{0.0}; // allpass:    y = c*x + x1 - c*y1
        };

        /// The bank: five voices, 22 per-sample-ramped parameters, and the preset-morph engine.
        class comb_bank {
          public:
            comb_bank() {
                const params d = params::defaults();
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = d.v[i];
                }
                m_presets.fill(d);
            }

            // -- lifecycle -----------------------------------------------------------------------------

            /// (Re)allocate voice buffers for the sample rate, snap all ramps to their targets (a DSP
            /// restart is not a morph), and clear signal state. Not real-time-safe.
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                for (auto& v : m_voice) {
                    v.prepare(m_sr);
                }
                snap();
            }

            /// Zero all delay lines and filter state; parameters are untouched.
            void clear() {
                for (auto& v : m_voice) {
                    v.clear();
                }
            }

            /// Finish all ramps immediately (jump to targets).
            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = 0.0;
                    r.remaining = 0;
                }
                m_ramps_active  = 0;
                m_derived_dirty = true;
            }

            // -- parameter targets (click-free; safe while audio runs) ----------------------------------

            /// Generic path: clamp and ramp to the new value over the anti-zipper smoothing time.
            void set_param(int index, double value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param(index, value), smooth_samples());
            }

            void set_gain(double db) { set_param(p_gain, db); }
            void set_mix(double pct) { set_param(p_mix, pct); }
            void set_freq_master(double x) { set_param(p_freq_master, x); }
            void set_res_master(double x) { set_param(p_res_master, x); }
            void set_lp_master(double x) { set_param(p_lp_master, x); }
            void set_warp(double pct) { set_param(p_warp, pct); }
            void set_phase(double pct) { set_param(p_phase, pct); }
            void set_freq(int voice, double hz) {
                if (valid_voice(voice)) {
                    set_param(p_freq1 + voice, hz);
                }
            }
            void set_res(int voice, double pct) {
                if (valid_voice(voice)) {
                    set_param(p_res1 + voice, pct);
                }
            }
            void set_lp(int voice, double hz) {
                if (valid_voice(voice)) {
                    set_param(p_lp1 + voice, hz);
                }
            }

            /// Anti-zipper ramp time for direct setters, in ms. 0 = instant (useful for tests).
            void   set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }
            double smooth_ms() const { return m_smooth_ms; }

            // -- presets / morph -----------------------------------------------------------------------

            /// Capture the current *targets* (knob positions, not mid-ramp instantaneous values).
            bool store_preset(int slot) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = snap_targets();
                return true;
            }

            /// Morph every parameter from wherever it currently is to the preset, over `seconds`.
            /// Re-targeting mid-morph stays continuous; seconds <= 0 jumps.
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

            /// Programmatic preset load (factory presets, state restore).
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

            // -- introspection ---------------------------------------------------------------------------

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

            // -- audio -----------------------------------------------------------------------------------

            double process(double in) {
                if (!m_voice[0].prepared()) {
                    return in;
                }

                if (m_ramps_active > 0) {
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
                if (m_derived_dirty) {
                    update_derived();
                    m_derived_dirty = (m_ramps_active > 0); // keep recomputing per sample while moving
                }

                double wet = 0.0;
                for (int v = 0; v < k_voices; ++v) {
                    wet += m_voice[v].process(in, m_d_read[v], m_d_half[v], m_fb[v], m_lp_a[v], m_ap_c, m_pickup);
                }

                return in * m_dry_gain + wet * m_wet_gain;
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

            static bool valid_voice(int v) { return v >= 0 && v < k_voices; }
            static bool valid_slot(int s) { return s >= 0 && s < k_presets; }

            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

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

            // Exact phase delay (in samples) of the first-order allpass (c + z^-1)/(1 + c z^-1) at
            // normalized frequency w. Used to keep each voice's fundamental in tune under warp.
            static double allpass_phase_delay(double c, double w) {
                if (w < 1e-9) {
                    return (1.0 - c) / (1.0 + c); // group delay at DC
                }
                const double num = std::atan2(std::sin(w), c + std::cos(w));
                const double den = std::atan2(c * std::sin(w), 1.0 + c * std::cos(w));
                return (num - den) / w;
            }

            // Recompute all derived per-sample DSP values from the (smoothed) parameter values.
            // Runs every sample while any ramp is active, once when everything has settled.
            void update_derived() {
                const auto& val = m_ramp;

                // warp allpass: c = 0 is a pure one-sample delay (linear phase), so warp 0 is exactly the
                // Classic harmonic comb. Negative c disperses the loop — its phase delay falls with
                // frequency, so upper partials round-trip faster and stretch sharp (piano-string-like).
                // The main tap is compensated at each voice's *fundamental*, which therefore stays in tune.
                m_ap_c = -k_warp_coef_max * std::clamp(val[p_warp].current, 0.0, 100.0) * 0.01;

                m_pickup = std::clamp(val[p_phase].current, 0.0, 100.0) * 0.01;

                const double f_ceil = std::min(k_freq_ceil_hz, m_sr / k_min_delay_samples);

                for (int v = 0; v < k_voices; ++v) {
                    const double f =
                        std::clamp(val[p_freq1 + v].current * val[p_freq_master].current, k_freq_floor_hz, f_ceil);
                    const double d_total = m_sr / f; // nominal loop delay
                    const double ap_tau  = allpass_phase_delay(m_ap_c, 2.0 * k_pi * f / m_sr);
                    // At extreme warp x high tuning the loop can't get shorter than the dispersion and the
                    // pitch flattens — physical, and documented in the maxref.
                    m_d_read[v] = std::max(d_total - ap_tau, k_min_delay_samples);
                    m_d_half[v] = std::max(d_total * 0.5, 1.0);

                    // res -> ring time (log curve) -> feedback for the *current* delay, so a voice keeps
                    // its ring time as its frequency sweeps. RT60 relation: fb = 10^(-3 * d / rt60).
                    const double res_eff = std::clamp(val[p_res1 + v].current * val[p_res_master].current, 0.0, 200.0);
                    if (res_eff <= 0.0) {
                        m_fb[v] = 0.0;
                    }
                    else {
                        const double rt60 = k_rt60_min * std::pow(k_rt60_max / k_rt60_min, res_eff * 0.01);
                        m_fb[v]           = std::min(std::pow(10.0, -3.0 * d_total / (rt60 * m_sr)), k_fb_max);
                    }

                    // exact one-pole coefficient (tap.comb~ used the cruder hz*2/sr approximation)
                    const double fc =
                        std::clamp(val[p_lp1 + v].current * val[p_lp_master].current, 0.0, k_freq_ceil_hz);
                    m_lp_a[v] = 1.0 - std::exp(-2.0 * k_pi * fc / m_sr);
                }

                const double g     = std::pow(10.0, val[p_gain].current * 0.05);
                const double theta = std::clamp(val[p_mix].current, 0.0, 100.0) * 0.01 * (k_pi * 0.5);
                m_dry_gain         = std::cos(theta) * g; // equal-power, like tap.crossfade~
                m_wet_gain         = std::sin(theta) * g * k_wet_norm;
            }

            double m_sr{48000.0};
            double m_smooth_ms{k_default_smooth_ms};

            std::array<ramp, k_num_params>   m_ramp;
            std::array<comb_voice, k_voices> m_voice;
            std::array<params, k_presets>    m_presets;
            int                              m_ramps_active{0};
            bool                             m_derived_dirty{true};

            // derived (cached while parameters are settled)
            std::array<double, k_voices> m_d_read{}, m_d_half{}, m_fb{}, m_lp_a{};
            double                       m_ap_c{0.0};
            double                       m_pickup{0.0};
            double                       m_dry_gain{0.0};
            double                       m_wet_gain{0.0};
        };

    } // namespace fivecomb
} // namespace taptools
