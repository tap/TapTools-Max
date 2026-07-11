/// @file
/// @brief      tap.5comb~ — five parallel comb filters modeled on GRM Tools Classic "Comb Filters".
/// @details    A thin Min wrapper around the portable kernel in grm_comb.h (all DSP lives there —
///             see its header comment for the algorithm, provenance, and deliberate deviations from
///             the legacy tap.5comb~ abstraction). The parameter names keep the legacy/GRM surface:
///             per-voice freq1..5 / res1..5 / lp1..5, master multipliers freq / res / lp, mix and
///             gain — plus warp and phase (the modern GRM Comb's partial-stretch and pickup
///             controls, neutral by default) and the preset-morph engine (store / recall with a
///             settable interpolation time — the GRM hallmark).
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min.h"
#include "grm_comb.h"

using namespace c74::min;
namespace kernel = taptools::fivecomb;


class fivecomb : public object<fivecomb>, public vector_operator<> {
private:
    // The kernel must be constructed before the attributes below: attribute construction fires
    // each setter with its default value, and those setters forward into the bank.
    kernel::comb_bank m_bank;

public:
    MIN_DESCRIPTION { "Five parallel comb filters with master controls and preset morphing, "
                      "modeled on the GRM Tools Classic 'Comb Filters' plugin. Each comb has a "
                      "frequency, a resonance (ring time), and a lowpass filter in its feedback "
                      "loop; master multipliers scale all five at once. All parameters are "
                      "interpolated per sample, so sweeps and preset recalls glide without zipper "
                      "noise. Warp and phase add the modern GRM Comb's inharmonic partial stretch "
                      "and pickup-position control." };
    MIN_TAGS    { "filters" };
    MIN_AUTHOR  { "Timothy Place" };
    MIN_RELATED { "tap.comb~, tap.svf~, tap.verb~, comb~" };

    inlet<>  m_in  { this, "(signal) audio input" };
    outlet<> m_out { this, "(signal) comb filter bank output", "signal" };

    fivecomb(const atoms& args = {}) {
        m_bank.prepare(samplerate());
    }

    // -- per-voice parameters (legacy/GRM names) -------------------------------------------------

#define TAP_5COMB_VOICE_ATTR(NAME, INDEX, DEFAULT, DESC)                                           \
    attribute<number> NAME { this, #NAME, DEFAULT,                                                 \
        setter { MIN_FUNCTION {                                                                    \
            const double v = kernel::clamp_param(INDEX, args[0]);                                  \
            m_bank.set_param(INDEX, v);                                                            \
            return { v };                                                                          \
        }},                                                                                        \
        description { DESC }                                                                       \
    };

    TAP_5COMB_VOICE_ATTR(freq1, kernel::p_freq1 + 0,  80.0, "Frequency of comb 1 in Hz (45..20000).")
    TAP_5COMB_VOICE_ATTR(freq2, kernel::p_freq1 + 1, 120.0, "Frequency of comb 2 in Hz (45..20000).")
    TAP_5COMB_VOICE_ATTR(freq3, kernel::p_freq1 + 2, 160.0, "Frequency of comb 3 in Hz (45..20000).")
    TAP_5COMB_VOICE_ATTR(freq4, kernel::p_freq1 + 3, 200.0, "Frequency of comb 4 in Hz (45..20000).")
    TAP_5COMB_VOICE_ATTR(freq5, kernel::p_freq1 + 4, 102.0, "Frequency of comb 5 in Hz (45..20000).")

    TAP_5COMB_VOICE_ATTR(res1, kernel::p_res1 + 0, 50.0, "Resonance of comb 1 (0..100). 0 is none; 100 rings effectively forever. Maps to ring time on a log curve.")
    TAP_5COMB_VOICE_ATTR(res2, kernel::p_res1 + 1, 50.0, "Resonance of comb 2 (0..100).")
    TAP_5COMB_VOICE_ATTR(res3, kernel::p_res1 + 2, 50.0, "Resonance of comb 3 (0..100).")
    TAP_5COMB_VOICE_ATTR(res4, kernel::p_res1 + 3, 50.0, "Resonance of comb 4 (0..100).")
    TAP_5COMB_VOICE_ATTR(res5, kernel::p_res1 + 4, 50.0, "Resonance of comb 5 (0..100).")

    TAP_5COMB_VOICE_ATTR(lp1, kernel::p_lp1 + 0, 20000.0, "Cutoff (Hz) of the lowpass in comb 1's feedback loop (0..20000). Low values smooth/darken the ring; high values keep it bright and metallic.")
    TAP_5COMB_VOICE_ATTR(lp2, kernel::p_lp1 + 1, 20000.0, "Cutoff (Hz) of the lowpass in comb 2's feedback loop (0..20000).")
    TAP_5COMB_VOICE_ATTR(lp3, kernel::p_lp1 + 2, 20000.0, "Cutoff (Hz) of the lowpass in comb 3's feedback loop (0..20000).")
    TAP_5COMB_VOICE_ATTR(lp4, kernel::p_lp1 + 3, 20000.0, "Cutoff (Hz) of the lowpass in comb 4's feedback loop (0..20000).")
    TAP_5COMB_VOICE_ATTR(lp5, kernel::p_lp1 + 4, 20000.0, "Cutoff (Hz) of the lowpass in comb 5's feedback loop (0..20000).")

#undef TAP_5COMB_VOICE_ATTR

