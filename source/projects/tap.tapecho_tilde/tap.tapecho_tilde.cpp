/// @file
/// @brief      tap.tapecho~ — the multi-head tape echo on the portable kernel.
/// @details    A Copicat / Space Echo-school tape echo wrapping tap::tools::tapecho::machine
///             (taptools/tapecho.h): one record head, a span of moving tape, up to four playback
///             heads at settable positions along it, and a regeneration path from the heads back
///             to the record head. @span is the motor — the delay of a head at the far end of the
///             path — and each head sits at @span times its @ratios entry, so moving @span moves
///             the whole layout together, as a tape speed does (and bends pitch while it moves:
///             that is what varispeed is; @smooth sets how fast).
///
///             It shares its tape machinery with tap.discreet~ (taptools/tape_loop.h) and carries
///             that object's inversion one step further. tap.delay~ caps feedback at 0.99;
///             tap.discreet~ lets regeneration reach exactly 1.0 because the wear path is the
///             stabilizer; here @regen goes *past* 1.0 into deliberate sound-on-sound
///             self-oscillation — the reason anyone reaches for this machine live — bounded by
///             the @drive saturator rather than by a gain cap. Because that bound only exists
///             while the saturator is engaged, the effective regeneration is capped back to 1.0
///             per sample whenever @drive is 0; the attribute keeps its value and it takes effect
///             again when drive returns.
///
///             The head layout defaults to four evenly spaced heads (0.25, 0.5, 0.75, 1.0 of the
///             span). That spacing is nominal, not measured from any unit — set @ratios for a
///             three-head Copicat-style layout or anything else. @levels doubles as the head
///             selector: a head's level is also its send into the regeneration path, as it is on
///             the machines.
///
///             The optional argument sets the worst-case tape buy in seconds (default 4 —
///             ~1.5 MB of double tape at 48 kHz). The tape survives DSP chain restarts: the
///             wrapper re-prepares (which erases the tape) only when the sample rate or the buy
///             actually changes. `clear` is the eject button, and the fastest way to stop a
///             self-oscillating loop.
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing. For
///             multichannel use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/tapecho.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::tapecho;

