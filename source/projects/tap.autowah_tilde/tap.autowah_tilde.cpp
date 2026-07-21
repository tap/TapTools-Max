/// @file
/// @brief      tap.autowah~ — envelope filter / auto-wah in the Snow White style.
/// @details    A thin Min wrapper around the portable kernel in autowah.h (all DSP lives there —
///             see its header comment for the model and its hardware provenance). A 2-pole
///             state-variable filter swept upward from the bias resting frequency by an envelope
///             follower: fast attack, decay-controlled release — the Mad Professor Snow White
///             AutoWah behavior, including its secondary mode (sensitivity at minimum turns the
///             object into a fixed, manually swept "cocked wah" with bias as the sweep control).
///             The right inlet takes an optional sidechain signal for the envelope detector; the
///             right outlet emits the envelope (0..1) as a signal for patching elsewhere.
///             Single-channel: wrap in mc. for multichannel (per-channel envelopes).
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/autowah.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::autowah;

class autowah : public object<autowah>, public vector_operator<> {
  private:
    // Constructed before the attributes below so attribute defaults can forward into it.
    kernel::wah_filter m_filter;
    double             m_range_mag{3.3}; // magnitude; sign applied from m_direction
    int                m_direction{0};   // 0 = up (the pedal), 1 = down

    void forward_range() { m_filter.set_range(m_direction ? -m_range_mag : m_range_mag); }

