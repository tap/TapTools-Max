/// @file
/// @brief      tap.808.cymbal~ — TR-808-style cymbal voice.
/// @details    A thin Min wrapper around the portable kernel in tr808_cymbal.h (all DSP lives
///             there — see its header comment for the circuit analysis and provenance): the
///             six-oscillator metal bank through the ~3.4/7.1 kHz voicings, split into strike/
///             ring/body components with separate decays; the tone knob sets their ratio and
///             the decay knob the ring, per the hardware panel.
///
///             Family conventions (the tap.808.* contract): signal-rate trigger — a rising
///             edge fires the voice, the edge's amplitude (0..1) is the accent; bang / trigger
///             messages. Single-channel; wrap in mc. and vary @seed / @tolerance.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/tr808_cymbal.h>

#include "c74_min.h"

using namespace c74::min;

class cymbal808 : public object<cymbal808>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so attribute setters can forward into it.
    tap::tools::tr808::cymbal m_cymbal;
    sample                  m_previous{0.0};
    static constexpr double k_edge_threshold{1e-3};

  public:
    MIN_DESCRIPTION{"A TR-808-style cymbal voice, synthesized from the analog circuit: six "
                    "square oscillators through the ~3.4 and ~7.1 kHz voicings, split into "
                    "strike, ring, and body components with separate decays. Tone balances "
                    "strike against body; decay sets the ring, spanning the hardware's ~350 ms "
                    "to ~1.2 s. Trigger with a signal rising edge (edge amplitude 0..1 is the "
                    "accent) or with bang / trigger messages. Tolerance spreads the six "
                    "oscillators per seed — no two 808 cymbals sound alike, and neither do two "
                    "seeds."};
    MIN_TAGS{"generators, drums"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.808.hat~, tap.808.cowbell~, tap.808.kick~, click~"};

    inlet<>  m_in{this, "(signal) trigger: rising edge fires, edge amplitude 0..1 is the accent"};
    outlet<> m_out{this, "(signal) cymbal output", "signal"};

    cymbal808(const atoms& args = {}) { m_cymbal.prepare(samplerate()); }

    attribute<number> tone{this, "tone", 0.5, setter{MIN_FUNCTION{
                               const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                               m_cymbal.set_tone(v);
                               return {v};
                           }},
                           description{"Strike/body balance, 0..1 (the panel tone knob, VR4): 0 is all body, 1 all "
                                       "strike."}};

    attribute<number> decay{this, "decay", 0.5, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                m_cymbal.set_decay(v);
                                return {v};
                            }},
                            description{"Ring length, 0..1 (the panel decay knob, VR2): the hardware spans roughly "
                                        "350 ms to 1.2 s overall."}};

    attribute<number> level{this, "level", 1.0, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                m_cymbal.set_level(v);
                                return {v};
                            }},
                            description{"Output level, 0..1."}};

    attribute<number> tuning{this, "tuning", 1.0, setter{MIN_FUNCTION{
                                 const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.25, 4.0);
                                 m_cymbal.set_tuning(v);
                                 return {v};
                             }},
                             description{"Circuit bend: pitch ratio of the oscillator bank (0.25..4; 1 = stock)."}};

    attribute<number> tolerance{this, "tolerance", 0.0, setter{MIN_FUNCTION{
                                    const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                    m_cymbal.set_tolerance(v);
                                    return {v};
                                }},
                                description{"Per-unit component spread of the six oscillators, 0..1 (0 = exact "
                                            "nominal; 1 = the full ~20% production tolerance). Deterministic per "
                                            "seed."}};

    attribute<int> seed{this, "seed", 1, setter{MIN_FUNCTION{
                            const int v = std::max(1, static_cast<int>(args[0]));
                            m_cymbal.set_seed(static_cast<uint64_t>(v));
                            return {v};
                        }},
                        description{"Unit identity for the tolerance spread (>= 1)."}};

    message<> bang{this, "bang", "Trigger the cymbal at full accent.",
                   MIN_FUNCTION{
                       m_cymbal.trigger(1.0);
                       return {};
                   }};

    message<> trigger{this, "trigger", "Trigger the cymbal with an accent value (0..1).",
                      MIN_FUNCTION{
                          const double accent = args.empty() ? 1.0 : MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                          m_cymbal.trigger(accent);
                          return {};
                      }};

    message<> clear{this, "clear", "Reset all circuit state.",
                    MIN_FUNCTION{
                        m_cymbal.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare the kernel for the current sample rate.",
                       MIN_FUNCTION{
                           m_cymbal.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample x) {
        if (x > k_edge_threshold && m_previous <= k_edge_threshold) {
            m_cymbal.trigger(MIN_CLAMP(static_cast<double>(x), 0.0, 1.0));
        }
        m_previous = x;
        return m_cymbal.process();
    }
};

MIN_EXTERNAL(cymbal808);
