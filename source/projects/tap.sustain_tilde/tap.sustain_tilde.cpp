/// @file
/// @brief      tap.sustain~ — capture recent audio into seamless sustaining loops.
/// @details    Continuously records the input into a ring buffer. On a bang, the most recently
///             recorded material is captured into a voice: trimmed to significant-power rising
///             zero-crossings at both ends, then played back as a crossfaded forward/backward loop
///             so it sustains without clicks. A bank of voices is round-robined so multiple
///             captures overlap and sum to the output, each with its own rise (fade-in) envelope.
///             DSP is plain portable C++.
///
///             Multi-voice + per-voice rise are now IMPLEMENTED (previously this was a single-voice
///             prototype ported from the 2019 "taptools-min" effort — originally Cycling '74, MIT).
///             The original multi-voice/rise design was not recoverable from the archive (the
///             redesign patchers referenced in REVIVAL.md §8 are not present in the branch tree), so
///             the polyphony and rise model below is a clean, documented reconstruction:
///
///               * Voice allocation is ROUND-ROBIN over `voices` slots (default 5). Each bang
///                 advances to the next slot and overwrites it. With N voices you get N overlapping
///                 sustaining loops before the oldest is recycled — i.e. round-robin doubles as an
///                 oldest-first voice-stealing policy, which is the natural fit for "freeze the last
///                 sound and keep stacking" usage.
///               * RISE is a one-shot equal-power fade-in applied per voice when it starts sustaining,
///                 ramping the freshly captured loop up to full power over `rise` milliseconds. It is
///                 counted independently per voice and applied only on the first playthrough, so a
///                 newly stacked voice swells in under the existing ones rather than clicking in.
///
///             Brought onto the modern toolchain (C++20 `std::numbers` instead of MSVC-fragile
///             `M_PI`). Still wants runtime audition in Max for feel (rise/fade timing, the
///             zero-crossing trim, and how stacked voices balance) — see REVIVAL.md §8.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2019-2026 Timothy Place.

#include <cmath>

#include "c74_min.h"

// The object proper is C++20 and uses std::numbers (MSVC-safe, vs the fragile M_PI). The unit-test
// harness, however, force-pins this translation unit to C++17, so provide a portable pi that resolves
// to std::numbers::pi_v when the C++20 header is available and a literal otherwise.
#if __has_include(<numbers>) && defined(__cpp_lib_math_constants)
#include <numbers>
#define TAP_SUSTAIN_PI (std::numbers::pi_v<sample>)
#else
#define TAP_SUSTAIN_PI (static_cast<sample>(3.14159265358979323846))
#endif

using namespace c74::min;

// A single sustaining voice: holds a captured snippet and plays it as a crossfaded forward/backward
// loop, with an independent one-shot rise (fade-in) envelope when it starts. Each voice owns its own
// capture buffer, loop position, crossfade, and rise/fade envelope so multiple voices overlap freely.
class sustain_voice {
  public:
    void resize(int newsize) { m_buffer.resize(newsize); }

    // Capture the `m_buffer.size()` samples immediately before `record_head` from the master input,
    // trimming each end to a rising zero-crossing whose level exceeds the snippet's RMS. Arms the
    // voice (resets the loop position and the rise envelope) so it begins sustaining from full silence.
    void capture(vector<sample>& input, int record_head) {
        const int start_initial = record_head - static_cast<int>(m_buffer.size());
        const int end_initial   = record_head - 1;
        int       start         = start_initial;
        int       end           = end_initial;

        // average power of the captured region
        number rms{};
        for (auto i = start; i < end; ++i) {
            auto j = i % input.size();
            rms += input[j] * input[j];
        }
        rms /= static_cast<number>(input.size());
        rms = std::sqrt(rms);

        // trim to a (rising) zero-crossing of significant power at the beginning
        for (auto i = start + 1; i < end; i++) {
            auto k = i % input.size();
            if (input[k] > 0.0 && input[k - 1] < 0.0) {
                for (auto j = k; j < input.size(); j++) {
                    if (input[j] > input[j - 1]) {
                        if (input[j] >= rms) {
                            start = i;
                            break;
                        }
                    }
                    else {
                        break;
                    }
                }
            }
            if (start != start_initial) {
                break;
            }
        }

        // trim to a (rising) zero-crossing of significant power at the end
        for (auto i = end; i > 0; --i) {
            auto k = i % input.size();
            if (input[k] > 0.0 && input[k - 1] < 0.0) {
                for (auto j = k; j > 0; j--) {
                    if (input[j] < input[j - 1]) {
                        break;
                    }
                    if (input[j] <= -rms) {
                        end = i;
                        break;
                    }
                }
            }
            if (end != end_initial) {
                break;
            }
        }

        m_size = end - start;
        for (auto i = 0; i < m_size; ++i) {
            auto j      = (start + i) % input.size();
            m_buffer[i] = input[j];
        }
        m_frame    = 0;
        m_rise_pos = 0; // arm the one-shot rise fade-in for this fresh capture
    }

