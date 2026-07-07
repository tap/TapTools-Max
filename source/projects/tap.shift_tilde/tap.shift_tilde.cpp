/// @file
/// @brief      tap.shift~ — delay-line pitch shifter.
/// @details    A classic two-grain overlapping-delay pitch shifter: a phasor sweeps two delay taps
///             (180 degrees apart) across a window of the recent input, each tap windowed by a padded
///             Welch envelope and summed. Faithful reconstruction of the ttblue tt_shift meta-object
///             (phasor + two interpolated delay lines + Welch window + overlap-add). The Welch window
///             is the original 256-point half-table, mirrored to 512 points. DSP is portable C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class shift : public object<shift>, public sample_operator<1, 1> {
  public:
    MIN_DESCRIPTION{"A delay-line pitch shifter. A phasor sweeps two windowed delay taps across the "
                    "recent input to transpose it by the given ratio."};
    MIN_TAGS{"processors"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"gizmo~, pitchshift~, tap.multitap~"};

    inlet<>  m_in{this, "(signal/float) audio input"};
    outlet<> m_out{this, "(signal) pitch-shifted output", "signal"};

    attribute<number> ratio{this, "ratio", 1.0, setter{MIN_FUNCTION{m_ratio = args[0];
    update_phasor();
    return args;
}
}
, description {
    "Pitch-shift ratio (1 = no shift, 2 = up an octave, 0.5 = down an octave)."
}
}
;

attribute<number> window_size{
    this, "window_size", 87.0,
    setter{MIN_FUNCTION{m_window_size = MIN_CLAMP(static_cast<double>(args[0]), 1.0, k_buffersize_ms - 1.0);
update_phasor();
return {m_window_size};
}
}
, description {
    "Size of the delay window in milliseconds."
}
}
;

message<> clear{this, "clear", "Clear the delay buffer.", MIN_FUNCTION{std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
return {};
}
}
;

message<> dspsetup{this, "dspsetup", "Allocate the delay buffer and recompute rates.", MIN_FUNCTION{allocate();
update_phasor();
return {};
}
}
;

shift(const atoms& = {}) {
    build_window();
    allocate();
}

sample operator()(sample x) {
    const long N = static_cast<long>(m_buffer.size());
    if (N < 2) {
        return 0.0;
    }

    const double ph  = m_phase;
    double       ph2 = ph + 0.5;
    if (ph2 >= 1.0) {
        ph2 -= 1.0;
    }

    m_buffer[m_write] = x;

    const double sr_ms    = samplerate() * 0.001;
    const double delayed1 = read_delay(ph * m_window_size * sr_ms, N);
    const double delayed2 = read_delay(ph2 * m_window_size * sr_ms, N);

    if (++m_write >= N) {
        m_write = 0;
    }

    m_phase += m_phasor_inc;
    m_phase -= std::floor(m_phase);

    return window_lookup(ph) * delayed1 + window_lookup(ph2) * delayed2;
}

private:
static constexpr double k_buffersize_ms{200.0};
static constexpr int    k_window_len{512};

std::vector<double>              m_buffer;
long                             m_write{0};
double                           m_ratio{1.0};
double                           m_window_size{87.0};
double                           m_phase{0.0};
double                           m_phasor_inc{0.0};
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

void build_window() {
    for (int i = 0; i < 256; ++i) {
        m_window[i]       = k_half_welch[i];
        m_window[511 - i] = k_half_welch[i]; // mirror the half-table to 512 points
    }
}

void allocate() {
    const long n = std::max(2L, static_cast<long>(k_buffersize_ms * samplerate() * 0.001));
    if (static_cast<long>(m_buffer.size()) != n) {
        m_buffer.assign(n, 0.0);
        m_write = 0;
    }
}

void update_phasor() {
    const double freq = -(m_ratio - 1.0) * (1.0 / (m_window_size * 0.001));
    m_phasor_inc      = freq / samplerate();
}

double read_delay(double delay_samples, long N) const {
    long         di   = static_cast<long>(delay_samples);
    const double frac = delay_samples - static_cast<double>(di);
    long         i0   = ((m_write - di) % N + N) % N;
    long         in1  = (i0 + 1) % N;
    return m_buffer[in1] * (1.0 - frac) + m_buffer[i0] * frac;
}

double window_lookup(double phase) const {
    const double index = phase * static_cast<double>(k_window_len);
    int          p1    = static_cast<int>(index);
    const int    p2    = (p1 + 1) & (k_window_len - 1);
    p1 &= (k_window_len - 1);
    const double diff = index - static_cast<double>(static_cast<int>(index));
    return m_window[p2] * diff + m_window[p1] * (1.0 - diff);
}
}
;

constexpr double shift::k_half_welch[256];

MIN_EXTERNAL(shift);
