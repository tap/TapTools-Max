/// @file
/// @brief      tap.808.rim~ — TR-808-style rimshot/claves voice.
/// @details    A thin Min wrapper around the portable kernel in tr808_rim.h (all DSP lives
///             there — see its header comment for the circuit analysis and provenance). The
///             hardware's RS/CL channel: the rimshot's ~1667 + ~455 Hz resonator pair cut to a
///             ~10 ms crack by the harmonic-generating VCA; or, via @model, the claves' pure
///             ~2500 Hz ~25 ms tick — one circuit, the hardware's switch.
///
///             Family conventions (the tap.808.* contract): signal-rate trigger — a rising
///             edge fires the voice, the edge's amplitude (0..1) is the accent; bang / trigger
///             messages. Fully deterministic (no noise source in this channel).
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/tr808_rim.h>

#include "c74_min.h"

using namespace c74::min;

class rim808 : public object<rim808>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so attribute setters can forward into it.
    tap::tools::tr808::rim  m_rim;
    sample                  m_previous{0.0};
    static constexpr double k_edge_threshold{1e-3};

  public:
    MIN_DESCRIPTION{"A TR-808-style rimshot/claves voice, synthesized from the analog circuit. "
                    "The rimshot is the hardware's ~1667 and ~455 Hz resonator pair cut to a "
                    "~10 ms crack by its harmonic-generating VCA; the claves model is the same "
                    "channel's pure ~2500 Hz woody tick (the hardware's RS/CL switch). Trigger "
                    "with a signal rising edge (edge amplitude 0..1 is the accent) or with "
                    "bang / trigger messages."};
    MIN_TAGS{"generators, drums"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.808.tom~, tap.808.snare~, tap.808.clap~, click~"};

    inlet<>  m_in{this, "(signal) trigger: rising edge fires, edge amplitude 0..1 is the accent"};
    outlet<> m_out{this, "(signal) rimshot/claves output", "signal"};

    rim808(const atoms& args = {}) { m_rim.prepare(samplerate()); }

    attribute<symbol> model{this,
                            "model",
                            "rimshot",
                            range{"rimshot", "claves"},
                            setter{MIN_FUNCTION{
                                m_rim.set_model(args[0] == "claves" ? tap::tools::tr808::rim::model_claves
                                                                    : tap::tools::tr808::rim::model_rimshot);
                                return args;
                            }},
                            description{"Voice model: rimshot or claves (the hardware's RS/CL switch — one "
                                        "circuit, two sounds)."}};

    attribute<number> level{this, "level", 1.0, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                m_rim.set_level(v);
                                return {v};
                            }},
                            description{"Output level, 0..1 (VR16, the channel's level pot)."}};

    message<> bang{this, "bang", "Trigger the voice at full accent.",
                   MIN_FUNCTION{
                       m_rim.trigger(1.0);
                       return {};
                   }};

    message<> trigger{this, "trigger", "Trigger the voice with an accent value (0..1).",
                      MIN_FUNCTION{
                          const double accent = args.empty() ? 1.0 : MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                          m_rim.trigger(accent);
                          return {};
                      }};

    message<> clear{this, "clear", "Reset all circuit state.",
                    MIN_FUNCTION{
                        m_rim.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare the kernel for the current sample rate.",
                       MIN_FUNCTION{
                           m_rim.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample x) {
        if (x > k_edge_threshold && m_previous <= k_edge_threshold) {
            m_rim.trigger(MIN_CLAMP(static_cast<double>(x), 0.0, 1.0));
        }
        m_previous = x;
        return m_rim.process();
    }
};

MIN_EXTERNAL(rim808);
