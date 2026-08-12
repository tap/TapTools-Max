/// @file
/// @brief      tap.airport~ — the Music for Airports incommensurate loop bank on the portable
///             kernel.
/// @details    Up to eight free-running tape loops of unequal lengths wrapping
///             tap::tools::airport::loop_bank (taptools/airport.h) — the "2/1" system: each loop
///             holds one recorded phrase, the lengths are deliberately incommensurate, and the
///             coincidences never repeat. The discipline that makes it the instrument: nothing
///             resets a phase — not `record`, not a setter, not a length change (an honest
///             splice: the head re-wraps, never rewinds), not even a DSP chain restart (the
///             wrapper re-prepares, which erases all tape and rewinds, only when the sample rate
///             or the tape buy actually changes). `record <loop> 1` punches the input onto that
///             loop at wherever its head happens to be, replacing (no overdub); `record <loop> 0`
///             freezes the tape bit-exactly. Per-loop @lengths / @levels / @pans (equal-power,
///             exact endpoints) / @darken (playback tone, bypassed to bit-transparency at 20 kHz)
///             place the phrases; `period` reports the composite period — the lcm of the active
///             lengths — out the right outlet (inf once it leaves the 64-bit range, which is the
///             point).
///
///             The optional argument sets the worst-case per-loop tape buy in seconds (default
///             30 — all eight reels are bought at DSP start, ~92 MB of double tape at 48 kHz;
///             size it to the piece). There is deliberately no wow here: the phasing engine is
///             the lengths. Run sources through tap.discreet~ on the way in for tape breath.
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

