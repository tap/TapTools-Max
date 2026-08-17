/// @file
/// @brief      tap.metallique~ — the Ondes Martenot's gong diffuseur, as a driven resonator.
/// @details    Wraps tap::tools::diffuseur::metallique (taptools/diffuseur.h). The Ondes Martenot
///             does not have a loudspeaker, it has a rack of them, and choosing between them is
///             part of playing the instrument. The *métallique* (1944–45, patented 1947) is a gong
///             driven by a motor transducer: everything sent through it picks up the body of a
///             large metal plate.
///
///             **Driven, not struck.** There is no trigger here and no strike envelope. Whatever
///             you send in excites the plate continuously and the plate rings at its own rates —
///             which is why this is worth reaching for on material that has nothing to do with an
///             ondes. Put a snare through it. Put a whole mix through it at a low @mix.
///
///             **The order matters, and it is the instrument's.** The signal reaches the
///             transducer first and the transducer's motion excites the body, so @drive,
///             @asymmetry and @saturation sit *upstream* of the plate. Driving a distorted
///             waveform into a gong is a different sound from distorting a gong, and this object
///             does the former because that is what the machine does.
///
///             **Honest about what it is.** The instrument, its date, its excitation and its
///             moving-iron transducer come from the peer-reviewed sources (Najnudel et al., IEEE/
///             ACM TASLP 28, 2020; Wijnand et al., Forum Acusticum 2023). The *mode data does
///             not* — no ondes-specific measurement of the body exists in any of them — so the
///             ratios are Fletcher & Rossing's free circular plate. This is a recreation of the
///             general physics, not a model of Martenot's gong, and it is labelled that way in
///             the kernel header too.
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing. For
///             multichannel use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/diffuseur.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::diffuseur;

class metallique : public object<metallique>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::metallique m_machine;

  public:
    MIN_DESCRIPTION{"The Ondes Martenot's gong diffuseur as a driven resonator. Eight plate modes "
                    "at the free circular plate's transverse ratios, each a slowly beating "
                    "doublet, excited continuously by whatever you send in — there is no strike. "
                    "A moving-iron transducer sits upstream of the body, which is the order the "
                    "instrument wires them: drive hard and you are driving a distorted waveform "
                    "into a gong. The mode ratios are Fletcher & Rossing's general plate physics, "
                    "not a measurement of Martenot's instrument, so this is a recreation. Useful "
                    "on anything that wants a metal body, not just on an ondes. For multichannel "
                    "signals, wrap this object in an mc. operator."};
    MIN_TAGS{"effects"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.palme~, tap.touche~, tap.chime~, tap.5comb~"};

    inlet<>  m_in{this, "(signal) audio input — drives the transducer"};
    outlet<> m_out{this, "(signal) the body, balanced against the input", "signal"};

    attribute<number> pitch{this, "pitch", kernel::k_default_pitch_hz, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), kernel::k_min_pitch_hz,
                                                            kernel::k_max_pitch_hz);
                                m_machine.set_pitch_hz(v);
                                return {v};
                            }},
                            description{"Frequency of the plate's (2,0) mode in Hz — the body's perceived pitch. "
                                        "The other seven modes follow at the published ratios. This retunes "
                                        "sixteen resonators, so it is set-and-hold rather than a knob to sweep "
                                        "fast."}};

    attribute<number> decay{this, "decay", kernel::k_default_decay_s, setter{MIN_FUNCTION{
                                const double v =
                                    std::clamp(static_cast<double>(args[0]), kernel::k_min_t60, kernel::k_max_t60);
                                m_machine.set_decay(v);
                                return {v};
                            }},
                            description{"Ring time of the fundamental in seconds (T60 — the time to fall 60 dB). "
                                        "Long values are a drone; short ones are a plate reverb."}};

    attribute<number> tilt{this, "tilt", kernel::k_default_tilt, setter{MIN_FUNCTION{
                               const double v = std::clamp(static_cast<double>(args[0]), 0.0, 3.0);
                               m_machine.set_tilt(v);
                               return {v};
                           }},
                           description{"How much faster the upper modes die than the fundamental: their ring time "
                                       "is decay / ratio^tilt. 0 rings everything equally; 2 is the roughly-f^2 "
                                       "radiation damping of a struck bar. 1 is the default because a driven "
                                       "gong's shimmer lives in its upper modes."}};

    attribute<number> brightness{this, "brightness", kernel::k_default_bright,
                                 setter{MIN_FUNCTION{
                                     const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                     m_machine.set_brightness(v);
                                     return {v};
                                 }},
                                 description{"Weight on the upper modes, 0 to 1, steepening per mode so the "
                                             "highest partials go first. 1 is the full published weight table; 0 "
                                             "leaves only the fundamental doublet."}};

    attribute<number> drive{this, "drive", kernel::k_default_drive, setter{MIN_FUNCTION{
                                const double v = std::max(0.0, static_cast<double>(args[0]));
                                m_machine.set_drive(v);
                                return {v};
                            }},
                            description{"Linear gain into the transducer, before the body. This is where the "
                                        "object gets loud and dirty, and it is upstream of the plate on purpose."}};

    attribute<number> asymmetry{this, "asymmetry", kernel::k_default_asymmetry,
                                setter{MIN_FUNCTION{
                                    const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                    m_machine.set_asymmetry(v);
                                    return {v};
                                }},
                                description{"The moving-iron squared term, 0 to 1: force follows the square of the "
                                            "gap flux, so this puts a second harmonic on the signal in proportion "
                                            "to level and nothing at the third. 0 is a linear driver — legitimate, "
                                            "but then a documented stage of the instrument is missing."}};

    attribute<number> saturation{this, "saturation", kernel::k_default_sat, setter{MIN_FUNCTION{
                                     const double v = std::max(0.0, static_cast<double>(args[0]));
                                     m_machine.set_saturation(v);
                                     return {v};
                                 }},
                                 description{"Soft-clip amount after the squared term; 0 is exactly linear. It is "
                                             "here because a squared law is expansive and something has to bound "
                                             "it — a modelling necessity, not a measured stage. Output is bounded "
                                             "by 2/saturation."}};

    attribute<number> mix{this, "mix", kernel::k_default_mix, setter{MIN_FUNCTION{
                              const double v = std::clamp(static_cast<double>(args[0]), 0.0, 100.0);
                              m_machine.set_mix(v);
                              return {v};
                          }},
                          description{"Balance between the dry input and the diffuseur, 0 to 100, equal-power. "
                                      "The ends are exact: 0 is the input bit for bit, 100 is the body alone."}};

    attribute<number> level{this, "level", kernel::k_default_level, setter{MIN_FUNCTION{
                                const double v = static_cast<double>(args[0]);
                                m_machine.set_level(v);
                                return {v};
                            }},
                            description{"Output level, linear."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_machine.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for the transducer, mix and level setters, in ms "
                                         "(0 = instant). The body's own parameters are not ramped."}};

    message<> clear{this, "clear", "Silence the body and reset the driver. Parameters are kept.",
                    MIN_FUNCTION{
                        m_machine.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Prepare the cabinet for the current sample rate.",
                       MIN_FUNCTION{
                           m_machine.prepare(samplerate());
                           return {};
                       }};

    metallique(const atoms& args = {}) { m_machine.prepare(samplerate()); }

    sample operator()(sample x) { return m_machine.process(x); }
};

MIN_EXTERNAL(metallique);
