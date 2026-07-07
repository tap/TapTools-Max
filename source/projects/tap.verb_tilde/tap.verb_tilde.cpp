/// @file
/// @brief      tap.verb~ — stereo algorithmic reverb.
/// @details    A Moorer-style reverberator, faithfully reconstructed from the ttblue tt_verb core:
///             an 18-tap early-reflection pattern feeds six parallel comb filters (each with a
///             damping lowpass in its feedback loop and an LFO chorusing its delay), summed and run
///             through a Schroeder allpass and an output lowpass, then crossfaded with the dry signal
///             and gain-scaled. Two independent cores (left/right), decorrelated by the original's
///             prime-"deviate" of each delay, give a stereo image. DSP is portable C++ (no Jamoma).
/// @note       This port covers the reverb core plus the DC blocker, the (default-on) stereo
///             look-ahead limiter, the clip stage, and internal oversampling. Oversampling runs the
///             reverb core at an integer multiple (1/2/4/8) of the host rate; it defaults to 1 (off),
///             so the default sound matches the legacy object exactly.
/// @note       Oversampling deviation from the legacy wrapper: the original tap.verb~ exposed a
///             "downsample" attribute that ran the core at a *lower* rate (sr/factor) using a crude
///             sample-and-hold decimator (tt_downsample) and a zero-order-hold reconstructor
///             (tt_upsample) — no antialiasing at all. That direction (run cheaper/grungier) is
///             inverted here to true oversampling (run the core at a *higher* rate for cleaner,
///             alias-suppressed feedback), per the revival roadmap. The crude SAH filters are
///             therefore replaced with a clean, documented antialiasing path: gain-compensated
///             zero-stuff + a cascaded one-pole lowpass on the way up, and a matching lowpass +
///             decimation on the way down (cutoff at the host Nyquist).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

namespace {

    long nearest_prime(long value) {
        if (value < 2) {
            return 2;
        }
        for (long candidate = value;; ++candidate) {
            bool is_prime = true;
            for (long i = 2; i * i <= candidate; ++i) {
                if (candidate % i == 0) {
                    is_prime = false;
                    break;
                }
            }
            if (is_prime) {
                return candidate;
            }
        }
    }