  public:
    MIN_DESCRIPTION{"An envelope filter (auto-wah) modeled on the Mad Professor Snow White "
                    "AutoWah: a resonant 2-pole filter swept up from the bias resting frequency "
                    "by your playing dynamics, with a fast attack and a decay control for how "
                    "quickly it falls back — wah on every note when fast, classic auto-wah "
                    "swells when slow. With sensitivity at minimum it becomes a fixed, manually "
                    "swept filter (bias is the sweep). A signal in the right inlet sidechains "
                    "the envelope detector; the right outlet emits the envelope (0..1). Factory "
                    "voicings live in preset slots 1-4 (guitar, bass, slow swell, cocked wah) "
                    "for the store/recall morph messages."};
    MIN_TAGS{"filters, effects"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.svf~, tap.ladder~, tap.filter~, tap.adsr~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_key{this, "(signal) optional sidechain input for the envelope detector"};
    outlet<> m_out{this, "(signal) filtered output", "signal"};
    outlet<> m_out_env{this, "(signal) envelope, 0..1", "signal"};

    autowah(const atoms& args = {}) { m_filter.prepare(samplerate()); }

#define TAP_AUTOWAH_ATTR(NAME, INDEX, DEFAULT, DESC)                                                                   \
    attribute<number> NAME{this, #NAME, DEFAULT,                                                                       \
                           setter{MIN_FUNCTION{const double v = kernel::clamp_param(INDEX, args[0]);                   \
    m_filter.set_param(INDEX, v);                                                                                      \
    return {v};                                                                                                        \
    }                                                                                                                  \
    }                                                                                                                  \
    , description { DESC }                                                                                             \
    }                                                                                                                  \
    ;

    TAP_AUTOWAH_ATTR(sensitivity, kernel::p_sensitivity, 0.0,
                     "Envelope-detector input gain in dB (-60..24) — the filter trigger level; tune it to your "
                     "instrument and touch. At -60 the envelope is off and the object becomes a fixed, manually "
                     "swept filter with bias as the sweep control (the pedal's cocked-wah mode).")
    TAP_AUTOWAH_ATTR(attack, kernel::p_attack, 2.0,
                     "Envelope attack in ms (0.05..100). The hardware's attack is fast and fixed (~2 ms) — raise "
                     "it for softer filter onsets.")
    TAP_AUTOWAH_ATTR(decay, kernel::p_decay, 250.0,
                     "How fast the filter falls back to the bias resting point, in ms (10..5000). Fast gives a "
                     "wah on every note; slow gives traditional auto-wah swells.")
    TAP_AUTOWAH_ATTR(bias, kernel::p_bias, 250.0,
                     "The filter's resting frequency in Hz (20..20000) — where the sweep starts from and returns "
                     "to. The hardware rests around 250 Hz.")
    TAP_AUTOWAH_ATTR(resonance, kernel::p_resonance, 0.55,
                     "Filter resonance / Q, normalized 0..1 (1 = the edge of self-oscillation).")
    TAP_AUTOWAH_ATTR(drive, kernel::p_drive, 0.0,
                     "Drive in dB (0..24) into the saturating filter circuit — an OTA-flavored color stage, "
                     "oversampled 2x. 0 runs the pure linear filter.")
    TAP_AUTOWAH_ATTR(gain, kernel::p_gain, 0.0, "Output gain in dB.")
    TAP_AUTOWAH_ATTR(mix, kernel::p_mix, 100.0,
                     "Dry/wet mix in percent, equal-power (0..100). The hardware is wet-only: 100.")

#undef TAP_AUTOWAH_ATTR

    attribute<number> range{this, "range", 3.3, setter{ MIN_FUNCTION {
                                const double v = std::clamp(static_cast<double>(args[0]), 0.0, kernel::k_range_max_oct);
                                m_range_mag    = v;
                                forward_range();
                                return {v};
                            }},
                            description{"Sweep span in octaves above (or below, see direction) the bias frequency "
                                        "(0..5). The hardware sweeps ~3.3 octaves (250 Hz to ~2500 Hz)."}};

    attribute<int> direction{this, "direction", 0, setter{ MIN_FUNCTION {
                                 const int v = std::clamp(static_cast<int>(args[0]), 0, 1);
                                 m_direction = v;
                                 forward_range();
                                 return {v};
                             }},
                             description{"Sweep direction: 0 = up from bias (the pedal), 1 = down from bias (a "
                                         "TapTools extension)."}};

    // Mode as an int attribute with named index constants — not attribute<enum class>, per the
    // tap.crossfade~/tap.pan~ GCC lesson (REVIVAL.md 9.5).
    attribute<int> mode{this, "mode", kernel::mode_lowpass, setter{ MIN_FUNCTION {
                            const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_modes - 1);
                            m_filter.set_mode(v);
                            return {v};
                        }},
                        description{"Filter tap: 0 = lowpass (the stock resonant voicing), 1 = bandpass (the "
                                    "circuit's other tap — quackier, quieter)."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{ MIN_FUNCTION {
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_filter.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper interpolation time in ms applied to parameter changes."}};

    attribute<number> interp{this, "interp", 500.0, setter{ MIN_FUNCTION {
                                 return {std::max(0.0, static_cast<double>(args[0]))};
                             }},
                             description{"Default preset-morph time in ms used by the 'recall' message."}};

    attribute<bool> bypass{this, "bypass", false, description{"Pass the input through unprocessed."}};

    attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

    // -- messages -------------------------------------------------------------------------------------

    message<> store{this, "store", "Store the current parameters in a preset slot (1..16).",
                    MIN_FUNCTION {
                        if (!args.empty()) {
                            m_filter.store_preset(static_cast<int>(args[0]) - 1);
                        }
                        return {};
                    }};

    message<> recall{this, "recall",
                     "Morph all parameters to a stored preset (1..16; slots 1-4 ship factory voicings: guitar, "
                     "bass, slow swell, cocked wah). An optional second argument overrides the morph time in ms "
                     "(default: the interp attribute).",
                     MIN_FUNCTION {
                         if (!args.empty()) {
                             const double ms =
                                 (args.size() > 1) ? static_cast<double>(args[1]) : static_cast<double>(interp);
                             m_filter.recall_preset(static_cast<int>(args[0]) - 1, ms * 0.001);
                         }
                         return {};
                     }};

    message<> clear{this, "clear", "Reset the filter state and the envelope.",
                    MIN_FUNCTION {
                        m_filter.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Reconfigure for the sample rate when DSP starts.",
                       MIN_FUNCTION {
                           m_filter.prepare(samplerate());
                           return {};
                       }};

    // -- perform --------------------------------------------------------------------------------------

    void operator()(audio_bundle input, audio_bundle output) override {
        const long    n       = input.frame_count();
        const double* in      = input.samples(0);
        double*       out     = output.samples(0);
        double*       env_out = output.samples(1);

        if (mute) {
            std::fill_n(out, n, 0.0);
            std::fill_n(env_out, n, 0.0);
        }
        else if (bypass) {
            std::copy_n(in, n, out);
            std::fill_n(env_out, n, 0.0);
        }
        else if (m_in_key.has_signal_connection()) {
            const double* key = input.samples(1);
            for (long i = 0; i < n; ++i) {
                out[i]     = m_filter.process(in[i], key[i]); // envelope tracks the sidechain
                env_out[i] = m_filter.envelope();
            }
        }
        else {
            for (long i = 0; i < n; ++i) {
                out[i]     = m_filter.process(in[i]); // envelope tracks the input (the pedal)
                env_out[i] = m_filter.envelope();
            }
        }
    }

    // Exposed for unit tests.
    kernel::wah_filter& filter() { return m_filter; }
};

MIN_EXTERNAL(autowah);
