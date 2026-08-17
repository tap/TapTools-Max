/// @file
/// @brief      tap.scrub~ — a granular scrub pad over live capture.
/// @details    Wraps tap::tools::scrub::machine (taptools/scrub.h). Record the input
///             continuously, then put a granular playhead on it whose **position** and **pitch**
///             are two independent performable signals. Drag the position and you rake back and
///             forth through the last few seconds of the performance; hold it still and you have
///             a granular freeze; move the pitch and the material transposes without the position
///             moving at all.
///
///             The two signal inlets are the two axes of a pad, which is the whole Max-side story:
///             @position in the middle inlet, @pitch in the right, both at signal rate, and the
///             gesture is the instrument. Either falls back to its attribute when nothing is
///             connected.
///
///             **The null it is built on.** Grains are Hann-windowed and fired every
///             @size / @overlap samples, and Hann overlap-adds to exactly 1 at those hops. So
///             held still at @pitch 0 with @spray 0, this object is a plain delay of @position
///             milliseconds — measured at 4.4e-16 in the kernel's suite. Everything else it does
///             is a departure from that, which is only trustworthy because the identity is exact.
///
///             **@freeze stops the recorder, not the playhead.** Frozen, the position addresses
///             fixed tape and the grains loop the same window, so you can keep scrubbing,
///             transposing and drifting through a moment that has already gone past.
///
///             **@pitch is a granular texture, not a hi-fi shift.** Transposing means reading the
///             tape at a rate the write head does not share, so the read pointer has to be
///             wrapped back periodically and each wrap is a splice. Measured, 98.8 % of a clean
///             shifter's energy still lands at the transposed pitch, but spread into a narrow
///             comb rather than one line — audibly a warble. For clean transposition reach for
///             tap.pitchaccum~ or tap.shift~.
///
///             The tape is the same class tap.stammer~ records into, shared rather than copied.
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing. For
///             multichannel use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cstdint>

#include <taptools/scrub.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::scrub;

