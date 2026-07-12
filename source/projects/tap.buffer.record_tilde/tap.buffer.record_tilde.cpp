/// @file
/// @brief      tap.buffer.record~ — record a signal into a buffer~ with smooth fade writes.
/// @details    Records the incoming signal into the bound buffer~ while spreading each written value
///             across a short fade window, which smooths the record head and avoids clicks. Outputs a
///             normalized sync ramp (record position / buffer length). Faithful port of the original
///             ttblue-based recorder — rebuilt on Min's buffer_reference.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2004-2026 Timothy Place.

#include <algorithm>
#include <array>

#include "c74_min.h"

using namespace c74::min;

class buffer_record : public object<buffer_record>, public vector_operator<> {
  public:
    MIN_DESCRIPTION{"Record a signal into a buffer~ with smooth fade writes. Each sample is blended "
                    "across a short fade window to avoid clicks; outputs a normalized sync ramp."};
    MIN_TAGS{"buffers"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.buffer.snap~, record~, poke~, buffer~"};

    inlet<>          m_in{this, "(signal) audio to record; int toggles recording"};
    outlet<>         m_out{this, "(signal) normalized sync output (record position)", "signal"};
    outlet<>         m_dump{this, "(anything) dumpout"};
    buffer_reference m_buffer{this}; // adds the 'set' and 'dblclick' messages automatically

    argument<symbol> name_arg{this, "buffer", "Name of the buffer~ to record into.",
                              MIN_ARGUMENT_FUNCTION{ m_buffer.set(arg); }};
    argument<int> channel_arg{this, "channel", "Buffer~ channel to record into (1-based).",
                              MIN_ARGUMENT_FUNCTION{ channel = static_cast<int>(arg); }};
    argument<int> fade_arg{this, "fade", "Fade length in samples.",
                           MIN_ARGUMENT_FUNCTION{ fade = static_cast<int>(arg); }};

    attribute<bool> loop{this, "loop", false,
                         description{"Loop back to the start of the buffer when the end is reached."}};

    attribute<int> fade{this, "fade", 5, setter{MIN_FUNCTION{
                            return {std::clamp(static_cast<int>(args[0]), 1, k_max_fade - 1)};
                        }},
                        description{"Length of the write fade window, in samples (1-255)."}};

    attribute<int> channel{this, "channel", 1, description{"Buffer~ channel to record into (1-based)."}};

    message<> number{this, "number", "A non-zero int starts recording; 0 stops and rewinds.",
                     MIN_FUNCTION{
                         m_record_toggle = static_cast<int>(args[0]);
                         if (m_record_toggle == 0) {
                             m_loc = 0;
                         }
                         return {};
                     }};

    message<> dspsetup{this, "dspsetup", "Clear the fade memory when the DSP chain starts.",
                       MIN_FUNCTION{
                           m_increment_steps.fill(0.0);
                           m_loc = 0;
                           return {};
                       }};

    void operator()(audio_bundle input, audio_bundle output) override {
        const long    n   = input.frame_count();
        const double* in  = input.samples(0);
        double*       out = output.samples(0);

        buffer_lock<true> b(m_buffer);
        if (!b.valid() || m_record_toggle == 0) {
            for (long i = 0; i < n; ++i) {
                out[i] = 0.0;
            }
            return;
        }

        const long   frames      = static_cast<long>(b.frame_count());
        const long   nc          = static_cast<long>(b.channel_count());
        const long   chan        = static_cast<int>(channel) - 1;
        const long   num_samples = frames * nc;
        const int    f           = std::clamp(static_cast<int>(fade), 1, k_max_fade - 1);
        const double frames_inv  = (frames > 0) ? 1.0 / static_cast<double>(frames) : 0.0;
        const double fade_inv    = 1.0 / static_cast<double>(f);

        if (frames == 0 || chan < 0 || chan >= nc) {
            for (long i = 0; i < n; ++i) {
                out[i] = 0.0;
            }
            return;
        }

        long frame_index = m_loc;
        auto inc         = m_increment_steps; // local copy (mirrors the original)

        long i = 0;
        for (; i < n; ++i) {
            const double xx = in[i];

            if (frame_index < 0) {
                frame_index = 0;
            }
            else if (frame_index >= frames && loop) {
                frame_index = 0;
            }
            else if (frame_index >= frames && !loop) {
                m_record_toggle = 0;
                frame_index     = 0;
                out[i]          = 0.0;
                ++i;
                break;
            }

            const long sample_index = (nc > 1) ? frame_index * nc + chan : frame_index;

            // 1. add the oldest step into the buffer, fade samples behind the head
            long iw = onewrap(frame_index - f, 0L, frames - 1);
            if (nc > 1) {
                iw = iw * nc + chan;
            }
            b[iw] += inc[f - 1];

            // 2. shift the stored steps along
            for (int k = f - 1; k > 0; --k) {
                inc[k] = inc[k - 1];
            }

            // 3. compute the new step from the current buffer value toward the input
            inc[0] = (xx - b[std::clamp(sample_index, 0L, num_samples - 1)]) * fade_inv;

            // 4. apply all of the in-flight steps across the fade window
            for (int k = 0; k < f - 1; ++k) {
                long w = onewrap(frame_index - k, 0L, frames - 1);
                if (nc > 1) {
                    w = w * nc + chan;
                }
                b[w] += inc[k];
            }

            out[i] = static_cast<double>(frame_index++) * frames_inv;
        }

        for (; i < n; ++i) { // zero any remaining samples if we stopped mid-vector
            out[i] = 0.0;
        }

        m_increment_steps = inc;
        m_loc             = frame_index;
        b.dirty();
    }

  private:
    static constexpr int k_max_fade{256};

    std::array<double, k_max_fade> m_increment_steps{};
    long                           m_loc{0};
    int                            m_record_toggle{0};

    // Wrap a value into [low, high] assuming it is at most one range outside the bounds.
    template <class T>
    static T onewrap(T value, T low_bound, T high_bound) {
        if (value >= low_bound && value < high_bound) {
            return value;
        }
        if (value >= high_bound) {
            return (low_bound - 1) + (value - high_bound);
        }
        return (high_bound + 1) - (low_bound - value);
    }
};

MIN_EXTERNAL(buffer_record);
