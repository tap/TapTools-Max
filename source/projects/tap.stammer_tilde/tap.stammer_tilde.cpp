/// @file
/// @brief      tap.stammer~ — the live buffer-stutter rig on the portable kernel.
/// @details    Wraps tap::tools::stammer::machine (taptools/stammer.h): the input is captured
///             continuously, and on a @step grid the machine rolls dice and re-fires a slice of
///             what just went past. @density is how often it grabs, @divisions how finely it
///             chops (a slice is @step divided by 1..@divisions), @repeats how many passes it
///             holds on for, @reverse the per-repeat chance of running backwards, and @jump how
///             far further back it may reach. Riding those while a part plays is the instrument —
///             none of them is a set-and-forget.
///
///             @seed makes it a contract rather than a suggestion: every draw comes from the
///             family's seeded generator in a fixed order, so the same seed is the same
///             performance, bit for bit, and two instances decorrelate by seed. At @density 0 the
///             dice are never rolled at all — the seed cannot matter, and the object is a bitwise
///             bypass.
///
///             An original design in the brassage tradition (Roads, *Microsound*), not a port of
///             anyone's patch.
///
///             The optional argument sets the worst-case capture in seconds (default 4). A slice
///             reads from the ring rather than a copy, so a repeat train longer than the captured
///             history will read fresher material as the write head laps it — size the argument
///             to the longest train you intend to fire. `clear` erases the capture, drops the
///             slice in flight and rewinds the seeded stream.
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing. For
///             multichannel use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/stammer.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::stammer;