    // -- masters and globals ---------------------------------------------------------------------

    attribute<number> freq { this, "freq", 1.0,
        setter { MIN_FUNCTION {
            const double v = kernel::clamp_param(kernel::p_freq_master, args[0]);
            m_bank.set_param(kernel::p_freq_master, v);
            return { v };
        }},
        description { "Master frequency multiplier (0..2) applied to freq1..freq5. 2 is an octave up; "
                      "0 pins all combs at the 5 Hz floor." }
    };

    attribute<number> res { this, "res", 1.0,
        setter { MIN_FUNCTION {
            const double v = kernel::clamp_param(kernel::p_res_master, args[0]);
            m_bank.set_param(kernel::p_res_master, v);
            return { v };
        }},
        description { "Master resonance multiplier (0..2) applied to res1..res5." }
    };

    attribute<number> lp { this, "lp", 1.0,
        setter { MIN_FUNCTION {
            const double v = kernel::clamp_param(kernel::p_lp_master, args[0]);
            m_bank.set_param(kernel::p_lp_master, v);
            return { v };
        }},
        description { "Master lowpass multiplier (0..2) applied to lp1..lp5." }
    };

    attribute<number> mix { this, "mix", 100.0,
        setter { MIN_FUNCTION {
            const double v = kernel::clamp_param(kernel::p_mix, args[0]);
            m_bank.set_param(kernel::p_mix, v);
            return { v };
        }},
        description { "Dry/wet crossfade (0 = dry .. 100 = wet), equal-power." }
    };

    attribute<number> gain { this, "gain", 0.0,
        setter { MIN_FUNCTION {
            m_bank.set_param(kernel::p_gain, args[0]);
            return args;
        }},
        description { "Output gain in dB." }
    };

    attribute<number> warp { this, "warp", 0.0,
        setter { MIN_FUNCTION {
            const double v = kernel::clamp_param(kernel::p_warp, args[0]);
            m_bank.set_param(kernel::p_warp, v);
            return { v };
        }},
        description { "Partial warp (0..100): an allpass inside each feedback loop stretches the "
                      "upper resonance partials from purely harmonic (0) toward inharmonic/metallic "
                      "(100). The fundamental tuning is compensated. 0 matches the GRM Classic." }
    };

    attribute<number> phase { this, "phase", 0.0,
        setter { MIN_FUNCTION {
            const double v = kernel::clamp_param(kernel::p_phase, args[0]);
            m_bank.set_param(kernel::p_phase, v);
            return { v };
        }},
        description { "Pickup position (0..100): moves where the output is tapped from the loop. "
                      "At 100 only the odd harmonics ring. 0 matches the GRM Classic." }
    };

    attribute<number> interp { this, "interp", 500.0,
        setter { MIN_FUNCTION {
            return { std::max(0.0, static_cast<double>(args[0])) };
        }},
        description { "Default preset-morph time in ms used by the 'recall' message." }
    };

    attribute<number> smooth { this, "smooth", kernel::k_default_smooth_ms,
        setter { MIN_FUNCTION {
            const double v = std::max(0.0, static_cast<double>(args[0]));
            m_bank.set_smooth_ms(v);
            return { v };
        }},
        description { "Anti-zipper interpolation time in ms applied to direct parameter changes." }
    };

    attribute<bool> bypass { this, "bypass", false,
        description { "Pass the input through unprocessed." }
    };

    attribute<bool> mute { this, "mute", false,
        description { "Silence the output." }
    };

    // -- messages --------------------------------------------------------------------------------

    message<> store { this, "store", "Store the current parameters in a preset slot (1..16).",
        MIN_FUNCTION {
            if (!args.empty())
                m_bank.store_preset(static_cast<int>(args[0]) - 1);
            return {};
        }
    };

    message<> recall { this, "recall",
        "Morph all parameters to a stored preset (1..16). An optional second argument overrides "
        "the morph time in ms (default: the interp attribute).",
        MIN_FUNCTION {
            if (!args.empty()) {
                const double ms = (args.size() > 1) ? static_cast<double>(args[1])
                                                    : static_cast<double>(interp);
                m_bank.recall_preset(static_cast<int>(args[0]) - 1, ms * 0.001);
            }
            return {};
        }
    };

    message<> clear { this, "clear", "Clear the delay buffers and filter state.",
        MIN_FUNCTION {
            m_bank.clear();
            return {};
        }
    };

    message<> dspsetup { this, "dspsetup", "Allocate and reset when the DSP chain starts.",
        MIN_FUNCTION {
            m_bank.prepare(samplerate());
            return {};
        }
    };

    // -- perform ---------------------------------------------------------------------------------

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
    kernel::comb_bank& bank() { return m_bank; }
};


MIN_EXTERNAL(fivecomb);
