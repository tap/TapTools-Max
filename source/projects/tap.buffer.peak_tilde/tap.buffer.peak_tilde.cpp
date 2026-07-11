/// @file
/// @brief      tap.buffer.peak~ — report the index of the hottest sample in a buffer~.
/// @details    On bang, scans the bound buffer~ for the sample with the greatest absolute value and
///             outputs its frame index. Faithful port of the original, rebuilt on Min's
///             buffer_reference (which handles binding, the `set` message, and notifications).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2004-2026 Timothy Place.

#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class buffer_peak : public object<buffer_peak> {
  public:
    MIN_DESCRIPTION{"Find the hottest sample in a buffer~. On bang, outputs the frame index of the "
                    "sample with the greatest absolute value."};
    MIN_TAGS{"buffers"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.buffer.snap~, buffer~, peek~, index~"};

    inlet<>          m_in{this, "(bang) find the peak; (set <name>) bind a buffer~"};
    outlet<>         m_out{this, "(int) frame index of the peak sample"};
    outlet<>         m_dump{this, "(anything) dumpout"};
    buffer_reference m_buffer{this}; // adds the 'set' and 'dblclick' messages automatically

    argument<symbol> name_arg{this, "buffer", "Name of the buffer~ to analyze.",
                              MIN_ARGUMENT_FUNCTION{ m_buffer.set(arg); }};

    message<> bang{this, "bang", "Find the peak sample and output its index.",
                   MIN_FUNCTION{
                       buffer_lock<false> b(m_buffer);
                       if (!b.valid()) {
                           cerr << "no valid buffer~ is bound to this object" << endl;
                           return {};
                       }

                       long         peak_index = 0;
                       double       peak       = 0.0;
                       const size_t frames     = b.frame_count();
                       const size_t channels   = b.channel_count();

                       for (size_t frame = 0; frame < frames; ++frame) {
                           for (size_t channel = 0; channel < channels; ++channel) {
                               const double v = std::abs(static_cast<double>(b.lookup(frame, channel)));
                               if (v > peak) {
                                   peak       = v;
                                   peak_index = static_cast<long>(frame);
                               }
                           }
                       }

                       m_out.send(peak_index);
                       return {};
                   }};
};

MIN_EXTERNAL(buffer_peak);
