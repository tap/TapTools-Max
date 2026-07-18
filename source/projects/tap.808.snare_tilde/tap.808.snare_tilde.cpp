/// @file
/// @brief      tap.808.snare~ — TR-808-style snare drum voice.
/// @details    A thin Min wrapper around the portable kernel in tr808_snare.h (all DSP lives
///             there — see its header comment for the circuit analysis and provenance). Two
///             bridged-T resonators (~173/336 Hz, the late-revision Roland tuning) blended by
///             the tone control, plus the "snappy" enveloped noise path, per the TR-808
///             Service Notes.
///
///             Family conventions (the tap.808.* contract): signal-rate trigger — a rising
///             edge fires the voice, the edge's amplitude (0..1) is the accent (the hardware's
///             4-14 V trigger bus); `bang` / `trigger <float>` messages; states persist across
///             triggers. Single-channel; wrap in mc. and vary @seed to decorrelate the noise.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/tr808_snare.h>

#include "c74_min.h"

using namespace c74::min;

class snare808 : public object<snare808>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so attribute setters can forward into it.
    taptools::tr808::snare  m_snare;
    sample                  m_previous{0.0};
    static constexpr double k_edge_threshold{1e-3};

  public:
    MIN_DESCRIPTION{"A TR-808-style snare drum voice, synthesized from the analog circuit: two "
                    "bridged-T resonators (~173 and ~336 Hz, the late-revision Roland tuning) "
                    "blended by the tone control, plus the snappy enveloped-noise path. Trigger "
                    "with a signal rising edge (edge amplitude 0..1 is the accent, driving the "
                    "hardware's 4-14 V trigger bus) or with bang / trigger messages. Tone, "
                    "snappy, and level mirror the hardware panel; tuning and seed are beyond-"
                    "panel extensions, stock at their defaults."};
    MIN_TAGS{"generators, drums"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.808.kick~, tap.808.clap~, tap.noise~, click~"};

    inlet<>  m_in{this, "(signal) trigger: rising edge fires, edge amplitude 0..1 is the accent"};
    outlet<> m_out{this, "(signal) snare drum output", "signal"};

    snare808(const atoms& args = {}) { m_snare.prepare(samplerate()); }

    attribute<number> tone{this, "tone", 0.5, setter{MIN_FUNCTION{
                               const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                               m_snare.set_tone(v);
                               return {v};
                           }},
                           description{"Resonator balance, 0..1 (the panel tone knob, VR8): 0 is all fundamental "
                                       "(~173 Hz), 1 all harmonic (~336 Hz)."}};

    attribute<number> snappy{this, "snappy", 0.5, setter{MIN_FUNCTION{
                                 const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                 m_snare.set_snappy(v);
                                 return {v};
                             }},
                             description{"Snappy amount, 0..1 (the panel snappy knob, VR9): depth of the bright "
                                         "enveloped noise burst."}};

    attribute<number> level{this, "level", 1.0, setter{MIN_FUNCTION{
                                const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                m_snare.set_level(v);
                                return {v};
                            }},
                            description{"Output level, 0..1 (the panel level knob, VR7)."}};

    attribute<number> tuning{this, "tuning", 1.0, setter{MIN_FUNCTION{
                                 const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.25, 4.0);
                                 m_snare.set_tuning(v);
                                 return {v};
                             }},
                             description{"Circuit bend: pitch as a ratio of the stock tuning (0.25..4), scaling "
                                         "both resonators together. 1 is the schematic (~173/336 Hz)."}};

    attribute<number> drive{this, "drive", 0.0, setter{MIN_FUNCTION{
                                 const double v = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 12.0);
                                 m_snare.set_drive(v);
                                 return {v};
                             }},
                             description{"Circuit bend: swing-VCA drive on the snappy noise path (0..12). 0 is the "
                                         "calibrated linear model (default); higher engages the swing VCA's "
                                         "symmetric harmonic saturation — grit and compression that ride the snappy "
                                         "envelope, hardest on the transient crack."}};

    attribute<int> seed{this, "seed", 1, setter{MIN_FUNCTION{
                            const int v = std::max(1, static_cast<int>(args[0]));
                            m_snare.set_seed(static_cast<uint64_t>(v));
                            return {v};
                        }},
                        description{"Noise seed (>= 1). Deterministic; give mc. instances different seeds to "
                                    "decorrelate their snappy noise."}};

    message<> bang{this, "bang", "Trigger the drum at full accent.",
                   MIN_FUNCTION{
                       m_snare.trigger(1.0);
                       return {};
                   }};

    message<> trigger{this, "trigger", "Trigger the drum with an accent value (0..1).",
                      MIN_FUNCTION{
                          const double accent = args.empty() ? 1.0 : MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                          m_snare.trigger(accent);
                          return {};
                      }};

    message<> clear{this, "clear", "Reset all circuit state (silences any ringing tail).",
                    MIN_FUNCTION{
                        m_snare.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare the kernel for the current sample rate.",
                       MIN_FUNCTION{
                           m_snare.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample x) {
        if (x > k_edge_threshold && m_previous <= k_edge_threshold) {
            m_snare.trigger(MIN_CLAMP(static_cast<double>(x), 0.0, 1.0));
        }
        m_previous = x;
        return m_snare.process();
    }
};

MIN_EXTERNAL(snare808);
