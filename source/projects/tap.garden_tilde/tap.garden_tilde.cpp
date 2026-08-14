/// @file
/// @brief      tap.garden~ — the generative event loop on the portable kernel.
/// @details    A garden you tend rather than play, wrapping tap::tools::garden::bed
///             (taptools/garden.h) — the family's recreation of the published principle behind
///             Eno/Chilvers' generative instruments: `note <pitch> <velocity>` plants a note,
///             which snaps to @root/@scale at entry, strikes a small modal wind chime (four
///             bar-mode doublets at @material's ratios — free-free tubes or tuned bars, per
///             Fletcher & Rossing — hardness-coupled, small high tubes ringing shorter), and
///             returns every @loop pass a step quieter (@decay) and purer (@soften) until it
///             retires below @floor. Each pitch is its own tube: its upper modes sit a fixed
///             few cents off the ideal ratios and it hangs at a fixed seat on the stereo rack
///             (@spread widens it; 0 collapses to mono) — both keyed by the pitch itself, so
///             the rack is the same rack in every instance. Left alone past @idle seconds, a
///             seeded gardener strikes on a calm/gust wind cycle (@gust sizes the flurries;
///             @seed makes it reproducible: same seed, same garden, bit for bit; @idle 0
///             disables it entirely). The population converges by construction — per-pass decay
///             is the stabilizer — and a fixed sixteen-chime pool (quietest stolen, envelopes
///             re-aimed, never reset) hard-bounds the audio.
///
///             A source: no signal inlet, a stereo signal outlet pair. Wrong notes are
///             unrepresentable (quantization is always on; @scale chromatic is the widest
///             field), event timing is the loop grid exactly, and the timbre is one chime
///             family in two materials — it is an instrument, not a polysynth. `clear` uproots
///             everything and re-seeds the gardener.
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing. For a
///             wider garden, run several instances with different seeds.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <array>

#include <taptools/garden.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::garden;

