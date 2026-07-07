/// @file
/// @brief      tap.pulsesub~ — pulse-based envelope substitution.
/// @details    Generates a repeating ADSR envelope, gated by an internal pulse train, and applies it
///             to the gain of the input signal. Faithful port of Jamoma's TTPulseSub, which was a
///             composite of a phasor + an offset (duty cycle) + an ADSR + a multiplier:
///                 - a phasor runs at `frequency` Hz, producing a 0..1 ramp
///                 - the ramp is offset by (length - 0.5); the ADSR is triggered while it exceeds
///                   0.5, so the envelope opens for the fraction `length` of every cycle
///                 - the resulting envelope multiplies the input signal
///             DSP is portable C++ (no Jamoma).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2004-2026 Timothy Place.

#include <algorithm>
#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class pulsesub : public object<pulsesub>, public sample_operator<1, 1> {
  private:
    static constexpr double k_noise_floor{-120.0};

    enum eg_state { eg_inactive = 0, eg_attack, eg_decay, eg_sustain, eg_release };
    enum mode_type { mode_linear = 0, mode_exponential };

  public:
    MIN_DESCRIPTION{"Pulse-based envelope substitution. Generates a repeating ADSR envelope gated "
                    "by an internal pulse train (set by frequency and length) and applies it to "
                    "the gain of the input signal."};
    MIN_TAGS{"processors"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.adsr~, tap.svf~, *~, train~"};

    inlet<>  m_in{this, "(signal) audio input"};
    outlet<> m_out{this, "(signal) input with the pulsed envelope applied", "signal"};

    attribute<number> frequency{this, "frequency", 1.0, description{"Rate of the envelope pulse train, in Hz."}};

    attribute<number> length{this, "length", 0.25, range{0.0, 1.0},
                             description{"Duty cycle: the fraction of each cycle for which the envelope is open."}};

    attribute<number> attack{this, "attack", 50.0,
                             setter{MIN_FUNCTION{m_attack_ms = MIN_CLAMP(static_cast<double>(args[0]), 1.0, 60000.0);
    update_steps();
    return {m_attack_ms};
}
}
, description {
    "Attack time in milliseconds."
}
}
;

attribute<number> decay{this, "decay", 100.0,
                        setter{MIN_FUNCTION{m_decay_ms = MIN_CLAMP(static_cast<double>(args[0]), 1.0, 60000.0);
update_steps();
return {m_decay_ms};
}
}
, description {
    "Decay time in milliseconds."
}
}
;

attribute<number> sustain{this, "sustain", -6.0,
                          setter{MIN_FUNCTION{m_sustain_amp = decibels_to_gain(static_cast<double>(args[0]));
return args;
}
}
, description {
    "Sustain level in decibels."
}
}
;

attribute<number> release{this, "release", 500.0,
                          setter{MIN_FUNCTION{m_release_ms = MIN_CLAMP(static_cast<double>(args[0]), 1.0, 60000.0);
update_steps();
return {m_release_ms};
}
}
, description {
    "Release time in milliseconds."
}
}
;

attribute<symbol> mode{this, "mode", "linear", range{"linear", "exponential"},
                       setter{MIN_FUNCTION{m_mode = (args[0] == "exponential") ? mode_exponential : mode_linear;
return args;
}
}
, description {
    "Envelope curve: linear or exponential."
}
}
;

message<> dspsetup{this, "dspsetup", "Recompute step sizes for the current sample rate.", MIN_FUNCTION{update_steps();
return {};
}
}
;

sample operator()(sample x) {
    // Phasor → duty-cycle offset → trigger.
    const double ramp = m_phase;
    m_phase += static_cast<double>(frequency) / samplerate();
    while (m_phase >= 1.0) {
        m_phase -= 1.0;
    }
    while (m_phase < 0.0) {
        m_phase += 1.0;
    }

    const bool trigger = (ramp + (static_cast<double>(length) - 0.5)) > 0.5;

    // ADSR state transitions.
    if (trigger) {
        if (m_state == eg_inactive || m_state == eg_release) {
            m_state = eg_attack;
        }
    }
    else {
        if (m_state != eg_inactive && m_state != eg_release) {
            m_state = eg_release;
        }
    }

    if (m_mode == mode_exponential) {
        process_exponential();
    }
    else {
        process_linear();
    }

    return x * m_output;
}

private:
double m_phase{0.0};

double m_attack_ms{50.0};
double m_decay_ms{100.0};
double m_release_ms{500.0};
double m_sustain_amp{0.5011872336272722}; // -6 dB

double m_attack_step{0.0};
double m_decay_step{0.0};
double m_release_step{0.0};
double m_attack_step_db{0.0};
double m_decay_step_db{0.0};
double m_release_step_db{0.0};

double m_output{0.0};
double m_output_db{k_noise_floor};
int    m_state{eg_inactive};
int    m_mode{mode_linear};

static double decibels_to_gain(double db) {
    return std::pow(10.0, db * 0.05);
}

void update_steps() {
    const double sr   = samplerate();
    const long   a    = std::max(1L, static_cast<long>((m_attack_ms / 1000.0) * sr));
    const long   d    = std::max(1L, static_cast<long>((m_decay_ms / 1000.0) * sr));
    const long   r    = std::max(1L, static_cast<long>((m_release_ms / 1000.0) * sr));
    m_attack_step     = 1.0 / a;
    m_decay_step      = 1.0 / d;
    m_release_step    = 1.0 / r;
    m_attack_step_db  = -(k_noise_floor / a);
    m_decay_step_db   = -(k_noise_floor / d);
    m_release_step_db = -(k_noise_floor / r);
}

void process_linear() {
    switch (m_state) {
    case eg_attack:
        m_output += m_attack_step;
        if (m_output >= 1.0) {
            m_output = 1.0;
            m_state  = eg_decay;
        }
        break;
    case eg_decay:
        m_output -= m_decay_step;
        if (m_output <= m_sustain_amp) {
            m_state  = eg_sustain;
            m_output = m_sustain_amp;
        }
        break;
    case eg_sustain:
        break;
    case eg_release:
        m_output -= m_release_step;
        if (m_output <= 0.0) {
            m_state  = eg_inactive;
            m_output = 0.0;
        }
        break;
    }
}

void process_exponential() {
    switch (m_state) {
    case eg_attack:
        m_output_db += m_attack_step_db;
        if (m_output_db >= 0.0) {
            m_state  = eg_decay;
            m_output = 1.0;
        }
        else {
            m_output = decibels_to_gain(m_output_db);
        }
        break;
    case eg_decay:
        m_output_db -= m_decay_step_db;
        m_output = decibels_to_gain(m_output_db);
        if (m_output <= m_sustain_amp) {
            m_state  = eg_sustain;
            m_output = m_sustain_amp;
        }
        break;
    case eg_sustain:
        break;
    case eg_release:
        m_output_db -= m_release_step_db;
        if (m_output_db <= k_noise_floor) {
            m_state  = eg_inactive;
            m_output = 0.0;
        }
        else {
            m_output = decibels_to_gain(m_output_db);
        }
        break;
    }
}
}
;

MIN_EXTERNAL(pulsesub);
