/// @file
/// @brief      tap.adsr~ — virtual-analog attack/decay/sustain/release envelope generator.
/// @details    Rebuilt on the portable kernel (taptools/adsr.h): the default `analog` mode is a
///             circuit model — an RC attack charging toward a 1.4× overshoot target and truncated
///             at full scale (the CEM 3310 architecture), decay and release as true RC discharges
///             that taper into their targets — and the 2003 Jamoma TTAdsr curves are preserved
///             verbatim as the `hybrid` / `linear` / `exponential` compatibility modes. This is a
///             deliberate default change: the analog curve is the sound the knobs always implied,
///             and the old curves remain one attribute away.
///
///             Triggering follows the family contract at last: a signal gate opens above
///             `threshold` (default 0.005, so a `tap.808.seq~` row's plain 0.01 level registers —
///             the old hard-coded 0.5 is retired), and the gate's amplitude is velocity under the
///             `velocity` sensitivity (0 = amplitude-blind legacy behavior; 1 = a 303-style 2.0
///             accent hits twice as hard). Without a signal connection the `trigger` attribute
///             drives the gate. This file is only the Max plumbing.
/// @author     Timothy Place, Dave Watson, Trond Lossius
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "c74_min.h"
#include "taptools/adsr.h"

using namespace c74::min;

class adsr : public object<adsr>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    tap::tools::adsr::generator m_engine;

  public:
    MIN_DESCRIPTION{"A virtual-analog ADSR envelope generator. The default analog mode models the "
                    "classic RC circuit (overshoot-target attack, asymptotic decay/release); the "
                    "legacy hybrid/linear/exponential curves remain as modes. Triggered by a signal "
                    "gate (amplitude is velocity) or the trigger attribute."};
    MIN_TAGS{"generators"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"adsr~, function, line~, curve~, tap.vca~"};

    inlet<>  m_in{this, "(signal/anything) gate signal (opens above threshold) or control messages"};
    outlet<> m_out{this, "(signal) envelope output", "signal"};

    adsr(const atoms& args = {}) { m_engine.prepare(samplerate()); }

    attribute<bool> trigger{this, "trigger", false,
                            description{"Open the envelope (attack) while on; release it while off. "
                                        "Ignored while a signal is connected to the inlet."}};

    attribute<number> attack{this, "attack", 50.0, setter{MIN_FUNCTION{
                                 m_engine.set_attack_ms(args[0]);
                                 return {m_engine.attack_ms()};
                             }},
                             description{"Attack time in milliseconds (1..60000)."}};

    attribute<number> decay{this, "decay", 100.0, setter{MIN_FUNCTION{
                                m_engine.set_decay_ms(args[0]);
                                return {m_engine.decay_ms()};
                            }},
                            description{"Decay time in milliseconds (1..60000)."}};

    attribute<number> sustain{this, "sustain", -6.0, setter{MIN_FUNCTION{
                                  m_engine.set_sustain_db(args[0]);
                                  return args;
                              }},
                              description{"Sustain level in decibels."}};

    attribute<number> release{this, "release", 500.0, setter{MIN_FUNCTION{
                                  m_engine.set_release_ms(args[0]);
                                  return {m_engine.release_ms()};
                              }},
                              description{"Release time in milliseconds (1..60000)."}};

    attribute<symbol> mode{this,
                           "mode",
                           "analog",
                           range{"analog", "hybrid", "linear", "exponential"},
                           setter{MIN_FUNCTION{
                               using kmode = tap::tools::adsr::mode;
                               if (args[0] == "hybrid") {
                                   m_engine.set_mode(kmode::hybrid);
                               }
                               else if (args[0] == "linear") {
                                   m_engine.set_mode(kmode::linear);
                               }
                               else if (args[0] == "exponential") {
                                   m_engine.set_mode(kmode::exponential);
                               }
                               else {
                                   m_engine.set_mode(kmode::analog);
                               }
                               return args;
                           }},
                           description{"Envelope curve: analog (the circuit model, default), or the "
                                       "legacy hybrid, linear, exponential."}};

    attribute<number> threshold{this, "threshold", tap::tools::adsr::k_default_threshold,
                                setter{MIN_FUNCTION{
                                    m_engine.set_threshold(args[0]);
                                    return {m_engine.threshold()};
                                }},
                                description{"Gate-open level (0..1). The default hears a sequencer "
                                            "row's plain 0.01 hits."}};

    attribute<number> velocity{this, "velocity", 0.0, setter{MIN_FUNCTION{
                                   m_engine.set_velocity(args[0]);
                                   return {m_engine.velocity()};
                               }},
                               description{"Velocity sensitivity (0..1): peak and sustain scale by "
                                           "1 + velocity x (gate amplitude - 1). 0 ignores amplitude."}};

    message<> clear{this, "clear", "Reset the envelope to silence and idle.",
                    MIN_FUNCTION{
                        m_engine.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Recompute the stage coefficients for the current sample rate.",
                       MIN_FUNCTION{
                           m_engine.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample x) {
        const double gate = m_in.has_signal_connection() ? static_cast<double>(x) : (trigger ? 1.0 : 0.0);
        return m_engine.process(gate);
    }
};

MIN_EXTERNAL(adsr);
