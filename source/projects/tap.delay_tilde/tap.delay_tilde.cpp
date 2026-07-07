/// @file
/// @brief      tap.delay~ — a simple sample-accurate audio delay line.
/// @details    A single-channel audio delay. The input signal is written into a circular buffer and
///             read back a number of milliseconds later. The delay time (ms) is set via the right
///             inlet or the @delay attribute. The buffer is sized at instantiation by the optional
///             buffersize argument (ms), which also bounds the maximum usable delay.
///
///             This object was RECONSTRUCTED from its surviving maxref documentation
///             (docs/tap.delay~.maxref.xml) and help/abstraction patchers — the original C++ source
///             was lost. The documented behavior is intentionally minimal ("A simple audio delay"),
///             so the implementation mirrors the established circular-buffer idiom used by
///             tap.multitap~. DSP is plain portable C++; Min is used only for the Max plumbing. For
///             multichannel use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 1999-2026 Timothy Place.

#include <algorithm>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class delay : public object<delay>, public sample_operator<2, 1> {
  public:
    MIN_DESCRIPTION{"A simple sample-accurate audio delay. The input signal is delayed by a number "
                    "of milliseconds set via the right inlet or the @delay attribute. The optional "
                    "argument sets the size (ms) of the delay buffer, which bounds the maximum "
                    "delay. For multichannel signals, wrap this object in an mc. operator."};
    MIN_TAGS{"delays"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"delay~, tapin~, tapout~, tap.multitap~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_dt{this, "(signal/float) delay time in milliseconds"};
    outlet<> m_out{this, "(signal) delayed audio output", "signal"};

    attribute<number> buffersize{
        this, "buffersize", 1000.0,
        description{"Size of the delay buffer in milliseconds (fixed at instantiation). This sets "
                    "the maximum possible delay time."}};

    attribute<number> delaytime{this, "delay", 0.0,
                                setter{MIN_FUNCTION{m_delay_ms = std::max(0.0, static_cast<double>(args[0]));
    update_delay();
    return {m_delay_ms};
}
}
, description {
    "Delay time in milliseconds. Clamped to the buffer size."
}
}
;

message<> clear{this, "clear", "Clear the contents of the delay buffer.",
                MIN_FUNCTION{std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
return {};
}
}
;

message<> dspsetup{this, "dspsetup", "Allocate and recompute when the DSP chain starts.", MIN_FUNCTION{allocate();
update_delay();
return {};
}
}
;

delay(const atoms& args = {}) {
    if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
        buffersize = args[0];
    }
    allocate();
}

sample operator()(sample x, sample dt) {
    const long N = static_cast<long>(m_buffer.size());
    if (N < 1) {
        return 0.0;
    }

    // A non-zero delay-time signal on the right inlet overrides the attribute value
    // sample-by-sample (mirroring delay~). 0.0 means "use the attribute".
    long delay_samples = m_delay_samples;
    if (dt != 0.0) {
        const double sr = samplerate();
        delay_samples   = std::clamp(static_cast<long>(static_cast<double>(dt) * (sr * 0.001)), 0L, N - 1);
    }

    m_buffer[m_write] = x;

    long read = m_write - delay_samples;
    read %= N;
    if (read < 0) {
        read += N;
    }
    const sample out = m_buffer[read];

    if (++m_write >= N) {
        m_write = 0;
    }

    return out;
}

private:
std::vector<double> m_buffer;
long                m_write{0};
double              m_delay_ms{0.0};
long                m_delay_samples{0};

void allocate() {
    const double sr = samplerate();
    long         n  = static_cast<long>(static_cast<double>(buffersize) * (sr * 0.001));
    if (n < 1) {
        n = 1;
    }
    if (static_cast<long>(m_buffer.size()) != n) {
        m_buffer.assign(n, 0.0);
        m_write = 0;
    }
}

void update_delay() {
    const double sr = samplerate();
    const long   N  = std::max(1L, static_cast<long>(m_buffer.size()));
    m_delay_samples = std::clamp(static_cast<long>(m_delay_ms * (sr * 0.001)), 0L, N - 1);
}
}
;

MIN_EXTERNAL(delay);
