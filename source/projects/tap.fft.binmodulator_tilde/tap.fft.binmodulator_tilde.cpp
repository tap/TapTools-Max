/// @file
/// @brief      tap.fft.binmodulator~ — modulate individual FFT bins with per-bin LFOs.
/// @details    Gives each FFT bin its own low-frequency oscillator (frequency, depth, phase, shape)
///             that modulates the bin's amplitude — a per-bin tremolo. Intended to live inside a
///             pfft~ subpatcher: inlets are real, imaginary, and the bin index; outlets are the
///             modulated real and imaginary parts. Faithful port of the original (built on the ttblue
///             wavetable LFO); the LFOs here are computed directly from phase accumulators using the
///             0..1 "modulator" waveforms.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include <algorithm>
#include <array>
#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class fft_binmodulator : public object<fft_binmodulator>, public vector_operator<> {
  private:
    enum lfo_shape { shape_sine = 0, shape_triangle, shape_square, shape_ramp };

  public:
    MIN_DESCRIPTION{"Modulate FFT bins with per-bin LFOs. Each frequency bin gets its own LFO "
                    "(frequency, depth, phase, shape) modulating its amplitude. Use inside pfft~."};
    MIN_TAGS{"fft"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"pfft~, tap.fft.normalize~, tap.fft.list~"};

    inlet<>  m_in_real{this, "(signal) real part of the FFT frame"};
    inlet<>  m_in_imag{this, "(signal) imaginary part of the FFT frame"};
    inlet<>  m_in_index{this, "(signal) FFT bin index"};
    outlet<> m_out_real{this, "(signal) modulated real part", "signal"};
    outlet<> m_out_imag{this, "(signal) modulated imaginary part", "signal"};

    attribute<bool> bypass{this, "bypass", false, description{"Pass the FFT frame through unmodulated."}};

    message<> frequency{this, "frequency", "Set per-bin LFO frequency: [index value] or a full list.",
                        MIN_FUNCTION{set_numbers(args, m_freq);
    return {};
}
}
;

message<> depth{this, "depth", "Set per-bin LFO depth: [index value] or a full list.",
                MIN_FUNCTION{set_numbers(args, m_depth);
return {};
}
}
;

message<> phase{this, "phase", "Set per-bin LFO phase (0..1): [index value] or a full list.",
                MIN_FUNCTION{set_numbers(args, m_phase_offset, 0.0, 1.0);
return {};
}
}
;

message<> shape{this, "shape", "Set per-bin LFO shape: [index name] or a full list of names.",
                MIN_FUNCTION{if (args.size() >= 2 && args[0].type() == message_type::int_argument){
                    const int idx = std::clamp(static_cast<int>(args[0]), 0, k_max_lfos - 1);
m_shape[idx] = shape_from_symbol(args[1]);
}
else {
    for (size_t i = 0; i < args.size() && i < static_cast<size_t>(k_max_lfos); ++i) {
        m_shape[i] = shape_from_symbol(args[i]);
    }
}
return {};
}
}
;

void operator()(audio_bundle input, audio_bundle output) override {
    const long    n   = input.frame_count();
    const double* re  = input.samples(0);
    const double* im  = input.samples(1);
    const double* idx = input.samples(2);
    double*       ore = output.samples(0);
    double*       oim = output.samples(1);

    if (bypass) {
        for (long k = 0; k < n; ++k) {
            ore[k] = re[k];
            oim[k] = im[k];
        }
        return;
    }

    const double sr = samplerate();
    const double vs = static_cast<double>(n);

    for (long k = 0; k < n; ++k) {
        const int bin = std::clamp(static_cast<int>(idx[k]), 0, k_max_lfos - 1);

        // advance this bin's LFO once per frame
        m_phase[bin] += (m_freq[bin] * vs) / sr;
        m_phase[bin] -= std::floor(m_phase[bin]);

        const double w = waveform_mod(m_shape[bin], m_phase[bin] + m_phase_offset[bin]) * m_depth[bin];
        ore[k]         = re[k] * w;
        oim[k]         = im[k] * w;
    }
}

private:
static constexpr int k_max_lfos{512};

std::array<double, k_max_lfos> m_freq{};
std::array<double, k_max_lfos> m_depth{};
std::array<double, k_max_lfos> m_phase{};
std::array<double, k_max_lfos> m_phase_offset{};
std::array<int, k_max_lfos>    m_shape{};

// Set either a single [index value] pair or a whole list into a per-bin array.
void set_numbers(const atoms& args, std::array<double, k_max_lfos>& dst, double lo = 0.0, double hi = 0.0) {
    auto store = [&](int i, double v) {
        if (hi > lo) {
            v = std::clamp(v, lo, hi);
        }
        dst[i] = v;
    };
    if (args.size() >= 2 && args[0].type() == message_type::int_argument) {
        const int idx = std::clamp(static_cast<int>(args[0]), 0, k_max_lfos - 1);
        store(idx, args[1]);
    }
    else {
        for (size_t i = 0; i < args.size() && i < static_cast<size_t>(k_max_lfos); ++i) {
            store(static_cast<int>(i), args[i]);
        }
    }
}

static int shape_from_symbol(const atom& a) {
    const symbol s = a;
    if (s == "triangle") {
        return shape_triangle;
    }
    if (s == "square") {
        return shape_square;
    }
    if (s == "ramp") {
        return shape_ramp;
    }
    return shape_sine;
}

static double waveform_mod(int shape, double phase) {
    phase -= std::floor(phase); // wrap to 0..1
    switch (shape) {
    case shape_square:
        return (phase < 0.5) ? 1.0 : 0.0;
    case shape_ramp:
        return phase;
    case shape_triangle: {
        double t;
        if (phase < 0.25) {
            t = 4.0 * phase;
        }
        else if (phase < 0.75) {
            t = 2.0 - 4.0 * phase;
        }
        else {
            t = 4.0 * phase - 4.0;
        }
        return (t + 1.0) * 0.5;
    }
    case shape_sine:
    default:
        return 0.5 + 0.5 * std::sin(6.28318530717958647692 * phase);
    }
}
}
;

MIN_EXTERNAL(fft_binmodulator);
