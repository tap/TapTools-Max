/// @file
/// @brief      tap.808.kick~ — TR-808-style bass drum voice.
/// @details    A thin Min wrapper around the portable kernel in tr808_kick.h (all DSP lives
///             there — see its header comment for the circuit analysis and provenance). The
///             voice is a circuit-informed model of the Roland TR-808 bass drum after Werner,
///             Abel & Smith (DAFx-14), with component values from the TR-808 Service Notes:
///             pulse shaper, bridged-T resonator, the Q43 attack shift, the retriggering pulse,
///             the pitch-sigh leakage nonlinearity, and the feedback-buffer decay loop.
///
///             Family conventions (the tap.808.* contract): the inlet takes a signal-rate
///             trigger — a rising edge fires the voice, and the edge's amplitude (0..1) is the
///             accent, mapped to the hardware's 4-14 V trigger bus, exciting the circuit harder
///             rather than scaling the output. `bang` triggers at full accent from the scheduler;
///             `trigger <float>` does the same with an accent value. Filter states persist
///             across triggers, so fast rolls interfere like the hardware (no machine-gun
///             effect). Single-channel; wrap in mc. for stacked voices.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/tr808_kick.h>

#include "c74_min.h"

using namespace c74::min;

class kick808 : public object<kick808>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so attribute setters can forward into it.
    taptools::tr808::kick   m_kick;
    sample                  m_previous{0.0};
    static constexpr double k_edge_threshold{1e-3};

  public:
    MIN_DESCRIPTION{"A TR-808-style bass drum voice, synthesized from the analog circuit. "
                    "A circuit-informed model of the original bridged-T voice — the ~49 Hz ring, "
                    "the punchy attack frequency jump, the retriggering pulse, and the "
                    "characteristic pitch sigh all emerge from the modeled schematic rather than "
                    "being bolted on. Trigger with a signal rising edge (the edge's amplitude, "
                    "0..1, is the accent — it drives the circuit's 4-14 V trigger bus, exciting "
                    "the network harder, not just louder), or with bang / trigger messages. "
                    "Decay, tone, and level mirror the hardware panel; tuning, pulse width, "
                    "sigh, and attack depth are circuit bends beyond the panel, stock at their "
                    "defaults."};
    MIN_TAGS{"generators, drums"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.adsr~, tap.noise~, tap.vco~, click~, kick~"};

    inlet<>  m_in{this, "(signal) trigger: rising edge fires, edge amplitude 0..1 is the accent"};
    outlet<> m_out{this, "(signal) bass drum output", "signal"};

    kick808(const atoms& args = {}) { m_kick.prepare(samplerate()); }

    attribute<number> decay{this, "decay", 0.5, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                m_kick.set_decay(v);
                                return {v};
                            }},
                            description{"Note length, 0..1 (the panel decay knob, VR6). At the top of its travel "
                                        "the note rings for seconds, as on the hardware."}};

    attribute<number> tone{this, "tone", 0.5, setter{MIN_FUNCTION{
                               const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                               m_kick.set_tone(v);
                               return {v};
                           }},
                           description{"Attack brightness, 0..1 (the panel tone knob, VR5): 1 keeps the click "
                                       "(~7 kHz), 0 rounds it off (~300 Hz)."}};

    attribute<number> level{this, "level", 1.0, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                m_kick.set_level(v);
                                return {v};
                            }},
                            description{"Output level, 0..1 (the panel level knob, VR4)."}};

    attribute<number> tuning{this, "tuning", 1.0, setter{MIN_FUNCTION{
                                 const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.25, 4.0);
                                 m_kick.set_tuning(v);
                                 return {v};
                             }},
                             description{"Circuit bend: pitch as a ratio of the stock tuning (0.25..4). 1 is the "
                                         "schematic (~49 Hz); scales the bridged-T arm capacitors."}};

    attribute<number> pulse{this, "pulse", 1.0, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.05, 50.0);
                                m_kick.set_pulse_ms(v);
                                return {v};
                            }},
                            description{"Circuit bend: trigger pulse width in milliseconds (0.05..50). "
                                        "Stock hardware: 1 ms."}};

    attribute<number> sigh{this, "sigh", 1.0, setter{MIN_FUNCTION{
                               const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                               m_kick.set_sigh(v);
                               return {v};
                           }},
                           description{"Circuit bend: pitch-sigh depth, 0..1. 1 is stock; 0 disconnects the "
                                       "characteristic downward pitch relaxation."}};

    attribute<number> attack{this, "attack", 1.0, setter{MIN_FUNCTION{
                                 const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                 m_kick.set_attack(v);
                                 return {v};
                             }},
                             description{"Circuit bend: depth of the attack frequency shift, 0..1. 1 is stock "
                                         "(the punch); 0 disconnects it for a much softer, quieter thud."}};

    message<> bang{this, "bang", "Trigger the drum at full accent.",
                   MIN_FUNCTION{
                       m_kick.trigger(1.0);
                       return {};
                   }};

    message<> trigger{this, "trigger", "Trigger the drum with an accent value (0..1).",
                      MIN_FUNCTION{
                          const double accent = args.empty() ? 1.0 : MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                          m_kick.trigger(accent);
                          return {};
                      }};

    message<> clear{this, "clear", "Reset all circuit state (silences any ringing tail).",
                    MIN_FUNCTION{
                        m_kick.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare the kernel for the current sample rate.",
                       MIN_FUNCTION{
                           m_kick.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample x) {
        if (x > k_edge_threshold && m_previous <= k_edge_threshold) {
            m_kick.trigger(MIN_CLAMP(static_cast<double>(x), 0.0, 1.0));
        }
        m_previous = x;
        return m_kick.process();
    }
};

MIN_EXTERNAL(kick808);