    // Randomize a millisecond value by +/-1 ms and snap it to the nearest prime number of samples.
    double deviate(double value_ms, double sr) {
        double v = value_ms + (2.0 * (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX)) - 1.0);
        v        = v * 0.001 * sr; // ms -> samples
        v        = static_cast<double>(nearest_prime(static_cast<long>(v)));
        return (v / sr) * 1000.0; // samples -> ms
    }

    double db_to_amp(double db) {
        return std::pow(10.0, db * 0.05);
    }

    // A small cascade of one-pole lowpass stages, used as the antialiasing/anti-imaging filter for
    // the oversampling path. A 4-stage cascade gives a steep-enough rolloff to suppress images and
    // aliasing for the 2x/4x/8x factors offered here while staying cheap and unconditionally stable.
    struct aa_filter {
        static constexpr int k_stages = 4;
        double               state[k_stages]{};
        double               coef{0.5};

        // cutoff_hz at sample rate sr (the oversampled rate); one-pole coefficient.
        void set(double cutoff_hz, double sr) { coef = std::clamp(cutoff_hz * 2.0 / sr, 0.0, 1.0); }
        void clear() {
            for (auto& s : state) {
                s = 0.0;
            }
        }

        double process(double x) {
            for (int i = 0; i < k_stages; ++i) {
                state[i] = x * coef + state[i] * (1.0 - coef);
                x        = state[i];
            }
            return x;
        }
    };

    // One mono Moorer reverb core.
    class verb_core {
      public:
        void prepare(double sr) {
            m_sr               = sr;
            const double sr_ms = sr * 0.001;

            m_er_buffer.assign(std::max<long>(2, static_cast<long>(100.0 * sr_ms)), 0.0); // 100 ms ER buffer
            for (auto& c : m_comb) {
                c.buffer.assign(std::max<long>(2, static_cast<long>(220.0 * sr_ms)), 0.0); // 200 ms + margin
            }

            const double ap_len = std::max<long>(2, static_cast<long>(7.0 * sr_ms)); // 7 ms allpass
            m_ap_ff.assign(ap_len, 0.0);
            m_ap_fb.assign(ap_len, 0.0);
            m_ap_delay = static_cast<long>(5.98 * sr_ms);

            configure();
        }

        void set_delay(double ms) {
            m_delay = ms;
            configure();
        }
        void set_decay(double sec) {
            m_decay = sec;
            configure();
        }
        void set_damping(double hz) {
            m_damping = hz;
            configure();
        }
        void set_lowpass(double hz) {
            m_lowpass_hz = hz;
            m_lp_coef    = std::clamp(hz * 2.0 / m_sr, 0.0, 1.0);
        }
        void set_modfreq(double hz) {
            for (auto& c : m_comb) {
                c.lfo_inc = deviate_or(hz) / m_sr;
            }
        }
        void set_moddepth(double ms) {
            for (auto& c : m_comb) {
                c.mod_depth = ms;
            }
        }

        void clear() {
            std::fill(m_er_buffer.begin(), m_er_buffer.end(), 0.0);
            m_er_write = 0;
            for (auto& c : m_comb) {
                std::fill(c.buffer.begin(), c.buffer.end(), 0.0);
                c.write    = 0;
                c.lp_state = 0.0;
            }
            std::fill(m_ap_ff.begin(), m_ap_ff.end(), 0.0);
            std::fill(m_ap_fb.begin(), m_ap_fb.end(), 0.0);
            m_ap_pos   = 0;
            m_lp_state = 0.0;
        }

        // Process one sample; returns the wet reverb signal.
        double process(double in, bool use_er) {
            const double sr_ms = m_sr * 0.001;
            const long   N     = static_cast<long>(m_er_buffer.size());

            // --- early reflections (18-tap) ---
            double er = in;
            if (use_er) {
                m_er_buffer[m_er_write] = in;
                er                      = 0.0;
                for (const auto& tap : k_er_taps) {
                    long d = static_cast<long>(tap.delay_ms * sr_ms);
                    long r = ((m_er_write - d) % N + N) % N;
                    er += m_er_buffer[r] * tap.gain;
                }
                er *= k_er_master;
                if (++m_er_write >= N) {
                    m_er_write = 0;
                }
            }

            // --- six modulated comb filters fed by the early reflections ---
            double sum = er;
            for (auto& c : m_comb) {
                c.lfo_phase += c.lfo_inc;
                c.lfo_phase -= std::floor(c.lfo_phase);
                const double mod = (0.5 + 0.5 * std::sin(6.28318530717958647692 * c.lfo_phase)) * c.mod_depth;

                const long c_n    = static_cast<long>(c.buffer.size());
                const long d      = std::clamp(static_cast<long>((c.delay_base + mod) * sr_ms), 1L, c_n - 1);
                const long rd     = ((c.write - d) % c_n + c_n) % c_n;
                double     fb     = c.buffer[rd];
                fb                = fb * m_damp_coef + c.lp_state * (1.0 - m_damp_coef); // damping lowpass in the loop
                c.lp_state        = fb;
                fb                = std::clamp(fb, -1.0, 1.0); // comb autoclip
                const double y    = er + c.fb_coef * fb;
                c.buffer[c.write] = y;
                if (++c.write >= c_n) {
                    c.write = 0;
                }
                sum += y;
            }

            double wet = sum * 0.125; // mixer master gain

            // --- Schroeder allpass ---
            wet = allpass(wet);

            // --- output damping lowpass ---
            m_lp_state = wet * m_lp_coef + m_lp_state * (1.0 - m_lp_coef);
            return m_lp_state;
        }

      private:
        struct comb {
            std::vector<double> buffer;
            long                write{0};
            double              lp_state{0.0};
            double              delay_base{50.0};
            double              fb_coef{0.8};
            double              lfo_phase{0.0};
            double              lfo_inc{0.0};
            double              mod_depth{0.1};
        };
        struct er_tap {
            double delay_ms;
            double gain;
        };

        static constexpr double                 k_er_master{0.0630957}; // -24 dB
        static constexpr std::array<er_tap, 18> k_er_taps{{{4.3, 0.841},
                                                           {21.5, 0.504},
                                                           {22.5, 0.491},
                                                           {26.8, 0.379},
                                                           {27.0, 0.380},
                                                           {29.8, 0.346},
                                                           {45.8, 0.289},
                                                           {48.5, 0.272},
                                                           {57.2, 0.192},
                                                           {58.7, 0.193},
                                                           {59.5, 0.217},
                                                           {61.2, 0.181},
                                                           {70.7, 0.180},
                                                           {70.8, 0.181},
                                                           {72.6, 0.176},
                                                           {74.1, 0.142},
                                                           {75.3, 0.167},
                                                           {79.7, 0.134}}};
        static constexpr std::array<double, 6>  k_delay_mult{0.641027, 0.717950, 0.782052, 0.871796, 0.923078, 1.0};

        double m_sr{44100.0};
        double m_delay{100.0}, m_decay{3.5}, m_damping{20000.0}, m_lowpass_hz{15000.0};
        double m_damp_coef{0.8}, m_lp_coef{0.5};

        std::vector<double> m_er_buffer;
        long                m_er_write{0};

        std::array<comb, 6> m_comb;

        std::vector<double>     m_ap_ff, m_ap_fb;
        long                    m_ap_pos{0}, m_ap_delay{0};
        static constexpr double k_ap_gain{0.7};

        double m_lp_state{0.0};

        double deviate_or(double v) { return deviate(v, m_sr); }

        void configure() {
            m_damp_coef = std::clamp(m_damping * 2.0 / m_sr, 0.0, 1.0);
            m_lp_coef   = std::clamp(m_lowpass_hz * 2.0 / m_sr, 0.0, 1.0);
            for (int i = 0; i < 6; ++i) {
                m_comb[i].delay_base = deviate(m_delay * k_delay_mult[i], m_sr);
                const double decay_s = deviate(m_decay * 1000.0, m_sr) * 0.001; // deviate works in ms
                const double delay_s = m_comb[i].delay_base * 0.001;
                m_comb[i].fb_coef    = (decay_s > 0.0) ? std::pow(10.0, ((delay_s / decay_s) * -60.0) / 20.0) : 0.0;
            }
        }

        double allpass(double in) {
            const long len = static_cast<long>(m_ap_ff.size());
            if (len < 2) {
                return in;
            }
            m_ap_ff[m_ap_pos] = in;
            const long   r    = ((m_ap_pos - m_ap_delay) % len + len) % len;
            const double y    = m_ap_ff[r] - k_ap_gain * in + k_ap_gain * m_ap_fb[r];
            m_ap_fb[m_ap_pos] = y;
            if (++m_ap_pos >= len) {
                m_ap_pos = 0;
            }
            return y;
        }
    };

    constexpr std::array<verb_core::er_tap, 18> verb_core::k_er_taps;
    constexpr std::array<double, 6>             verb_core::k_delay_mult;

} // namespace

