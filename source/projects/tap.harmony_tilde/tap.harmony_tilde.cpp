/// @file
/// @brief      tap.harmony~ — formant-preserving multi-voice harmonizer.
/// @details    The keyboard-harmonizer effect (the DigiTech Vocalist lineage): up to four
///             pitch-shifted copies of a monophonic source, each holding a musical interval in
///             fractional semitones, summed with a dry path the kernel delays to the voices'
///             emission time so chords land as chords rather than slapback. Formant
///             preservation is on by default — shifted voices keep the singer's spectral
///             envelope, which is what separates a harmonizer from a chipmunk chorus.
///
///             All DSP lives in the portable kernel (taptools/harmonizer.h, composing the
///             DspTap phase vocoder's peak-locked shifting and LPC source-filter formant
///             preservation — published-literature implementations only); this file is only
///             the Max plumbing. Latency is one FFT frame (fftsize samples, dry included).
///             Monophonic source material by design; wrap in mc. for multichannel.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min.h"
#include "taptools/harmonizer.h"

using namespace c74::min;

class harmony : public object<harmony>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    tap::tools::harmony::harmonizer m_engine;
    int                             m_fft{static_cast<int>(tap::tools::harmony::k_default_fft)};

  public:
    MIN_DESCRIPTION{"Formant-preserving multi-voice harmonizer. Up to four pitch-shifted copies "
                    "of a monophonic input hold musical intervals (fractional semitones) over a "
                    "latency-aligned dry path. Formant preservation keeps the singer's envelope; "
                    "latency is one FFT frame."};
    MIN_TAGS{"audio, effects, pitch"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.tune~, tap.shift~, tap.pitchaccum~, tap.vocoder~"};

    inlet<>  m_in{this, "(signal) audio input"};
    outlet<> m_out{this, "(signal) harmonized output", "signal"};

    harmony(const atoms& args = {}) { m_engine.prepare(samplerate(), static_cast<size_t>(m_fft)); }

    attribute<number> interval1{this, "interval1", 0.0, setter{MIN_FUNCTION{
                                    m_engine.set_interval(0, args[0]);
                                    return args;
                                }},
                                description{"Voice 1 interval in semitones (-24..24, fractional allowed)."}};
    attribute<number> interval2{this, "interval2", 0.0, setter{MIN_FUNCTION{
                                    m_engine.set_interval(1, args[0]);
                                    return args;
                                }},
                                description{"Voice 2 interval in semitones (-24..24, fractional allowed)."}};
    attribute<number> interval3{this, "interval3", 0.0, setter{MIN_FUNCTION{
                                    m_engine.set_interval(2, args[0]);
                                    return args;
                                }},
                                description{"Voice 3 interval in semitones (-24..24, fractional allowed)."}};
    attribute<number> interval4{this, "interval4", 0.0, setter{MIN_FUNCTION{
                                    m_engine.set_interval(3, args[0]);
                                    return args;
                                }},
                                description{"Voice 4 interval in semitones (-24..24, fractional allowed)."}};

    attribute<number> level1{this, "level1", 0.0, setter{MIN_FUNCTION{
                                 m_engine.set_gain(0, args[0]);
                                 return args;
                             }},
                             description{"Voice 1 level, linear 0..2. 0 disables the voice."}};
    attribute<number> level2{this, "level2", 0.0, setter{MIN_FUNCTION{
                                 m_engine.set_gain(1, args[0]);
                                 return args;
                             }},
                             description{"Voice 2 level, linear 0..2. 0 disables the voice."}};
    attribute<number> level3{this, "level3", 0.0, setter{MIN_FUNCTION{
                                 m_engine.set_gain(2, args[0]);
                                 return args;
                             }},
                             description{"Voice 3 level, linear 0..2. 0 disables the voice."}};
    attribute<number> level4{this, "level4", 0.0, setter{MIN_FUNCTION{
                                 m_engine.set_gain(3, args[0]);
                                 return args;
                             }},
                             description{"Voice 4 level, linear 0..2. 0 disables the voice."}};

    attribute<number> dry{this, "dry", 1.0, setter{MIN_FUNCTION{
                              m_engine.set_dry(args[0]);
                              return args;
                          }},
                          description{"Dry level, linear 0..2. The dry path is delayed inside the "
                                      "kernel to align with the voices."}};

    attribute<bool> formant{this, "formant", true, setter{MIN_FUNCTION{
                                m_engine.set_formant(args[0]);
                                return args;
                            }},
                            description{"LPC formant preservation on every voice. On keeps the "
                                        "singer's envelope; off is the chipmunk-chorus bend."}};

    attribute<number> glide{this, "glide", tap::tools::harmony::k_default_glide_ms,
                            setter{MIN_FUNCTION{
                                m_engine.set_glide(args[0]);
                                return args;
                            }},
                            description{"Interval glide time constant in ms (0..2000). Click-free "
                                        "at the default; an audible portamento in the hundreds."}};

    attribute<int> fftsize{this, "fftsize", static_cast<int>(tap::tools::harmony::k_default_fft),
                           setter{MIN_FUNCTION{
                               int n = args[0];
                               n     = std::clamp(n, 256, 4096);
                               // round up to a power of two, per the kernel's contract
                               int p = 256;
                               while (p < n) {
                                   p <<= 1;
                               }
                               m_fft = p;
                               return {m_fft};
                           }},
                           description{"FFT frame size in samples (256..4096, rounded up to a power "
                                       "of two). This is the latency. Applies when DSP restarts."}};

    /// Set up to four intervals at once and enable exactly those voices at level 1.
    /// Writes through the attributes (not the engine directly) so queries stay truthful
    /// and a DSP restart's re-apply preserves the chord instead of reverting it.
    message<> chord{this, "chord", "Set up to four intervals (semitones) and enable those voices.",
                    MIN_FUNCTION{
                        attribute<number>* intervals[] = {&interval1, &interval2, &interval3, &interval4};
                        attribute<number>* levels[]    = {&level1, &level2, &level3, &level4};
                        for (int v = 0; v < 4; ++v) {
                            if (v < static_cast<int>(args.size())) {
                                *intervals[v] = static_cast<double>(args[v]);
                                *levels[v]    = 1.0;
                            }
                            else {
                                *levels[v] = 0.0;
                            }
                        }
                        return {};
                    }};

    message<> clear{this, "clear", "Zero all running state (delay lines, phases, slews).",
                    MIN_FUNCTION{
                        m_engine.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare for the current sample rate and FFT size.",
                       MIN_FUNCTION{
                           m_engine.prepare(samplerate(), static_cast<size_t>(m_fft));
                           // re-apply the current attribute state to the fresh engine
                           m_engine.set_interval(0, interval1);
                           m_engine.set_interval(1, interval2);
                           m_engine.set_interval(2, interval3);
                           m_engine.set_interval(3, interval4);
                           m_engine.set_gain(0, level1);
                           m_engine.set_gain(1, level2);
                           m_engine.set_gain(2, level3);
                           m_engine.set_gain(3, level4);
                           m_engine.set_dry(dry);
                           m_engine.set_formant(formant);
                           m_engine.set_glide(glide);
                           return {};
                       }};

    sample operator()(sample x) { return m_engine.process(x); }
};

MIN_EXTERNAL(harmony);
