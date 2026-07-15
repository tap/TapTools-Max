/// @file
/// @brief      tap.vco~ — virtual-analog oscillator.
/// @details    A thin Min wrapper around the portable kernel in vco.h (all DSP lives there — see
///             its header comment). PolyBLEP alias-suppressed saw/pulse (with PWM), an
///             integrated-square triangle, and a pure sine, all morphable through a continuous
///             shape parameter. Hard sync (third inlet), through-zero linear FM in Hz (second
///             inlet), and an analog-character section, deterministic per seed: slow pitch
///             drift, fast pitch jitter, static detune, V/oct tracking error, and a waveform
///             imperfection control with per-seed component tolerances. Single-channel;
///             mc.-wrap for stacks and give each instance a different seed — each becomes a
///             slightly different unit off the production line.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/vco.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = taptools::vco;

class vco : public object<vco>, public vector_operator<> {
  private:
    // Constructed before the attributes below so attribute defaults can forward into it.
    kernel::vco_osc m_osc;

  public:
    MIN_DESCRIPTION{"A virtual-analog oscillator: alias-suppressed saw, pulse (with PWM), "
                    "triangle, and sine, morphable through a continuous shape parameter. "
                    "Hard sync on the third inlet (rising zero crossing resets the phase, "
                    "alias-corrected), through-zero linear FM in Hz on the second inlet, and "
                    "an analog-character section, deterministic per seed: slow pitch drift, "
                    "fast jitter, static detune, V/oct tracking error, and a waveform "
                    "imperfection control that gives every seed the component tolerances of a "
                    "different vintage unit. All parameters glide smoothly and sixteen preset "
                    "slots can be stored and morphed between."};
    MIN_TAGS{"generators"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.ladder~, tap.noise~, saw~, rect~, tri~, cycle~"};

    inlet<>  m_in_freq{this, "(signal/float) frequency in Hz"};
    inlet<>  m_in_fm{this, "(signal) through-zero linear FM, in Hz"};
    inlet<>  m_in_sync{this, "(signal) hard sync (resets on a rising zero crossing)"};
    outlet<> m_out{this, "(signal) oscillator output", "signal"};

    vco(const atoms& args = {}) { m_osc.prepare(samplerate()); }

#define TAP_VCO_ATTR(NAME, INDEX, DEFAULT, DESC)                                                                       \
    attribute<number> NAME{this, #NAME, DEFAULT,                                                                       \
                           setter{MIN_FUNCTION{const double v = kernel::clamp_param(INDEX, args[0]);                   \
    m_osc.set_param(INDEX, v);                                                                                         \
    return {v};                                                                                                        \
    }                                                                                                                  \
    }                                                                                                                  \
    , description { DESC }                                                                                             \
    }                                                                                                                  \
    ;

    TAP_VCO_ATTR(frequency, kernel::p_frequency, 220.0,
                 "Frequency in Hz (0.01..20000). A signal in the left inlet overrides this per sample.")
    TAP_VCO_ATTR(shape, kernel::p_shape, 2.0,
                 "Waveform morph (0..3): 0 = sine, 1 = triangle, 2 = saw, 3 = pulse; fractional values "
                 "crossfade between the adjacent shapes.")
    TAP_VCO_ATTR(pw, kernel::p_pw, 50.0, "Pulse width in percent (1..99), active as shape approaches 3.")
    TAP_VCO_ATTR(drift, kernel::p_drift, 0.0,
                 "Analog-style slow random pitch drift depth in cents (0..100). Deterministic per seed.")
    TAP_VCO_ATTR(detune, kernel::p_detune, 0.0, "Static detune in cents (-1200..1200).")
    TAP_VCO_ATTR(imperfect, kernel::p_imperfect, 0.0,
                 "Waveform imperfection (0..1): the saw takes on the curved ramp and rounded reset corner of a "
                 "vintage oscillator core, the triangle goes slightly asymmetric (even harmonics), the sine picks "
                 "up mild shaper color, and the pulse width takes a small static offset. The amounts are per-unit "
                 "component tolerances drawn from the seed, so each seed is a slightly different oscillator. 0 is "
                 "the exact ideal waveform regardless of seed.")
    TAP_VCO_ATTR(jitter, kernel::p_jitter, 0.0,
                 "Fast pitch noise depth in cents (0..20) - the short-time instability of a real oscillator core, "
                 "the quick companion to drift. Deterministic per seed.")
    TAP_VCO_ATTR(track, kernel::p_track, 0.0,
                 "V/oct calibration error in cents per octave (-10..10), measured from A440: the pitch offset "
                 "grows with distance from the trim point, like an analog exponential converter drifting out of "
                 "calibration.")
    TAP_VCO_ATTR(gain, kernel::p_gain, 0.0, "Output gain in dB.")

#undef TAP_VCO_ATTR

    attribute<int> seed{this, "seed", 1, setter{ MIN_FUNCTION {
                            const int v = std::max(1, static_cast<int>(args[0]));
                            m_osc.set_seed(static_cast<uint32_t>(v));
                            return {v};
                        }},
                        description{"Seed for the drift generator (>= 1). Give mc. instances different seeds "
                                    "to decorrelate their drift."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{ MIN_FUNCTION {
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_osc.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper interpolation time in ms applied to parameter changes."}};

    attribute<number> interp{this, "interp", 500.0, setter{ MIN_FUNCTION {
                                 return {std::max(0.0, static_cast<double>(args[0]))};
                             }},
                             description{"Default preset-morph time in ms used by the 'recall' message."}};

    attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

    // -- messages ------------------------------------------------------------------------------------

    message<> waveform{this, "waveform", "Snap the shape to a classic waveform: sine, triangle, saw, or pulse.",
                       MIN_FUNCTION {
                           if (!args.empty()) {
                               const std::string w = static_cast<std::string>(args[0]);
                               if (w == "sine") {
                                   m_osc.set_waveform(kernel::wave_sine);
                               }
                               else if (w == "triangle" || w == "tri") {
                                   m_osc.set_waveform(kernel::wave_triangle);
                               }
                               else if (w == "saw") {
                                   m_osc.set_waveform(kernel::wave_saw);
                               }
                               else if (w == "pulse" || w == "square" || w == "rect") {
                                   m_osc.set_waveform(kernel::wave_pulse);
                               }
                           }
                           return {};
                       }};

    message<> m_number{this, "number", "A float in the left inlet sets the frequency.",
                       MIN_FUNCTION {
                           if (inlet == 0) {
                               frequency = args[0];
                           }
                           return {};
                       }};

    message<> store{this, "store", "Store the current parameters in a preset slot (1..16).",
                    MIN_FUNCTION {
                        if (!args.empty()) {
                            m_osc.store_preset(static_cast<int>(args[0]) - 1);
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
                             m_osc.recall_preset(static_cast<int>(args[0]) - 1, ms * 0.001);
                         }
                         return {};
                     }};

    message<> clear{this, "clear", "Reset the oscillator phase and drift state.",
                    MIN_FUNCTION {
                        m_osc.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Reconfigure for the sample rate when DSP starts.",
                       MIN_FUNCTION {
                           m_osc.prepare(samplerate());
                           return {};
                       }};

    // -- perform -------------------------------------------------------------------------------------

    void operator()(audio_bundle input, audio_bundle output) override {
        const long n   = input.frame_count();
        double*    out = output.samples(0);

        if (mute) {
            std::fill_n(out, n, 0.0);
            return;
        }

        const bool    has_freq = m_in_freq.has_signal_connection();
        const bool    has_fm   = m_in_fm.has_signal_connection();
        const bool    has_sync = m_in_sync.has_signal_connection();
        const double* freq     = has_freq ? input.samples(0) : nullptr;
        const double* fm       = has_fm ? input.samples(1) : nullptr;
        const double* sync     = has_sync ? input.samples(2) : nullptr;

        for (long i = 0; i < n; ++i) {
            const double fm_v   = has_fm ? fm[i] : 0.0;
            const double sync_v = has_sync ? sync[i] : 0.0;
            out[i]              = has_freq ? m_osc.process_at(freq[i], fm_v, sync_v) : m_osc.process(fm_v, sync_v);
        }
    }

    // Exposed for unit tests.
    kernel::vco_osc& osc() { return m_osc; }
};

MIN_EXTERNAL(vco);
