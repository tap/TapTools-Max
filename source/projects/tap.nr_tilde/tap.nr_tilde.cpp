/// @file
/// @brief      tap.nr~ — spectral noise reduction (a spectral expander / gate).
/// @details    Reconstructed from its surviving reference documentation (no source survived the
///             revival). The original was a `pfft~`-hosted object (`tap.xnr~`) wrapped in an
///             abstraction; per the author's decision to reinvent the lost spectral DSP, this is a
///             **self-contained** external that runs its own short-time Fourier transform — no
///             `pfft~` needed.
///
///             Each analysis frame is windowed, transformed, and every bin whose magnitude falls
///             below `threshold` is attenuated; bins above it pass. The `slope` parameter controls
///             how gradually the gate engages across the threshold (a soft knee): low values fade
///             gently, high values approach a hard gate. The frame is inverse-transformed, windowed,
///             and overlap-added back to the output. Faithful to the documented behaviour: "a
///             spectral expander/gate to silence bins whose signal falls below a threshold," with
///             the left inlet setting the threshold and the right inlet setting how gradually it
///             engages.
///
///             The DSP lives in the portable, Min-free kernel `tap::tools::nr::reducer` (nr.h, which
///             runs a Hann-windowed 4×-overlap STFT with COLA-normalised overlap-add). This file is
///             the Min wrapper. Latency = one FFT frame.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <taptools/nr.h>

#include "c74_min.h"

using namespace c74::min;

class nr : public object<nr>, public vector_operator<> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    tap::tools::nr::reducer m_reducer;

    static bool is_power_of_two(int v) { return v > 0 && (v & (v - 1)) == 0; }

  public:
    MIN_DESCRIPTION{"Spectral noise reduction: a spectral expander/gate that silences FFT bins "
                    "whose magnitude falls below a threshold. Self-contained — it runs its own "
                    "STFT, so no pfft~ wrapper is required."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.gate~, gate~, pfft~, fft~"};

    inlet<>  m_in{this, "(signal) input — and (float) sets the threshold"};
    inlet<>  m_thr{this, "(float) sets how gradually the threshold engages (the gate slope)"};
    outlet<> m_out{this, "(signal) the de-noised output", "signal"};

    // First argument = FFT size (a power of two). The original abstraction also took fftsize/2 as a
    // second argument for its pfft~; here it is redundant (derived internally) and ignored.
    nr(const atoms& args = {}) {
        int fftsize = 1024;
        if (!args.empty()) {
            const int requested = static_cast<int>(args[0]);
            if (is_power_of_two(requested) && requested >= 16) {
                fftsize = requested;
            }
        }
        m_reducer.configure(fftsize);
    }

    attribute<number> threshold{
        this,
        "threshold",
        0.01,
        range{0.0, 1.0},
        setter{MIN_FUNCTION{
            m_reducer.set_threshold(args[0]);
            return {args[0]};
        }},
        description{"Linear-amplitude threshold. Bins below this level are attenuated; bins above "
                    "it pass. 0 disables the gate (the output then reconstructs the input)."}};

    attribute<number> slope{this,
                            "slope",
                            2.0,
                            range{0.0, 64.0},
                            setter{MIN_FUNCTION{
                                m_reducer.set_slope(args[0]);
                                return {args[0]};
                            }},
                            description{"How gradually the gate engages across the threshold (soft knee). Low values "
                                        "fade gently; high values approach a hard gate; 0 passes everything."}};

    message<> m_number{this, "number",
                       "A float in the left inlet sets the threshold; in the right "
                       "inlet sets the slope.",
                       MIN_FUNCTION{
                           if (inlet == 1) {
                               slope = args[0];
                           }
                           else {
                               threshold = args[0];
                           }
                           return {};
                       }};

    message<> clear{this, "clear", "Reset the internal STFT buffers.",
                    MIN_FUNCTION{
                        m_reducer.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Reset buffers when the DSP chain starts.",
                       MIN_FUNCTION{
                           m_reducer.reset();
                           return {};
                       }};

    void operator()(audio_bundle input, audio_bundle output) {
        m_reducer.process(input.samples(0), output.samples(0), input.frame_count());
    }
};

MIN_EXTERNAL(nr);
