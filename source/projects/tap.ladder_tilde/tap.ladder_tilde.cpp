/// @file
/// @brief      tap.ladder~ — virtual-analog transistor-ladder filter.
/// @details    A thin Min wrapper around the portable kernel in ladder.h (all DSP lives there —
///             see its header comment for the model). Zero-delay-feedback 4-stage ladder with
///             per-stage tanh saturation: accurate prewarped tuning, clean self-oscillation at
///             high resonance, an input drive control, 1x/2x/4x oversampling, and multimode
///             outputs via Xpander-style pole mixing. The right inlet takes a signal for true
///             per-sample cutoff modulation (or a float to set the frequency attribute).
///             This is the nonlinear sibling of tap.fourpole~ (the simple linear Stilson/Smith
///             ladder, which remains available as the cheap/vintage option). Single-channel:
///             wrap in mc. for multichannel.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/ladder.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = taptools::ladder;

class ladder : public object<ladder>, public vector_operator<> {
  private:
    // Constructed before the attributes below so attribute defaults can forward into it.
    kernel::ladder_filter m_filter;

  public:
    MIN_DESCRIPTION{"A virtual-analog transistor-ladder filter: a zero-delay-feedback 4-pole "
                    "ladder with tanh saturation in every stage. The tuning is prewarped and "
                    "stays accurate into the top octaves, resonance reaches clean "
                    "self-oscillation (give it a ping to start it), drive pushes the input into "
                    "the nonlinearity, and a mode attribute selects lowpass, bandpass, and "
                    "highpass responses via Oberheim-Xpander-style pole mixing. Runs "
                    "oversampled (2x by default). A signal in the right inlet modulates the "
                    "cutoff per sample. For the simple linear ladder, see tap.fourpole~."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.fourpole~, tap.svf~, tap.filter~, lores~, moogff~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_freq{this, "(signal/float) cutoff frequency in Hz"};
    outlet<> m_out{this, "(signal) filtered output", "signal"};

    ladder(const atoms& args = {}) { m_filter.prepare(samplerate()); }

#define TAP_LADDER_ATTR(NAME, INDEX, DEFAULT, DESC)                                                                    \
    attribute<number> NAME{this, #NAME, DEFAULT,                                                                       \
                           setter{MIN_FUNCTION{const double v = kernel::clamp_param(INDEX, args[0]);                   \
    m_filter.set_param(INDEX, v);                                                                                      \
    return {v};                                                                                                        \
    }                                                                                                                  \
    }                                                                                                                  \
    , description { DESC }                                                                                             \
    }                                                                                                                  \
    ;

    TAP_LADDER_ATTR(frequency, kernel::p_frequency, 1000.0,
                    "Cutoff frequency in Hz (10..20000). A signal in the right inlet overrides this per sample.")
    TAP_LADDER_ATTR(resonance, kernel::p_resonance, 0.0,
                    "Resonance (0..1.1). 1.0 is the edge of self-oscillation; above it the filter oscillates "
                    "at the cutoff frequency, amplitude-limited by the tanh stages.")
    TAP_LADDER_ATTR(drive, kernel::p_drive, 0.0,
                    "Input drive in dB (-24..24): how hard the signal hits the saturating ladder.")
    TAP_LADDER_ATTR(comp, kernel::p_comp, 0.0,
                    "Passband-loss compensation (0..1). 0 is the authentic ladder passband droop as resonance "
                    "rises; 1 fully compensates.")
    TAP_LADDER_ATTR(asym, kernel::p_asym, 0.0,
                    "Saturation asymmetry (0..1): a small operating-point shift in every ladder stage, "
                    "modeling transistor mismatch. 0 is perfectly symmetric (odd harmonics only); raising it "
                    "adds the even-harmonic warmth of real hardware. May produce slight signal-dependent DC "
                    "when engaged — follow with tap.dcblock~ if that matters downstream.")
    TAP_LADDER_ATTR(gain, kernel::p_gain, 0.0, "Output gain in dB.")

#undef TAP_LADDER_ATTR

    attribute<int> solver{this, "solver", kernel::solver_fast, setter{ MIN_FUNCTION {
                              const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_solvers - 1);
                              m_filter.set_solver(v);
                              return {v};
                          }},
                          description{"Nonlinear feedback solver: 0 = fast (linear zero-delay prediction plus one "
                                      "corrective saturation pass — the default), 1 = exact (Newton iteration to "
                                      "convergence, circuit-simulation accuracy). The two are audibly identical "
                                      "until drive and resonance are both pushed hard; exact costs more CPU."}};

    // Mode as an int attribute with named index constants — not attribute<enum class>, per the
    // tap.crossfade~/tap.pan~ GCC lesson (REVIVAL.md 9.5).
    attribute<int> mode{this, "mode", kernel::mode_lp24, setter{ MIN_FUNCTION {
                            const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_modes - 1);
                            m_filter.set_mode(v);
                            return {v};
                        }},
                        description{"Filter response via pole mixing: 0 = lp24, 1 = lp12, 2 = bp12, 3 = bp24, "
                                    "4 = hp12, 5 = hp24."}};

    attribute<int> oversample{this, "oversample", 2, setter{ MIN_FUNCTION {
                                  const int req = static_cast<int>(args[0]);
                                  const int v   = (req >= 4) ? 4 : (req >= 2) ? 2 : 1;
                                  m_filter.set_oversample(v);
                                  return {v};
                              }},
                              description{"Internal oversampling factor (1, 2, or 4; default 2). Higher factors "
                                          "reduce aliasing from the saturation and tuning error near Nyquist."}};

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

    message<> m_number{this, "number", "A float in the right inlet sets the cutoff frequency.",
                       MIN_FUNCTION {
                           if (inlet == 1) {
                               frequency = args[0];
                           }
                           return {};
                       }};

    message<> store{this, "store", "Store the current parameters in a preset slot (1..16).",
                    MIN_FUNCTION {
                        if (!args.empty()) {
                            m_filter.store_preset(static_cast<int>(args[0]) - 1);
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
                             m_filter.recall_preset(static_cast<int>(args[0]) - 1, ms * 0.001);
                         }
                         return {};
                     }};

    message<> clear{this, "clear", "Reset the filter state (stops self-oscillation).",
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
        const long    n   = input.frame_count();
        const double* in  = input.samples(0);
        double*       out = output.samples(0);

        if (mute) {
            std::fill_n(out, n, 0.0);
        }
        else if (bypass) {
            std::copy_n(in, n, out);
        }
        else if (m_in_freq.has_signal_connection()) {
            const double* freq = input.samples(1);
            for (long i = 0; i < n; ++i) {
                out[i] = m_filter.process(in[i], freq[i]); // true per-sample cutoff modulation
            }
        }
        else {
            m_filter.process(in, out, static_cast<size_t>(n));
        }
    }

    // Exposed for unit tests.
    kernel::ladder_filter& filter() { return m_filter; }
};

MIN_EXTERNAL(ladder);