    void clear() {
        m_buffer.assign(m_buffer.size(), 0.0);
        m_size     = 0;
        m_frame    = 0;
        m_rise_pos = m_rise_size; // any future read is at full power until re-captured
    }

    void fade(int fadetime_in_samples) {
        m_fade_size      = fadetime_in_samples;
        m_fade_step_size = (m_fade_size > 0) ? 1.0 / m_fade_size : 0.0;
    }
    int fade() const { return m_fade_size; }

    void rise(int risetime_in_samples) {
        m_rise_size      = risetime_in_samples;
        m_rise_step_size = (m_rise_size > 0) ? 1.0 / m_rise_size : 0.0;
    }
    int rise() const { return m_rise_size; }

    // Reachable state for testing: is this voice currently sounding (has captured material)?
    bool active() const { return m_size > 0 && m_fade_size > 0; }
    // Reachable state for testing: how far through the one-shot rise envelope we are.
    int rise_position() const { return m_rise_pos; }

    void operator()(audio_bundle output) {
        if (m_size <= 0 || m_fade_size <= 0) {
            return; // nothing captured yet — stay silent rather than read uninitialized state
        }

        auto      out = output.samples(0);
        const int N   = m_size * 2 - m_fade_size * 2;
        if (N <= 0) {
            return;
        }

        for (auto n = 0; n < output.frame_count(); ++n) {
            sample s{};
            if (m_frame < m_fade_size) {
                sample s1 = m_buffer[m_frame] * equal_power(m_fade_step_size * m_frame);
                int    i  = (m_fade_size - 1) - m_frame;
                sample s2 = m_buffer[i] * equal_power(m_fade_step_size * i);
                s         = s1 + s2;
            }
            else if (m_frame < m_size - m_fade_size) {
                s = m_buffer[m_frame];
            }
            else if (m_frame < m_size) {
                sample s1 = m_buffer[m_frame] * equal_power(m_fade_step_size * (m_size - m_frame));
                int    i  = (N - m_frame) + (m_fade_size - 1);
                sample s2 = m_buffer[i] * equal_power(m_fade_step_size * (m_size - i));
                s         = s1 + s2;
            }
            else {
                int i = (N - m_frame) + (m_fade_size - 1);
                s     = m_buffer[i];
            }

            // one-shot per-voice rise: equal-power fade-in over the first m_rise_size samples
            if (m_rise_pos < m_rise_size) {
                s *= equal_power(m_rise_step_size * m_rise_pos);
                ++m_rise_pos;
            }

            out[n] += s;

            ++m_frame;
            if (m_frame >= N) {
                m_frame = 0;
            }
        }
    }

  private:
    vector<sample> m_buffer;
    int            m_size{0};
    int            m_fade_size{0};
    number         m_fade_step_size{0.0};
    int            m_rise_size{0};
    number         m_rise_step_size{0.0};
    int            m_rise_pos{0}; // position within the one-shot rise fade-in
    int            m_frame{0};

    static sample equal_power(sample position) { return std::sin(position * TAP_SUSTAIN_PI / 2.0); }
};

class sustain : public object<sustain>, public vector_operator<> {
    static constexpr int k_max_voices{5};

    vector<sustain_voice> m_voices;
    vector<sample>        m_buffer;
    int                   m_buffer_sizemask{0};
    int                   m_record_head{0};
    number                m_ms_samplerate{0.0};
    number                m_ms_inv_samplerate{0.0};
    bool                  m_need_to_capture{false};
    int                   m_next_voice{0}; // round-robin allocation cursor