class garden : public object<garden>, public sample_operator<0, 2> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::bed m_bed;

    // Same symbol-or-index helper as tap.ladder~ / tap.diode~ (min enum attributes report
    // indices; the house pattern is attribute<symbol> + range{} with a numeric fallback).
    template <size_t N>
    static int index_from_atom(const atom& a, const std::array<const char*, N>& names, int fallback) {
        if (a.type() == message_type::symbol_argument) {
            for (size_t i = 0; i < N; ++i) {
                if (a == names[i]) {
                    return static_cast<int>(i);
                }
            }
            return fallback; // unknown symbol: the kernel default
        }
        return std::clamp(static_cast<int>(a), 0, static_cast<int>(N) - 1);
    }

    static constexpr std::array<const char*, kernel::k_num_scales>    k_scale_names{"chromatic", "major", "minor",
                                                                                 "majorpentatonic", "minorpentatonic"};
    static constexpr std::array<const char*, kernel::k_num_materials> k_material_names{"chime", "bar"};

  public:
    MIN_DESCRIPTION{"A generative event loop: note messages plant notes that snap to the scale, "
                    "strike a small modal wind chime (tubes or tuned bars), and return every "
                    "loop pass a step quieter (decay) and purer (soften) until they retire below "
                    "the floor. Every pitch is its own tube with its own fixed flaws and its own "
                    "seat on the stereo rack (spread). Left idle, a seeded gardener strikes on a "
                    "calm/gust wind cycle — same seed, same garden, bit for bit. A fixed "
                    "sixteen-chime pool bounds the sound; a sixty-four-event ring bounds the "
                    "score (the oldest bloom yields when full). A source: no signal inlet."};
    MIN_TAGS{"generators"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.discreet~, tap.airport~, tap.808.seq~, makenote"};

    inlet<>  m_in{this, "(message) note <pitch> <velocity>; 'clear' uproots the garden"};
    outlet<> m_out_left{this, "(signal) the garden, left", "signal"};
    outlet<> m_out_right{this, "(signal) the garden, right", "signal"};

    attribute<number> loop{this, "loop", kernel::k_default_loop_seconds, setter{MIN_FUNCTION{
                               const double v = std::clamp(static_cast<double>(args[0]), kernel::k_min_loop_seconds,
                                                           kernel::k_max_loop_seconds);
                               m_bed.set_loop_seconds(v);
                               return {v};
                           }},
                           description{"Loop length in seconds (0.25..120). Instant — the loop is a counter, no "
                                       "tape is bought; blooms keep their positions modulo the new length."}};

    attribute<number> decay{this, "decay", kernel::k_default_decay, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                m_bed.set_decay(v);
                                return {v};
                            }},
                            description{"Velocity multiplier per pass (0..1) — the stabilizer: a bloom lives "
                                        "ceil(log(floor/velocity)/log(decay)) passes, always."}};

    attribute<number> soften{this, "soften", kernel::k_default_soften, setter{MIN_FUNCTION{
                                 const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                 m_bed.set_soften(v);
                                 return {v};
                             }},
                             description{"Brightness multiplier per pass (0..1): each return is purer, collapsing "
                                         "toward a sine as it recedes."}};

    attribute<number> floor{this, "floor", kernel::k_default_floor, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), 1e-4, 1.0);
                                m_bed.set_floor(v);
                                return {v};
                            }},
                            description{"Retirement threshold (0.0001..1): a bloom whose velocity decays below "
                                        "this is done."}};

    attribute<std::vector<number>> bell{
        this,
        "bell",
        {kernel::k_default_attack_s, kernel::k_default_decay_s, kernel::k_default_brightness},
        setter{MIN_FUNCTION{
            const double attack  = (!args.empty()) ? static_cast<double>(args[0]) : kernel::k_default_attack_s;
            const double decay_s = (args.size() > 1) ? static_cast<double>(args[1]) : kernel::k_default_decay_s;
            const double brightness =
                (args.size() > 2) ? std::clamp(static_cast<double>(args[2]), 0.0, 1.0) : kernel::k_default_brightness;
            m_bed.set_bell(attack, decay_s, brightness);
            return {attack, decay_s, brightness};
        }},
        description{"The chime as an attack/decay/brightness triple: envelope times "
                    "in seconds, brightness 0..1 weighting the upper modes. Applies "
                    "to future blooms; ringing chimes keep their envelope."}};

    attribute<symbol> material{this,
                               "material",
                               "chime",
                               range{"chime", "bar"},
                               setter{MIN_FUNCTION{
                                   const int v = index_from_atom(args[0], k_material_names, kernel::material_chime);
                                   m_bed.set_material(v);
                                   return {symbol(k_material_names[static_cast<size_t>(v)])};
                               }},
                               description{"What the tubes are made of: chime (free-free tube, ratios "
                                           "1 : 2.756 : 5.404 : 8.933) or bar (tuned bar, double octaves "
                                           "1 : 4 : 10 : 20). Accepts the symbol or the index. Instant — read at "
                                           "strike time, so live blooms re-voice at their next return."}};

    attribute<number> spread{this, "spread", kernel::k_default_spread, setter{MIN_FUNCTION{
                                 const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                 m_bed.set_spread(v);
                                 return {v};
                             }},
                             description{"Stereo width of the rack (0..1). Every tube hangs at a fixed seat keyed "
                                         "by its pitch; spread scales how far off center. 0 collapses to mono."}};

    attribute<int> root{this, "root", 0, setter{MIN_FUNCTION{
                            const int v = ((static_cast<int>(args[0]) % 12) + 12) % 12;
                            m_bed.set_root(v);
                            return {v};
                        }},
                        description{"Root pitch class, 0..11 (0 = C). Instant; affects future plants only — "
                                    "nothing already planted re-pitches."}};

    attribute<symbol> scale{this,
                            "scale",
                            "majorpentatonic",
                            range{"chromatic", "major", "minor", "majorpentatonic", "minorpentatonic"},
                            setter{MIN_FUNCTION{
                                const int v = index_from_atom(args[0], k_scale_names, kernel::scale_major_pentatonic);
                                m_bed.set_scale(v);
                                return {symbol(k_scale_names[static_cast<size_t>(v)])};
                            }},
                            description{"The field plants snap to. Accepts the symbol or the index (0 chromatic, "
                                        "1 major, 2 minor, 3 majorpentatonic, 4 minorpentatonic). Instant; "
                                        "affects future plants only."}};

    attribute<number> idle{this, "idle", kernel::k_default_idle_seconds, setter{MIN_FUNCTION{
                               const double v = std::max(0.0, static_cast<double>(args[0]));
                               m_bed.set_idle_seconds(v);
                               return {v};
                           }},
                           description{"The gardener's patience: seconds of silence before the garden starts "
                                       "planting for itself. 0 disables self-seeding entirely."}};

    attribute<number> gust{this, "gust", kernel::k_default_gust, setter{MIN_FUNCTION{
                               const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                               m_bed.set_gust(v);
                               return {v};
                           }},
                           description{"The gardener's wind (0..1): at 0 single unhurried strikes, about one per "
                                       "pass; up from there, flurries of up to five neighboring tubes with longer "
                                       "calms between — the average rate holds."}};

    attribute<int> seed{this, "seed", 0, setter{MIN_FUNCTION{
                            const int v = static_cast<int>(args[0]);
                            m_bed.set_seed(static_cast<uint64_t>(v));
                            return {v};
                        }},
                        description{"The gardener's seed. Deterministic: same seed, same garden, bit for bit; "
                                    "different instances decorrelate by seed. 0 folds to the stock seed."}};

    attribute<number> level{this, "level", 1.0, setter{MIN_FUNCTION{
                                const double v = static_cast<double>(args[0]);
                                m_bed.set_level(v);
                                return {v};
                            }},
                            description{"Master linear output level, slewed over smooth ms."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_bed.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Slew time for the level attribute, in ms (0 = instant)."}};

    message<> note{this, "note",
                   "note <pitch> <velocity>: plant a note. MIDI pitch (fractional accepted) snaps to root/scale "
                   "at entry; velocity 0..1 (default 0.8). The bell sounds on the next sample and the bloom "
                   "returns at this loop position every pass. Resets the gardener's idle clock.",
                   MIN_FUNCTION{
                       if (!args.empty()) {
                           const double pitch    = args[0];
                           const double velocity = (args.size() > 1) ? static_cast<double>(args[1]) : 0.8;
                           m_bed.note(pitch, std::clamp(velocity, 0.0, 1.0));
                       }
                       return {};
                   }};

    message<> clear{this, "clear",
                    "Uproot everything: kill all blooms and bells, rewind the loop, re-seed the gardener, "
                    "restart the idle clock. Parameters are kept.",
                    MIN_FUNCTION{
                        m_bed.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup",
                       "Re-prepare only when the sample rate changed — a DSP chain restart must not uproot a "
                       "running garden.",
                       MIN_FUNCTION{
                           if (!m_bed.prepared() || samplerate() != m_bed.samplerate()) {
                               m_bed.prepare(samplerate());
                           }
                           return {};
                       }};

    garden(const atoms& = {}) { m_bed.prepare(samplerate()); }

    samples<2> operator()() {
        double left  = 0.0;
        double right = 0.0;
        m_bed.process(left, right);
        return {left, right};
    }
};

MIN_EXTERNAL(garden);
