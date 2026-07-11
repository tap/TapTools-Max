/// @file
/// @brief      tap.sift~ — sift specific sample values out of a signal.
/// @details    Removes samples equal to the sift value. Two modes, chosen by a creation argument:
///                 mode 0 (default): the surviving, *changed* sample values are emitted as floats
///                                   out a control outlet (a sample-stream → control-value extractor)
///                 mode 1:           the signal passes through, but any sample equal to the sift
///                                   value is replaced by the previously surviving value (a
///                                   sample-and-hold that skips the sift value)
///             Because the two modes need different outlet types, the single outlet is created to
///             match the mode at instantiation. DSP is plain portable C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include <atomic>
#include <memory>

#include "c74_min.h"

using namespace c74::min;

class sift : public object<sift>, public vector_operator<> {
  public:
    MIN_DESCRIPTION{"Sift sample values out of a signal. Removes samples equal to the sift value; "
                    "in the default mode the surviving changed values are output as floats, and in "
                    "mode 1 the signal passes through with sifted samples replaced by the held value."};
    MIN_TAGS{"analysis"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.change, change, edge~, snapshot~"};

    inlet<> m_in{this, "(signal) signal to be sifted"};

    attribute<number> value{this, "value", 0.0, description{"The sample value to sift out (remove)."}};

    argument<number> value_arg{this, "value", "The sample value to sift out (same as the value attribute).",
                               MIN_ARGUMENT_FUNCTION{ value = arg; }};

    attribute<bool> high_priority{
        this, "high_priority", true,
        description{"Deliver the float output from the high-priority scheduler thread (true, the "
                    "default — matching the original object) or defer it to the main thread "
                    "(false). Only affects the default float-output mode."}};

    argument<int> mode_arg{this, "mode", "Output mode: 0 = float output (default), 1 = signal output."};

    sift(const atoms& args = {}) {
        if (args.size() > 1 && static_cast<int>(args[1]) != 0) {
            m_signal_mode = true;
            m_out         = std::make_unique<outlet<>>(this, "(signal) sifted samples", "signal");
        }
        else {
            m_signal_mode = false;
            m_out         = std::make_unique<outlet<>>(this, "(float) surviving sample values");
        }
    }

    void operator()(audio_bundle input, audio_bundle output) override {
        const double* in         = input.samples(0);
        const long    n          = input.frame_count();
        const double  sift_value = value;

        if (m_signal_mode) {
            if (output.channel_count() < 1) {
                return;
            }
            double* out = output.samples(0);
            for (long i = 0; i < n; ++i) {
                const double v = in[i];
                if (v != sift_value) {
                    m_hold = v;
                }
                out[i] = m_hold;
            }
        }
        else {
            bool changed = false;
            for (long i = 0; i < n; ++i) {
                const double v = in[i];
                if (v != sift_value && v != m_hold) {
                    push(v);
                    changed = true;
                }
                m_hold = v;
            }
            if (changed) {
                m_deliver.delay(0);
            }
        }
    }

  private:
    static constexpr long k_qmax{256};

    bool                      m_signal_mode{false};
    double                    m_hold{0.0};
    std::unique_ptr<outlet<>> m_out;

    // Single-producer (audio thread) / single-consumer (main thread) ring buffer for the float dump.
    double            m_ring[k_qmax]{};
    std::atomic<long> m_qhead{0};
    std::atomic<long> m_qtail{0};

    // The timer fires on the high-priority scheduler thread. When high_priority is true we drain
    // there directly (faithful to the original's clock-based delivery); otherwise we hand off to the
    // main-thread queue. The ring is single-producer (audio) / single-consumer (whichever drains).
    timer<> m_deliver{this,
                      MIN_FUNCTION{
                          if (high_priority) {
                              drain();
                          }
                          else {
                              m_drain.set();
                          }
                          return {};
                      }};

    queue<> m_drain{this,
                    MIN_FUNCTION{
                        drain();
                        return {};
                    }};

    void push(double v) {
        const long h    = m_qhead.load(std::memory_order_relaxed);
        const long next = (h + 1) % k_qmax;
        if (next == m_qtail.load(std::memory_order_acquire)) {
            return; // ring full — drop, as the original would once it wrapped
        }
        m_ring[h] = v;
        m_qhead.store(next, std::memory_order_release);
    }

    void drain() {
        long       t = m_qtail.load(std::memory_order_relaxed);
        const long h = m_qhead.load(std::memory_order_acquire);
        while (t != h) {
            m_out->send(m_ring[t]);
            t = (t + 1) % k_qmax;
        }
        m_qtail.store(t, std::memory_order_release);
    }
};

MIN_EXTERNAL(sift);
