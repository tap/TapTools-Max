/// @file
/// @brief      tap.discreet~ — the Discreet Music two-machine tape loop on the portable kernel.
/// @details    A long-loop tape regeneration machine wrapping tap::tools::discreet::machine
///             (taptools/discreet.h): input is recorded onto tape, spools @loop seconds to a
///             second machine, and the playback is both the output and the signal folded back
///             into the record head. Unlike tap.delay~ (feedback capped at 0.99), @regen legally
///             reaches 1.0: stability comes from the wear path — @darken lowpass, bounded @drive
///             saturation, DC blocker — not from a gain cap; each pass survives because it is
///             degraded. @loop moves glide as honest tape-speed doppler (they bend pitch — by
///             design; @smooth sets how fast the transport re-spools), the @wow/@flutter
///             transport is periodic and deterministic, and @input is the send fader: fade it to
///             zero while the loop sustains and the piece keeps evolving without you.
///
///             The optional argument sets the worst-case tape buy in seconds (default 30 —
///             ~11.5 MB of double tape at 48 kHz; size it to the piece). The tape survives DSP
///             chain restarts: the wrapper re-prepares (which erases the tape) only when the
///             sample rate or the buy actually changes. `clear` is the eject button.
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing. For
///             multichannel use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/discreet.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::discreet;

