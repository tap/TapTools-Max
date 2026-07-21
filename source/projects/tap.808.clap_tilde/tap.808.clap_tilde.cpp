/// @file
/// @brief      tap.808.clap~ — TR-808-style handclap/maracas voice.
/// @details    A thin Min wrapper around the portable kernel in tr808_clap.h (all DSP lives
///             there — see its header comment for the circuit analysis and provenance). The
///             hardware's CP/MA channel: the clap's ~2 kHz band-passed noise driven by the
///             three-teeth sawtooth envelope (the "multiple hands" transient, Service Notes
///             Figure 13) plus the reverberation tail; or, via @model, the maracas' short
///             bright noise burst. The hardware switches one circuit between the two sounds —
///             here that switch is the model attribute.
///
///             Family conventions (the tap.808.* contract): signal-rate trigger — a rising
///             edge fires the voice, the edge's amplitude (0..1) is the accent; `bang` /
///             `trigger <float>` messages. Single-channel; wrap in mc. and vary @seed to
///             decorrelate the noise.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/tr808_clap.h>

#include "c74_min.h"

using namespace c74::min;

class clap808 : public object<clap808>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so attribute setters can forward into it.
    tap::tools::tr808::clap m_clap;
    sample                  m_previous{0.0};
    static constexpr double k_edge_threshold{1e-3};

  public:
    MIN_DESCRIPTION{"A TR-808-style handclap/maracas voice, synthesized from the analog "
                    "circuit. The clap is band-passed noise (~2 kHz, per the schematic) driven "
                    "by the hardware's three-teeth sawtooth envelope — the 'multiple hands' "
                    "transient — plus a longer reverberation wash; the maracas model is the "
                    "same channel's short bright noise burst (the hardware's CP/MA switch). "
                    "Trigger with a signal rising edge (edge amplitude 0..1 is the accent) or "
                    "with bang / trigger messages."};
    MIN_TAGS{"generators, drums"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.808.kick~, tap.808.snare~, tap.noise~, click~"};

    inlet<>  m_in{this, "(signal) trigger: rising edge fires, edge amplitude 0..1 is the accent"};
    outlet<> m_out{this, "(signal) handclap/maracas output", "signal"};

    clap808(const atoms& args = {}) { m_clap.prepare(samplerate()); }

    attribute<symbol> model{this,
                            "model",
                            "clap",
                            range{"clap", "maracas"},
                            setter{MIN_FUNCTION{
                                m_clap.set_model(args[0] == "maracas" ? tap::tools::tr808::clap::model_maracas
                                                                      : tap::tools::tr808::clap::model_clap);
                                return args;
                            }},
                            description{"Voice model: clap or maracas (the hardware's CP/MA switch — one circuit, "
                                        "two sounds)."}};

    attribute<number> level{this, "level", 1.0, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                m_clap.set_level(v);
                                return {v};
                            }},
                            description{"Output level, 0..1 (the panel level knob, VR17)."}};

    attribute<number> tail{this, "tail", 1.0, setter{MIN_FUNCTION{
                               const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 2.0);
                               m_clap.set_tail(v);
                               return {v};
                           }},
                           description{"Circuit bend (clap model): reverberation-tail level, 0..2. 1 is stock; 0 "
                                       "disconnects the wash, leaving only the three teeth."}};

    attribute<number> drive{this, "drive", 0.0, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 12.0);
                                m_clap.set_drive(v);
                                return {v};
                            }},
                            description{"Circuit bend: swing-VCA drive on the output VCA (0..12), for both the "
                                        "clap and maracas models. 0 is the calibrated linear model (default); "
                                        "higher engages the swing VCA's symmetric harmonic saturation — grit and "
                                        "compression that ride the envelope."}};

    attribute<int> seed{this, "seed", 1, setter{MIN_FUNCTION{
                            const int v = std::max(1, static_cast<int>(args[0]));
                            m_clap.set_seed(static_cast<uint64_t>(v));
                            return {v};
                        }},
                        description{"Noise seed (>= 1). Deterministic; give mc. instances different seeds to "
                                    "decorrelate their noise."}};

    message<> bang{this, "bang", "Trigger the voice at full accent.",
                   MIN_FUNCTION{
                       m_clap.trigger(1.0);
                       return {};
                   }};

    message<> trigger{this, "trigger", "Trigger the voice with an accent value (0..1).",
                      MIN_FUNCTION{
                          const double accent = args.empty() ? 1.0 : MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                          m_clap.trigger(accent);
                          return {};
                      }};

    message<> clear{this, "clear", "Reset all circuit state.",
                    MIN_FUNCTION{
                        m_clap.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare the kernel for the current sample rate.",
                       MIN_FUNCTION{
                           m_clap.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample x) {
        if (x > k_edge_threshold && m_previous <= k_edge_threshold) {
            m_clap.trigger(MIN_CLAMP(static_cast<double>(x), 0.0, 1.0));
        }
        m_previous = x;
        return m_clap.process();
    }
};

MIN_EXTERNAL(clap808);
