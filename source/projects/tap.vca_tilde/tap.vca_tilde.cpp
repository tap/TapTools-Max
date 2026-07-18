/// @file
/// @brief      tap.vca~ — voltage-controlled amplifier: a clean multiply or a warm transistor stage.
/// @details    A thin Min wrapper around the portable kernel in vca.h (all DSP lives there — see its
///             header comment for the model). The left inlet is the audio; the right inlet is the
///             gain / control voltage (a signal for true per-sample modulation, or a float to set
///             the gain attribute). Two circuits, the svf~ idiom:
///
///             - circuit clean (default): the pure linear multiply, out = in * gain — bit-identical
///               to a *~. Transparent, cheap, no added harmonics or DC.
///             - circuit warm: the TB-303's one-transistor class-A VCA stage — the same code
///               tap.303~ runs (shared kernel, vca.h). A slope-normalized biased tanh applied to
///               the post-gain signal: unity gain through zero (quiet signals near-clean), even
///               harmonics and gentle compression that ride the control voltage on hot signals, and
///               an output-coupling DC block (the dcblock attribute) that sheds the shaper's
///               signal-dependent offset the way the hardware's coupling capacitor does.
///
///             Single-channel: wrap in mc. for multichannel (the kernel is reusable outside Max).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/vca.h>

#include "c74_min.h"

using namespace c74::min;

class vca : public object<vca>, public vector_operator<> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    taptools::vca m_vca;

  public:
    MIN_DESCRIPTION{"A voltage-controlled amplifier. The left inlet is the audio, the right inlet "
                    "is the gain / control voltage (a signal for per-sample modulation, or a float "
                    "for a fixed gain). The clean circuit is a pure linear multiply (bit-identical "
                    "to *~). The warm circuit is the TB-303's one-transistor class-A VCA stage — a "
                    "biased, slope-normalized tanh applied after the gain, so quiet signals stay "
                    "essentially clean while hot signals pick up even-harmonic warmth and gentle "
                    "compression that track the control voltage; an output DC block removes the "
                    "stage's signal-dependent offset. Single-channel: wrap in mc. for multichannel."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.303~, tap.crossfade~, tap.pan~, *~, gain~, matrix~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_gain{this, "(signal/float) gain / control voltage (linear)"};
    outlet<> m_out{this, "(signal) amplified output", "signal"};

    vca(const atoms& args = {}) { m_vca.prepare(samplerate()); }

    attribute<symbol> circuit{
        this, "circuit", "clean", range{"clean", "warm"},
        setter{ MIN_FUNCTION {
            m_vca.set_mode(args[0] == "warm" ? taptools::vca::mode_warm : taptools::vca::mode_clean);
            return args;
        }},
        description{"clean: the pure linear multiply (transparent, bit-identical to *~). "
                    "warm: the TB-303 one-transistor class-A stage — biased-tanh saturation applied "
                    "after the gain, with even-harmonic warmth and compression that ride the control "
                    "voltage, plus the output DC block (see the dcblock attribute)."}};

    attribute<number> gain{this, "gain", 1.0,
                           description{"Linear gain applied when no signal is connected to the right "
                                       "inlet. A signal in the right inlet overrides this per sample."}};

    attribute<number> drive{
        this, "drive", taptools::vca::k_default_drive, range{0.1, 12.0},
        setter{ MIN_FUNCTION {
            const double v = std::clamp(static_cast<double>(args[0]), 0.1, 12.0);
            m_vca.set_drive(v);
            return {v};
        }},
        description{"Warm-circuit drive into the transistor stage's tanh (0.1..12; default 2.0, the "
                    "stock 303 value). Higher drive means more harmonics and more compression. "
                    "Ignored by the clean circuit."}};

    attribute<number> bias{
        this, "bias", taptools::vca::k_default_bias, range{-2.0, 2.0},
        setter{ MIN_FUNCTION {
            const double v = std::clamp(static_cast<double>(args[0]), -2.0, 2.0);
            m_vca.set_bias(v);
            return {v};
        }},
        description{"Warm-circuit operating-point bias (-2..2; default 0.3, the stock 303 value). "
                    "The asymmetry that produces even harmonics; 0 is a symmetric (odd-only) shaper. "
                    "Ignored by the clean circuit."}};

    attribute<bool> dcblock{this, "dcblock", true,
                            setter{ MIN_FUNCTION {
                                m_vca.set_dc_block(static_cast<bool>(args[0]));
                                return args;
                            }},
                            description{"Warm-circuit output DC block (default on). The biased shaper "
                                        "leaves a signal-dependent DC offset on AC material; this "
                                        "one-pole coupling high-pass removes it, as the hardware's "
                                        "output capacitor does. Turn off to hear the raw asymmetry."}};

    attribute<bool> bypass{this, "bypass", false, description{"Pass the input through unprocessed."}};

    attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

    // -- messages -------------------------------------------------------------------------------

    message<> m_number{this, "number", "A float in the right inlet sets the gain.",
                       MIN_FUNCTION {
                           if (inlet == 1) {
                               gain = args[0];
                           }
                           return {};
                       }};

    message<> clear{this, "clear", "Reset the stage's DC-block state.",
                    MIN_FUNCTION {
                        m_vca.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Reconfigure for the sample rate when DSP starts.",
                       MIN_FUNCTION {
                           m_vca.prepare(samplerate());
                           return {};
                       }};

    // -- perform --------------------------------------------------------------------------------

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
        else if (m_in_gain.has_signal_connection()) {
            const double* g = input.samples(1);
            for (long i = 0; i < n; ++i) {
                out[i] = m_vca.process(in[i], g[i]); // true per-sample gain / CV modulation
            }
        }
        else {
            const double g = gain;
            for (long i = 0; i < n; ++i) {
                out[i] = m_vca.process(in[i], g);
            }
        }
    }

    // Exposed for unit tests.
    taptools::vca& stage() { return m_vca; }
};

MIN_EXTERNAL(vca);