class airport : public object<airport>, public sample_operator<1, 2> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::loop_bank m_bank;
    bool              m_geometry_dirty{false};

  public:
    MIN_DESCRIPTION{"The Music for Airports loop bank: up to eight free-running tape loops of "
                    "unequal, incommensurate lengths, each with a single head that both plays "
                    "and records. No setter ever resets a phase — the free-run is the piece. "
                    "record <loop> 1 punches the input onto a loop at its head; record <loop> 0 "
                    "freezes it bit-exactly. Per-loop lengths, levels, equal-power pans, and a "
                    "playback darken corner place the phrases in the stereo field; the period "
                    "message reports the composite period (the lcm of the lengths). The optional "
                    "argument sets the maximum loop length in seconds (all eight reels are "
                    "bought at DSP start)."};
    MIN_TAGS{"delays"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.discreet~, tap.garden~, tap.multitap~, groove~"};

    inlet<>  m_in{this, "(signal) audio input — punched onto any recording loop"};
    outlet<> m_out_left{this, "(signal) the loop sum, left", "signal"};
    outlet<> m_out_right{this, "(signal) the loop sum, right", "signal"};
    outlet<> m_out_info{this, "(list) reports: period <seconds>, phase <loop> <0..1>"};

    attribute<number> maxloop{this, "maxloop", kernel::k_default_max_seconds,
                              setter{MIN_FUNCTION{
                                  const double v   = std::max(kernel::k_min_loop_seconds, static_cast<double>(args[0]));
                                  m_geometry_dirty = true; // re-bought (all tape erased) when the DSP chain starts
                                  return {v};
                              }},
                              description{"Worst-case loop length in seconds (set by the first object argument; all "
                                          "eight reels are bought at DSP start — 30 s is ~92 MB of tape at 48 kHz). "
                                          "Changing it erases every loop when the DSP chain restarts."}};

    attribute<int> loops{this, "loops", 1, setter{MIN_FUNCTION{
                             const int v = std::clamp(static_cast<int>(args[0]), 1, kernel::k_max_loops);
                             m_bank.set_loops(v);
                             return {v};
                         }},
                         description{"Number of active loops (1..8). Newly activated loops come in at their "
                                     "stored settings, their heads wherever they last were."}};

    attribute<std::vector<number>> lengths{
        this,
        "lengths",
        {1.0},
        setter{MIN_FUNCTION{
            for (size_t i = 0; i < args.size() && i < static_cast<size_t>(kernel::k_max_loops); ++i) {
                m_bank.set_length_seconds(static_cast<int>(i), args[i]);
            }
            return args;
        }},
        description{"Per-loop length in seconds (0.5 up to maxloop). The lengths "
                    "are the score: keep them incommensurate and the coincidences "
                    "never repeat. A change is a splice — the head re-wraps and "
                    "never rewinds. It can click; splices do."}};

    attribute<std::vector<number>> levels{
        this,
        "levels",
        {1.0},
        setter{MIN_FUNCTION{
            for (size_t i = 0; i < args.size() && i < static_cast<size_t>(kernel::k_max_loops); ++i) {
                m_bank.set_level(static_cast<int>(i), args[i]);
            }
            return args;
        }},
        description{"Per-loop linear playback level, slewed. Unclamped (negative "
                    "flips polarity, like a mixer)."}};

    attribute<std::vector<number>> pans{this,
                                        "pans",
                                        {0.0},
                                        setter{MIN_FUNCTION{
                                            for (size_t i = 0;
                                                 i < args.size() && i < static_cast<size_t>(kernel::k_max_loops); ++i) {
                                                m_bank.set_pan(static_cast<int>(i), args[i]);
                                            }
                                            return args;
                                        }},
                                        description{"Per-loop equal-power pan, -1 (hard left) to 1 (hard right), "
                                                    "slewed. Endpoints are exact: a hard-panned loop is bitwise "
                                                    "absent from the far bus."}};

    attribute<std::vector<number>> darken{
        this,
        "darken",
        {tap::tools::tape::k_darken_ceil_hz},
        setter{MIN_FUNCTION{
            for (size_t i = 0; i < args.size() && i < static_cast<size_t>(kernel::k_max_loops); ++i) {
                m_bank.set_darken_hz(static_cast<int>(i), args[i]);
            }
            return args;
        }},
        description{"Per-loop playback darkening corner in Hz. A static tone, not "
                    "generation loss (a frozen loop replays the same imprint); at "
                    "20000 (the default) the stage is bypassed and playback is "
                    "bit-transparent."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_bank.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for level/pan/darken moves, in ms (0 = instant)."}};

    message<> record{this, "record",
                     "record <loop> <0|1>: punch the input onto that loop's tape at wherever its head happens to "
                     "be (1), or freeze the tape bit-exactly (0). Recording replaces — no overdub.",
                     MIN_FUNCTION{
                         if (args.size() >= 2) {
                             m_bank.record(static_cast<int>(args[0]), static_cast<int>(args[1]) != 0);
                         }
                         return {};
                     }};

    message<> period{this, "period",
                     "Report the composite period — the lcm of the active loop lengths, in seconds — out the "
                     "right outlet. inf means it left the 64-bit range: the piece will not repeat.",
                     MIN_FUNCTION{
                         m_out_info.send("period", m_bank.composite_period_seconds());
                         return {};
                     }};

    message<> phase{this, "phase",
                    "phase <loop>: report that loop's head position as a fraction of its length (0..1) out the "
                    "right outlet.",
                    MIN_FUNCTION{
                        if (!args.empty()) {
                            const int i = static_cast<int>(args[0]);
                            m_out_info.send("phase", i, m_bank.phase(i));
                        }
                        return {};
                    }};

    message<> clear{this, "clear",
                    "Erase every tape and rewind every head — a full restart of the piece. Parameters are kept.",
                    MIN_FUNCTION{
                        m_bank.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup",
                       "Re-prepare only when the sample rate or the tape buy changed — a DSP chain restart must "
                       "not erase eight running loops.",
                       MIN_FUNCTION{
                           if (m_geometry_dirty || !m_bank.prepared() || samplerate() != m_bank.samplerate()) {
                               m_bank.prepare(samplerate(), maxloop);
                               m_geometry_dirty = false;
                           }
                           return {};
                       }};

    airport(const atoms& args = {}) {
        if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
            maxloop = args[0];
        }
        m_bank.prepare(samplerate(), maxloop);
        m_geometry_dirty = false;
    }

    samples<2> operator()(sample x) {
        double left  = 0.0;
        double right = 0.0;
        m_bank.process(x, left, right);
        return {left, right};
    }
};

MIN_EXTERNAL(airport);
