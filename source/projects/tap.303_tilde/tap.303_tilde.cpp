/// @file
/// @brief      tap.303~ — TB-303-style acid bass voice.
/// @details    A thin Min wrapper around the portable voice kernel in tb303_voice.h (all DSP
///             lives there — oscillator, diode-ladder filter, MEG/VCA envelopes, slide; see its
///             header for the model and provenance). This wrapper implements the package-wide
///             melodic-voice contract:
///
///             - Inlet 1: pitch as a MIDI note number — signal (read per sample) or float.
///               Fractional values are cents.
///             - Inlet 2: gate signal, amplitude-as-accent. A rising edge above a small
///               threshold triggers the note at the pitch inlet's current value; edge
///               amplitude 1.0 is a plain note, above 1.0 adds accent (depth = amplitude - 1,
///               up to 2.0 = fully accented). The gate falling releases.
///             - Slide is legato: changing the pitch while the gate is held glides through the
///               hardware's ~60 ms RC without retriggering — no separate slide input.
///             - Message surface: 'note <pitch> [accent 0/1] [slide 0/1]', 'bang' (retrigger
///               the last note), 'release' (gate off) — for patching without signals.
///
///             Single-channel voice; wrap in mc. for polyphony (each instance is one 303).
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/tb303_voice.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = taptools::tb303;

class voice303 : public object<voice303>, public vector_operator<> {
  private:
    // Constructed before the attributes below so attribute defaults can forward into it.
    kernel::voice m_voice;
    double        m_pending_note{45.0}; // the note the next trigger plays
    double        m_prev_gate{0.0};

    static constexpr double k_gate_thresh = 0.001;