class tapecho : public object<tapecho>, public sample_operator<1, 2> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::machine m_machine;
    bool            m_geometry_dirty{false};

  public:
    MIN_DESCRIPTION{"A multi-head tape echo. One record head, a span of moving tape, and up to "
                    "four playback heads at settable positions along it, summed to stereo. span "
                    "is the motor (the delay of a head at the far end of the path) and moves "
                    "every head together, bending pitch as it goes — honest varispeed. "
                    "Regeneration may pass 1.0 into deliberate self-oscillation, bounded by the "
                    "drive saturator rather than a feedback cap, and capped back to 1.0 whenever "
                    "drive is 0. Shares its tape machinery with tap.discreet~. The optional "
                    "argument sets the maximum span in seconds (the tape bought at DSP start). "
                    "For multichannel signals, wrap this object in an mc. operator."};
    MIN_TAGS{"delays"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.delay~, tap.multitap~, tap.discreet~, tap.comb~"};

    inlet<>  m_in{this, "(signal) audio input — the send into the record head"};
    outlet<> m_out_left{this, "(signal) the head sum plus dry, left", "signal"};
    outlet<> m_out_right{this, "(signal) the head sum plus dry, right", "signal"};

    attribute<number> maxspan{
        this, "maxspan", kernel::k_default_max_seconds, setter{MIN_FUNCTION{
            const double v   = std::max(kernel::k_min_span_ms * 0.001, static_cast<double>(args[0]));
            m_geometry_dirty = true; // re-bought (and the tape erased) when the DSP chain starts
            return {v};
        }},
        description{"Worst-case tape span in seconds (set by the first object argument; the "
                    "tape is bought at DSP start and bounds the span attribute). Changing it "
                    "erases the tape when the DSP chain restarts."}};

    attribute<number> span{this, "span", kernel::k_default_span_ms, setter{MIN_FUNCTION{
                               const double v = std::max(kernel::k_min_span_ms, static_cast<double>(args[0]));
                               m_machine.set_span_ms(v);
                               return {v};
                           }},
                           description{"The motor: the delay in ms of a head at the far end of the path (ratio 1.0), "
                                       "so every head moves with it. Slewed — and the slew is honest tape-speed "
                                       "doppler: the pitch bends while the motor changes speed. smooth sets how "
                                       "long that takes."}};

    attribute<int> heads{this, "heads", kernel::k_max_heads, setter{MIN_FUNCTION{
                             const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_max_heads);
                             m_machine.set_heads(v);
                             return {v};
                         }},
                         description{"Number of active playback heads (0 to 4). A mute, not a freeze: inactive heads "
                                     "keep ramping, so bringing one back mid-glide does not jump."}};

    attribute<std::vector<number>> ratios{
        this,
        "ratios",
        {0.25, 0.5, 0.75, 1.0},
        setter{MIN_FUNCTION{
            for (size_t i = 0; i < args.size() && i < static_cast<size_t>(kernel::k_max_heads); ++i) {
                m_machine.set_head_ratio(static_cast<int>(i), static_cast<double>(args[i]));
            }
            return args;
        }},
        description{"Per-head position along the tape path as a fraction of span, 0 to 1, slewed. The default is "
                    "four evenly spaced heads — a nominal layout, not measured from any machine. 0.333 0.667 1. "
                    "with heads 3 is a Copicat-style three."}};

    attribute<std::vector<number>> levels{
        this,
        "levels",
        {1.0},
        setter{MIN_FUNCTION{
            for (size_t i = 0; i < args.size() && i < static_cast<size_t>(kernel::k_max_heads); ++i) {
                m_machine.set_head_level(static_cast<int>(i), static_cast<double>(args[i]));
            }
            return args;
        }},
        description{"Per-head linear level, slewed. Unclamped (negative flips polarity, like a mixer). This is also "
                    "the head selector: a head's level is its send into the regeneration path too."}};

    attribute<std::vector<number>> pans{
        this,
        "pans",
        {0.0},
        setter{MIN_FUNCTION{
            for (size_t i = 0; i < args.size() && i < static_cast<size_t>(kernel::k_max_heads); ++i) {
                m_machine.set_head_pan(static_cast<int>(i), static_cast<double>(args[i]));
            }
            return args;
        }},
        description{"Per-head equal-power pan, -1 (hard left) to 1 (hard right), slewed. Endpoints are exact: a "
                    "hard-panned head is bitwise absent from the far bus."}};

    attribute<number> regen{this, "regen", kernel::k_default_regen, setter{MIN_FUNCTION{
                                const double v =
                                    std::clamp(static_cast<double>(args[0]), 0.0, kernel::k_regen_max_driven);
                                m_machine.set_regen(v);
                                return {v};
                            }},
                            description{"Regeneration into the record head (0 to 1.5). Past 1.0 the loop "
                                        "self-oscillates — bounded by the drive saturator, not by a gain cap — and "
                                        "values above 1.0 only take effect while drive is above zero."}};

    attribute<number> darken{this, "darken", kernel::k_default_darken_hz, setter{MIN_FUNCTION{
                                 const double v =
                                     std::clamp(static_cast<double>(args[0]), tap::tools::tape::k_darken_floor_hz,
                                                tap::tools::tape::k_darken_ceil_hz);
                                 m_machine.set_darken_hz(v);
                                 return {v};
                             }},
                             description{"Per-pass darkening corner in Hz (20..20000). Every trip through the "
                                         "regeneration path runs through this one-pole lowpass: the repeats lose "
                                         "treble first. Riding it while the loop howls is a performance control."}};

    attribute<number> drive{this, "drive", kernel::k_default_drive, setter{MIN_FUNCTION{
                                const double v = std::max(0.0, static_cast<double>(args[0]));
                                m_machine.set_drive(v);
                                return {v};
                            }},
                            description{"Record-head saturation drive (0 or more). Any value above zero bounds the "
                                        "loop absolutely (the returned signal can never exceed 1/drive) and is what "
                                        "makes regen above 1.0 safe; 0 is exactly linear and caps the effective "
                                        "regen back to 1.0."}};

    attribute<number> input{this, "input", 1.0, setter{MIN_FUNCTION{
                                const double v = static_cast<double>(args[0]);
                                m_machine.set_input_level(v);
                                return {v};
                            }},
                            description{"Input level into the record head, linear, slewed. Fading this to zero while "
                                        "the loop self-oscillates is the sound-on-sound move: the machine keeps "
                                        "playing what you already gave it."}};

    attribute<number> mix{this, "mix", kernel::k_default_mix, setter{MIN_FUNCTION{
                              const double v = std::clamp(static_cast<double>(args[0]), 0.0, 100.0);
                              m_machine.set_mix(v);
                              return {v};
                          }},
                          description{"Dry/wet mix in percent (0..100), equal-power. 0 is bitwise dry on both "
                                      "outlets, 100 bitwise the head sum."}};

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
                                       description{"Wow as a depth/rate pair: excursion in ms of tape position, rate "
                                                   "in Hz (up to 5). One motor moves the whole path, so every head "
                                                   "is displaced together. Deterministic and periodic; 0 0 parks "
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
                                           description{"Flutter as a depth/rate pair — the faster, shallower partner "
                                                       "(rate up to 30 Hz)."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_machine.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for the setters in ms (0 = instant). For span moves "
                                         "this is how long the motor takes to change speed, and therefore how deep "
                                         "the doppler bend is."}};

    message<> clear{this, "clear",
                    "Erase the tape and the transport/wear state — the eject button, and the fastest way to stop a "
                    "self-oscillating loop. Parameters are kept.",
                    MIN_FUNCTION{
                        m_machine.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup",
                       "Re-prepare only when the sample rate or the tape buy changed — a DSP chain restart must "
                       "not erase a sustaining loop.",
                       MIN_FUNCTION{
                           if (m_geometry_dirty || !m_machine.prepared() || samplerate() != m_machine.samplerate()) {
                               m_machine.prepare(samplerate(), maxspan);
                               m_geometry_dirty = false;
                           }
                           return {};
                       }};

    tapecho(const atoms& args = {}) {
        if (!args.empty() && static_cast<double>(args[0]) > 0.0) {
            maxspan = args[0];
        }
        m_machine.prepare(samplerate(), maxspan);
        m_geometry_dirty = false;
    }

    samples<2> operator()(sample x) {
        double left  = 0.0;
        double right = 0.0;
        m_machine.process(x, left, right);
        return {left, right};
    }
};

MIN_EXTERNAL(tapecho);
