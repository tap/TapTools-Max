/// @file
/// @brief      tap.spectra~ — spectral remapping.
/// @details    Reconstructed from its surviving reference documentation (no source survived the
///             revival). The original ran a `pfft~` subpatcher (`tap.spectra.pfft`) that used
///             `tap.scale~` to reorient FFT bins onto different IFFT bins; per the author's decision
///             to reinvent the lost spectral DSP, this is a **self-contained** external that runs
///             its own short-time Fourier transform — no `pfft~` needed.
///
///             Per the reference page: "performs a remapping of spectral data by reorienting the
///             bins of an FFT to different bins of an IFFT. The result is an ultra-non-linear effect
///             that can yield unexpected results." Here, each output bin <i>k</i> takes its complex
///             value from input bin round(<i>k</i> · `remap`). `remap` = 1 is the identity (output
///             reconstructs the input); &gt; 1 stretches the spectrum upward, &lt; 1 compresses it
///             toward DC. Hermitian symmetry is enforced so the output stays real.
///
///             The DSP lives in the portable, Min-free kernel `tap::tools::spectra::remapper`
///             (spectra.h, a Hann-windowed 4×-overlap STFT with COLA-normalised overlap-add). This
///             file is the Min wrapper. Latency = one FFT frame.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include <taptools/spectra.h>

#include "c74_min.h"

using namespace c74::min;

class spectra : public object<spectra>, public vector_operator<> {
  private:
    // Constructed before the attribute below so its default can forward into it.
    tap::tools::spectra::remapper m_remapper;

    static bool is_power_of_two(int v) { return v > 0 && (v & (v - 1)) == 0; }

  public:
    MIN_DESCRIPTION{"Spectral remapping: reorients the bins of an FFT onto different bins of an "
                    "IFFT, an ultra-non-linear effect. Self-contained — it runs its own STFT, so "
                    "no pfft~ wrapper is required."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.nr~, pfft~, fft~, gizmo~"};

    inlet<>  m_in{this, "(signal) input — and (float) sets the remapping ratio"};
    outlet<> m_out{this, "(signal) the spectrally remapped output", "signal"};

    spectra(const atoms& args = {}) {
        int fftsize = 1024;
        if (!args.empty()) {
            const int requested = static_cast<int>(args[0]);
            if (is_power_of_two(requested) && requested >= 16) {
                fftsize = requested;
            }
        }
        m_remapper.configure(fftsize);
    }

    attribute<number> remap{this,
                            "remap",
                            1.0,
                            range{0.0, 16.0},
                            setter{MIN_FUNCTION{
                                m_remapper.set_remap(args[0]);
                                return {args[0]};
                            }},
                            description{"Spectral remapping ratio. Output bin k takes its value from input bin "
                                        "round(k * remap). 1 is the identity; >1 stretches the spectrum upward, <1 "
                                        "compresses it toward DC."}};

    message<> m_number{this, "number", "A float sets the remapping ratio.",
                       MIN_FUNCTION{
                           remap = args[0];
                           return {};
                       }};

    message<> clear{this, "clear", "Reset the internal STFT buffers.",
                    MIN_FUNCTION{
                        m_remapper.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Reset buffers when the DSP chain starts.",
                       MIN_FUNCTION{
                           m_remapper.reset();
                           return {};
                       }};

    void operator()(audio_bundle input, audio_bundle output) {
        m_remapper.process(input.samples(0), output.samples(0), input.frame_count());
    }
};

MIN_EXTERNAL(spectra);
