/// @file
/// @brief      tap.multitap~ — a self-contained multitap delay line.
/// @details    Records the input into a circular buffer and sums any number of delayed taps, each
///             with its own delay time (ms) and gain (dB). Faithful port of the ttblue tt_multitap —
///             DSP is portable C++ (no Jamoma).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class multitap : public object<multitap>, public sample_operator<1, 1> {
  public:
    MIN_DESCRIPTION{"A self-contained multitap delay. Records the input into a buffer and sums any "
                    "number of taps, each with its own delay time (ms) and gain (dB)."};
    MIN_TAGS{"delays"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tapin~, tapout~, delay~, tap.procrastinate~"};

    inlet<>  m_in{this, "(signal) audio input"};
    outlet<> m_out{this, "(signal) summed delay taps", "signal"};

    attribute<number> buffersize{this, "buffersize", 1000.0,
                                 description{"Size of the delay buffer in milliseconds (fixed at instantiation)."}};

    attribute<int> taps{this, "taps", 1,
                        setter{MIN_FUNCTION{m_num_taps = std::clamp(static_cast<int>(args[0]), 1, k_max_taps - 1);
    return {m_num_taps};
}
}
, description {
    "Number of active delay taps."
}
}
;

attribute<std::vector<number>> delay{
    this, "delay", {0.0},
    setter{MIN_FUNCTION{
        for (size_t i = 0; i < args.size() && i < static_cast<size_t>(k_max_taps); ++i){m_delay_ms[i] = args[i];
}
update_delays();
return args;
}
}
, description {
    "Delay time (ms) for each tap."
}
}
;

attribute<std::vector<number>> gain{
    this, "gain", {0.0},
    setter{MIN_FUNCTION{for (size_t i = 0; i < args.size() && i < static_cast<size_t>(k_max_taps);
                             ++i){m_gain_lin[i] = std::pow(10.0, static_cast<double>(args[i]) * 0.05); // dB -> linear
}
return args;
}
}
, description {
    "Gain (dB) for each tap."
}
}
;

message<> clear{this, "clear", "Clear the delay buffer.", MIN_FUNCTION{std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
return {};
}
}
;

message<> dspsetup{this, "dspsetup", "Allocate and recompute when the DSP chain starts.", MIN_FUNCTION{allocate();
update_delays();
return {};
}
}
;

multitap(const atoms& args = {}) {
    if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
        buffersize = args[0];
    }
    allocate();
}

sample operator()(sample x) {
    const long N = static_cast<long>(m_buffer.size());
    if (N < 1) {
        return 0.0;
    }

    m_buffer[m_write] = x;

    double out = 0.0;
    for (int i = 0; i < m_num_taps; ++i) {
        long read = m_write - m_delay_samples[i];
        read %= N;
        if (read < 0) {
            read += N;
        }
        out += m_buffer[read] * m_gain_lin[i];
    }

    if (++m_write >= N) {
        m_write = 0;
    }

    return out * k_master_gain;
}

private:
static constexpr int    k_max_taps{100};
static constexpr double k_master_gain{1.0};

std::vector<double>            m_buffer;
long                           m_write{0};
int                            m_num_taps{1};
std::array<double, k_max_taps> m_delay_ms{};
std::array<long, k_max_taps>   m_delay_samples{};
std::array<double, k_max_taps> m_gain_lin{};

void allocate() {
    const double sr = samplerate();
    long         n  = static_cast<long>(static_cast<double>(buffersize) * (sr * 0.001));
    if (n < 1) {
        n = 1;
    }
    if (static_cast<long>(m_buffer.size()) != n) {
        m_buffer.assign(n, 0.0);
        m_write = 0;
    }
}

void update_delays() {
    const double sr = samplerate();
    const long   N  = std::max(1L, static_cast<long>(m_buffer.size()));
    for (int i = 0; i < k_max_taps; ++i) {
        m_delay_samples[i] = std::clamp(static_cast<long>(m_delay_ms[i] * (sr * 0.001)), 0L, N - 1);
    }
}
}
;

MIN_EXTERNAL(multitap);
