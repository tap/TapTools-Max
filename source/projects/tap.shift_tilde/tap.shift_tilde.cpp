/// @file
/// @brief      tap.shift~ — delay-line pitch shifter.
/// @details    A classic two-grain overlapping-delay pitch shifter: a phasor sweeps two delay taps
///             (180 degrees apart) across a window of the recent input, each windowed and summed.
///             Originally a faithful reconstruction of the ttblue tt_shift meta-object; in 2026 the
///             author approved modernizing the engine rather than preserving its implementation
///             artifacts. The surface (attributes, inlets, messages) still matches tt_shift.
///
///             Engine changes vs. the original (author-approved, 2026-07-11):
///             - Fractional taps are read with 4-point Hermite interpolation (the original used
///               linear interpolation with a constant one-sample offset).
///             - The grain envelopes are an exact complementary Hann pair — sin^2 and cos^2 sum to
///               one at every phase — replacing the 256-point padded-Welch table whose non-constant
///               sum imposed an amplitude ripple at the grain rate.
///             - ratio and window_size ride per-sample linear ramps (default 20 ms, settable via
///               the smooth attribute), so parameter changes glide instead of stepping; window
///               changes no longer click (the taps used to jump).
///             - The two float inlets of the original (ratio, window size in ms) are restored —
///               the first Min port had dropped them.
/// @author     Timothy Place
/// @copyright  Copyright 2000-2026 Timothy Place. Distributed under the New BSD License.

#include <algorithm>
#include <cmath>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class shift : public object<shift>, public sample_operator<1, 1> {
  public:
    MIN_DESCRIPTION{"A delay-line pitch shifter. A phasor sweeps two windowed delay taps across "
                    "the recent input to transpose it by the given ratio. The two grain "
                    "envelopes sum to exactly one and the taps are Hermite-interpolated, so the "
                    "output level is steady and parameter changes glide smoothly."};
    MIN_TAGS{"processors"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.pitchaccum~, tap.semitone2ratio, gizmo~, pitchshift~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_rat{this, "(float) pitch-shift ratio"};
    inlet<>  m_in_win{this, "(float) window size in ms"};
    outlet<> m_out{this, "(signal) pitch-shifted output", "signal"};

    attribute<number> ratio{this, "ratio", 1.0, setter{ MIN_FUNCTION {
                                set_ramp(m_ratio, args[0]);
                                return args;
                            }},
                            description{"Pitch-shift ratio (1 = no shift, 2 = up an octave, 0.5 = down an octave)."}};

    attribute<number> window_size{this, "window_size", 87.0, setter{ MIN_FUNCTION {
                                      const double v =
                                          MIN_CLAMP(static_cast<double>(args[0]), 1.0, k_c_buffersize_ms - 6.0);
                                      set_ramp(m_window, v);
                                      return {v};
                                  }},
                                  description{"Size of the delay window in milliseconds."}};

    attribute<number> smooth{this, "smooth", 20.0, setter{ MIN_FUNCTION {
                                 m_smooth_ms = std::max(0.0, static_cast<double>(args[0]));
                                 return {m_smooth_ms};
                             }},
                             description{"Interpolation time in ms applied to ratio and window_size changes "
                                         "(0 = immediate)."}};

    message<> m_number{this, "number", "Set the ratio (middle inlet) or window size (right inlet).",
                       MIN_FUNCTION {
                           if (inlet == 1) {
                               ratio = args[0];
                           }
                           else if (inlet == 2) {
                               window_size = args[0];
                           }
                           return {};
                       }};

    message<> clear{this, "clear", "Clear the delay buffer.",
                    MIN_FUNCTION {
                        std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Allocate the delay buffer and recompute rates.",
                       MIN_FUNCTION {
                           allocate();
                           snap_ramps();
                           return {};
                       }};

    shift(const atoms& = {}) {
        allocate();
        snap_ramps();
    }

    sample operator()(sample x) {
        const long N = static_cast<long>(m_buffer.size());
        if (N < 8) {
            return 0.0;
        }

        tick(m_ratio);
        tick(m_window);
        const double window_samples = std::max(4.0, m_window.current * samplerate() * 0.001);

        m_buffer[m_write] = x;

        const double ph  = m_phase;
        double       ph2 = ph + 0.5;
        if (ph2 >= 1.0) {
            ph2 -= 1.0;
        }

        // complementary Hann pair: sin^2 + cos^2 == 1 at every phase, so the output level is
        // constant (the original's padded-Welch table summed unevenly and imposed a ripple)
        const double s  = std::sin(k_c_pi * ph);
        const double e1 = s * s;
        const double e2 = 1.0 - e1;

        double y = 0.0;
        if (e1 > 0.0) {
            y += e1 * read_hermite(k_c_base_delay + ph * window_samples, N);
        }
        if (e2 > 0.0) {
            y += e2 * read_hermite(k_c_base_delay + ph2 * window_samples, N);
        }

        if (++m_write >= N) {
            m_write = 0;
        }

        // phasor rate: -(ratio - 1) / window (tt_shift provenance), from the ramped values
        m_phase += -(m_ratio.current - 1.0) / window_samples;
        m_phase -= std::floor(m_phase);

        return y;
    }

  private:
    static constexpr double k_c_pi{3.14159265358979323846};
    static constexpr double k_c_buffersize_ms{200.0};
    static constexpr double k_c_base_delay{3.0}; // headroom so Hermite never reads the write point

    struct ramp {
        double current{0.0};
        double target{0.0};
        double inc{0.0};
        long   remaining{0};
    };

    std::vector<double> m_buffer;
    long                m_write{0};
    double              m_phase{0.0};
    double              m_smooth_ms{20.0};
    ramp                m_ratio{1.0, 1.0, 0.0, 0};
    ramp                m_window{87.0, 87.0, 0.0, 0};

    void set_ramp(ramp& r, double target) {
        const long n = static_cast<long>(m_smooth_ms * 0.001 * samplerate());
        if (n < 1 || target == r.current) {
            r.current   = target;
            r.target    = target;
            r.inc       = 0.0;
            r.remaining = 0;
        }
        else {
            r.target    = target;
            r.inc       = (target - r.current) / static_cast<double>(n);
            r.remaining = n;
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

    void snap_ramps() {
        m_ratio.current    = m_ratio.target;
        m_ratio.remaining  = 0;
        m_window.current   = m_window.target;
        m_window.remaining = 0;
    }

    void allocate() {
        const long n = std::max(16L, static_cast<long>(k_c_buffersize_ms * samplerate() * 0.001));
        if (static_cast<long>(m_buffer.size()) != n) {
            m_buffer.assign(n, 0.0);
            m_write = 0;
        }
    }

    // 4-point Hermite fractional read, d samples behind the write position (d >= 3).
    double read_hermite(double d, long N) const {
        const double pos  = static_cast<double>(m_write) - d;
        const double fpos = std::floor(pos);
        const double frac = pos - fpos;
        long         base = static_cast<long>(fpos) % N;
        if (base < 0) {
            base += N;
        }
        const auto at = [&](long i) {
            long j = (base + i) % N;
            if (j < 0) {
                j += N;
            }
            return m_buffer[j];
        };
        const double xm1 = at(-1);
        const double x0  = at(0);
        const double x1  = at(1);
        const double x2  = at(2);
        const double c   = (x1 - xm1) * 0.5;
        const double v   = x0 - x1;
        const double w   = c + v;
        const double a   = w + v + (x2 - x0) * 0.5;
        const double b   = w + a;
        return (((a * frac - b) * frac + c) * frac + x0);
    }
};

MIN_EXTERNAL(shift);
