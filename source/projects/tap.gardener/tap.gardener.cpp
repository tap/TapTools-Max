/// @file
/// @brief      tap.gardener — the idle wind that plants tap.garden~'s notes when you stop.
/// @details    Wrapping tap::tools::garden::gardener (taptools/garden.h): after @idle seconds
///             without anyone planting, this starts planting for you. Strikes arrive on a
///             calm/gust cycle — @gust sizes the clusters, up to five neighbouring tubes within a
///             fraction of a second, with the calms between them stretched so the average rate
///             stays near one strike per loop pass at any gust setting. It is wind, not a
///             sequence.
///
///             The rng is drawn from ONLY while idling. That discipline is what makes @seed a
///             contract rather than a suggestion: the same seed gives the same wind every time, a
///             different seed decorrelates two instances, and with @idle 0 the seed cannot matter
///             at all because the stream is never touched. Send `planted` (or patch anything that
///             plants into the left inlet) to close the idle gate; the gardener stops, and the
///             gust resumes only if the garden goes quiet again.
///
///             Pitches come out RAW and unquantized, deliberately — the scale is the patch's
///             field, not the wind's. Put tap.scale between this and tap.bloom.
///
///             Like tap.bloom this runs on Max's scheduler rather than the audio clock, so
///             @interval sets both the tick and the time resolution.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/garden.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::garden;

class gardener : public object<gardener> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::gardener m_wind;
    // Attributes are constructed in declaration order and their setters run as they go, so
    // reprepare() must not read attributes that do not exist yet. The constructor flips this.
    bool m_ready{false};

  public:
    MIN_DESCRIPTION{"The idle wind from tap.garden~, on its own. After idle seconds with nobody "
                    "planting, it starts planting: strikes arrive in gusts of up to five "
                    "neighbouring tubes, separated by calms stretched so the average rate holds "
                    "near one strike per loop pass. Its rng is drawn from only while idling, so "
                    "seed is a real contract — same seed, same wind; idle 0, and the seed cannot "
                    "matter. Pitches come out raw: put tap.scale after it. Send planted to close "
                    "the idle gate when something else plants."};
    MIN_TAGS{"time"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.garden~, tap.bloom, tap.scale, tap.chime~, tap.random"};

    inlet<>  m_in{this, "(planted) close the idle gate — something else just planted"};
    outlet<> m_out_note{this, "(list) plant <pitch> <velocity> — raw, unquantized"};

    attribute<number> interval{this, "interval", 1.0, setter{MIN_FUNCTION{
                                   const double v = std::clamp(static_cast<double>(args[0]), 0.1, 100.0);
                                   reprepare(v);
                                   return {v};
                               }},
                               description{"Scheduler tick in ms (0.1..100) — both how often the wind is advanced "
                                           "and its time resolution. Changing it re-seeds and restarts the wind."}};

    attribute<number> loop{
        this, "loop", kernel::k_default_loop_seconds, setter{MIN_FUNCTION{
            return {std::clamp(static_cast<double>(args[0]), kernel::k_min_loop_seconds, kernel::k_max_loop_seconds)};
        }},
        description{"The loop length in seconds this wind is blowing across — match it to the "
                    "tap.bloom it feeds. It sizes the gusts and the calms, so that the average "
                    "rate stays near one strike per pass."}};

    attribute<number> idle{this, "idle", kernel::k_default_idle_seconds, setter{MIN_FUNCTION{
                               const double v = std::max(0.0, static_cast<double>(args[0]));
                               m_wind.set_idle_seconds(v);
                               return {v};
                           }},
                           description{"Seconds of quiet before the wind picks up. 0 disables it entirely — and "
                                       "then the seed cannot matter, because the rng is never consumed."}};

    attribute<number> gust{this, "gust", kernel::k_default_gust, setter{MIN_FUNCTION{
                               const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                               m_wind.set_gust(v);
                               return {v};
                           }},
                           description{"The wind, 0 to 1. At 0 strikes arrive singly and evenly, about one per "
                                       "pass; up from there they cluster into gusts with longer calms between, at "
                                       "the same average rate."}};

    attribute<int> seed{this, "seed", 0, setter{MIN_FUNCTION{
                            const int v = static_cast<int>(args[0]);
                            m_wind.set_seed(static_cast<uint64_t>(v));
                            return {v};
                        }},
                        description{"The wind's seed. Same seed, same wind, every run — so a render reproduces "
                                    "and two instances decorrelate by seeding them differently."}};

    message<> planted{this, "planted",
                      "Close the idle gate: something else just planted, so the wind should drop and wait out "
                      "another idle period before it starts again.",
                      MIN_FUNCTION{
                          m_wind.notice_plant();
                          return {};
                      }};

    message<> bang{this, "bang", "Same as planted — close the idle gate.",
                   MIN_FUNCTION{
                       m_wind.notice_plant();
                       return {};
                   }};

    message<> clear{this, "clear", "Re-seed the rng and restart both the idle clock and the wind.",
                    MIN_FUNCTION{
                        m_wind.clear();
                        return {};
                    }};

    // Advanced from Max's scheduler, one kernel tick per fire — see the header's honest limit.
    timer<> m_clock{this,
                    MIN_FUNCTION{
                        const long ticks =
                            static_cast<long>(static_cast<double>(loop) * 1000.0 / static_cast<double>(interval));
                        const kernel::gardener::request req = m_wind.tick(std::max(1L, ticks));
                        if (req.wanted) {
                            m_out_note.send("plant", req.pitch, req.velocity);
                        }
                        m_clock.delay(interval);
                        return {};
                    }};

    gardener(const atoms& args = {}) {
        if (!args.empty()) {
            seed = args[0];
        }
        m_ready = true;
        reprepare(interval);
        m_clock.delay(interval);
    }

  private:
    /// The wind counts in ticks, so its "sample rate" is the tick rate. Re-preparing re-seeds and
    /// restarts it; there is no honest way to carry a gust across a change of grid.
    void reprepare(double interval_ms) {
        if (!m_ready) {
            return; // still building the attributes; the constructor prepares once at the end
        }
        m_wind.prepare(1000.0 / interval_ms);
        m_wind.set_idle_seconds(idle);
        m_wind.set_gust(gust);
        m_wind.set_seed(static_cast<uint64_t>(static_cast<int>(seed)));
    }
};

MIN_EXTERNAL(gardener);