class verb : public object<verb>, public sample_operator<2, 2> {
    // Declared first so it is initialized before any attribute setter can run during construction.
    // Guards reconfigure_oversampling() from touching cores that aren't built yet; the constructor
    // body flips it true and performs the initial configuration.
    bool m_ready{false};

  public:
    MIN_DESCRIPTION{"A stereo algorithmic reverb (Moorer-style: early reflections + six modulated "
                    "comb filters + allpass + damping), with a dry/wet mix and output gain."};
    MIN_TAGS{"effects"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"yafr2, gigaverb~, tap.comb~"};

    inlet<>  m_in_l{this, "(signal) left input"};
    inlet<>  m_in_r{this, "(signal) right input"};
    outlet<> m_out_l{this, "(signal) left output", "signal"};
    outlet<> m_out_r{this, "(signal) right output", "signal"};

    attribute<number> mix{this, "mix", 100.0,
                          setter{MIN_FUNCTION{m_mix = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 100.0) * 0.01;
    return args;
}
}
, description {
    "Dry/wet mix, 0 (dry) to 100 (wet)."
}
}
;
attribute<number> gain{this, "gain", 0.0, setter{MIN_FUNCTION{m_gain = db_to_amp(args[0]);
return args;
}
}
, description {
    "Output gain in decibels."
}
}
;
attribute<number> delay{this, "delay", 100.0, setter{MIN_FUNCTION{m_l.set_delay(args[0]);
m_r.set_delay(args[0]);
return args;
}
}
, description {
    "Base comb delay time in milliseconds (sets the room size)."
}
}
;
attribute<number> decay{this, "decay", 3.5, setter{MIN_FUNCTION{m_l.set_decay(args[0]);
m_r.set_decay(args[0]);
return args;
}
}
, description {
    "Reverb decay time in seconds."
}
}
;
attribute<number> damping{this, "damping", 20000.0, setter{MIN_FUNCTION{m_l.set_damping(args[0]);
m_r.set_damping(args[0]);
return args;
}
}
, description {
    "Cutoff (Hz) of the lowpass in each comb's feedback loop."
}
}
;
attribute<number> lowpass{this, "lowpass", 15000.0, setter{MIN_FUNCTION{m_l.set_lowpass(args[0]);
m_r.set_lowpass(args[0]);
return args;
}
}
, description {
    "Output lowpass cutoff in Hz."
}
}
;
attribute<number> modfreq{this, "modfreq", 0.1, setter{MIN_FUNCTION{m_l.set_modfreq(args[0]);
m_r.set_modfreq(args[0]);
return args;
}
}
, description {
    "Comb-delay modulation frequency in Hz."
}
}
;
attribute<number> moddepth{this, "moddepth", 0.1, setter{MIN_FUNCTION{m_l.set_moddepth(args[0]);
m_r.set_moddepth(args[0]);
return args;
}
}
, description {
    "Comb-delay modulation depth in milliseconds."
}
}
;
attribute<bool>   er{this, "er", true, description{"Include the early-reflection stage."}};
attribute<bool>   dcblock{this, "dcblock", true, description{"Apply a DC blocker to the output."}};
attribute<bool>   clip{this, "clip", false, description{"Hard-clip the output to +/-1."}};
attribute<bool>   limit{this, "limit", true, description{"Apply a look-ahead limiter to the output."}};
attribute<number> limiter_threshold{this, "limiter_threshold", 0.0,
                                    setter{MIN_FUNCTION{m_lim_threshold = db_to_amp(args[0]);
return args;
}
}
, description {
    "Limiter threshold in decibels."
}
}
;
attribute<int> limiter_lookahead{
    this, "limiter_lookahead", 100,
    setter{MIN_FUNCTION{m_lim_lookahead = std::clamp(static_cast<int>(args[0]), 1, k_lim_size - 1);
m_lim_recip = 1.0 / static_cast<double>(m_lim_lookahead);
return {m_lim_lookahead};
}
}
, description {
    "Limiter look-ahead in samples (1-255)."
}
}
;
attribute<number> limiter_release{this, "limiter_release", 1000.0, setter{MIN_FUNCTION{m_lim_release = args[0];
set_recover();
return args;
}
}
, description {
    "Limiter release time in milliseconds."
}
}
;
attribute<int> oversampling{this, "oversampling", 1, setter{MIN_FUNCTION{int v = static_cast<int>(args[0]);
// Snap to the legacy factor set {1, 2, 4, 8}; anything else rounds down to the nearest.
if (v >= 8) {
    v = 8;
}
else if (v >= 4) {
    v = 4;
}
else if (v >= 2) {
    v = 2;
}
else {
    v = 1;
}
m_oversampling = v;
if (m_ready) { // skip during attribute construction; constructor calls it once cores exist
    reconfigure_oversampling();
}
return {v};
}
}
, description {
    "Internal oversampling factor (1, 2, 4, or 8). When >1 the reverb core runs at "
    "that multiple of the host sample rate, with antialiasing up/downsampling around "
    "it [default: 1 = off]."
}
}
;
attribute<bool> bypass{this, "bypass", false, description{"Pass the input through unprocessed."}};
attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

