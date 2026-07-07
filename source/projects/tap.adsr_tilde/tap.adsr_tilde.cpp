/// @file
/// @brief      tap.adsr~ — attack/decay/sustain/release envelope generator.
/// @details    Faithful port of Jamoma's TTAdsr. The envelope is triggered either by the `trigger`
///             attribute (control rate) or, when a signal is connected to the inlet, by that signal
///             crossing 0.5 (signal rate). Three curve modes are provided:
///                 - linear:       linear ramps for every stage
///                 - exponential:  exponential (dB-linear) ramps for every stage
///                 - hybrid:       linear attack with exponential decay/release
///             DSP is plain portable C++ — Min only wires the object into Max.
/// @note       The original Max wrapper's struct reported a default mode of "linear", but the
///             underlying TTAdsr always defaulted to "hybrid" — which is what users actually heard —
///             so this port defaults to hybrid to preserve the original sound.
/// @author     Timothy Place, Dave Watson, Trond Lossius
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <algorithm>
#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class adsr : public object<adsr>, public sample_operator<1, 1> {
  private:
    static constexpr double k_noise_floor{-120.0}; ///< envelope basement, in dB

    enum eg_state { eg_inactive = 0, eg_attack, eg_decay, eg_sustain, eg_release };
    enum mode_type { mode_linear = 0, mode_exponential, mode_hybrid };

  public:
    MIN_DESCRIPTION{"An attack/decay/sustain/release envelope generator. Triggered by the trigger "
                    "attribute, or by a signal (crossing 0.5) connected to the inlet. Linear, "
                    "exponential, or hybrid curves."};
    MIN_TAGS{"generators"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"adsr~, function, line~, curve~"};

    inlet<>  m_in{this, "(signal/anything) signal trigger (>0.5) or control messages"};
    outlet<> m_out{this, "(signal) envelope output", "signal"};

    attribute<bool> trigger{this, "trigger", false, setter{MIN_FUNCTION{m_trigger = args[0];
    return args;
}
}
, description {
    "Open the envelope (attack) while on; release it while off."
}
}
;

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

attribute<number> sustain{this, "sustain", -6.0, setter{MIN_FUNCTION{m_sustain_db = args[0];
m_sustain_amp = decibels_to_gain(m_sustain_db);
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

attribute<symbol> mode{this, "mode", "hybrid", range{"hybrid", "linear", "exponential"},
                       setter{MIN_FUNCTION{if (args[0] == "linear"){m_mode = mode_linear;
}
else if (args[0] == "exponential") {
    m_mode = mode_exponential;
}
else {
    m_mode = mode_hybrid;
}
return args;
}
}
, description {
    "Envelope curve: hybrid, linear, or exponential."
}
}
;

message<> dspsetup{this, "dspsetup", "Recompute step sizes for the current sample rate.", MIN_FUNCTION{update_steps();
return {};
}
}
;

sample operator()(sample x) {
    if (m_in.has_signal_connection()) {
        m_trigger = (x > 0.5);
    }

    // Shared state transitions.
    if (m_trigger) {
        if (m_state == eg_inactive || m_state == eg_release) {
            m_state = eg_attack;
        }
    }
    else {
        if (m_state != eg_inactive && m_state != eg_release) {
            m_state = eg_release;
        }
    }

    switch (m_mode) {
    case mode_linear:
        process_linear();
        break;
    case mode_exponential:
        process_exponential();
        break;
    case mode_hybrid:
    default:
        process_hybrid();
        break;
    }
    return m_output;
}

private:
// Parameters (durations in ms; sustain in dB / linear).
double m_attack_ms{50.0};
double m_decay_ms{100.0};
double m_release_ms{500.0};
double m_sustain_db{-6.0};
double m_sustain_amp{0.5011872336272722}; // -6 dB

// Per-sample step sizes (recomputed from the sample rate).
double m_attack_step{0.0};
double m_decay_step{0.0};
double m_release_step{0.0};
double m_attack_step_db{0.0};
double m_decay_step_db{0.0};
double m_release_step_db{0.0};

// Running state.
double m_output{0.0};
double m_output_db{k_noise_floor};
int    m_state{eg_inactive};
int    m_mode{mode_hybrid};
bool   m_trigger{false};

static double decibels_to_gain(double db) {
    return std::pow(10.0, db * 0.05);
}

static double gain_to_decibels(double amp) {
    return (amp <= 0.0) ? k_noise_floor : 20.0 * std::log10(amp);
}

void update_steps() {
    const double sr = samplerate();

    const long attack_samples  = std::max(1L, static_cast<long>((m_attack_ms / 1000.0) * sr));
    const long decay_samples   = std::max(1L, static_cast<long>((m_decay_ms / 1000.0) * sr));
    const long release_samples = std::max(1L, static_cast<long>((m_release_ms / 1000.0) * sr));

    m_attack_step     = 1.0 / attack_samples;
    m_decay_step      = 1.0 / decay_samples;
    m_release_step    = 1.0 / release_samples;
    m_attack_step_db  = -(k_noise_floor / attack_samples);
    m_decay_step_db   = -(k_noise_floor / decay_samples);
    m_release_step_db = -(k_noise_floor / release_samples);
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

// Hybrid: linear attack (good for short times) + exponential decay/release.
void process_hybrid() {
    switch (m_state) {
    case eg_attack:
        m_output += m_attack_step;
        if (m_output >= 1.0) {
            m_output    = 1.0;
            m_output_db = 0.0;
            m_state     = eg_decay;
        }
        else {
            m_output_db = gain_to_decibels(m_output);
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

MIN_EXTERNAL(adsr);
