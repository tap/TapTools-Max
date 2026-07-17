/// @file
/// @brief      tap.808.hat~ — TR-808-style hi-hat voice (closed + open, one circuit).
/// @details    A thin Min wrapper around the portable kernel in tr808_hat.h (all DSP lives
///             there — see its header comment for the circuit analysis and provenance). On the
///             TR-808 the closed and open hi-hats are one circuit sharing one sound source —
///             which is why this is one object with two trigger inlets, and why the hardware
///             choke works: a closed hit terminates a sounding open hat (Q23/R173).
///
///             Family conventions (the tap.808.* contract): signal-rate triggers — a rising
///             edge fires the path, the edge's amplitude (0..1) is the accent; bang / trigger
///             messages for the scheduler. Single-channel out; wrap in mc. and vary @seed /
///             @tolerance for a spread of units.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/tr808_hat.h>

#include "c74_min.h"

using namespace c74::min;

class hat808 : public object<hat808>, public sample_operator<2, 1> {
  private:
    // Constructed before the attributes below so attribute setters can forward into it.
    taptools::tr808::hat    m_hat;
    sample                  m_prev_closed{0.0};
    sample                  m_prev_open{0.0};
    static constexpr double k_edge_threshold{1e-3};

  public:
    MIN_DESCRIPTION{"A TR-808-style hi-hat voice, synthesized from the analog circuit. Closed "
                    "and open are one circuit sharing the six-oscillator metal bank — so this "
                    "is one object with two trigger inlets, and a closed hit chokes a sounding "
                    "open hat exactly like the hardware. Trigger with signal rising edges (edge "
                    "amplitude 0..1 is the accent) or with bang / trigger (closed) and open "
                    "messages. The open hat's decay mirrors the panel knob; the closed hat is "
                    "fixed, like the hardware. Tolerance spreads the six oscillators per seed — "
                    "every seed is a different unit off the line."};
    MIN_TAGS{"generators, drums"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.808.cymbal~, tap.808.cowbell~, tap.808.kick~, click~"};

    inlet<>  m_in_closed{this, "(signal) closed-hat trigger: rising edge fires, amplitude = accent"};
    inlet<>  m_in_open{this, "(signal) open-hat trigger: rising edge fires, amplitude = accent"};
    outlet<> m_out{this, "(signal) hi-hat output", "signal"};

    hat808(const atoms& args = {}) { m_hat.prepare(samplerate()); }

    attribute<number> decay{this, "decay", 0.5, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                m_hat.set_decay(v);
                                return {v};
                            }},
                            description{"Open-hat ring length, 0..1 (the panel OH decay knob, VR3). The closed "
                                        "hat is fixed, like the hardware."}};

    attribute<number> closed_level{this, "closed_level", 1.0, setter{MIN_FUNCTION{
                                       const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                       m_hat.set_closed_level(v);
                                       return {v};
                                   }},
                                   description{"Closed-hat level, 0..1 (the CH channel's level pot)."}};

    attribute<number> open_level{this, "open_level", 1.0, setter{MIN_FUNCTION{
                                     const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                     m_hat.set_open_level(v);
                                     return {v};
                                 }},
                                 description{"Open-hat level, 0..1 (the OH channel's level pot)."}};

    attribute<number> tuning{this, "tuning", 1.0, setter{MIN_FUNCTION{
                                 const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.25, 4.0);
                                 m_hat.set_tuning(v);
                                 return {v};
                             }},
                             description{"Circuit bend: pitch ratio of the oscillator bank (0.25..4; 1 = stock)."}};

    attribute<number> tolerance{this, "tolerance", 0.0, setter{MIN_FUNCTION{
                                    const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                    m_hat.set_tolerance(v);
                                    return {v};
                                }},
                                description{"Per-unit component spread of the six oscillators, 0..1 (0 = exact "
                                            "nominal; 1 = the full ~20% production tolerance). Deterministic per "
                                            "seed."}};

    attribute<int> seed{this, "seed", 1, setter{MIN_FUNCTION{
                            const int v = std::max(1, static_cast<int>(args[0]));
                            m_hat.set_seed(static_cast<uint64_t>(v));
                            return {v};
                        }},
                        description{"Unit identity for the tolerance spread (>= 1). Give mc. instances different "
                                    "seeds for a rack of different 808s."}};

    message<> bang{this, "bang", "Trigger the closed hat at full accent.",
                   MIN_FUNCTION{
                       m_hat.trigger_closed(1.0);
                       return {};
                   }};

    message<> trigger{this, "trigger", "Trigger the closed hat with an accent value (0..1).",
                      MIN_FUNCTION{
                          const double accent = args.empty() ? 1.0 : MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                          m_hat.trigger_closed(accent);
                          return {};
                      }};

    message<> open{this, "open", "Trigger the open hat with an accent value (0..1, default 1).",
                   MIN_FUNCTION{
                       const double accent = args.empty() ? 1.0 : MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                       m_hat.trigger_open(accent);
                       return {};
                   }};

    message<> clear{this, "clear", "Reset all circuit state.",
                    MIN_FUNCTION{
                        m_hat.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare the kernel for the current sample rate.",
                       MIN_FUNCTION{
                           m_hat.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample closed, sample open_in) {
        if (closed > k_edge_threshold && m_prev_closed <= k_edge_threshold) {
            m_hat.trigger_closed(MIN_CLAMP(static_cast<double>(closed), 0.0, 1.0));
        }
        if (open_in > k_edge_threshold && m_prev_open <= k_edge_threshold) {
            m_hat.trigger_open(MIN_CLAMP(static_cast<double>(open_in), 0.0, 1.0));
        }
        m_prev_closed = closed;
        m_prev_open   = open_in;
        return m_hat.process();
    }
};

MIN_EXTERNAL(hat808);
