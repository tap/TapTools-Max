/// @file
/// @brief      tap.fft.list~ — convert an FFT frame from a signal stream to a list.
/// @details    Collects the spectral values arriving on the signal inlet (indexed by the bin-index
///             inlet) into a frame and outputs them as a list once per frame. Intended to live inside
///             a pfft~ subpatcher. Faithful port — the frame is gathered on the audio thread and the
///             list is emitted on the main thread via a queue.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include <algorithm>
#include <array>

#include "c74_min.h"

using namespace c74::min;

class fft_list : public object<fft_list>, public vector_operator<> {
  public:
    MIN_DESCRIPTION{"Convert spectral data from a signal stream into a list, frame by frame. Use "
                    "inside pfft~ with the FFT signal and its bin index."};
    MIN_TAGS{"fft"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"pfft~, fftin~, tap.fft.normalize~, zl"};

    inlet<>  m_in_data{this, "(signal) spectral data from the FFT"};
    inlet<>  m_in_index{this, "(signal) FFT bin index"};
    outlet<> m_out{this, "(list) the spectral frame as a list"};
    outlet<> m_dump{this, "(anything) dumpout"};

    argument<int> bins_arg{this, "fftsize", "FFT size (number of bins).",
                           MIN_ARGUMENT_FUNCTION{m_bins = std::clamp(static_cast<int>(arg), 2, k_max_size);
}
}
;

attribute<number> mult{this, "mult", 1.0, description{"Scaling factor applied to each value."}};

attribute<bool> nyquist{this, "nyquist", true,
                        description{"Truncate the output list at the Nyquist bin (output fftsize/2 values)."}};

attribute<bool> autopoll{this, "autopoll", true,
                         description{"Automatically output the list every time a frame completes."}};

message<> bang{this, "bang", "Output the current frame as a list.", MIN_FUNCTION{emit();
return {};
}
}
;

void operator()(audio_bundle input, audio_bundle output) override {
    const long    n        = input.frame_count();
    const double* in       = input.samples(0);
    const double* idx      = input.samples(1);
    const int     last     = m_bins - 1;
    const double  m        = mult;
    bool          complete = false;

    for (long k = 0; k < n; ++k) {
        const int bin = static_cast<int>(idx[k] + 0.49);
        if (bin >= 0 && bin < k_max_size) {
            m_compiled[bin] = in[k] * m;
        }
        if (bin == last) {
            complete = true;
        }
    }

    if (complete && autopoll) {
        m_drain.set();
    }
}

private:
static constexpr int k_max_size{4096};

std::array<double, k_max_size> m_compiled{};
int                            m_bins{512};

queue<> m_drain{this, MIN_FUNCTION{emit();
return {};
}
}
;

void emit() {
    const int count = nyquist ? (m_bins / 2) : m_bins;
    atoms     out;
    out.reserve(count);
    for (int i = 0; i < count; ++i) {
        out.push_back(m_compiled[i]);
    }
    m_out.send(out);
}
}
;

MIN_EXTERNAL(fft_list);
