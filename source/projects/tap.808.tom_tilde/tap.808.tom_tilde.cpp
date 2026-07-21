/// @file
/// @brief      tap.808.tom~ — TR-808-style tom/conga voice.
/// @details    A thin Min wrapper around the portable kernel in tr808_tom.h (all DSP lives
///             there — see its header comment for the circuit analysis and provenance). The
///             hardware's three tom/conga channels: a bridged-T resonator with the D80/D81
///             attack pitch fall, the panel tuning knob sweeping the documented span per size,
///             and — for the toms only — the pink-noise "artificial reverberation" layer.
///             The size attribute picks the channel (low/mid/high); the model attribute is the
///             hardware's tom/conga switch.
///
///             Family conventions (the tap.808.* contract): signal-rate trigger — a rising
///             edge fires the voice, the edge's amplitude (0..1) is the accent; bang / trigger
///             messages. Single-channel; wrap in mc. and vary @seed to decorrelate tom noise.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/tr808_tom.h>

#include "c74_min.h"

using namespace c74::min;

class tom808 : public object<tom808>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so attribute setters can forward into it.
    tap::tools::tr808::tom  m_tom;
    sample                  m_previous{0.0};
    static constexpr double k_edge_threshold{1e-3};

  public:
    MIN_DESCRIPTION{"A TR-808-style tom/conga voice, synthesized from the analog circuit: a "
                    "bridged-T resonator whose attack rings sharp and falls as it damps (the "
                    "hardware's diode behavior), with the panel tuning knob sweeping the "
                    "documented span per size — toms add the pink-noise 'artificial "
                    "reverberation' layer, congas ring pure, exactly like the hardware's "
                    "switch. Trigger with a signal rising edge (edge amplitude 0..1 is the "
                    "accent) or with bang / trigger messages."};
    MIN_TAGS{"generators, drums"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.808.kick~, tap.808.snare~, tap.808.rim~, click~"};

    inlet<>  m_in{this, "(signal) trigger: rising edge fires, edge amplitude 0..1 is the accent"};
    outlet<> m_out{this, "(signal) tom/conga output", "signal"};

    tom808(const atoms& args = {}) { m_tom.prepare(samplerate()); }

    attribute<symbol> size{this,
                           "size",
                           "low",
                           range{"low", "mid", "high"},
                           setter{MIN_FUNCTION{
                               const int s = args[0] == "high" ? 2 : (args[0] == "mid" ? 1 : 0);
                               m_tom.set_size(s);
                               return args;
                           }},
                           description{"Which of the hardware's three channels: low, mid, or high."}};

    attribute<symbol> model{this,
                            "model",
                            "tom",
                            range{"tom", "conga"},
                            setter{MIN_FUNCTION{
                                m_tom.set_model(args[0] == "conga" ? tap::tools::tr808::tom::model_conga
                                                                   : tap::tools::tr808::tom::model_tom);
                                return args;
                            }},
                            description{"Voice model: tom (with the noise layer) or conga (pure ring) — the "
                                        "hardware's panel switch."}};

    attribute<number> tuning{this, "tuning", 0.5, setter{MIN_FUNCTION{
                                 const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                 m_tom.set_tuning(v);
                                 return {v};
                             }},
                             description{"Tuning knob, 0..1 (VR11/13/15): sweeps the documented span for the "
                                         "selected size and model (e.g. mid tom 120..160 Hz)."}};

    attribute<number> level{this, "level", 1.0, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                m_tom.set_level(v);
                                return {v};
                            }},
                            description{"Output level, 0..1."}};

    attribute<number> drive{this, "drive", 0.0, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 12.0);
                                m_tom.set_drive(v);
                                return {v};
                            }},
                            description{"Circuit bend: swing-VCA drive on the noise 'reverberation' layer (0..12; "
                                        "toms only — congas have no noise layer). 0 is the calibrated linear model "
                                        "(default); higher engages the swing VCA's symmetric harmonic saturation "
                                        "riding the noise envelope."}};

    attribute<int> seed{this, "seed", 1, setter{MIN_FUNCTION{
                            const int v = std::max(1, static_cast<int>(args[0]));
                            m_tom.set_seed(static_cast<uint64_t>(v));
                            return {v};
                        }},
                        description{"Noise seed (>= 1) for the tom's noise layer. Deterministic; give mc. "
                                    "instances different seeds."}};

    message<> bang{this, "bang", "Trigger the voice at full accent.",
                   MIN_FUNCTION{
                       m_tom.trigger(1.0);
                       return {};
                   }};

    message<> trigger{this, "trigger", "Trigger the voice with an accent value (0..1).",
                      MIN_FUNCTION{
                          const double accent = args.empty() ? 1.0 : MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                          m_tom.trigger(accent);
                          return {};
                      }};

    message<> clear{this, "clear", "Reset all circuit state.",
                    MIN_FUNCTION{
                        m_tom.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare the kernel for the current sample rate.",
                       MIN_FUNCTION{
                           m_tom.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample x) {
        if (x > k_edge_threshold && m_previous <= k_edge_threshold) {
            m_tom.trigger(MIN_CLAMP(static_cast<double>(x), 0.0, 1.0));
        }
        m_previous = x;
        return m_tom.process();
    }
};

MIN_EXTERNAL(tom808);
