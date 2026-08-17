/// @file
/// @brief      tap.ondes~ — the Ondes Martenot voice, minus its loudspeaker.
/// @details    Wraps tap::tools::ondes::voice (taptools/ondes.h). A **source**, not an effect:
///             the heterodyne envelope detector into the demodulator triode into the
///             preamplifier triode into the intensity key. Patch a <o>tap.palme~</o> or a
///             <o>tap.metallique~</o> after it and you have the instrument.
///
///             **It is not a VCO with waveform switches.** The Ondes Martenot is heterodyne: two
///             oscillators near 80 kHz, one fixed and one moved by the ribbon, summed into an
///             amplitude-modulated signal whose envelope is the note. Najnudel, Hélie, Roze &
///             Boutin (IEEE/ACM TASLP 28, 2020) measure those oscillators at about 0.03 % second
///             harmonic even coupled to the rest of the circuit, so the character is entirely in
///             what comes after them.
///
///             **The demodulator is the biggest source of harmonics, before any valve.** The
///             envelope of two equal oscillators is 2|cos|, which puts the second harmonic 14 dB
///             below the fundamental and the third 21 dB down. That is why this object does not
///             synthesize a difference tone: it would throw the instrument's timbre away. The
///             carrier is not simulated either — the envelope has a closed form, and running the
///             published 200 µs detector on it matches a full 80 kHz simulation to within a tenth
///             of a dB on every harmonic.
///
///             The performance surface is the instrument's. @ribbon is **semitones above A1**,
///             because the published ribbon law is linear in semitones — a hand moving at
///             constant speed makes a constant-rate glissando, and nothing here quantizes.
///             @key is the intensity key (see <o>tap.touche~</o>, whose published curve this
///             uses): the bottom 45 % of its travel is silent, which is the key bending before it
///             reaches the powder bag. Both take signals.
///
///             Two controls exist because the sources do not settle them, and both measure as
///             audible: @keyplacement (the paper's five stages do not include the intensity key,
///             so whether pressure means level or dirt is a choice) and @polarity (the winding
///             sense of the transformer between the two valves, which decides which side of the
///             waveform the preamplifier's asymmetry acts on).
///
///             What is missing, and deliberately: the **waveform registers**. The real instrument
///             has switchable timbres whose filter shapes are in none of the sources obtained,
///             and inventing them is the one thing this object will not do.
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/ondes.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::ondes;