message<> clear{this, "clear", "Clear the reverb's internal state.", MIN_FUNCTION{m_l.clear();
m_r.clear();
m_up_l.clear();
m_up_r.clear();
m_down_l.clear();
m_down_r.clear();
reset_limiter();
return {};
}
}
;

message<> dspsetup{this, "dspsetup", "Allocate buffers and recompute for the current sample rate.",
                   MIN_FUNCTION{reconfigure_oversampling();
reset_limiter();
push_all();
return {};
}
}
;

verb(const atoms& = {}) {
    m_ready = true;
    reconfigure_oversampling();
    reset_limiter();
}

samples<2> operator()(sample in_l, sample in_r) {
    if (mute) {
        return {0.0, 0.0};
    }
    if (bypass) {
        return {in_l, in_r};
    }

    double wl, wr;
    if (m_oversampling == 1) {
        // Factor 1: bypass the resampler entirely so the path is bit-identical to no oversampling.
        wl = m_l.process(in_l, er);
        wr = m_r.process(in_r, er);
    }
    else {
        // Upsample by zero-stuffing (gain-compensated so the passband level is preserved), run
        // the core once per oversampled subsample, then decimate through the anti-aliasing filter.
        const int    N = m_oversampling;
        const double g = static_cast<double>(N); // zero-stuff gain compensation
        for (int k = 0; k < N; ++k) {
            const double xl = (k == 0) ? in_l * g : 0.0;
            const double xr = (k == 0) ? in_r * g : 0.0;
            const double ul = m_up_l.process(xl);
            const double ur = m_up_r.process(xr);
            const double cl = m_l.process(ul, er);
            const double cr = m_r.process(ur, er);
            wl              = m_down_l.process(cl); // running the decimation lowpass on every subsample;
            wr              = m_down_r.process(cr); // the value retained after the last subsample is the output
        }
    }

    // equal-power dry/wet mix
    const double wet_g = std::sin(m_mix * 1.57079632679489661923);
    const double dry_g = std::cos(m_mix * 1.57079632679489661923);
    double       out_l = (in_l * dry_g + wl * wet_g) * m_gain;
    double       out_r = (in_r * dry_g + wr * wet_g) * m_gain;

    if (dcblock) {
        out_l = dc_block(out_l, m_dc_l);
        out_r = dc_block(out_r, m_dc_r);
    }
    if (limit) {
        limit_stereo(out_l, out_r);
    }
    if (clip) {
        out_l = std::clamp(out_l, -1.0, 1.0);
        out_r = std::clamp(out_r, -1.0, 1.0);
    }
    return {out_l, out_r};
}

