/// @file
/// @brief      tap.buffer.snap~ — snap a time value to the nearest zero crossing in a buffer~.
/// @details    Given a position (in ms or samples), searches outward in the bound buffer~ for the
///             nearest zero crossing and returns that position. Works at signal rate (left/signal
///             outlet) and for individual floats (right/float outlet). Faithful port of the original
///             — rebuilt on Min's buffer_reference; the raw interleaved sample access mirrors the
///             original exactly, including its "+1/+2" round-off hacks.
/// @note       The original's loop relied on a range check made *after* clamping the search indices,
///             which could never terminate the search on an all-same-sign buffer; this port checks
///             the unclamped indices so the search always terminates.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2004-2026 Timothy Place.

#include <algorithm>
#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class buffer_snap : public object<buffer_snap>, public vector_operator<> {
  public:
    MIN_DESCRIPTION{"Snap a time value to the nearest zero crossing in a buffer~. Accepts a "
                    "position in ms or samples and returns the nearest zero-crossing position."};
    MIN_TAGS{"buffers"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.buffer.peak~, buffer~, peek~"};

    inlet<>          m_in{this, "(signal/float) position to snap"};
    outlet<>         m_out_sig{this, "(signal) snapped position", "signal"};
    outlet<>         m_out_float{this, "(float) snapped position (for float input)"};
    outlet<>         m_dump{this, "(anything) dumpout"};
    buffer_reference m_buffer{this}; // adds the 'set' and 'dblclick' messages automatically

    argument<symbol> name_arg{this, "buffer", "Name of the buffer~ to search.", MIN_ARGUMENT_FUNCTION{m_buffer.set(arg);
}
}
;

attribute<symbol> mode{this, "mode", "ms", range{"ms", "samps"},
                       description{"Whether the input/output positions are in milliseconds or samples."}};

attribute<int> channel{this, "channel", 1, description{"Which buffer~ channel to search (1-based)."}};

message<> m_number{this, "number", "Snap a single position and output it as a float.",
                   MIN_FUNCTION{buffer_lock<false> b(m_buffer);
if (!b.valid()) {
    cerr << "invalid buffer" << endl;
    return {};
}
m_out_float.send(snap_calc(b, static_cast<double>(args[0])));
return {};
}
}
;

message<> dspsetup{this, "dspsetup", "Update sample-rate conversions.", MIN_FUNCTION{update_rates();
return {};
}
}
;

void operator()(audio_bundle input, audio_bundle output) override {
    const long    n   = input.frame_count();
    const double* in  = input.samples(0);
    double*       out = output.samples(0);

    buffer_lock<true> b(m_buffer);
    if (!b.valid()) {
        for (long i = 0; i < n; ++i) {
            out[i] = 0.0;
        }
        return;
    }
    for (long i = 0; i < n; ++i) {
        out[i] = snap_calc(b, in[i]);
    }
}

private:
double m_srx{44.1};       // sr/1000   (ms -> samples)
double m_sry{1.0 / 44.1}; // 1000/sr   (samples -> ms)

void update_rates() {
    const double sr = samplerate();
    m_srx           = sr * 0.001;
    m_sry           = (1.0 / sr) * 1000.0;
}

template <bool AT>
double snap_calc(buffer_lock<AT>& b, double value) {
    const long frames = static_cast<long>(b.frame_count());
    const long nc     = static_cast<long>(b.channel_count());
    const long chan   = static_cast<int>(channel) - 1;

    if (chan < 0 || (chan + 1) > nc || frames == 0) {
        return value;
    }

    const bool ms = (mode == symbol("ms"));
    if (ms) {
        value *= m_srx;
    }

    long snap  = std::clamp(static_cast<long>(std::lround(value)), 0L, frames - 1);
    long index = snap * nc + chan;

    const double current = b[index];
    if (current != 0.0) {
        const bool above   = (current > 0.0);
        const long max_idx = frames * nc - 1;
        long       i       = 0;
        int        flag    = 0;

        while (!flag) {
            i += nc;
            const long raw1   = index + i;
            const long raw2   = index - i;
            const long index1 = std::clamp(raw1, 0L, max_idx);
            const long index2 = std::clamp(raw2, 0L, max_idx);

            if (above != (b[index1] > 0.0)) {
                flag = 1;
            }
            else if (above != (b[index2] > 0.0)) {
                flag = 2;
            }

            if (raw1 > max_idx && raw2 < 0) { // unclamped check so the search always terminates
                flag = 3;
            }

            if (flag == 1) {
                snap = index1 + 1; // +1 hack, preserved from the original
            }
            else if (flag == 2) {
                snap = index2 + 2; // +2 hack, preserved from the original
            }
        }
    }

    snap = (snap - chan) / nc; // interleaved index back to a frame index
    return ms ? (snap * m_sry) : static_cast<double>(snap);
}
}
;

MIN_EXTERNAL(buffer_snap);
