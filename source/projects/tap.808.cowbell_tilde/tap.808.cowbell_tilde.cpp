/// @file
/// @brief      tap.808.cowbell~ — TR-808-style cowbell voice.
/// @details    A thin Min wrapper around the portable kernel in tr808_cowbell.h (all DSP lives
///             there — see its header comment for the circuit analysis and provenance): the
///             metal bank's factory-trimmed 540/800 Hz pair through the two-slope attack-heavy
///             envelope and the ~860 Hz voicing filter, straight off the voicing-board
///             schematic.
///
///             Family conventions (the tap.808.* contract): signal-rate trigger — a rising
///             edge fires the voice, the edge's amplitude (0..1) is the accent; bang / trigger
///             messages. Single-channel; wrap in mc. and vary @seed / @tolerance.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/tr808_cowbell.h>

#include "c74_min.h"

using namespace c74::min;

class cowbell808 : public object<cowbell808>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so attribute setters can forward into it.
    taptools::tr808::cowbell m_cowbell;
    sample                   m_previous{0.0};
    static constexpr double  k_edge_threshold{1e-3};

  public:
    MIN_DESCRIPTION{"A TR-808-style cowbell voice, synthesized from the analog circuit: the "
                    "metal bank's factory-trimmed 540 and 800 Hz square pair through the "
                    "hardware's two-slope attack-heavy envelope and ~860 Hz voicing filter. "
                    "Trigger with a signal rising edge (edge amplitude 0..1 is the accent) or "
                    "with bang / trigger messages."};
    MIN_TAGS{"generators, drums"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.808.hat~, tap.808.cymbal~, tap.808.kick~, click~"};

    inlet<>  m_in{this, "(signal) trigger: rising edge fires, edge amplitude 0..1 is the accent"};
    outlet<> m_out{this, "(signal) cowbell output", "signal"};

    cowbell808(const atoms& args = {}) { m_cowbell.prepare(samplerate()); }

    attribute<number> level{this, "level", 1.0, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                m_cowbell.set_level(v);
                                return {v};
                            }},
                            description{"Output level, 0..1 (the panel level knob, VR5 — the cowbell's only "
                                        "hardware control)."}};

    attribute<number> tuning{this, "tuning", 1.0, setter{MIN_FUNCTION{
                                 const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.25, 4.0);
                                 m_cowbell.set_tuning(v);
                                 return {v};
                             }},
                             description{"Circuit bend: pitch ratio of the 540/800 Hz pair (0.25..4; 1 = stock — "
                                         "the hardware's internal trimpots, brought out)."}};

    attribute<number> tolerance{this, "tolerance", 0.0, setter{MIN_FUNCTION{
                                    const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                    m_cowbell.set_tolerance(v);
                                    return {v};
                                }},
                                description{"Per-unit component spread, 0..1 (0 = exact nominal). Deterministic "
                                            "per seed."}};

    attribute<int> seed{this, "seed", 1, setter{MIN_FUNCTION{
                            const int v = std::max(1, static_cast<int>(args[0]));
                            m_cowbell.set_seed(static_cast<uint64_t>(v));
                            return {v};
                        }},
                        description{"Unit identity for the tolerance spread (>= 1)."}};

    message<> bang{this, "bang", "Trigger the cowbell at full accent.",
                   MIN_FUNCTION{
                       m_cowbell.trigger(1.0);
                       return {};
                   }};

    message<> trigger{this, "trigger", "Trigger the cowbell with an accent value (0..1).",
                      MIN_FUNCTION{
                          const double accent = args.empty() ? 1.0 : MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                          m_cowbell.trigger(accent);
                          return {};
                      }};

    message<> clear{this, "clear", "Reset all circuit state.",
                    MIN_FUNCTION{
                        m_cowbell.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare the kernel for the current sample rate.",
                       MIN_FUNCTION{
                           m_cowbell.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample x) {
        if (x > k_edge_threshold && m_previous <= k_edge_threshold) {
            m_cowbell.trigger(MIN_CLAMP(static_cast<double>(x), 0.0, 1.0));
        }
        m_previous = x;
        return m_cowbell.process();
    }
};

MIN_EXTERNAL(cowbell808);