class ondes : public object<ondes>, public sample_operator<2, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::voice m_voice;

  public:
    MIN_DESCRIPTION{"The Ondes Martenot voice, minus its loudspeaker: a heterodyne envelope "
                    "detector into two triode stages into the intensity key. Not a VCO — the "
                    "instrument is heterodyne, its oscillators measure as essentially pure, and "
                    "the harmonics come from the demodulator and the valves. A source: patch "
                    "tap.palme~ or tap.metallique~ after it for the rest of the instrument. "
                    "ribbon is semitones above A1 because the published ribbon law is linear in "
                    "semitones; key is the published intensity-key curve, whose bottom 45% is "
                    "silent. No waveform registers — no source obtained describes them."};
    MIN_TAGS{"synth"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.touche~, tap.palme~, tap.metallique~, tap.triode~"};

    inlet<>  m_in_ribbon{this, "(signal) ribbon position, semitones above A1"};
    inlet<>  m_in_key{this, "(signal) intensity key, 0 to 1 of the travel"};
    outlet<> m_out{this, "(signal) the voice, before any diffuseur", "signal"};

    attribute<number> ribbon{this, "ribbon", 24.0, setter{MIN_FUNCTION{
                                 const double v =
                                     std::clamp(static_cast<double>(args[0]), 0.0, kernel::k_max_semitones);
                                 m_voice.set_ribbon(v);
                                 return {v};
                             }},
                             description{"The note, in semitones above A1 (55 Hz). The published ribbon law is "
                                         "linear in semitones, so this really is where the hand is — move it "
                                         "linearly and you get a linear glissando. Fractional and unquantized, "
                                         "because the instrument is. Ignored while a signal is connected to the "
                                         "left inlet."}};

    attribute<number> key{this, "key", 1.0, setter{MIN_FUNCTION{
                              const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                              m_voice.set_key(v);
                              return {v};
                          }},
                          description{"The intensity key, 0 to 1 over its physical travel — the published curve "
                                      "from tap.touche~. Roughly the bottom 45% is silent, which is the key "
                                      "bending before it reaches the powder bag rather than a dead spot in the "
                                      "object. Ignored while a signal is connected to the right inlet."}};

    attribute<number> depth{this, "depth", kernel::k_default_depth, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                m_voice.set_depth(v);
                                return {v};
                            }},
                            description{"Relative amplitude of the second oscillator, 0 to 1. At 1 (the published "
                                        "case, equal amplitudes) the envelope closes completely and the harmonic "
                                        "series is full; below that it never closes and the tone thins toward a "
                                        "sinusoid. A real mismatch between two oscillators, and the cheapest "
                                        "timbre control here."}};

    attribute<number> detect{this, "detect", kernel::k_detect_ms, setter{MIN_FUNCTION{
                                 const double v = std::clamp(static_cast<double>(args[0]), kernel::k_min_detect_ms,
                                                             kernel::k_max_detect_ms);
                                 m_voice.set_detect_ms(v);
                                 return {v};
                             }},
                             description{"Envelope-detector time constant in ms. The published value is 0.2 "
                                         "(R4 x C21 = 1 M x 200 pF). Longer smooths the envelope and takes the "
                                         "harmonics with it; it is also why the instrument gets purer and quieter "
                                         "as it goes up, since the RC cannot follow a fast envelope back down."}};

    attribute<number> drive{this, "drive", 1.0, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), 0.0, 8.0);
                                m_voice.set_drive(v);
                                return {v};
                            }},
                            description{"Grid drive into the two valve stages, as a multiple of the published "
                                        "nominal. The harmonics control — the circuit paper's own plugin exposes "
                                        "demodulator input gain the same way, a knob the real instrument does not "
                                        "have. Normalized, so it changes the distortion and not the level. Note "
                                        "that at 0 the tone is still harmonically rich, because the demodulator "
                                        "made those harmonics."}};

    attribute<int> keyplacement{this, "keyplacement", kernel::key_after, setter{MIN_FUNCTION{
                                    const int v =
                                        std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_key_placements - 1);
                                    m_voice.set_key_placement(v);
                                    return {v};
                                }},
                                description{"Where the intensity key sits: 0 after the valves (a clean output law "
                                            "— pressure is level), 1 before them (pressure drives the valves, so "
                                            "soft is clean and hard is dirty). The circuit paper's five stages do "
                                            "not include the key, so this is a choice rather than a "
                                            "reconstruction — and it is a real one, worth about 0.09 of total "
                                            "harmonic content at a half-press."}};

    attribute<int> polarity{this, "polarity", 1, setter{MIN_FUNCTION{
                                const int v = (static_cast<int>(args[0]) < 0) ? -1 : 1;
                                m_voice.set_polarity(v);
                                return {v};
                            }},
                            description{"Sign of the coupling between the two valve stages, 1 or -1. They are "
                                        "coupled through a transformer whose winding sense is not in the source, "
                                        "and the sign decides which side of the waveform the preamplifier's "
                                        "asymmetry acts on. Audible: the two settings differ by about 0.12 of "
                                        "total harmonic content."}};

    attribute<bool> power{this, "power", false, setter{MIN_FUNCTION{
                              const bool v = static_cast<bool>(args[0]);
                              m_voice.set_power_stage(v);
                              return {v};
                          }},
                          description{"Run the 2A3 power stage. Off by default, following the circuit paper: they "
                                      "measure almost 5% second harmonic there but report its contribution as "
                                      "much less important than the two stages before it, and drop it for "
                                      "real-time. Measured here, switching it on moves total harmonic content by "
                                      "about 0.003 — so they were right, and it is a switch rather than a "
                                      "deletion."}};

    attribute<number> level{this, "level", kernel::k_default_level, setter{MIN_FUNCTION{
                                const double v = static_cast<double>(args[0]);
                                m_voice.set_level(v);
                                return {v};
                            }},
                            description{"Output level, linear."}};

    attribute<int> oversample{this, "oversample", kernel::k_default_os, setter{MIN_FUNCTION{
                                  const int raw = std::clamp(static_cast<int>(args[0]), 1, kernel::k_max_oversample);
                                  const int v   = (raw >= 8) ? 8 : (raw >= 4) ? 4 : (raw >= 2) ? 2 : 1;
                                  m_voice.set_oversample(v);
                                  return {v};
                              }},
                              description{"Oversampling for the nonlinear chain: 1, 2, 4 or 8. Every doubling is "
                                          "worth about 12 dB of alias rejection up to 4x, and 7-12 dB more at 8x "
                                          "in the top octave. 4x is the default because that is where the cost "
                                          "stops buying uniformly."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_voice.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for the attribute-driven drive, level and key, in "
                                         "ms (0 = instant). The signal inlets are not ramped — a control signal is "
                                         "assumed smooth already."}};

    message<> clear{this, "clear", "Silence the detector and every filter. Parameters are kept.",
                    MIN_FUNCTION{
                        m_voice.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Prepare the voice for the current sample rate.",
                       MIN_FUNCTION{
                           m_voice.prepare(samplerate());
                           return {};
                       }};

    ondes(const atoms& args = {}) { m_voice.prepare(samplerate()); }

    sample operator()(sample rib, sample k) {
        // Signals on the performance inlets play the instrument; otherwise the attributes do,
        // through their ramps. The signal path bypasses those ramps, because re-targeting a 20 ms
        // slew every sample would make the key lag the hand. With one inlet connected and the
        // other not, the unconnected side falls back to its attribute target — the simple,
        // predictable reading of a mixed patch.
        const bool rib_signal = m_in_ribbon.has_signal_connection();
        const bool key_signal = m_in_key.has_signal_connection();
        if (!rib_signal && !key_signal) {
            return m_voice.process();
        }
        return m_voice.process(rib_signal ? static_cast<double>(rib) : static_cast<double>(ribbon),
                               key_signal ? static_cast<double>(k) : static_cast<double>(key));
    }
};

MIN_EXTERNAL(ondes);
