/// @file
/// @brief      tap.fft.normalize~ — normalize an FFT frame (for use inside pfft~).
/// @details    Scales the real and imaginary parts of an FFT frame by 1/(fftsize/2), negates the
///             imaginary part, and halves the DC and Nyquist bins. Intended to live inside a pfft~
///             subpatcher: inlets are real, imaginary, and the bin index; outlets are the normalized
///             real and imaginary parts. The FFT size is given as the argument.
/// @note       The original compared the (0.49-biased) bin index for equality, so its DC/Nyquist
///             halving never actually fired; this port rounds the index and applies the intended
///             halving.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include <algorithm>

#include "c74_min.h"

using namespace c74::min;

class fft_normalize : public object<fft_normalize>, public vector_operator<> {
  public:
    MIN_DESCRIPTION{"Normalize an FFT frame inside pfft~. Scales by 1/(fftsize/2), negates the "
                    "imaginary part, and halves the DC and Nyquist bins."};
    MIN_TAGS{"fft"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"pfft~, fftin~, fftout~, cartopol~"};

    inlet<>  m_in_real{this, "(signal) real part of the FFT frame"};
    inlet<>  m_in_imag{this, "(signal) imaginary part of the FFT frame"};
    inlet<>  m_in_index{this, "(signal) FFT bin index"};
    outlet<> m_out_real{this, "(signal) normalized real part", "signal"};
    outlet<> m_out_imag{this, "(signal) normalized imaginary part", "signal"};

    argument<int> bins_arg{this, "fftsize", "FFT size (number of points).",
                           MIN_ARGUMENT_FUNCTION{
                               m_bins  = std::max(2, static_cast<int>(arg));
                               m_bins2 = m_bins / 2;
                           }};

    void operator()(audio_bundle input, audio_bundle output) override {
        const long    n   = input.frame_count();
        const double* re  = input.samples(0);
        const double* im  = input.samples(1);
        const double* idx = input.samples(2);
        double*       ore = output.samples(0);
        double*       oim = output.samples(1);

        const double scale = (m_bins2 > 0) ? 1.0 / static_cast<double>(m_bins2) : 0.0;
        const int    last  = m_bins - 1;

        for (long k = 0; k < n; ++k) {
            double val  = re[k] * scale;
            double val2 = -im[k] * scale;

            const int bin = static_cast<int>(idx[k] + 0.49); // round to the nearest bin
            if (bin == 0 || bin == last) {
                val *= 0.5;
            }

            ore[k] = val;
            oim[k] = val2;
        }
    }

  private:
    int m_bins{512};
    int m_bins2{256};
};

MIN_EXTERNAL(fft_normalize);
