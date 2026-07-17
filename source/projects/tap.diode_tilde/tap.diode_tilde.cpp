/// @file
/// @brief      tap.diode~ — virtual-analog diode-ladder filter (the TB-303 topology).
/// @details    A thin Min wrapper around the portable kernel in diode_ladder.h (all DSP lives
///             there — see its header comment for the model and its provenance against Tim
///             Stinchcombe's TB-303 circuit analysis). A zero-delay-feedback 4-stage diode
///             ladder: unbuffered, mutually-loading stages (the poles spread ~25:1 — the honest
///             version of the "18 dB" story), tanh saturation on every diode pair, a high-passed
///             resonance feedback path (the hardware's own bass-preserving trick, and the reason
///             a stock 303 famously never quite self-oscillates), prewarped tuning, and 1x/2x/4x
///             oversampling. The right inlet takes a signal for true per-sample cutoff modulation
///             (or a float to set the frequency attribute). This is the acid-flavored sibling of
///             tap.ladder~ (the buffered Moog-style transistor ladder). Single-channel: wrap in
///             mc. for multichannel.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/diode_ladder.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = taptools::diode;

class diode : public object<diode>, public vector_operator<> {
  private:
    // Constructed before the attributes below so attribute defaults can forward into it.
    kernel::diode_filter m_filter;

  public:
    MIN_DESCRIPTION{"A virtual-analog diode-ladder filter — the TB-303 lowpass topology. Unlike "
                    "the transistor ladder (tap.ladder~), the diode ladder's stages load each "
                    "other, spreading the poles: a shallow knee near cutoff opening into a "
                    "24 dB/octave slope, with broad, squelchy resonance. The resonance feedback "
                    "is high-passed like the hardware, so the bass survives high resonance — and, "
                    "like a stock TB-303, the standard resonance range never quite "
                    "self-oscillates; the extended range (above 1) and the fbhp attribute are "
                    "the documented circuit bends that push it over. Saturating diode pairs, "
                    "prewarped tuning, oversampled 2x by default. A signal in the right inlet "
                    "modulates the cutoff per sample."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.ladder~, tap.fourpole~, tap.svf~, tap.filter~, lores~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_freq{this, "(signal/float) cutoff frequency in Hz"};
    outlet<> m_out{this, "(signal) filtered output", "signal"};

    diode(const atoms& args = {}) { m_filter.prepare(samplerate()); }

#define TAP_DIODE_ATTR(NAME, INDEX, DEFAULT, DESC)                                                                     \
    attribute<number> NAME{this, #NAME, DEFAULT,                                                                       \
                           setter{MIN_FUNCTION{const double v = kernel::clamp_param(INDEX, args[0]);                   \
    m_filter.set_param(INDEX, v);                                                                                      \
    return {v};                                                                                                        \
    }                                                                                                                  \
    }                                                                                                                  \
    , description { DESC }                                                                                             \
    }                                                                                                                  \
    ;

    TAP_DIODE_ATTR(frequency, kernel::p_frequency, 1000.0,
                   "Cutoff frequency in Hz (10..20000) — calibrated to the resonance peak / oscillation "
                   "frequency. At low resonance the -3 dB point sits well below this (the diode ladder's "
                   "spread poles — authentic). A signal in the right inlet overrides this per sample.")
    TAP_DIODE_ATTR(resonance, kernel::p_resonance, 0.0,
                   "Resonance (0..1.5). 0..1 is the hardware-realistic range — like a stock TB-303 it gets "
                   "squelchy but never quite self-oscillates (the high-passed feedback sees to that). Above "
                   "1 is the documented bend: the filter sings at high cutoffs, amplitude-limited by the "
                   "diodes. With fbhp 0 the threshold is exactly 1.0 and oscillation lands exactly on the "
                   "cutoff frequency.")
    TAP_DIODE_ATTR(drive, kernel::p_drive, 0.0,
                   "Input drive in dB (-24..24): how hard the signal hits the saturating diode pairs.")
    TAP_DIODE_ATTR(fbhp, kernel::p_fbhp, kernel::k_fbhp_default_hz,
                   "Resonance-feedback high-pass corner in Hz (0..2000; default 150, the hardware value). "
                   "This is why resonance thins as cutoff comes down and why the bass survives. 0 bends the "
                   "circuit to DC-coupled feedback: transistor-ladder-style behavior, exact self-oscillation "
                   "tuning, and low-end droop at high resonance.")
    TAP_DIODE_ATTR(gain, kernel::p_gain, 0.0, "Output gain in dB.")

#undef TAP_DIODE_ATTR

    attribute<int> solver{this, "solver", kernel::solver_fast, setter{ MIN_FUNCTION {
                              const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_solvers - 1);
                              m_filter.set_solver(v);
                              return {v};
                          }},
                          description{"Nonlinear solver for the coupled diode equations: 0 = fast (secant-"
                                      "linearized solve with one corrective pass — the default), 1 = exact "
                                      "(re-linearization iterated to convergence). The two are audibly identical "
                                      "until drive and resonance are both pushed hard; exact costs more CPU."}};

    attribute<int> oversample{this, "oversample", 2, setter{ MIN_FUNCTION {
                                  const int req = static_cast<int>(args[0]);
                                  const int v   = (req >= 4) ? 4 : (req >= 2) ? 2 : 1;
                                  m_filter.set_oversample(v);
                                  return {v};
                              }},
                              description{"Internal oversampling factor (1, 2, or 4; default 2). Higher factors "
                                          "reduce aliasing from the diode saturation and tuning error near "
                                          "Nyquist."}};

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
    kernel::diode_filter& filter() { return m_filter; }
};

MIN_EXTERNAL(diode);