private:
struct dc_state {
    double x1{0.0}, y1{0.0};
};

verb_core m_l, m_r;
double    m_mix{1.0};
double    m_gain{1.0};
dc_state  m_dc_l, m_dc_r;

// --- oversampling ---
int       m_oversampling{1};
aa_filter m_up_l, m_up_r;     // anti-imaging lowpass after zero-stuffing (upsample)
aa_filter m_down_l, m_down_r; // anti-aliasing lowpass before decimation (downsample)

// Re-prepare the reverb cores for the (possibly oversampled) core rate and set the
// antialiasing filter cutoffs. Called from the constructor, dspsetup, and on factor change.
void reconfigure_oversampling() {
    const double host_sr = samplerate();
    const double core_sr = host_sr * static_cast<double>(m_oversampling);

    m_l.prepare(core_sr);
    m_r.prepare(core_sr);
    push_all();

    // Cutoff at (just under) the host Nyquist so images/aliasing above it are suppressed.
    const double cutoff = host_sr * 0.45;
    for (auto* f : {&m_up_l, &m_up_r, &m_down_l, &m_down_r}) {
        f->set(cutoff, core_sr);
    }
    m_up_l.clear();
    m_up_r.clear();
    m_down_l.clear();
    m_down_r.clear();
}

static double dc_block(double x, dc_state& s) {
    const double y = x - s.x1 + 0.9997 * s.y1;
    s.x1           = x;
    s.y1           = y;
    return y;
}

