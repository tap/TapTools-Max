/// @file
/// @brief      tap.tune~ — real-time monophonic pitch correction.
/// @details    The classic retune effect: detect the input's pitch, snap it to the nearest allowed
///             note, and glide the correction in at a settable retune speed — transparent at slow
///             speeds, the hard-quantize effect at zero. Targets come from a key + scale (with
///             per-note enables) or, in midi mode, from whatever MIDI notes are currently held.
///
///             All DSP lives in the portable kernel (taptools/tune.h — a YIN detector from the
///             shared DspTap library, a scale/MIDI target mapper, and a period-locked two-tap
///             delay-line transposer); this file is only the Max plumbing. Monophonic by design —
///             pitch detection needs a single voice. For multichannel use, wrap the object in an
///             mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min.h"
#include "taptools/tune.h"

using namespace c74::min;

class tune : public object<tune>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    tap::tools::tune::corrector m_engine;

    // Shadows for the detection range: the kernel takes both bounds together.
    double m_min_hz{tap::tools::tune::k_default_min_hz};
    double m_max_hz{tap::tools::tune::k_default_max_hz};

    static int pitch_class_of(const symbol& s) {
        const std::string name = s;
        constexpr const char* k_names[12] = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};
        for (int i = 0; i < 12; ++i) {
            if (name == k_names[i]) {
                return i;
            }
        }
        return 0;
    }

    static unsigned scale_mask_of(const symbol& s) {
        using namespace tap::tools::tune;
        if (s == "major") {
            return k_scale_major;
        }
        if (s == "minor") {
            return k_scale_minor;
        }
        if (s == "harmonic") {
            return k_scale_harmonic_minor;
        }
        if (s == "melodic") {
            return k_scale_melodic_minor;
        }
        if (s == "pentatonic") {
            return k_scale_major_pentatonic;
        }
        if (s == "minorpentatonic") {
            return k_scale_minor_pentatonic;
        }
        return k_scale_chromatic;
    }

  public:
    MIN_DESCRIPTION{"Real-time monophonic pitch correction. Detects the input's pitch, snaps it to "
                    "the nearest note of a key/scale (or the nearest held MIDI note), and retunes "
                    "with a settable retune speed — transparent correction at slow speeds, the "
                    "hard-snap quantize effect at zero. For multichannel signals, wrap this object "
                    "in an mc. operator."};
    MIN_TAGS{"pitch"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.shift~, tap.pitchaccum~, retune~, pitchshift~, fzero~"};

    inlet<>  m_in{this, "(signal) audio input"};
    outlet<> m_out{this, "(signal) pitch-corrected output", "signal"};

    tune(const atoms& args = {}) { m_engine.prepare(samplerate()); }

    attribute<symbol> key{this,
                          "key",
                          "c",
                          range{"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"},
                          setter{MIN_FUNCTION{
                              m_engine.set_key(pitch_class_of(args[0]));
                              return {args[0]};
                          }},
                          description{"Key root for the scale. Setting it re-derives the allowed notes from key + "
                                      "scale, replacing any custom 'notes' mask."}};

    attribute<symbol> scale{this,
                            "scale",
                            "chromatic",
                            range{"chromatic", "major", "minor", "harmonic", "melodic", "pentatonic",
                                  "minorpentatonic"},
                            setter{MIN_FUNCTION{
                                m_engine.set_scale(scale_mask_of(args[0]));
                                return {args[0]};
                            }},
                            description{"Scale the corrector snaps to, relative to the key. Setting it re-derives "
                                        "the allowed notes, replacing any custom 'notes' mask."}};

    attribute<symbol> mode{this,
                           "mode",
                           "scale",
                           range{"scale", "midi"},
                           setter{MIN_FUNCTION{
                               m_engine.set_mode(args[0] == "midi" ? tap::tools::tune::mode::midi
                                                                   : tap::tools::tune::mode::scale);
                               return {args[0]};
                           }},
                           description{"Target selection: 'scale' snaps to the key/scale notes; 'midi' snaps to the "
                                       "nearest currently-held MIDI note (send 'note' messages) and leaves the "
                                       "signal untouched when none are held."}};

    attribute<symbol> backend{this,
                              "backend",
                              "grain",
                              range{"grain", "psola", "pvoc"},
                              setter{MIN_FUNCTION{
                                  if (args[0] == "psola") {
                                      m_engine.set_backend(tap::tools::tune::backend::psola);
                                  }
                                  else if (args[0] == "pvoc") {
                                      m_engine.set_backend(tap::tools::tune::backend::pvoc);
                                  }
                                  else {
                                      m_engine.set_backend(tap::tools::tune::backend::grain);
                                  }
                                  return {args[0]};
                              }},
                              description{"Resynthesis engine. 'grain' is the low-latency two-tap default; 'psola' "
                                          "is formant-preserving TD-PSOLA, best on voice (needs harmonic-rich "
                                          "input); 'pvoc' is a peak-locked phase vocoder with one FFT frame of "
                                          "latency. Switching starts the new engine from silence."}};

    attribute<number> speed{this,
                            "speed",
                            tap::tools::tune::k_default_speed_ms,
                            range{0.0, 5000.0},
                            setter{MIN_FUNCTION{
                                m_engine.set_speed(args[0]);
                                return {args[0]};
                            }},
                            description{"Retune speed in milliseconds — the time constant of the glide onto the "
                                        "target note. 0 snaps instantly (the hard quantize effect); slow values "
                                        "correct transparently while letting vibrato through."}};

    attribute<number> amount{this,
                             "amount",
                             100.0,
                             range{0.0, 100.0},
                             setter{MIN_FUNCTION{
                                 m_engine.set_amount(args[0]);
                                 return {args[0]};
                             }},
                             description{"Correction depth in percent. 100 lands on the target; 50 corrects half "
                                         "the distance; 0 leaves the pitch untouched."}};

    attribute<number> minfreq{this,
                              "minfreq",
                              tap::tools::tune::k_default_min_hz,
                              setter{MIN_FUNCTION{
                                  m_min_hz = args[0];
                                  m_engine.set_range(m_min_hz, m_max_hz);
                                  return {args[0]};
                              }},
                              description{"Lowest input frequency (Hz) treated as pitched. Estimates below it are "
                                          "ignored (no correction)."}};

    attribute<number> maxfreq{this,
                              "maxfreq",
                              tap::tools::tune::k_default_max_hz,
                              setter{MIN_FUNCTION{
                                  m_max_hz = args[0];
                                  m_engine.set_range(m_min_hz, m_max_hz);
                                  return {args[0]};
                              }},
                              description{"Highest input frequency (Hz) treated as pitched. Estimates above it are "
                                          "ignored (no correction)."}};

    attribute<bool> formant{this,
                            "formant",
                            false,
                            setter{MIN_FUNCTION{
                                m_engine.set_formant(args[0]);
                                return {args[0]};
                            }},
                            description{"Formant preservation for the pvoc backend (LPC source-filter): the "
                                        "correction shifts the pitch while the spectral envelope stays put — "
                                        "matters for large corrections (midi mode). The psola backend preserves "
                                        "formants inherently; the grain engine is unaffected."}};

    attribute<bool> bypass{this, "bypass", false, description{"Pass the input through unprocessed."}};

    attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

    message<> notes{this, "notes",
                    "Set the allowed notes directly, as absolute pitch classes (replacing the key/scale "
                    "derivation). Send either twelve 0/1 toggles (C through B, Auto-panel style) or a "
                    "list of the enabled pitch classes (0 = C .. 11 = B).",
                    MIN_FUNCTION{
                        if (args.size() == 12) {
                            unsigned mask = 0u;
                            for (int i = 0; i < 12; ++i) {
                                if (static_cast<int>(args[static_cast<size_t>(i)]) != 0) {
                                    mask |= 1u << i;
                                }
                            }
                            m_engine.set_notes(mask);
                        }
                        else {
                            unsigned mask = 0u;
                            for (const auto& a : args) {
                                const int pc = static_cast<int>(a);
                                mask |= 1u << (((pc % 12) + 12) % 12);
                            }
                            m_engine.set_notes(mask);
                        }
                        return {};
                    }};

    message<> note{this, "note",
                   "MIDI note-on/off for midi mode: a pitch (0-127) and a velocity. Velocity 0 "
                   "releases the note; anything else holds it as a correction target.",
                   MIN_FUNCTION{
                       const int pitch    = args[0];
                       const int velocity = (args.size() > 1) ? static_cast<int>(args[1]) : 0;
                       if (velocity > 0) {
                           m_engine.note_on(pitch);
                       }
                       else {
                           m_engine.note_off(pitch);
                       }
                       return {};
                   }};

    message<> flush{this, "flush", "Release all held MIDI notes.",
                    MIN_FUNCTION{
                        m_engine.notes_off();
                        return {};
                    }};

    message<> clear{this, "clear", "Reset the audio state (buffers and glides).",
                    MIN_FUNCTION{
                        m_engine.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare for the current sample rate when the DSP chain starts.",
                       MIN_FUNCTION{
                           m_engine.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample x) {
        if (bypass) {
            return x;
        }
        const sample y = m_engine.process(x);
        return mute ? 0.0 : y;
    }

    /// Kernel access for the unit tests.
    const tap::tools::tune::corrector& engine() const { return m_engine; }
};

MIN_EXTERNAL(tune);
