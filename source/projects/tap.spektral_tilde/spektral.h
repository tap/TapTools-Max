/// @file
/// @brief      Portable spectral-delay kernel for tap.spektral~ — no Max/Min dependency.
/// @details    Modeled on Native Instruments' Spektral Delay (2001, discontinued): the signal is
///             split by an STFT into frequency bands, and every band gets its own level, delay
///             (up to 12 seconds), and feedback. A Freeze mode mutes the input and recirculates
///             the delay matrix losslessly (the original's signature button). Frequency-domain
///             Mod FX are applied to the input spectrum before the matrix: rotate (cyclic bin
///             shift over time) and smear (per-bin amplitude diffusion — the original's "reverb
///             simulation by smearing the amplitude courses"), plus an LFO assignable to band
///             shift, feedback, or delay scale.
///
///             The STFT core (in-house radix-2 complex FFT, Hann analysis+synthesis windows, 4x
///             overlap, empirically COLA-normalized overlap-add, one-FFT-frame latency) is lifted
///             from tap.nr~ / tap.spectra~ — this kernel moves that machinery into the portable
///             header-only form introduced by grm_comb.h. Per-band edits are applied to bins
///             0..N/2 only and Hermitian-mirrored before the inverse transform (the tap.spectra~
///             template), so the output stays real.
///
///             The delay matrix stores one ring of complex spectra; per band b (bin k):
///                 node[t][k] = level[b]*in[k] + fb[b]*node[t - max(D,1)][k]
///                 wet[k]     = node[t - D][k]
///             which realizes y[t] = level*x[t-D] + fb*y[t-max(D,1)] with a single ring. Delays
///             quantize to the hop (N/4 samples). The dry path is delayed by N samples so dry and
///             wet stay time-aligned through the equal-power mix.
///
///             As in the GRM kernels: parameters ride ramps (sample-rate for gain/mix, frame-rate
///             for everything spectral — overlap-add crossfades frames, so frame-rate changes are
///             click-free), and the 16-slot preset-morph engine snapshots the scalars AND all
///             three band curves. Curve setters resample any-length input to the band count.
///             Setters are safe while audio runs. Allocation-free after prepare().
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace taptools {
    namespace spektral {

        constexpr int    k_max_bands         = 512;
        constexpr int    k_default_bands     = 160; // the original's editable band count
        constexpr int    k_min_bands         = 8;
        constexpr int    k_presets           = 16;
        constexpr int    k_default_fftsize   = 2048; // 1024 internal bands, like the original
        constexpr int    k_min_fftsize       = 256;
        constexpr int    k_max_fftsize       = 8192;
        constexpr int    k_overlap           = 4;
        constexpr double k_max_delay_s       = 12.0; // the original's maximum per-band delay
        constexpr double k_fb_curve_max      = 0.99;
        constexpr double k_band_fmin         = 40.0; // low edge of the log band spacing
        constexpr double k_default_smooth_ms = 20.0;
        constexpr double k_pi                = 3.14159265358979323846;

        enum param_index : int {
            // sample-rate ramps (applied in the time domain)
            p_gain = 0, // output gain, dB
            p_mix,      // 0..100 dry->wet, equal-power (dry is latency-aligned)
            // frame-rate ramps (applied per STFT frame)
            p_level_master, // 0..2, scales the level curve
            p_fb_master,    // 0..2, scales the feedback curve
            p_delayscale,   // 0..2, scales the delay curve
            p_rotate,       // cyclic spectral rotation, bins per second (bipolar)
            p_smear,        // 0..100 amplitude diffusion
            p_lforate,      // Hz
            p_lfodepth,     // 0..100 (bins for bandshift dest; % for feedback/delayscale dests)
            k_num_params
        };

        enum lfo_dest : int {
            dest_off = 0,
            dest_bandshift,  // oscillating spectral shift, +-depth bins
            dest_feedback,   // wobbles the feedback master, +-depth %
            dest_delayscale, // wobbles the delay scale, +-depth %
            k_num_dests
        };

        enum curve_id : int {
            curve_level = 0, // 0..2 linear per band
            curve_delay,     // 0..12000 ms per band
            curve_fb,        // 0..0.99 per band
            k_num_curves
        };

        /// One full parameter snapshot: the scalars plus all three band curves.
        struct params {
            std::array<double, k_num_params>                          v{};
            std::array<std::array<double, k_max_bands>, k_num_curves> curve{};

            static params defaults() {
                params p;
                p.v[p_gain]         = 0.0;
                p.v[p_mix]          = 50.0;
                p.v[p_level_master] = 1.0;
                p.v[p_fb_master]    = 1.0;
                p.v[p_delayscale]   = 1.0;
                p.v[p_rotate]       = 0.0;
                p.v[p_smear]        = 0.0;
                p.v[p_lforate]      = 1.0;
                p.v[p_lfodepth]     = 0.0;
                for (int b = 0; b < k_max_bands; ++b) {
                    p.curve[curve_level][b] = 1.0;
                    p.curve[curve_delay][b] = 0.0;
                    p.curve[curve_fb][b]    = 0.0;
                }
                return p;
            }
        };

        /// Clamp a scalar parameter to its legal range. Gain (dB) is unclamped.
        inline double clamp_param(int index, double value) {
            switch (index) {
            case p_gain:
                return value;
            case p_mix:
                return std::clamp(value, 0.0, 100.0);
            case p_level_master:
            case p_fb_master:
            case p_delayscale:
                return std::clamp(value, 0.0, 2.0);
            case p_rotate:
                return std::clamp(value, -500.0, 500.0);
            case p_smear:
                return std::clamp(value, 0.0, 100.0);
            case p_lforate:
                return std::clamp(value, 0.0, 20.0);
            case p_lfodepth:
                return std::clamp(value, 0.0, 100.0);
            default:
                return value;
            }
        }

        /// Clamp a band-curve value to its legal range.
        inline double clamp_curve(int which, double value) {
            switch (which) {
            case curve_level:
                return std::clamp(value, 0.0, 2.0);
            case curve_delay:
                return std::clamp(value, 0.0, k_max_delay_s * 1000.0);
            case curve_fb:
                return std::clamp(value, 0.0, k_fb_curve_max);
            default:
                return value;
            }
        }

        class spektral_engine {
          public:
            explicit spektral_engine(int fftsize = k_default_fftsize) {
                if (fftsize >= k_min_fftsize && fftsize <= k_max_fftsize && is_power_of_two(fftsize)) {
                    m_fftsize = fftsize;
                }
                const params d = params::defaults();
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = d.v[i];
                }
                for (int c = 0; c < k_num_curves; ++c) {
                    for (int b = 0; b < k_max_bands; ++b) {
                        m_curve[c][b].current = m_curve[c][b].target = d.curve[c][b];
                    }
                }
                m_presets.fill(d);
            }

            // -- lifecycle -------------------------------------------------------------------------------

            /// Allocate everything for the sample rate. Not real-time safe. Snaps all ramps.
            void prepare(double sr) {
                m_sr  = (sr > 0.0) ? sr : 48000.0;
                m_hop = m_fftsize / k_overlap;

                const int N = m_fftsize;
                m_half      = N / 2;

                m_window.assign(N, 0.0);
                for (int k = 0; k < N; ++k) {
                    m_window[k] = 0.5 - 0.5 * std::cos(2.0 * k_pi * k / N);
                }

                // COLA normalization: overlap-add `overlap` copies of window^2, read steady state.
                // (tap.nr~ provenance — makes analysis+synthesis windowing reconstruct unity.)
                std::vector<double> cola(N, 0.0);
                for (int o = 0; o < k_overlap; ++o) {
                    for (int k = 0; k < N; ++k) {
                        cola[(o * m_hop + k) % N] += m_window[k] * m_window[k];
                    }
                }
                m_norm = (cola[N / 2] > 0.0) ? (1.0 / cola[N / 2]) : 1.0;

                m_inbuf.assign(N, 0.0);
                m_outbuf.assign(N, 0.0);
                m_drybuf.assign(N, 0.0);
                m_re.assign(N, 0.0);
                m_im.assign(N, 0.0);
                m_scratch_re.assign(m_half + 1, 0.0);
                m_scratch_im.assign(m_half + 1, 0.0);
                m_smear_mag.assign(m_half + 1, 0.0);
                m_ph_re.assign(m_half + 1, 1.0);
                m_ph_im.assign(m_half + 1, 0.0);

                m_ring_frames = static_cast<int>(std::ceil(k_max_delay_s * m_sr / m_hop)) + 2;
                m_ring_re.assign(static_cast<size_t>(m_ring_frames) * (m_half + 1), 0.0);
                m_ring_im.assign(static_cast<size_t>(m_ring_frames) * (m_half + 1), 0.0);
                m_frame = 0;

                m_bin2band.assign(m_half + 1, 0);
                rebuild_band_map();

                m_pos       = 0;
                m_hopcount  = 0;
                m_lfo_phase = 0.0;
                m_rot_accum = 0.0;
                m_prepared  = true;
                snap();
            }

            /// Zero all signal state (delay matrix, STFT buffers, smear memory); parameters untouched.
            void clear() {
                if (!m_prepared) {
                    return;
                }
                std::fill(m_inbuf.begin(), m_inbuf.end(), 0.0);
                std::fill(m_outbuf.begin(), m_outbuf.end(), 0.0);
                std::fill(m_drybuf.begin(), m_drybuf.end(), 0.0);
                std::fill(m_ring_re.begin(), m_ring_re.end(), 0.0);
                std::fill(m_ring_im.begin(), m_ring_im.end(), 0.0);
                std::fill(m_smear_mag.begin(), m_smear_mag.end(), 0.0);
            }

            /// Finish all ramps immediately.
            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = 0.0;
                    r.remaining = 0;
                }
                for (int c = 0; c < k_num_curves; ++c) {
                    for (int b = 0; b < k_max_bands; ++b) {
                        auto& r     = m_curve[c][b];
                        r.current   = r.target;
                        r.inc       = 0.0;
                        r.remaining = 0;
                    }
                }
                m_gains_dirty = true;
            }

            // -- configuration (modes; not part of the morphable parameter set) ---------------------------

            /// Number of editable bands (8..512). Curves keep their indices; band meanings shift.
            void set_bands(int n) {
                m_bands = std::clamp(n, k_min_bands, k_max_bands);
                if (m_prepared) {
                    rebuild_band_map();
                }
            }
            int bands() const { return m_bands; }

            /// Freeze: mute the matrix input and recirculate losslessly (level -> 0, fb -> exactly 1).
            void set_freeze(bool on) { m_freeze = on; }
            bool freeze() const { return m_freeze; }

            void set_lfodest(int dest) { m_lfodest = std::clamp(dest, 0, k_num_dests - 1); }
            int  lfodest() const { return m_lfodest; }

            void   set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }
            double smooth_ms() const { return m_smooth_ms; }

            // -- scalar parameters -------------------------------------------------------------------------

            void set_param(int index, double value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                const double v = clamp_param(index, value);
                if (is_sample_rate_param(index)) {
                    ramp_to(m_ramp[index], v, static_cast<long>(m_smooth_ms * 0.001 * m_sr));
                }
                else {
                    ramp_to(m_ramp[index], v, smooth_frames());
                }
                if (index == p_gain || index == p_mix) {
                    m_gains_dirty = true;
                }
            }

            void set_gain(double db) { set_param(p_gain, db); }
            void set_mix(double pct) { set_param(p_mix, pct); }
            void set_level_master(double x) { set_param(p_level_master, x); }
            void set_fb_master(double x) { set_param(p_fb_master, x); }
            void set_delayscale(double x) { set_param(p_delayscale, x); }
            void set_rotate(double bins_sec) { set_param(p_rotate, bins_sec); }
            void set_smear(double pct) { set_param(p_smear, pct); }
            void set_lforate(double hz) { set_param(p_lforate, hz); }
            void set_lfodepth(double d) { set_param(p_lfodepth, d); }

            // -- band curves -------------------------------------------------------------------------------

            /// Set a whole curve from `n` points, linearly resampled to the current band count.
            void set_curve(int which, const double* values, size_t n) {
                if (which < 0 || which >= k_num_curves || n == 0 || !values) {
                    return;
                }
                const long frames = smooth_frames();
                for (int b = 0; b < m_bands; ++b) {
                    double v;
                    if (n == 1) {
                        v = values[0];
                    }
                    else {
                        const double x  = static_cast<double>(b) * (n - 1) / (m_bands - 1);
                        const size_t i0 = std::min(static_cast<size_t>(x), n - 2);
                        const double t  = x - static_cast<double>(i0);
                        v               = values[i0] + t * (values[i0 + 1] - values[i0]);
                    }
                    ramp_to(m_curve[which][b], clamp_curve(which, v), frames);
                }
            }

            /// Set a single band of a curve.
            void set_curve_point(int which, int band, double value) {
                if (which < 0 || which >= k_num_curves || band < 0 || band >= m_bands) {
                    return;
                }
                ramp_to(m_curve[which][band], clamp_curve(which, value), smooth_frames());
            }

            /// Current target of one curve point (introspection / tests).
            double curve_value(int which, int band) const {
                if (which < 0 || which >= k_num_curves || band < 0 || band >= k_max_bands) {
                    return 0.0;
                }
                return m_curve[which][band].target;
            }

            // -- presets / morph ---------------------------------------------------------------------------

            bool store_preset(int slot) {
                if (!valid_slot(slot)) {
                    return false;
                }
                params& p = m_presets[slot];
                for (int i = 0; i < k_num_params; ++i) {
                    p.v[i] = m_ramp[i].target;
                }
                for (int c = 0; c < k_num_curves; ++c) {
                    for (int b = 0; b < k_max_bands; ++b) {
                        p.curve[c][b] = m_curve[c][b].target;
                    }
                }
                return true;
            }

            bool recall_preset(int slot, double seconds) {
                if (!valid_slot(slot)) {
                    return false;
                }
                const params& p        = m_presets[slot];
                const long    nsamples = static_cast<long>(std::max(0.0, seconds) * m_sr);
                const long    nframes  = (m_hop > 0) ? nsamples / m_hop : 0;
                for (int i = 0; i < k_num_params; ++i) {
                    ramp_to(m_ramp[i], clamp_param(i, p.v[i]), is_sample_rate_param(i) ? nsamples : nframes);
                }
                for (int c = 0; c < k_num_curves; ++c) {
                    for (int b = 0; b < k_max_bands; ++b) {
                        ramp_to(m_curve[c][b], clamp_curve(c, p.curve[c][b]), nframes);
                    }
                }
                m_gains_dirty = true;
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

            bool morphing() const {
                for (const auto& r : m_ramp) {
                    if (r.remaining > 0) {
                        return true;
                    }
                }
                for (int c = 0; c < k_num_curves; ++c) {
                    for (int b = 0; b < m_bands; ++b) {
                        if (m_curve[c][b].remaining > 0) {
                            return true;
                        }
                    }
                }
                return false;
            }

            // -- introspection -----------------------------------------------------------------------------

            double samplerate() const { return m_sr; }
            int    fftsize() const { return m_fftsize; }
            int    hop() const { return m_hop; }
            int    latency() const { return m_fftsize; } // output = input delayed by one FFT frame

            /// The band containing a frequency (after prepare()).
            int band_of_freq(double hz) const {
                if (!m_prepared) {
                    return 0;
                }
                const int bin = std::clamp(static_cast<int>(std::lround(hz * m_fftsize / m_sr)), 0, m_half);
                return m_bin2band[bin];
            }

            // -- audio -------------------------------------------------------------------------------------

            double process(double in) {
                if (!m_prepared) {
                    return in;
                }

                // sample-rate ramps (gain, mix)
                if (m_ramp[p_gain].remaining > 0 || m_ramp[p_mix].remaining > 0) {
                    tick(m_ramp[p_gain]);
                    tick(m_ramp[p_mix]);
                    m_gains_dirty = true;
                }
                if (m_gains_dirty) {
                    const double g     = std::pow(10.0, m_ramp[p_gain].current * 0.05);
                    const double theta = std::clamp(m_ramp[p_mix].current, 0.0, 100.0) * 0.01 * (k_pi * 0.5);
                    m_dry_gain         = std::cos(theta) * g;
                    m_wet_gain         = std::sin(theta) * g;
                    m_gains_dirty      = (m_ramp[p_gain].remaining > 0 || m_ramp[p_mix].remaining > 0);
                }

                const int N = m_fftsize;

                // dry path, delayed by N samples to stay aligned with the wet path's STFT latency
                const double dry = m_drybuf[m_pos];
                m_drybuf[m_pos]  = in;

                // STFT framing (tap.nr~ provenance)
                m_inbuf[m_pos]   = in;
                const double wet = m_outbuf[m_pos];
                m_outbuf[m_pos]  = 0.0;
                m_pos            = (m_pos + 1) % N;
                if (++m_hopcount >= m_hop) {
                    m_hopcount = 0;
                    process_frame();
                }

                return dry * m_dry_gain + wet * m_wet_gain;
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

            static bool is_power_of_two(int v) { return v > 0 && (v & (v - 1)) == 0; }
            static bool valid_slot(int s) { return s >= 0 && s < k_presets; }

            static bool is_sample_rate_param(int index) { return index == p_gain || index == p_mix; }

            long smooth_frames() const {
                return (m_hop > 0) ? static_cast<long>(m_smooth_ms * 0.001 * m_sr) / m_hop : 0;
            }

            static void ramp_to(ramp& r, double tgt, long steps) {
                if (steps < 1 || tgt == r.current) {
                    r.current   = tgt;
                    r.target    = tgt;
                    r.inc       = 0.0;
                    r.remaining = 0;
                }
                else {
                    r.target    = tgt;
                    r.inc       = (tgt - r.current) / static_cast<double>(steps);
                    r.remaining = steps;
                }
            }

            static void tick(ramp& r) {
                if (r.remaining > 0) {
                    r.current += r.inc;
                    if (--r.remaining == 0) {
                        r.current = r.target;
                    }
                }
            }

            void rebuild_band_map() {
                // log-spaced bands from k_band_fmin to Nyquist; bins below the floor go to band 0
                const double nyq   = m_sr * 0.5;
                const double ratio = std::log(nyq / k_band_fmin);
                for (int k = 0; k <= m_half; ++k) {
                    const double f = k * m_sr / m_fftsize;
                    int          b = 0;
                    if (f > k_band_fmin && ratio > 0.0) {
                        b = static_cast<int>(m_bands * std::log(f / k_band_fmin) / ratio);
                    }
                    m_bin2band[k] = std::clamp(b, 0, m_bands - 1);
                }
            }

            void process_frame() {
                const int N    = m_fftsize;
                const int half = m_half;

                // frame-rate ramps
                for (int i = 0; i < k_num_params; ++i) {
                    if (!is_sample_rate_param(i)) {
                        tick(m_ramp[i]);
                    }
                }
                for (int c = 0; c < k_num_curves; ++c) {
                    for (int b = 0; b < m_bands; ++b) {
                        tick(m_curve[c][b]);
                    }
                }

                // LFO
                const double hop_s = m_hop / m_sr;
                m_lfo_phase += m_ramp[p_lforate].current * hop_s;
                m_lfo_phase -= std::floor(m_lfo_phase);
                const double lfo   = std::sin(2.0 * k_pi * m_lfo_phase); // -1..1
                const double depth = m_ramp[p_lfodepth].current;

                double fb_master  = m_ramp[p_fb_master].current;
                double dly_scale  = m_ramp[p_delayscale].current;
                double shift_bins = 0.0;
                if (m_lfodest == dest_feedback) {
                    fb_master = std::max(0.0, fb_master * (1.0 + lfo * depth * 0.01));
                }
                else if (m_lfodest == dest_delayscale) {
                    dly_scale = std::max(0.0, dly_scale * (1.0 + lfo * depth * 0.01));
                }
                else if (m_lfodest == dest_bandshift) {
                    shift_bins = lfo * depth; // +-depth bins
                }

                // analyze
                for (int k = 0; k < N; ++k) {
                    m_re[k] = m_inbuf[(m_pos + k) % N] * m_window[k];
                    m_im[k] = 0.0;
                }
                fft(m_re, m_im, false);

                // ---- Mod FX on the input spectrum (bins 0..half) ----

                // rotate: cyclic bin shift; the rate accumulates fractionally, applied in whole bins
                m_rot_accum += m_ramp[p_rotate].current * hop_s + shift_bins - m_last_shift;
                m_last_shift = shift_bins; // LFO shift is an offset, not a rate — undo previous offset
                m_rot_accum -= std::floor(m_rot_accum / (half + 1)) * (half + 1); // wrap to [0, half+1)
                const int shift = static_cast<int>(m_rot_accum) % (half + 1);
                if (shift != 0) {
                    for (int k = 0; k <= half; ++k) {
                        int src = k - shift;
                        if (src < 0) {
                            src += half + 1;
                        }
                        m_scratch_re[k] = m_re[src];
                        m_scratch_im[k] = m_im[src];
                    }
                    for (int k = 0; k <= half; ++k) {
                        m_re[k] = m_scratch_re[k];
                        m_im[k] = m_scratch_im[k];
                    }
                }

                // smear: per-bin peak-hold-with-decay on magnitude, phase from the live bin (or the last
                // known phase when the bin is silent) — diffuses amplitude over time, reverb-like
                const double smear = m_ramp[p_smear].current;
                if (smear > 0.0) {
                    const double tail_s = 0.02 * std::pow(400.0, smear * 0.01); // 20 ms .. 8 s
                    const double decay  = std::pow(0.001, hop_s / tail_s);      // -60 dB over tail_s
                    for (int k = 0; k <= half; ++k) {
                        const double mag  = std::sqrt(m_re[k] * m_re[k] + m_im[k] * m_im[k]);
                        const double held = std::max(mag, m_smear_mag[k] * decay);
                        m_smear_mag[k]    = held;
                        if (mag > 1e-12) {
                            m_ph_re[k] = m_re[k] / mag;
                            m_ph_im[k] = m_im[k] / mag;
                        }
                        m_re[k] = held * m_ph_re[k];
                        m_im[k] = held * m_ph_im[k];
                    }
                }
                else {
                    for (int k = 0; k <= half; ++k) {
                        m_smear_mag[k] = 0.0;
                    }
                }

                // ---- the delay matrix ----
                // node[t][k] = level*in[k] + fb*node[t - max(D,1)][k];  wet[k] = node[t - D][k]
                const double level_master = m_ramp[p_level_master].current;
                double*      ring_re_now  = &m_ring_re[static_cast<size_t>(m_frame) * (half + 1)];
                double*      ring_im_now  = &m_ring_im[static_cast<size_t>(m_frame) * (half + 1)];

                for (int k = 0; k <= half; ++k) {
                    const int    b   = m_bin2band[k];
                    const double dms = m_curve[curve_delay][b].current * dly_scale;
                    int          D   = static_cast<int>(std::lround(dms * 0.001 * m_sr / m_hop));
                    D                = std::clamp(D, 0, m_ring_frames - 1);
                    const int Dfb    = std::max(D, 1);

                    double lvl = m_curve[curve_level][b].current * level_master;
                    double fb  = std::min(m_curve[curve_fb][b].current * fb_master, k_fb_curve_max);
                    if (m_freeze) {
                        lvl = 0.0;
                        fb  = 1.0; // lossless recirculation
                    }

                    const size_t fb_idx  = ring_index(m_frame - Dfb, k);
                    const double node_re = lvl * m_re[k] + fb * m_ring_re[fb_idx];
                    const double node_im = lvl * m_im[k] + fb * m_ring_im[fb_idx];
                    ring_re_now[k]       = node_re;
                    ring_im_now[k]       = node_im;

                    if (D == 0) {
                        m_re[k] = node_re;
                        m_im[k] = node_im;
                    }
                    else {
                        const size_t out_idx = ring_index(m_frame - D, k);
                        m_re[k]              = m_ring_re[out_idx];
                        m_im[k]              = m_ring_im[out_idx];
                    }
                }
                m_frame = (m_frame + 1) % m_ring_frames;

                // Hermitian mirror (tap.spectra~ template): DC/Nyquist real, conjugate the upper half.
                m_im[0]    = 0.0;
                m_im[half] = 0.0;
                for (int k = 1; k < half; ++k) {
                    m_re[N - k] = m_re[k];
                    m_im[N - k] = -m_im[k];
                }

                fft(m_re, m_im, true);

                for (int k = 0; k < N; ++k) {
                    m_outbuf[(m_pos + k) % N] += m_re[k] * m_window[k] * m_norm;
                }
            }

            size_t ring_index(int frame, int bin) const {
                int f = frame % m_ring_frames;
                if (f < 0) {
                    f += m_ring_frames;
                }
                return static_cast<size_t>(f) * (m_half + 1) + bin;
            }

            // In-place iterative radix-2 Cooley-Tukey FFT (tap.nr~ provenance). `inverse` divides by N.
            static void fft(std::vector<double>& re, std::vector<double>& im, bool inverse) {
                const int n = static_cast<int>(re.size());

                for (int i = 1, j = 0; i < n; ++i) {
                    int bit = n >> 1;
                    for (; j & bit; bit >>= 1) {
                        j ^= bit;
                    }
                    j ^= bit;
                    if (i < j) {
                        std::swap(re[i], re[j]);
                        std::swap(im[i], im[j]);
                    }
                }

                for (int len = 2; len <= n; len <<= 1) {
                    const double ang = 2.0 * k_pi / len * (inverse ? 1.0 : -1.0);
                    const double wr  = std::cos(ang);
                    const double wi  = std::sin(ang);
                    for (int i = 0; i < n; i += len) {
                        double cwr = 1.0, cwi = 0.0;
                        for (int k = 0; k < len / 2; ++k) {
                            const double vr     = re[i + k + len / 2] * cwr - im[i + k + len / 2] * cwi;
                            const double vi     = re[i + k + len / 2] * cwi + im[i + k + len / 2] * cwr;
                            const double ur     = re[i + k];
                            const double ui     = im[i + k];
                            re[i + k]           = ur + vr;
                            im[i + k]           = ui + vi;
                            re[i + k + len / 2] = ur - vr;
                            im[i + k + len / 2] = ui - vi;
                            const double ncwr   = cwr * wr - cwi * wi;
                            cwi                 = cwr * wi + cwi * wr;
                            cwr                 = ncwr;
                        }
                    }
                }

                if (inverse) {
                    for (int i = 0; i < n; ++i) {
                        re[i] /= n;
                        im[i] /= n;
                    }
                }
            }

            // configuration
            int    m_fftsize{k_default_fftsize};
            int    m_half{k_default_fftsize / 2};
            int    m_hop{k_default_fftsize / k_overlap};
            int    m_bands{k_default_bands};
            int    m_lfodest{dest_off};
            bool   m_freeze{false};
            bool   m_prepared{false};
            double m_sr{48000.0};
            double m_smooth_ms{k_default_smooth_ms};
            double m_norm{1.0};

            // parameters
            std::array<ramp, k_num_params>                          m_ramp;
            std::array<std::array<ramp, k_max_bands>, k_num_curves> m_curve;
            std::array<params, k_presets>                           m_presets;
            bool                                                    m_gains_dirty{true};
            double                                                  m_dry_gain{0.0};
            double                                                  m_wet_gain{0.0};

            // STFT state
            std::vector<double> m_window, m_inbuf, m_outbuf, m_drybuf, m_re, m_im;
            std::vector<double> m_scratch_re, m_scratch_im;
            std::vector<int>    m_bin2band;
            int                 m_pos{0};
            int                 m_hopcount{0};

            // delay matrix
            std::vector<double> m_ring_re, m_ring_im;
            int                 m_ring_frames{1};
            int                 m_frame{0};

            // mod fx state
            std::vector<double> m_smear_mag, m_ph_re, m_ph_im;
            double              m_lfo_phase{0.0};
            double              m_rot_accum{0.0};
            double              m_last_shift{0.0};
        };

    } // namespace spektral
} // namespace taptools