  public:
    MIN_DESCRIPTION{"Capture recent audio into seamless sustaining loops. Continuously records the "
                    "input; on a bang the most recent material is trimmed to zero-crossings and "
                    "played back as a crossfaded loop so it sustains without clicks. A bank of "
                    "voices is round-robined so captures overlap, each rising in over the rise time. "
                    "EXPERIMENTAL — recovered from the 2019 taptools-min effort; multi-voice + rise "
                    "are implemented but still want a Max audition for feel."};
    MIN_TAGS{"buffers"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.buffer.record~, buffer~, record~"};

    inlet<>  input{this, "(signal) audio to record; bang captures a sustaining loop"};
    outlet<> output_signal{this, "(signal) sustaining loop output (all voices summed)", "signal"};

    sustain(const atoms& args = {})
        : m_voices(k_max_voices) {
        update_samplerate();
    }

    message<> bang{this, "bang",
                   "Capture the most recently recorded audio into a sustaining loop "
                   "(allocated round-robin across the voice bank).",
                   MIN_FUNCTION{
                       m_need_to_capture = true;
                       return {};
                   }};

    message<> clear{this, "clear", "Erase all captured loops (all voices).",
                    MIN_FUNCTION{
                        for (auto& voice : m_voices) {
                            voice.clear();
                        }
                        m_next_voice = 0;
                        return {};
                    }};

    attribute<number> length{
        this, "length", 1000.0,
        description{"Maximum length of the captured loop, in milliseconds. (Changing this restarts "
                    "the recording buffer.)"}};

    attribute<int> voices{this,
                          "voices",
                          k_max_voices,
                          description{"Number of overlapping sustaining voices. Each bang captures into the next "
                                      "voice round-robin, so up to this many loops can stack before the oldest is "
                                      "recycled."},
                          range{1, k_max_voices},
                          setter{MIN_FUNCTION{
                              int n = std::clamp(static_cast<int>(args[0]), 1, k_max_voices);
                              if (n < m_next_voice) {
                                  m_next_voice = 0; // keep the round-robin cursor inside the active range
                              }
                              return {n};
                          }}};

    attribute<number> fade{this, "fade", 100.0,
                           description{"Length of the crossfade across the loop points, in milliseconds."},
                           setter{MIN_FUNCTION{
                               number fadetime_in_samples = static_cast<number>(args[0]) * m_ms_samplerate;
                               if (!m_voices.empty() && fadetime_in_samples != m_voices[0].fade()) {
                                   for (auto& voice : m_voices) {
                                       voice.fade(static_cast<int>(fadetime_in_samples));
                                   }
                               }
                               return args;
                           }}};

    attribute<number> rise{
        this, "rise", 100.0,
        description{"Time over which each newly captured voice grows to full power, in milliseconds. "
                    "Applied independently per voice as a one-shot fade-in when it starts sustaining."},
        setter{MIN_FUNCTION{
            number rise_in_samples = static_cast<number>(args[0]) * m_ms_samplerate;
            if (!m_voices.empty() && rise_in_samples != m_voices[0].rise()) {
                for (auto& voice : m_voices) {
                    voice.rise(static_cast<int>(rise_in_samples));
                }
            }
            return args;
        }}};

    message<> dspsetup{this, "dspsetup",
                       MIN_FUNCTION{
                           update_samplerate();
                           return {};
                       }};

    // Reachable state for unit testing.
    const vector<sustain_voice>& voice_bank() const { return m_voices; }
    int                          next_voice() const { return m_next_voice; }

    void operator()(audio_bundle input_bundle, audio_bundle output) override {
        auto in = input_bundle.samples(0);

        // always recording into the master ring buffer
        for (auto n = 0; n < output.frame_count(); ++n) {
            m_buffer[m_record_head] = in[n];
            ++m_record_head;
            m_record_head &= m_buffer_sizemask;
        }

        if (m_need_to_capture) {
            m_need_to_capture = false;
            capture();
        }

        // accumulate all of the active voices
        output.clear();
        const int n = std::min(static_cast<int>(voices), static_cast<int>(m_voices.size()));
        for (auto i = 0; i < n; ++i) {
            m_voices[i](output);
        }
    }

  private:
    void update_samplerate() {
        m_ms_samplerate     = samplerate() * 0.001;
        m_ms_inv_samplerate = (1.0 / samplerate()) * 1000.0;
        resize();
        // re-derive the time-based envelopes for the (possibly new) sample rate
        fade = static_cast<number>(fade);
        rise = static_cast<number>(rise);
    }

    void resize() {
        int  size_in_samples       = static_cast<int>(static_cast<number>(length) * m_ms_samplerate);
        auto size_in_samples_voice = size_in_samples;

        size_in_samples = limit_to_power_of_two(size_in_samples);
        if (size_in_samples != static_cast<int>(m_buffer.size())) {
            m_buffer.resize(size_in_samples);
            m_buffer_sizemask = size_in_samples - 1;
            m_record_head     = 0;
            for (auto& voice : m_voices) {
                voice.resize(size_in_samples_voice);
            }
        }
    }

    // Round-robin voice allocation: each capture advances to the next slot (within the active
    // `voices` count) and overwrites it. Cycling through the bank means the oldest voice is the one
    // recycled — round-robin doubles as an oldest-first voice-stealing policy.
    void capture() {
        const int n = std::min(static_cast<int>(voices), static_cast<int>(m_voices.size()));
        if (n <= 0) {
            return;
        }
        if (m_next_voice >= n) {
            m_next_voice = 0;
        }
        m_voices[m_next_voice].capture(m_buffer, m_record_head);
        m_next_voice = (m_next_voice + 1) % n;
    }
};

MIN_EXTERNAL(sustain);
