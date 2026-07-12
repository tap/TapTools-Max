/// @file
/// @brief      tap.pitchaccum~ — dual transposer with accumulating feedback, after GRM PitchAccum.
/// @details    A thin Min wrapper around the portable kernel in grm_pitchaccum.h (all DSP lives
///             there — see its header comment for the algorithm and provenance). Two granular
///             transposers ("shadows"), each with transposition, delay, feedback that re-enters
///             the transposer (so pitch accumulates pass after pass), and gain; global LFO and
///             random transposition modulation; window/crossfade grain controls; an optional
///             pitch follower; and the 16-slot preset-morph engine shared with tap.5comb~.
///             GRM's stereo-width fader is intentionally dropped: the object is single-channel
///             (wrap in mc. for multichannel, per house style).
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min.h"
#include "grm_pitchaccum.h"

using namespace c74::min;
namespace kernel = taptools::pitchaccum;

class pitchaccum : public object<pitchaccum>, public vector_operator<> {
  private:
    // Constructed before the attributes below: attribute construction fires each setter with its
    // default value, and those setters forward into the bank.
    kernel::accum_bank m_bank;

  public:
    MIN_DESCRIPTION{"Two granular pitch transposers with accumulating feedback, modeled on the "
                    "GRM Tools Classic 'PitchAccum' plugin. Each transposer shifts up to two "
                    "octaves up or down, with its own delay (up to 3 seconds), feedback that "
                    "re-enters the transposer so the pitch accumulates on every pass, and gain. "
                    "A global LFO and random generators modulate the transposition; window and "
                    "xfade expose the grain engine; an optional pitch follower adapts the window "
                    "to the input. All parameters are interpolated per sample, and sixteen "
                    "preset slots can be stored and morphed between over a settable time."};
    MIN_TAGS{"processors"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.shift~, tap.5comb~, gizmo~, pitchshift~"};

    inlet<>  m_in{this, "(signal) audio input"};
    outlet<> m_out{this, "(signal) transposed output", "signal"};

    pitchaccum(const atoms& args = {}) { m_bank.prepare(samplerate()); }

#define TAP_PITCHACCUM_ATTR(NAME, INDEX, DEFAULT, DESC)                                                                \
    attribute<number> NAME{this, #NAME, DEFAULT,                                                                       \
                           setter{MIN_FUNCTION{const double v = kernel::clamp_param(INDEX, args[0]);                   \
    m_bank.set_param(INDEX, v);                                                                                        \
    return {v};                                                                                                        \
    }                                                                                                                  \
    }                                                                                                                  \
    , description { DESC }                                                                                             \
    }                                                                                                                  \
    ;

    // -- per-transposer parameters ----------------------------------------------------------------

    TAP_PITCHACCUM_ATTR(trans1, kernel::p_trans1, 0.0,
                        "Transposition of voice 1 in semitones (-24..24, fractional values allowed).")
    TAP_PITCHACCUM_ATTR(trans2, kernel::p_trans2, 0.0, "Transposition of voice 2 in semitones (-24..24).")
    TAP_PITCHACCUM_ATTR(delay1, kernel::p_delay1, 0.0, "Delay of voice 1 in ms (0..3000).")
    TAP_PITCHACCUM_ATTR(delay2, kernel::p_delay2, 0.0, "Delay of voice 2 in ms (0..3000).")
    TAP_PITCHACCUM_ATTR(fb1, kernel::p_fb1, 0.0,
                        "Feedback of voice 1 (0..99). The feedback re-enters the transposer, so each pass "
                        "transposes again - the pitch accumulates.")
    TAP_PITCHACCUM_ATTR(fb2, kernel::p_fb2, 0.0, "Feedback of voice 2 (0..99).")
    TAP_PITCHACCUM_ATTR(gain1, kernel::p_gain1, 50.0, "Gain of voice 1 (0..100, linear).")
    TAP_PITCHACCUM_ATTR(gain2, kernel::p_gain2, 50.0, "Gain of voice 2 (0..100, linear).")

    // -- grain engine -------------------------------------------------------------------------------

    TAP_PITCHACCUM_ATTR(window, kernel::p_window, 87.0,
                        "Grain window in ms (5..200). Small windows smear less but modulate more; large windows "
                        "are smoother but blur transients. The pitch follower can override this when enabled.")
    TAP_PITCHACCUM_ATTR(xfade, kernel::p_xfade, 50.0,
                        "Crossfade between the two grain taps as a percentage (1..100). 100 is a full raised-"
                        "cosine overlap; small values give longer single-tap plateaus.")

    // -- modulation ---------------------------------------------------------------------------------

    TAP_PITCHACCUM_ATTR(modfreq, kernel::p_modfreq, 1.0, "Periodic (LFO) modulation frequency in Hz (0..20).")
    TAP_PITCHACCUM_ATTR(moddepth, kernel::p_moddepth, 0.0, "Periodic modulation depth in semitones (0..24).")
    TAP_PITCHACCUM_ATTR(modphase, kernel::p_modphase, 90.0,
                        "LFO phase offset between the two voices in degrees (0..360).")
    TAP_PITCHACCUM_ATTR(randdepth, kernel::p_randdepth, 0.0,
                        "Random modulation depth in semitones (0..24). Each voice has an independent generator.")
    TAP_PITCHACCUM_ATTR(randrate, kernel::p_randrate, 5.0, "Random modulation rate in Hz (0..50).")

    // -- globals ------------------------------------------------------------------------------------

    TAP_PITCHACCUM_ATTR(mix, kernel::p_mix, 50.0, "Dry/wet crossfade (0 = dry .. 100 = wet), equal-power.")
    TAP_PITCHACCUM_ATTR(gain, kernel::p_gain, 0.0, "Output gain in dB.")

#undef TAP_PITCHACCUM_ATTR

    attribute<bool> follow{this, "follow", false, setter{ MIN_FUNCTION {
                               m_bank.set_follow(args[0]);
                               return args;
                           }},
                           description{"Enable the pitch follower: the grain window adapts to the detected period "
                                       "of the input (falls back to the window attribute when the input is "
                                       "unpitched). Off by default for fixed, predictable grain behavior."}};

    attribute<number> interp{this, "interp", 500.0, setter{ MIN_FUNCTION {
                                 return {std::max(0.0, static_cast<double>(args[0]))};
                             }},
                             description{"Default preset-morph time in ms used by the 'recall' message."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{ MIN_FUNCTION {
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_bank.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper interpolation time in ms applied to direct parameter changes."}};

    attribute<bool> bypass{this, "bypass", false, description{"Pass the input through unprocessed."}};

    attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

    // -- messages -----------------------------------------------------------------------------------

    message<> store{this, "store", "Store the current parameters in a preset slot (1..16).",
                    MIN_FUNCTION {
                        if (!args.empty()) {
                            m_bank.store_preset(static_cast<int>(args[0]) - 1);
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
                             m_bank.recall_preset(static_cast<int>(args[0]) - 1, ms * 0.001);
                         }
                         return {};
                     }};

    message<> clear{this, "clear", "Clear the delay buffers and feedback state.",
                    MIN_FUNCTION {
                        m_bank.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Allocate and reset when the DSP chain starts.",
                       MIN_FUNCTION {
                           m_bank.prepare(samplerate());
                           return {};
                       }};

    // -- perform ------------------------------------------------------------------------------------

    void operator()(audio_bundle input, audio_bundle output) override {
        const long    n   = input.frame_count();
        const double* in  = input.samples(0);
        double*       out = output.samples(0);

        if (mute) {
            std::fill_n(out, n, 0.0);
        }
        else if (bypass) {
            std::copy_n(in, n, out);
        }
        else {
            m_bank.process(in, out, static_cast<size_t>(n));
        }
    }

    // Exposed for unit tests.
    kernel::accum_bank& bank() { return m_bank; }
};

MIN_EXTERNAL(pitchaccum);