class stammer : public object<stammer>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::machine m_machine;
    bool            m_geometry_dirty{false};

  public:
    MIN_DESCRIPTION{"A live buffer-stutter rig. The input is captured continuously; on a step "
                    "grid the machine rolls dice and re-fires a slice of what just went past. "
                    "density is how often it grabs, divisions how finely it chops, repeats how "
                    "long it holds on, reverse how often a repeat runs backwards, jump how far "
                    "back it may reach — ride them and the part comes apart in your hands. The "
                    "seed is a real contract: same seed, same performance, bit for bit; at "
                    "density 0 the dice are never rolled and the object is a bitwise bypass. The "
                    "optional argument sets the captured history in seconds. Wants transient "
                    "material — on a sustained pad a stutter is barely a tremolo. For "
                    "multichannel signals, wrap this object in an mc. operator."};
    MIN_TAGS{"delays"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.tapecho~, tap.sustain~, tap.multitap~, tap.shift~"};

    inlet<>  m_in{this, "(signal) audio input — captured continuously"};
    outlet<> m_out{this, "(signal) the input, or the slice while one is firing", "signal"};

    attribute<number> maxhistory{
        this, "maxhistory", kernel::k_default_max_history_ms * 0.001, setter{MIN_FUNCTION{
            const double v   = std::max(kernel::k_min_step_ms * 0.001, static_cast<double>(args[0]));
            m_geometry_dirty = true; // re-bought (and the capture erased) when the DSP chain starts
            return {v};
        }},
        description{"Captured history in seconds (set by the first object argument; bought at "
                    "DSP start). A repeat train longer than this reads fresher material as the "
                    "write head laps it. Changing it erases the capture on the next DSP restart."}};

    attribute<number> step{this, "step", kernel::k_default_step_ms, setter{MIN_FUNCTION{
                               const double v = std::max(kernel::k_min_step_ms, static_cast<double>(args[0]));
                               m_machine.set_step_ms(v);
                               return {v};
                           }},
                           description{"The rhythmic grid in ms: how often the machine may decide to grab. Takes "
                                       "effect at the next grid point — it is a rhythm, not a level, so there is "
                                       "nothing to zipper."}};

    attribute<number> density{this, "density", kernel::k_default_density, setter{MIN_FUNCTION{
                                  const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                  m_machine.set_density(v);
                                  return {v};
                              }},
                              description{"Chance of firing at an idle grid point (0..1). At exactly 0 the dice are "
                                          "never rolled, so the seed cannot matter and the object is a bitwise "
                                          "bypass."}};

    attribute<int> divisions{this, "divisions", kernel::k_default_divisions, setter{MIN_FUNCTION{
                                 const int v = std::clamp(static_cast<int>(args[0]), 1, kernel::k_max_divisions);
                                 m_machine.set_divisions(v);
                                 return {v};
                             }},
                             description{"How finely the grid may be chopped (1..8): a slice is step divided by a "
                                         "number drawn from 1 to this. 1 means whole-step slices only."}};

    attribute<int> repeats{this, "repeats", kernel::k_default_repeats, setter{MIN_FUNCTION{
                               const int v = std::clamp(static_cast<int>(args[0]), 1, kernel::k_max_repeats);
                               m_machine.set_repeats(v);
                               return {v};
                           }},
                           description{"Upper bound on passes per fired slice (1..16); the count is drawn from 1 to "
                                       "this. A slice in flight is never interrupted, so this — not density — is "
                                       "what decides how long the machine stays busy."}};

    attribute<number> reverse{this, "reverse", kernel::k_default_reverse, setter{MIN_FUNCTION{
                                  const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                  m_machine.set_reverse(v);
                                  return {v};
                              }},
                              description{"Chance that any given repeat plays backwards (0..1). Drawn per repeat, "
                                          "so one train can stagger forwards and back."}};

    attribute<number> jump{this, "jump", kernel::k_default_jump_ms, setter{MIN_FUNCTION{
                               const double v = std::max(0.0, static_cast<double>(args[0]));
                               m_machine.set_jump_ms(v);
                               return {v};
                           }},
                           description{"How far back beyond the material just past a slice may reach, in ms; the "
                                       "actual reach is drawn from 0 to this. 0 is the classic stutter — open it "
                                       "and the machine starts quoting older material."}};

    attribute<number> fade{this, "fade", kernel::k_default_fade_ms, setter{MIN_FUNCTION{
                               const double v = std::max(0.0, static_cast<double>(args[0]));
                               m_machine.set_fade_ms(v);
                               return {v};
                           }},
                           description{"Raised-sine flank per repeat in ms — the anti-click. Clamped per slice to "
                                       "half its length. Repeats are sequential, so each junction dips to zero: "
                                       "that is the articulation of a stutter, not a crossfade that failed."}};

    attribute<int> seed{this, "seed", 1, setter{MIN_FUNCTION{
                            const int v = static_cast<int>(args[0]);
                            m_machine.set_seed(static_cast<uint64_t>(v));
                            return {v};
                        }},
                        description{"The performance seed. Deterministic: same seed and same moves, same render, "
                                    "bit for bit; different instances decorrelate by seed. 0 folds to the stock "
                                    "seed."}};

    attribute<number> input{this, "input", 1.0, setter{MIN_FUNCTION{
                                const double v = static_cast<double>(args[0]);
                                m_machine.set_input_level(v);
                                return {v};
                            }},
                            description{"Input level into the capture, linear, slewed."}};

    attribute<number> mix{this, "mix", kernel::k_default_mix, setter{MIN_FUNCTION{
                              const double v = std::clamp(static_cast<double>(args[0]), 0.0, 100.0);
                              m_machine.set_mix(v);
                              return {v};
                          }},
                          description{"Balance between the live input and the slice in percent (0..100), "
                                      "equal-power — and it only bites while a slice is firing. When the machine "
                                      "is idle the input passes through bitwise at any mix."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_machine.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for the input and mix setters, in ms (0 = "
                                         "instant)."}};

    message<> clear{this, "clear",
                    "Erase the capture, drop the slice in flight, and rewind the seeded stream — the same seed "
                    "replays the same performance from here. Parameters are kept.",
                    MIN_FUNCTION{
                        m_machine.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup",
                       "Re-prepare only when the sample rate or the captured history changed — a DSP chain "
                       "restart must not silently throw away a performance in progress.",
                       MIN_FUNCTION{
                           if (m_geometry_dirty || !m_machine.prepared() || samplerate() != m_machine.samplerate()) {
                               m_machine.prepare(samplerate(), static_cast<double>(maxhistory) * 1000.0);
                               m_geometry_dirty = false;
                           }
                           return {};
                       }};

    stammer(const atoms& args = {}) {
        if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
            maxhistory = args[0];
        }
        m_machine.prepare(samplerate(), static_cast<double>(maxhistory) * 1000.0);
        m_geometry_dirty = false;
    }

    sample operator()(sample x) { return m_machine.process(x); }
};

MIN_EXTERNAL(stammer);