void push_all() {
    for (auto* c : {&m_l, &m_r}) {
        c->set_delay(delay);
        c->set_decay(decay);
        c->set_damping(damping);
        c->set_lowpass(lowpass);
        c->set_modfreq(modfreq);
        c->set_moddepth(moddepth);
    }
}

// --- stereo look-ahead limiter (exponential recovery), matching tap.limi~ ---
static constexpr int k_lim_size{256};

double m_lim_threshold{1.0};
int    m_lim_lookahead{100};
double m_lim_recip{1.0 / 100.0};
double m_lim_release{1000.0};
double m_lim_recover{0.0};
double m_lim_buf1[k_lim_size]{};
double m_lim_buf2[k_lim_size]{};
double m_lim_gain[k_lim_size]{};
long   m_lim_bp{0};
double m_lim_last{1.0};

void set_recover() {
    m_lim_recover = 1000.0 / (m_lim_release * samplerate());
    if (m_lim_recover == 0.0) {
        m_lim_recover = 1000.0 / (100000.0 * samplerate());
    }
    m_lim_recover *= 0.707; // exponential
}

void reset_limiter() {
    for (int i = 0; i < k_lim_size; ++i) {
        m_lim_buf1[i] = 0.0;
        m_lim_buf2[i] = 0.0;
        m_lim_gain[i] = 1.0;
    }
    m_lim_bp   = 0;
    m_lim_last = 1.0;
    set_recover();
}

void limit_stereo(double& l, double& r) {
    m_lim_buf1[m_lim_bp] = l;
    m_lim_buf2[m_lim_bp] = r;

    const double peak    = std::max(std::abs(l), std::abs(r));
    const double rising  = m_lim_last + m_lim_recover * ((m_lim_last > 0.01) ? m_lim_last : 1.0);
    double       maybe   = (rising > m_lim_threshold) ? m_lim_threshold : rising;
    m_lim_gain[m_lim_bp] = maybe;

    if (peak * maybe > m_lim_threshold) {
        const double curgain = m_lim_threshold / peak;
        const double inc     = m_lim_threshold - curgain;
        double       acc     = 0.0;
        bool         stop    = false;
        for (int i = 0; !stop && i < m_lim_lookahead; ++i) {
            long ind = m_lim_bp - i;
            if (ind < 0) {
                ind += k_lim_size;
            }
            const double newgain = curgain + inc * (acc * acc);
            if (newgain < m_lim_gain[ind]) {
                m_lim_gain[ind] = newgain;
            }
            else {
                stop = true;
            }
            acc += m_lim_recip;
        }
    }

    long bbp = m_lim_bp - m_lim_lookahead;
    if (bbp < 0) {
        bbp += k_lim_size;
    }

    l = m_lim_buf1[bbp] * m_lim_gain[bbp];
    r = m_lim_buf2[bbp] * m_lim_gain[bbp];

    m_lim_last = m_lim_gain[m_lim_bp];
    if (++m_lim_bp >= k_lim_size) {
        m_lim_bp = 0;
    }
}
}
;

MIN_EXTERNAL(verb);