  public:
    MIN_DESCRIPTION{"A TB-303-style acid bass voice: sawtooth or square oscillator into the "
                    "diode-ladder filter (tap.diode~'s kernel), with the 303's envelope "
                    "generators — a decay-only Main Envelope driving the cutoff (env mod, with "
                    "the hardware's 2/3-up sweep law) and the fixed-decay VCA envelope — plus "
                    "accent and slide. Pitch is a MIDI note number in the left inlet (signal or "
                    "float); the right inlet is a gate signal whose edge amplitude encodes "
                    "accent (1 = plain, 2 = fully accented). Changing pitch while the gate is "
                    "held slides (legato, ~60 ms), exactly like the hardware. One voice; wrap "
                    "in mc. for more 303s than anyone needs."};
    MIN_TAGS{"synthesis"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.diode~, tap.vco~, tap.808.kick~, tap.adsr~, mc.noteallocator~"};

    inlet<>  m_in_pitch{this, "(signal/float) pitch as MIDI note number"};
    inlet<>  m_in_gate{this, "(signal) gate; edge amplitude 1..2 = accent"};
    outlet<> m_out{this, "(signal) voice output", "signal"};

    voice303(const atoms& args = {}) { m_voice.prepare(samplerate()); }

#define TAP_303_ATTR(NAME, INDEX, DEFAULT, DESC)                                                                       \
    attribute<number> NAME{this, #NAME, DEFAULT,                                                                       \
                           setter{MIN_FUNCTION{const double v = kernel::clamp_param(INDEX, args[0]);                   \
    m_voice.set_param(INDEX, v);                                                                                       \
    return {v};                                                                                                        \
    }                                                                                                                  \
    }                                                                                                                  \
    , description { DESC }                                                                                             \
    }                                                                                                                  \
    ;

    TAP_303_ATTR(cutoff, kernel::p_cutoff, 1000.0,
                 "Filter cutoff knob in Hz (100..5000; the stock knob spans ~302..2394 — the wider range is "
                 "the Devil-Fish-style bend). The Main Envelope sweeps around this per the envmod law.")
    TAP_303_ATTR(resonance, kernel::p_resonance, 0.5,
                 "Filter resonance (0..1.5). Up to 1 is the stock range — squelchy but, like a real 303, "
                 "never quite self-oscillating; above 1 is the documented bend.")
    TAP_303_ATTR(envmod, kernel::p_envmod, 0.5,
                 "Envelope modulation (0..1): how much of the Main Envelope reaches the cutoff. 2/3 of the "
                 "sweep goes above the cutoff knob and 1/3 below, like the hardware.")
    TAP_303_ATTR(decay, kernel::p_decay, 400.0,
                 "Main Envelope decay in ms (200..2000, the stock knob range). Accented notes bypass this "
                 "and run at ~200 ms, like the hardware.")
    TAP_303_ATTR(accent, kernel::p_accent, 0.5,
                 "Accent amount (0..1, the AC knob): how strongly accented notes hit — louder, with a "
                 "faster envelope, and driving the accent-sweep circuit: consecutive accents charge a "
                 "capacitor that pushes the cutoff higher note over note (the wow), scaled by the "
                 "resonance knob, then drains during rests.")
    TAP_303_ATTR(tuning, kernel::p_tuning, 0.0, "Master tuning in cents (-1200..1200).")
    TAP_303_ATTR(gain, kernel::p_gain, 0.0, "Output gain in dB.")

#undef TAP_303_ATTR

    attribute<symbol> waveform{this, "waveform", "saw", setter{ MIN_FUNCTION {
                                   const symbol s = args[0];
                                   m_voice.set_waveform(s == symbol("square") ? kernel::wave_square : kernel::wave_saw);
                                   return {m_voice.wave() == kernel::wave_square ? symbol("square") : symbol("saw")};
                               }},
                               description{"Oscillator waveform: saw or square, the hardware's two. Switching "
                                           "glides click-free."}};

    attribute<int> solver{this, "solver", 0, setter{ MIN_FUNCTION {
                              const int v = std::clamp(static_cast<int>(args[0]), 0, 1);
                              m_voice.set_solver(v);
                              return {v};
                          }},
                          description{"Filter nonlinear solver: 0 = fast (default), 1 = exact. See tap.diode~."}};

    attribute<int> oversample{this, "oversample", 2, setter{ MIN_FUNCTION {
                                  const int req = static_cast<int>(args[0]);
                                  const int v   = (req >= 4) ? 4 : (req >= 2) ? 2 : 1;
                                  m_voice.set_oversample(v);
                                  return {v};
                              }},
                              description{"Filter oversampling factor (1, 2, or 4; default 2)."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{ MIN_FUNCTION {
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_voice.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper interpolation time in ms applied to parameter changes."}};

    attribute<number> interp{this, "interp", 500.0, setter{ MIN_FUNCTION {
                                 return {std::max(0.0, static_cast<double>(args[0]))};
                             }},
                             description{"Default preset-morph time in ms used by the 'recall' message."}};

    attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

    // -- messages -------------------------------------------------------------------------------------

    message<> m_number{this, "number",
                       "A float in the left inlet sets the pitch (MIDI note number): the next trigger plays "
                       "it, and if the gate is held it slides there now.",
                       MIN_FUNCTION {
                           if (inlet == 0) {
                               m_pending_note = args[0];
                               if (m_voice.gate()) {
                                   m_voice.set_pitch(m_pending_note);
                               }
                           }
                           return {};
                       }};

    message<> note{this, "note",
                   "Play a note: note <pitch> [accent 0/1] [slide 0/1]. With slide 1 (and a note already "
                   "sounding) the pitch glides legato instead of retriggering.",
                   MIN_FUNCTION {
                       if (args.empty()) {
                           return {};
                       }
                       const double pitch = args[0];
                       const double accent =
                           (args.size() > 1) ? std::clamp(static_cast<double>(args[1]), 0.0, 1.0) : 0.0;
                       const bool slide = (args.size() > 2) && (static_cast<int>(args[2]) != 0);
                       m_pending_note   = pitch;
                       if (slide && m_voice.gate()) {
                           m_voice.set_pitch(pitch);
                       }
                       else {
                           m_voice.note_off();
                           m_voice.note_on(pitch, accent);
                       }
                       return {};
                   }};

    message<> bang{this, "bang", "Retrigger the last note (plain, no accent).",
                   MIN_FUNCTION {
                       m_voice.note_off();
                       m_voice.note_on(m_pending_note, 0.0);
                       return {};
                   }};

    message<> release{this, "release", "Release the gate (the VCA chops, like the hardware's gate-off).",
                      MIN_FUNCTION {
                          m_voice.note_off();
                          return {};
                      }};

    message<> store{this, "store", "Store the current parameters in a preset slot (1..16).",
                    MIN_FUNCTION {
                        if (!args.empty()) {
                            m_voice.store_preset(static_cast<int>(args[0]) - 1);
                        }
                        return {};
                    }};

    message<> recall{this, "recall",
                     "Morph all parameters to a stored preset (1..16). An optional second argument overrides "
                     "the morph time in ms (default: the interp attribute).",
                     MIN_FUNCTION {
                         if (!args.empty()) {
                             const double ms =
                                 (args.size() > 1) ? static_cast<double>(args[1]) : static_cast<double>(interp);
                             m_voice.recall_preset(static_cast<int>(args[0]) - 1, ms * 0.001);
                         }
                         return {};
                     }};

    message<> clear{this, "clear", "Silence the voice and reset all DSP state.",
                    MIN_FUNCTION {
                        m_voice.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Reconfigure for the sample rate when DSP starts.",
                       MIN_FUNCTION {
                           m_voice.prepare(samplerate());
                           m_prev_gate = 0.0;
                           return {};
                       }};

    // -- perform --------------------------------------------------------------------------------------

    void operator()(audio_bundle input, audio_bundle output) override {
        const long    n         = input.frame_count();
        const double* pitch_in  = input.samples(0);
        const double* gate_in   = input.samples(1);
        double*       out       = output.samples(0);
        const bool    has_pitch = m_in_pitch.has_signal_connection();
        const bool    has_gate  = m_in_gate.has_signal_connection();

        for (long i = 0; i < n; ++i) {
            if (has_gate) {
                const double g = gate_in[i];
                if (m_prev_gate < k_gate_thresh && g >= k_gate_thresh) {
                    if (has_pitch) {
                        m_pending_note = pitch_in[i];
                    }
                    m_voice.note_on(m_pending_note, std::clamp(g - 1.0, 0.0, 1.0));
                }
                else if (m_prev_gate >= k_gate_thresh && g < k_gate_thresh) {
                    m_voice.note_off();
                }
                m_prev_gate = g;
            }
            if (has_pitch && m_voice.gate()) {
                m_voice.set_pitch(pitch_in[i]); // held pitch motion = slide (legato, 60 ms RC)
            }
            out[i] = m_voice.process();
        }

        if (mute) {
            std::fill_n(out, n, 0.0);
        }
    }

    // Exposed for unit tests.
    kernel::voice& voice() { return m_voice; }
};

MIN_EXTERNAL(voice303);
