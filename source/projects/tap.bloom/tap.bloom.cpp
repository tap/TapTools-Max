/// @file
/// @brief      tap.bloom — the event ring tap.garden~ recirculates its notes on.
/// @details    Plant a note and it comes back at its own position on a loop, every pass a step
///             quieter (`decay`) and a step purer (`soften`), until it falls below `floor` and
///             retires. That is the whole of tap::tools::garden::ring (taptools/garden.h), and it
///             is the piece of tap.garden~ with the most reach, because it knows nothing about
///             chimes: it emits notes, and what sounds them is yours to choose. Point it at
///             tap.chime~ and you have the garden's voice; point it at makenote, poly~, a
///             sampler, or MIDI out and you have the same generative principle driving something
///             else entirely.
///
///             The stabiliser and its theorem: a plant at velocity v under decay d retires after
///             exactly ceil(log(floor/v)/log(d)) strikes, so the live population converges no
///             matter how fast you plant. Sixty-four blooms fit; a full ring retires its OLDEST to
///             make room, because a touch must always speak and the oldest is the quietest.
///
///             Honest limit — the one real cost of patching this rather than using tap.garden~:
///             the ring runs on MAX'S SCHEDULER here, not the audio clock, so returns land within
///             an @interval tick (1 ms by default) rather than on the exact sample. Inside
///             tap.garden~ the same ring is sample-accurate. For loops measured in seconds this is
///             inaudible, but it is a difference, and it is why the null test against tap.garden~
///             is run on the kernel rather than on the patch.
///
///             @interval also sets the ring's time resolution: the loop is counted in ticks of
///             that length. Changing it re-prepares the ring, which uproots everything.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <array>

#include <taptools/garden.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::garden;