class scrub : public object<scrub>, public sample_operator<3, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::machine m_machine;
    bool            m_geometry_dirty{false};

  public:
    MIN_DESCRIPTION{"A granular scrub pad over live capture. The input is recorded continuously "
                    "and a Hann-windowed granular playhead reads it, with position and pitch as "
                    "two independent signals — the two axes of a pad. Held still at pitch 0 the "
                    "object is exactly a delay; move the position and you rake through the last "
                    "few seconds; freeze stops the recorder so the position addresses fixed tape. "
                    "Pitch here is a granular texture rather than a hi-fi shift — use "
                    "tap.pitchaccum~ for clean transposition. Wants transient material. For "
                    "multichannel signals, wrap this object in an mc. operator."};
    MIN_TAGS{"effects"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.stammer~, tap.pitchaccum~, tap.tapecho~, tap.reel~"};

    inlet<>  m_in{this, "(signal) audio input — captured continuously"};
    inlet<>  m_in_pos{this, "(signal) playhead position, ms behind the live edge"};
    inlet<>  m_in_pitch{this, "(signal) transposition in semitones"};
    outlet<> m_out{this, "(signal) the scrub, balanced against the input", "signal"};

    attribute<number> maxhistory{
        this, "maxhistory", kernel::k_default_max_history_ms * 0.001, setter{MIN_FUNCTION{
            const double v   = std::max(kernel::k_min_size_ms * 0.001, static_cast<double>(args[0]));
            m_geometry_dirty = true; // re-bought (and the tape erased) when the DSP chain starts
            return {v};
        }},
        description{"Captured history in seconds (set by the first object argument; bought at "
                    "DSP start). It is also the ceiling on position. Changing it erases the tape "
                    "on the next DSP restart."}};

    attribute<number> position{this, "position", kernel::k_default_position_ms,
                               setter{MIN_FUNCTION{
                                   const double v = std::max(0.0, static_cast<double>(args[0]));
                                   m_machine.set_position_ms(v);
                                   return {v};
                               }},
                               description{"Where the playhead sits, as a lag behind the live edge in ms. 0 is the "
                                           "newest sample; the ceiling is maxhistory. This is the scrub gesture, "
                                           "so it is slewed by smooth. Ignored while a signal is connected to the "
                                           "middle inlet."}};

    attribute<number> pitch{this, "pitch", kernel::k_default_pitch_st, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), -kernel::k_max_pitch_st,
                                                            kernel::k_max_pitch_st);
                                m_machine.set_pitch(v);
                                return {v};
                            }},
                            description{"Transposition in semitones, plus or minus two octaves. Independent of "
                                        "position — that independence is the object. A granular texture rather "
                                        "than a clean shift: the wraps leave a warble. Ignored while a signal is "
                                        "connected to the right inlet."}};

    attribute<number> drift{this, "drift", kernel::k_default_drift, setter{MIN_FUNCTION{
                                const double v =
                                    std::clamp(static_cast<double>(args[0]), -kernel::k_max_drift, kernel::k_max_drift);
                                m_machine.set_drift(v);
                                return {v};
                            }},
                            description{"The playhead's own motion through the tape, in playback-rate units: "
                                        "positive runs forward toward the live edge, negative backwards, 0 holds "
                                        "station. It wraps around the captured history rather than clamping, so a "
                                        "slow drift is a loop. Most useful with freeze on."}};

    attribute<bool> freeze{this, "freeze", false, setter{MIN_FUNCTION{
                               const bool v = static_cast<bool>(args[0]);
                               m_machine.set_freeze(v);
                               return {v};
                           }},
                           description{"Stop the recorder. The playhead keeps going, so the position now addresses "
                                       "fixed tape — a granular hold you can still scrub, transpose and drift "
                                       "through. It does not stop time inside a grain already in flight."}};

    attribute<number> size{this, "size", kernel::k_default_size_ms, setter{MIN_FUNCTION{
                               const double v = std::clamp(static_cast<double>(args[0]), kernel::k_min_size_ms,
                                                           kernel::k_max_size_ms);
                               m_machine.set_size_ms(v);
                               return {v};
                           }},
                           description{"Grain length in ms. Grains in flight keep the length they were born with. "
                                       "Sizes that divide evenly by overlap have an exactly flat window sum; "
                                       "others leave a small ripple."}};

    attribute<int> overlap{this, "overlap", kernel::k_default_overlap, setter{MIN_FUNCTION{
                               const int v = std::clamp(static_cast<int>(args[0]), 1, kernel::k_max_overlap);
                               m_machine.set_overlap(v);
                               return {v};
                           }},
                           description{"How many grains overlap: the hop is size / overlap. 1 leaves gaps between "
                                       "grains, which is a chopped texture rather than a defect; 2 and up hold a "
                                       "constant level, and 2 is the setting the delay null depends on."}};

    attribute<number> spray{this, "spray", kernel::k_default_spray_ms, setter{MIN_FUNCTION{
                                const double v = std::max(0.0, static_cast<double>(args[0]));
                                m_machine.set_spray_ms(v);
                                return {v};
                            }},
                            description{"Random scatter of each grain's origin, in ms back from the position. At "
                                        "exactly 0 the dice are never rolled, so the seed cannot matter and the "
                                        "object is deterministic without it. Also the way to trade the "
                                        "transposition's comb for a broadband smear."}};

    attribute<int> seed{this, "seed", 1, setter{MIN_FUNCTION{
                            const int v = static_cast<int>(args[0]);
                            m_machine.set_seed(static_cast<uint64_t>(v));
                            return {v};
                        }},
                        description{"The spray seed. Deterministic: same seed and same moves, same render, bit "
                                    "for bit; different instances decorrelate by seed. Only consumed while spray "
                                    "is above 0."}};

    attribute<number> mix{this, "mix", kernel::k_default_mix, setter{MIN_FUNCTION{
                              const double v = std::clamp(static_cast<double>(args[0]), 0.0, 100.0);
                              m_machine.set_mix(v);
                              return {v};
                          }},
                          description{"Balance between the live input and the scrub, 0 to 100, equal-power. The "
                                      "ends are exact: 0 is the input bit for bit, 100 is the scrub alone."}};

    attribute<number> level{this, "level", kernel::k_default_level, setter{MIN_FUNCTION{
                                const double v = static_cast<double>(args[0]);
                                m_machine.set_level(v);
                                return {v};
                            }},
                            description{"Output level, linear."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_machine.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for the attribute-driven position, pitch, drift, "
                                         "mix and level, in ms (0 = instant). The signal inlets are not ramped — "
                                         "a control signal is assumed smooth already."}};

    message<> clear{this, "clear",
                    "Erase the tape, kill every grain, rewind the drift, and restart the seeded stream. "
                    "Parameters are kept.",
                    MIN_FUNCTION{
                        m_machine.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup",
                       "Re-prepare only when the sample rate or the captured history changed — a DSP chain "
                       "restart must not silently throw away what has been recorded.",
                       MIN_FUNCTION{
                           if (m_geometry_dirty || !m_machine.prepared() || samplerate() != m_machine.samplerate()) {
                               m_machine.prepare(samplerate(), static_cast<double>(maxhistory) * 1000.0);
                               m_geometry_dirty = false;
                           }
                           return {};
                       }};

    scrub(const atoms& args = {}) {
        if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
            maxhistory = args[0];
        }
        m_machine.prepare(samplerate(), static_cast<double>(maxhistory) * 1000.0);
        m_geometry_dirty = false;
    }

    sample operator()(sample x, sample pos, sample semitones) {
        // Signals on the performance inlets drive the playhead directly; otherwise the
        // attributes do, through their ramps. With one inlet connected and the other not, the
        // unconnected side falls back to its attribute TARGET rather than its ramp, so a change
        // there lands immediately instead of slewing — the mixed case is a patching choice, and
        // this is the simple, predictable reading of it.
        const bool pos_signal   = m_in_pos.has_signal_connection();
        const bool pitch_signal = m_in_pitch.has_signal_connection();
        if (!pos_signal && !pitch_signal) {
            return m_machine.process(x);
        }
        return m_machine.process(x, pos_signal ? static_cast<double>(pos) : static_cast<double>(position),
                                 pitch_signal ? static_cast<double>(semitones) : static_cast<double>(pitch));
    }
};

MIN_EXTERNAL(scrub);
