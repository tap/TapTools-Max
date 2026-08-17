/// @file
/// @brief      tap.palme~ — the Ondes Martenot's string diffuseur, as a driven resonator.
/// @details    Wraps tap::tools::diffuseur::palme (taptools/diffuseur.h). The *palme* (1949–50)
///             is the strangest loudspeaker in music: an electromagnet driving twelve metal
///             strings stretched on a lyre-shaped soundboard. Anything sent through it comes back
///             with a halo, because whichever strings share partials with the signal ring in
///             sympathy and the rest stay quiet.
///
///             **Run a guitar through it.** That is the reason this object exists on its own
///             rather than only inside an ondes: sympathetic resonance is useful on any pitched
///             material, and the twelve chromatic strings mean it answers whatever key you are in.
///
///             **Driven, not struck**, and the driver is upstream: the signal reaches the
///             electromagnet first and its motion excites the strings, so @drive, @asymmetry and
///             @saturation sit before the body, which is the order the instrument wires them.
///
///             **Twelve strings**, per the peer-reviewed source (Wijnand et al., Forum Acusticum
///             2023). Widely copied hobbyist build pages say twenty-four; this follows the
///             peer-reviewed number. Their *tuning* is not published anywhere found, so @tuning
///             is a parameter rather than a reconstruction — chromatic across an octave (a string
///             for every pitch class) or the harmonic series on the root (a drone that answers
///             one key).
///
///             Note that @decay and @damping are not independent: a heavily damped string cannot
///             ring for the time you ask, and past that point the damping wins.
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

class palme : public object<palme>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::palme m_machine;

  public:
    MIN_DESCRIPTION{"The Ondes Martenot's string diffuseur: an electromagnet driving twelve metal "
                    "strings on a soundboard, here twelve damped waveguide loops behind a "
                    "moving-iron driver. Whichever strings share partials with the input ring in "
                    "sympathy and the rest stay quiet, which is the halo the instrument is known "
                    "for. Twelve strings per the peer-reviewed source, not the twenty-four of the "
                    "hobbyist pages; their tuning is not published anywhere found, so it is a "
                    "parameter. Run a guitar through it. For multichannel signals, wrap this "
                    "object in an mc. operator."};
    MIN_TAGS{"effects"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.metallique~, tap.touche~, tap.5comb~, tap.chime~"};

    inlet<>  m_in{this, "(signal) audio input — drives the electromagnet"};
    outlet<> m_out{this, "(signal) the strings, balanced against the input", "signal"};

    attribute<number> root{this, "root", kernel::k_default_root_hz, setter{MIN_FUNCTION{
                               const double v = std::clamp(static_cast<double>(args[0]), kernel::k_min_pitch_hz,
                                                           kernel::k_max_pitch_hz);
                               m_machine.set_root_hz(v);
                               return {v};
                           }},
                           description{"Pitch of the lowest string in Hz. The other eleven are laid out from it "
                                       "by the tuning attribute."}};

    attribute<int> tuning{this, "tuning", kernel::tuning_chromatic, setter{MIN_FUNCTION{
                              const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_tunings - 1);
                              m_machine.set_tuning(v);
                              return {v};
                          }},
                          description{"How the twelve strings are laid out: 0 chromatic across an octave from the "
                                      "root (a string for every pitch class, so the board answers whatever you "
                                      "play), 1 the harmonic series on the root (a drone that answers one key). "
                                      "Not a published detail of the instrument — a design choice, offered as a "
                                      "choice."}};

    attribute<number> decay{this, "decay", kernel::k_default_decay_s, setter{MIN_FUNCTION{
                                const double v =
                                    std::clamp(static_cast<double>(args[0]), kernel::k_min_t60, kernel::k_max_t60);
                                m_machine.set_decay(v);
                                return {v};
                            }},
                            description{"Ring time of a string in seconds (T60). Not independent of damping: a "
                                        "heavily damped string cannot ring for the time you ask, and past that "
                                        "point the damping decides."}};

    attribute<number> damping{this, "damping", kernel::k_default_damp_hz, setter{MIN_FUNCTION{
                                  const double v = std::clamp(static_cast<double>(args[0]), kernel::k_min_damp_hz,
                                                              kernel::k_max_damp_hz);
                                  m_machine.set_damping(v);
                                  return {v};
                              }},
                              description{"In-loop damping corner in Hz: how quickly a string loses its upper "
                                          "partials. Low values are felt cloth on the strings."}};

    attribute<number> detune{this, "detune", kernel::k_default_detune_c, setter{MIN_FUNCTION{
                                 const double v = std::clamp(static_cast<double>(args[0]), 0.0, 50.0);
                                 m_machine.set_detune(v);
                                 return {v};
                             }},
                             description{"Depth in cents of the fixed per-string scatter — no two strings on a "
                                         "real board are in perfect relation. Deterministic, so the board is the "
                                         "same board in every instance. This is scatter between strings, not "
                                         "stiffness within one: the partials of each string stay exactly "
                                         "harmonic."}};

    attribute<number> drive{this, "drive", kernel::k_default_drive, setter{MIN_FUNCTION{
                                const double v = std::max(0.0, static_cast<double>(args[0]));
                                m_machine.set_drive(v);
                                return {v};
                            }},
                            description{"Linear gain into the electromagnet, before the strings."}};

    attribute<number> asymmetry{this, "asymmetry", kernel::k_default_asymmetry,
                                setter{MIN_FUNCTION{
                                    const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                    m_machine.set_asymmetry(v);
                                    return {v};
                                }},
                                description{"The moving-iron squared term, 0 to 1: force follows the square of the "
                                            "gap flux, so this puts a second harmonic on the signal in proportion "
                                            "to level. 0 is a linear driver — legitimate, but then a documented "
                                            "stage of the instrument is missing."}};

    attribute<number> saturation{this, "saturation", kernel::k_default_sat, setter{MIN_FUNCTION{
                                     const double v = std::max(0.0, static_cast<double>(args[0]));
                                     m_machine.set_saturation(v);
                                     return {v};
                                 }},
                                 description{"Soft-clip amount after the squared term; 0 is exactly linear. A "
                                             "modelling necessity rather than a measured stage — the squared law "
                                             "is expansive and something has to bound it."}};

    attribute<number> mix{this, "mix", kernel::k_default_mix, setter{MIN_FUNCTION{
                              const double v = std::clamp(static_cast<double>(args[0]), 0.0, 100.0);
                              m_machine.set_mix(v);
                              return {v};
                          }},
                          description{"Balance between the dry input and the diffuseur, 0 to 100, equal-power. "
                                      "The ends are exact: 0 is the input bit for bit, 100 is the strings alone."}};

    attribute<number> level{this, "level", kernel::k_default_level, setter{MIN_FUNCTION{
                                const double v = static_cast<double>(args[0]);
                                m_machine.set_level(v);
                                return {v};
                            }},
                            description{"Output level, linear. Twelve resonant loops add up — a driven board can "
                                        "be much louder than what went into it."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_machine.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for the driver, mix and level setters, in ms "
                                         "(0 = instant). The strings' own parameters are not ramped."}};

    message<> clear{this, "clear", "Damp every string and reset the driver. Parameters are kept.",
                    MIN_FUNCTION{
                        m_machine.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Prepare the board for the current sample rate.",
                       MIN_FUNCTION{
                           m_machine.prepare(samplerate());
                           return {};
                       }};

    palme(const atoms& args = {}) { m_machine.prepare(samplerate()); }

    sample operator()(sample x) { return m_machine.process(x); }
};

MIN_EXTERNAL(palme);