class discreet : public object<discreet>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::machine m_machine;
    bool            m_geometry_dirty{false};

  public:
    MIN_DESCRIPTION{"The Discreet Music two-machine tape loop. Input is recorded onto tape, "
                    "spools loop seconds to the playback machine, and returns through a wear path "
                    "(darkening lowpass, bounded saturation, DC blocker) into the record head — "
                    "so regen legally reaches 1.0 and sustains, bounded, because every pass is "
                    "degraded. Loop changes glide as tape-speed doppler; wow and flutter are a "
                    "deterministic periodic transport. The optional argument sets the maximum "
                    "loop in seconds (the tape bought at DSP start). For multichannel signals, "
                    "wrap this object in an mc. operator."};
    MIN_TAGS{"delays"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.delay~, tap.airport~, tap.garden~, tap.comb~"};

    inlet<>  m_in{this, "(signal) audio input — the send into the record head"};
    outlet<> m_out{this, "(signal) the machine's output (equal-power dry/wet mix)", "signal"};

    attribute<number> maxloop{this, "maxloop", kernel::k_default_max_seconds,
                              setter{MIN_FUNCTION{
                                  const double v   = std::max(kernel::k_min_loop_seconds, static_cast<double>(args[0]));
                                  m_geometry_dirty = true; // re-bought (and the tape erased) when the DSP chain starts
                                  return {v};
                              }},
                              description{"Worst-case loop length in seconds (set by the first object argument; the "
                                          "tape is bought at DSP start and bounds the loop attribute). Changing it "
                                          "erases the tape when the DSP chain restarts."}};

    attribute<number> loop{this, "loop", kernel::k_default_loop_seconds, setter{MIN_FUNCTION{
                               const double v = std::max(kernel::k_min_loop_seconds, static_cast<double>(args[0]));
                               m_machine.set_loop_seconds(v);
                               return {v};
                           }},
                           description{"Tape span between the machines, in seconds (0.1 up to maxloop). Slewed — "
                                       "and the slew is honest tape-speed doppler: the pitch bends while the "
                                       "transport re-spools, then re-locks. smooth sets the re-spool time."}};

    attribute<number> regen{this, "regen", 0.0, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), 0.0, kernel::k_regen_max);
                                m_machine.set_regen(v);
                                return {v};
                            }},
                            description{"Return level into the record head (0..1). 1.0 is legal and sustains "
                                        "forever, bounded by the wear path — bring it down (or darken harder) to "
                                        "end a piece."}};

    attribute<number> darken{this, "darken", kernel::k_default_darken_hz, setter{MIN_FUNCTION{
                                 const double v =
                                     std::clamp(static_cast<double>(args[0]), tap::tools::tape::k_darken_floor_hz,
                                                tap::tools::tape::k_darken_ceil_hz);
                                 m_machine.set_darken_hz(v);
                                 return {v};
                             }},
                             description{"Per-pass darkening corner in Hz (20..20000). Every trip through the loop "
                                         "runs through this one-pole lowpass: the tape forgets treble first."}};

    attribute<number> drive{this, "drive", kernel::k_default_drive, setter{MIN_FUNCTION{
                                const double v = std::max(0.0, static_cast<double>(args[0]));
                                m_machine.set_drive(v);
                                return {v};
                            }},
                            description{"Record-head saturation drive (0 or more). Any value above zero bounds the "
                                        "loop absolutely (output can never exceed 1/drive); 0 is an exactly linear "
                                        "path that leans on darkening alone."}};

    attribute<number> input{this, "input", 1.0, setter{MIN_FUNCTION{
                                const double v = static_cast<double>(args[0]);
                                m_machine.set_input_level(v);
                                return {v};
                            }},
                            description{"The send fader: input level into the record head, linear, slewed. Fading "
                                        "this to zero while the loop sustains is the Discreet Music performance "
                                        "move — the piece continues without you."}};

    attribute<number> mix{this, "mix", kernel::k_default_mix, setter{MIN_FUNCTION{
                              const double v = std::clamp(static_cast<double>(args[0]), 0.0, 100.0);
                              m_machine.set_mix(v);
                              return {v};
                          }},
                          description{"Dry/wet mix in percent (0..100), equal-power. 0 is bitwise dry, 100 bitwise "
                                      "the playback machine."}};

    attribute<std::vector<number>> wow{this,
                                       "wow",
                                       {kernel::k_default_wow_ms, kernel::k_default_wow_hz},
                                       setter{MIN_FUNCTION{
                                           const double depth =
                                               (!args.empty()) ? std::max(0.0, static_cast<double>(args[0])) : 0.0;
                                           const double rate = (args.size() > 1) ? static_cast<double>(args[1]) : 0.0;
                                           m_machine.set_wow(depth, rate);
                                           return {depth, rate};
                                       }},
                                       description{"Wow as a depth/rate pair: excursion in ms of tape position, "
                                                   "rate in Hz (up to 5). Deterministic and periodic; 0 0 parks "
                                                   "the transport."}};

    attribute<std::vector<number>> flutter{this,
                                           "flutter",
                                           {kernel::k_default_flutter_ms, kernel::k_default_flutter_hz},
                                           setter{MIN_FUNCTION{
                                               const double depth =
                                                   (!args.empty()) ? std::max(0.0, static_cast<double>(args[0])) : 0.0;
                                               const double rate =
                                                   (args.size() > 1) ? static_cast<double>(args[1]) : 0.0;
                                               m_machine.set_flutter(depth, rate);
                                               return {depth, rate};
                                           }},
                                           description{"Flutter as a depth/rate pair — the faster, shallower "
                                                       "partner (rate up to 30 Hz)."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_machine.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for the setters in ms (0 = instant). For loop "
                                         "moves this is the re-spool time, and therefore the depth of the "
                                         "doppler bend."}};

    message<> clear{this, "clear", "Erase the tape and the wear state — the eject button. Parameters are kept.",
                    MIN_FUNCTION{
                        m_machine.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup",
                       "Re-prepare only when the sample rate or the tape buy changed — a DSP chain restart must "
                       "not erase a sustaining loop.",
                       MIN_FUNCTION{
                           if (m_geometry_dirty || !m_machine.prepared() || samplerate() != m_machine.samplerate()) {
                               m_machine.prepare(samplerate(), maxloop);
                               m_geometry_dirty = false;
                           }
                           return {};
                       }};

    discreet(const atoms& args = {}) {
        if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
            maxloop = args[0];
        }
        m_machine.prepare(samplerate(), maxloop);
        m_geometry_dirty = false;
    }

    sample operator()(sample x) { return m_machine.process(x); }
};

MIN_EXTERNAL(discreet);
