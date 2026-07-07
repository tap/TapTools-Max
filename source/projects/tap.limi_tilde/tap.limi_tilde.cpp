/// @file
/// @brief      tap.limi~ — stereo look-ahead limiter.
/// @details    A linked-stereo look-ahead limiter with an integrated DC blocker, pre/post gain, and
///             linear or exponential gain recovery. Faithful port of the ttblue tt_limiter algorithm
///             (the same limiter TapTools shipped via Jamoma). DSP is portable C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <algorithm>
#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class limi : public object<limi>, public vector_operator<> {
  public:
    MIN_DESCRIPTION{"A stereo look-ahead limiter with an integrated DC blocker, pre/post gain, and "
                    "linear or exponential recovery."};
    MIN_TAGS{"dynamics"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"limi~, omx.peaklim~, tap.elixir~"};

    inlet<>  m_in_l{this, "(signal) left input"};
    inlet<>  m_in_r{this, "(signal) right input"};
    outlet<> m_out_l{this, "(signal) left limited output", "signal"};
    outlet<> m_out_r{this, "(signal) right limited output", "signal"};

    attribute<number> threshold{this, "threshold", 0.0, setter{MIN_FUNCTION{m_threshold = db_to_amp(args[0]);
    return args;
}
}
, description {
    "Limiting threshold in decibels."
}
}
;

attribute<number> preamp{this, "preamp", 0.0, setter{MIN_FUNCTION{m_preamp = db_to_amp(args[0]);
return args;
}
}
, description {
    "Gain (dB) applied before limiting."
}
}
;

attribute<number> postamp{this, "postamp", 0.0, setter{MIN_FUNCTION{m_postamp = db_to_amp(args[0]);
return args;
}
}
, description {
    "Gain (dB) applied after limiting."
}
}
;

attribute<number> release{this, "release", 1000.0, setter{MIN_FUNCTION{m_release = args[0];
set_recover();
return args;
}
}
, description {
    "Gain recovery (release) time in milliseconds."
}
}
;

attribute<int> lookahead{this, "lookahead", 100,
                         setter{MIN_FUNCTION{m_lookahead = std::clamp(static_cast<int>(args[0]), 1, k_max_samples - 1);
m_recip = 1.0 / static_cast<double>(m_lookahead);
return {m_lookahead};
}
}
, description {
    "Look-ahead window in samples (1-255)."
}
}
;

attribute<symbol> mode{this, "mode", "exponential", range{"linear", "exponential"},
                       setter{MIN_FUNCTION{m_linear = (args[0] == "linear");
set_recover();
return args;
}
}
, description {
    "Gain-recovery curve: linear or exponential."
}
}
;

attribute<bool> bypass_dcblocker{this, "bypass_dcblocker", false, description{"Bypass the integrated DC blocker."}};

attribute<bool> bypass{this, "bypass", false, description{"Pass the input through without limiting."}};

attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

message<> clear{this, "clear", "Reset the limiter's internal state.", MIN_FUNCTION{reset();
return {};
}
}
;

message<> dspsetup{this, "dspsetup", "Reset state and recompute the recovery rate.", MIN_FUNCTION{reset();
return {};
}
}
;

void operator()(audio_bundle input, audio_bundle output) override {
    const long    n  = input.frame_count();
    const double* i1 = input.samples(0);
    const double* i2 = input.samples(1);
    double*       o1 = output.samples(0);
    double*       o2 = output.samples(1);

    if (mute) {
        for (long k = 0; k < n; ++k) {
            o1[k] = 0.0;
            o2[k] = 0.0;
        }
        return;
    }
    if (bypass) {
        for (long k = 0; k < n; ++k) {
            o1[k] = i1[k];
            o2[k] = i2[k];
        }
        return;
    }

    const bool block_dc = !bypass_dcblocker;

    for (long k = 0; k < n; ++k) {
        double ch1 = i1[k] * m_preamp;
        double ch2 = i2[k] * m_preamp;

        if (block_dc) {
            m_last_output1 = anti_denormal(ch1 - m_last_input1 + (m_last_output1 * 0.9997));
            m_last_input1  = anti_denormal(ch1);
            m_last_output2 = anti_denormal(ch2 - m_last_input2 + (m_last_output2 * 0.9997));
            m_last_input2  = anti_denormal(ch2);
            ch1            = m_last_output1;
            ch2            = m_last_output2;
        }

        m_buf1[m_bp] = ch1 * m_postamp;
        m_buf2[m_bp] = ch2 * m_postamp;

        double peak = std::max(std::abs(ch1), std::abs(ch2));

        double rising = m_linear ? (m_last + m_recover) : (m_last + m_recover * ((m_last > 0.01) ? m_last : 1.0));
        double maybe  = (rising > m_threshold) ? m_threshold : rising;
        m_gain[m_bp]  = maybe;

        if (peak * maybe > m_threshold) {
            const double curgain = m_threshold / peak;
            const double inc     = m_threshold - curgain;
            double       acc     = 0.0;
            bool         stop    = false;
            for (int i = 0; !stop && i < m_lookahead; ++i) {
                long ind = m_bp - i;
                if (ind < 0) {
                    ind += k_max_samples;
                }
                const double newgain = m_linear ? (curgain + inc * acc) : (curgain + inc * (acc * acc));
                if (newgain < m_gain[ind]) {
                    m_gain[ind] = newgain;
                }
                else {
                    stop = true;
                }
                acc += m_recip;
            }
        }

        long bbp = m_bp - m_lookahead;
        if (bbp < 0) {
            bbp += k_max_samples;
        }

        o1[k] = m_buf1[bbp] * m_gain[bbp];
        o2[k] = m_buf2[bbp] * m_gain[bbp];

        m_last = m_gain[m_bp];
        if (++m_bp >= k_max_samples) {
            m_bp = 0;
        }
    }
}

private:
static constexpr int k_max_samples{256};

double m_threshold{1.0}; // amplitude (0 dB)
double m_preamp{1.0};
double m_postamp{1.0};
double m_release{1000.0};
int    m_lookahead{100};
double m_recip{1.0 / 100.0};
bool   m_linear{false};
double m_recover{0.0};

double m_buf1[k_max_samples]{};
double m_buf2[k_max_samples]{};
double m_gain[k_max_samples]{};
long   m_bp{0};
double m_last{1.0};

double m_last_input1{0.0}, m_last_output1{0.0};
double m_last_input2{0.0}, m_last_output2{0.0};

static double db_to_amp(double db) {
    return std::pow(10.0, db * 0.05);
}

static double anti_denormal(double x) {
    return (std::abs(x) < 1e-15) ? 0.0 : x;
}

void set_recover() {
    const double sr = samplerate();
    m_recover       = 1000.0 / (m_release * sr);
    if (m_recover == 0.0) {
        m_recover = 1000.0 / (100000.0 * sr);
    }
    m_recover *= m_linear ? 0.5 : 0.707;
}

void reset() {
    for (int i = 0; i < k_max_samples; ++i) {
        m_buf1[i] = 0.0;
        m_buf2[i] = 0.0;
        m_gain[i] = 1.0;
    }
    m_bp   = 0;
    m_last = 1.0;
    set_recover();
    m_last_input1 = m_last_output1 = 0.0;
    m_last_input2 = m_last_output2 = 0.0;
}
}
;

MIN_EXTERNAL(limi);
