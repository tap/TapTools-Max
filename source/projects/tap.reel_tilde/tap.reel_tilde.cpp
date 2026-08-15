/// @file
/// @brief      tap.reel~ — one free-running tape loop: the lane tap.airport~ is built from.
/// @details    A single reel wrapping tap::tools::airport::loop (taptools/airport.h): one head
///             that both plays and records, a length, a level, an equal-power pan, and a playback
///             darken corner. `tap.airport~` is up to eight of these summed and nothing more, so
///             seven `tap.reel~` into a stereo sum ARE the airport — patched, where you can see
///             the lengths and reach into any single loop. That identity is pinned bitwise in the
///             kernel's own test suite, not merely asserted here.
///
///             What you get by patching it rather than using the monolith: an insert on ONE loop
///             (a filter, a reverse, `tap.discreet~` for tape breath on just that phrase), a
///             varispeed on one reel, more than eight loops, and per-reel tape sizing — the bank
///             buys all eight worst-case reels at DSP start whether you use them or not, while N
///             of these buy exactly N.
///
///             The discipline is the same one loop deep: NOTHING resets the phase — not `record`,
///             not a setter, not a length change (an honest splice: the head re-wraps modulo the
///             new length, never rewinds), not a DSP chain restart (the wrapper re-prepares, which
///             erases the tape and rewinds, only when the sample rate or the tape buy actually
///             changes). `record 1` punches the input onto the tape at wherever the head happens
///             to be, replacing — no overdub; `record 0` freezes it bit-exactly.
///
///             The optional argument sets the worst-case tape buy in seconds (default 30 — ~11 MB
///             of double tape at 48 kHz for this one reel; size it to the piece).
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/airport.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::airport;

class reel : public object<reel>, public sample_operator<1, 2> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::loop m_loop;
    bool         m_geometry_dirty{false};

  public:
    MIN_DESCRIPTION{"One free-running tape loop — the lane tap.airport~ is made of. A single head "
                    "both plays and records, and nothing ever resets its phase: not record, not a "
                    "setter, not a length change (a splice — the head re-wraps, never rewinds). "
                    "record 1 punches the input onto the tape at wherever the head is; record 0 "
                    "freezes it bit-exactly. Sum several of these, each at an awkward length, and "
                    "you have the Music for Airports system with every loop reachable — patch an "
                    "insert into one of them, or run one through tap.discreet~ for tape breath. "
                    "The optional argument sets the maximum loop length in seconds."};
    MIN_TAGS{"delays"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.airport~, tap.discreet~, tap.multitap~, groove~"};

    inlet<>  m_in{this, "(signal) audio input — punched onto the tape while recording"};
    outlet<> m_out_left{this, "(signal) playback, left", "signal"};
    outlet<> m_out_right{this, "(signal) playback, right", "signal"};
    outlet<> m_out_info{this, "(list) reports: phase <0..1>, length <seconds>, samples <n>"};

    attribute<number> maxloop{this, "maxloop", kernel::k_default_max_seconds,
                              setter{MIN_FUNCTION{
                                  const double v   = std::max(kernel::k_min_loop_seconds, static_cast<double>(args[0]));
                                  m_geometry_dirty = true; // re-bought (tape erased) when the DSP chain starts
                                  return {v};
                              }},
                              description{"Worst-case loop length in seconds (set by the first object argument; the "
                                          "reel is bought at DSP start — 30 s is ~11 MB of tape at 48 kHz). Changing "
                                          "it erases the tape when the DSP chain restarts."}};

    attribute<number> length{this, "length", 1.0, setter{MIN_FUNCTION{
                                 m_loop.set_length_seconds(args[0]);
                                 return {m_loop.length_seconds()}; // report what the sample grid actually allows
                             }},
                             description{"Loop length in seconds (0.5 up to maxloop). Across several reels the "
                                         "lengths are the score: keep them incommensurate and the coincidences "
                                         "never repeat. A change is a splice — the head re-wraps and never "
                                         "rewinds. It can click; splices do."}};

    attribute<number> level{this, "level", 1.0, setter{MIN_FUNCTION{
                                m_loop.set_level(args[0]);
                                return args;
                            }},
                            description{"Linear playback level, slewed. Unclamped (negative flips polarity)."}};

    attribute<number> pan{this, "pan", 0.0, setter{MIN_FUNCTION{
                              const double v = std::clamp(static_cast<double>(args[0]), -1.0, 1.0);
                              m_loop.set_pan(v);
                              return {v};
                          }},
                          description{"Equal-power pan, -1 (hard left) to 1 (hard right), slewed. Endpoints are "
                                      "exact: a hard-panned reel is bitwise absent from the far bus."}};

    attribute<number> darken{
        this, "darken", tap::tools::tape::k_darken_ceil_hz, setter{MIN_FUNCTION{
            const double v = std::clamp(static_cast<double>(args[0]), tap::tools::tape::k_darken_floor_hz,
                                        tap::tools::tape::k_darken_ceil_hz);
            m_loop.set_darken_hz(v);
            return {v};
        }},
        description{"Playback darkening corner in Hz. A static tone, not generation loss (a "
                    "frozen loop replays the same imprint every pass); at 20000 (the default) "
                    "the stage is bypassed and playback is bit-transparent."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_loop.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for level/pan/darken moves, in ms (0 = instant)."}};

    message<> record{this, "record",
                     "record <0|1>: punch the input onto the tape at wherever the head happens to be (1), or "
                     "freeze the tape bit-exactly (0). Recording replaces — no overdub.",
                     MIN_FUNCTION{
                         if (!args.empty()) {
                             m_loop.record(static_cast<int>(args[0]) != 0);
                         }
                         return {};
                     }};

    message<> phase{this, "phase",
                    "Report the head position as a fraction of the loop length (0..1) out the right outlet.",
                    MIN_FUNCTION{
                        m_out_info.send("phase", m_loop.phase());
                        return {};
                    }};

    // Not named `samples`: that would shadow Min's samples<2> return type inside this class.
    message<> loopsamples{this, "loopsamples",
                          "Report the loop length in samples, and in seconds as quantized to the sample grid, "
                          "out the right outlet — what tap.period needs to work out when a set of reels "
                          "realigns.",
                          MIN_FUNCTION{
                              m_out_info.send("samples", static_cast<int>(m_loop.loop_samples()));
                              m_out_info.send("length", m_loop.length_seconds());
                              return {};
                          }};

    message<> clear{this, "clear", "Erase the tape and rewind the head. Parameters are kept.",
                    MIN_FUNCTION{
                        m_loop.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup",
                       "Re-prepare only when the sample rate or the tape buy changed — a DSP chain restart must "
                       "not erase a running loop.",
                       MIN_FUNCTION{
                           if (m_geometry_dirty || !m_loop.prepared() || samplerate() != m_loop.samplerate()) {
                               m_loop.prepare(samplerate(), maxloop);
                               m_geometry_dirty = false;
                           }
                           return {};
                       }};

    reel(const atoms& args = {}) {
        if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
            maxloop = args[0];
        }
        m_loop.prepare(samplerate(), maxloop);
        m_geometry_dirty = false;
    }

    samples<2> operator()(sample x) {
        double left  = 0.0;
        double right = 0.0;
        m_loop.process(x, left, right); // the kernel accumulates, so start from zero
        return {left, right};
    }
};

MIN_EXTERNAL(reel);