class bloom : public object<bloom> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::ring                                     m_ring;
    std::array<kernel::strike, kernel::k_max_events> m_fired{};
    // Attributes are constructed in declaration order and their setters run as they go, so
    // reprepare() must not read attributes that do not exist yet. The constructor flips this.
    bool m_ready{false};

  public:
    MIN_DESCRIPTION{"The generative event loop from tap.garden~, on its own. Plant a note and it "
                    "returns at its own loop position every pass, a step quieter and a step "
                    "purer, until it falls below the floor and retires — so the live population "
                    "converges however fast you plant. It emits notes rather than sound, so it "
                    "will drive tap.chime~, makenote, poly~, or MIDI out equally well. Note that "
                    "the loop runs on Max's scheduler here: returns land within an interval tick "
                    "rather than on the exact sample, as they do inside tap.garden~."};
    MIN_TAGS{"time"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.garden~, tap.chime~, tap.gardener, tap.scale, seq, makenote"};

    inlet<>  m_in{this, "(list) plant <pitch> <velocity>, or a bare pitch/velocity pair"};
    outlet<> m_out_note{this, "(list) note <pitch> <velocity> <brightness> — a bloom firing"};
    outlet<> m_out_info{this, "(list) reports: events <n>"};

    attribute<number> interval{this, "interval", 1.0, setter{MIN_FUNCTION{
                                   const double v = std::clamp(static_cast<double>(args[0]), 0.1, 100.0);
                                   reprepare(v);
                                   return {v};
                               }},
                               description{"Scheduler tick in ms (0.1..100). This is both how often the ring is "
                                           "advanced and its time resolution — the loop is counted in ticks of "
                                           "this length. Changing it re-prepares the ring, which uproots every "
                                           "live bloom."}};

    attribute<number> loop{this, "loop", kernel::k_default_loop_seconds, setter{MIN_FUNCTION{
                               const double v = std::clamp(static_cast<double>(args[0]), kernel::k_min_loop_seconds,
                                                           kernel::k_max_loop_seconds);
                               m_ring.set_loop_seconds(v);
                               return {v};
                           }},
                           description{"Loop length in seconds (0.25..120). Instant — the loop is a counter, so "
                                       "live blooms keep their positions modulo the new length."}};

    attribute<number> decay{this, "decay", kernel::k_default_decay, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                m_ring.set_decay(v);
                                return {v};
                            }},
                            description{"Velocity multiplier per pass (0..1) — the stabiliser. A plant at velocity "
                                        "v retires after exactly ceil(log(floor/v)/log(decay)) strikes."}};

    attribute<number> soften{this, "soften", kernel::k_default_soften, setter{MIN_FUNCTION{
                                 const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                 m_ring.set_soften(v);
                                 return {v};
                             }},
                             description{"Brightness multiplier per pass (0..1): each return is purer than the "
                                         "last, so a bloom does not merely fade, it loses its edge first."}};

    attribute<number> floor{this, "floor", kernel::k_default_floor, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), 1e-4, 1.0);
                                m_ring.set_floor(v);
                                return {v};
                            }},
                            description{"Retirement threshold (1e-4..1): a bloom whose velocity falls below this "
                                        "is done and leaves the ring."}};

    attribute<number> brightness{this, "brightness", kernel::k_default_brightness,
                                 setter{MIN_FUNCTION{
                                     const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                     m_ring.set_brightness(v);
                                     return {v};
                                 }},
                                 description{"The brightness a new plant starts at (0..1); it softens from there."}};

    message<> plant{this, "plant",
                    "plant <pitch> <velocity>: plant a bloom at the current loop position. It fires on the next "
                    "tick and then every pass until it retires. Pitches are NOT quantized here — put tap.scale "
                    "in front if you want a scale.",
                    MIN_FUNCTION{
                        if (args.size() >= 2 && static_cast<double>(args[1]) > 0.0) {
                            m_ring.plant(args[0], args[1]);
                        }
                        return {};
                    }};

    message<> list{this, "list", "A bare <pitch> <velocity> pair is the same as plant.",
                   MIN_FUNCTION{
                       if (args.size() >= 2 && static_cast<double>(args[1]) > 0.0) {
                           m_ring.plant(args[0], args[1]);
                       }
                       return {};
                   }};

    message<> events{this, "events", "Report how many blooms are live, out the right outlet.",
                     MIN_FUNCTION{
                         m_out_info.send("events", m_ring.active_events());
                         return {};
                     }};

    message<> clear{this, "clear", "Uproot everything: kill every bloom and rewind the loop. Parameters are kept.",
                    MIN_FUNCTION{
                        m_ring.clear();
                        return {};
                    }};

    // The ring is advanced from Max's scheduler, one kernel step per tick — see the honest
    // limit in the file header. Output is a list per strike, newest last.
    timer<> m_clock{this,
                    MIN_FUNCTION{
                        const int n = m_ring.due(m_fired.data(), kernel::k_max_events);
                        m_ring.step();
                        for (int i = 0; i < n; ++i) {
                            const kernel::strike& s = m_fired[static_cast<size_t>(i)];
                            m_out_note.send("note", s.pitch, s.velocity, s.brightness);
                        }
                        m_clock.delay(interval);
                        return {};
                    }};

    bloom(const atoms& args = {}) {
        if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
            loop = args[0];
        }
        m_ready = true;
        reprepare(interval);
        m_clock.delay(interval);
    }

  private:
    /// The ring counts in ticks, so its "sample rate" is the tick rate. Re-preparing clears it —
    /// there is no way to rescale live bloom positions honestly when the grid itself changes.
    void reprepare(double interval_ms) {
        if (!m_ready) {
            return; // still building the attributes; the constructor prepares once at the end
        }
        m_ring.prepare(1000.0 / interval_ms);
        m_ring.set_loop_seconds(loop);
        m_ring.set_decay(decay);
        m_ring.set_soften(soften);
        m_ring.set_floor(floor);
        m_ring.set_brightness(brightness);
    }
};

MIN_EXTERNAL(bloom);
