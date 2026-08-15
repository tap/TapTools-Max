/// @file
/// @brief      tap.chime~ — the wind-chime rack tap.garden~ sounds its blooms on.
/// @details    A polyphonic rack of sixteen small wind chimes wrapping tap::tools::garden::rack
///             (taptools/garden.h). Each strike is four decaying mode doublets at the transverse-
///             vibration ratios of the chosen material — the free-free tube's 1 : 2.756 : 5.404 :
///             8.933 or the tuned bar's double-octave 1 : 4 : 10 : 20 (Fletcher & Rossing, The
///             Physics of Musical Instruments, 2nd ed.) — with the upper modes softer, faster-
///             dying, and scattered a fixed few cents by the struck tube's own identity. Ring time
///             scales with sqrt(440/f), so small high tubes ring shorter.
///
///             The rack is a rack: each tube hangs at a fixed seat in the stereo field drawn from
///             a stateless hash of its pitch, so the same note always rings from the same place
///             with the same flaws, in every instance of the object, forever. @spread scales the
///             width; 0 collapses it to bitwise mono.
///
///             Polyphony is the object's own, deliberately, rather than something you wrap in
///             `poly~`: an idle bell is used if there is one, otherwise the QUIETEST ringing bell
///             is stolen and RE-AIMED — its phases keep free-running and its seat glides — so a
///             steal sounds like a chime being struck again, not like a voice being cut. `poly~`
///             steals round-robin and cannot do that, and the kernel has to run off Max too.
///             Sixteen bells is also a hard bound on the audio: however fast you strike it, that
///             is all that can ever be ringing.
///
///             Drive it from `tap.bloom` and you have the garden's voice; drive it from `makenote`
///             or a sequencer and it is simply a chime rack. All DSP lives in the Min-free kernel;
///             this file is only the Max plumbing.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/garden.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::garden;

class chime : public object<chime>, public sample_operator<0, 2> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::rack m_rack;

  public:
    MIN_DESCRIPTION{"A rack of sixteen wind chimes. Each strike is four decaying mode doublets at "
                    "the transverse ratios of the chosen material, with the upper modes softer, "
                    "faster-dying, and scattered a few cents by the struck tube's own identity — "
                    "so a given note always rings from the same seat in the stereo field with the "
                    "same flaws. Voice stealing is the object's own and takes the QUIETEST bell, "
                    "re-aiming it rather than cutting it, so a steal glides. Strike it with "
                    "note <pitch> <velocity> [brightness], or drive it from tap.bloom to get "
                    "tap.garden~'s voice."};
    MIN_TAGS{"synthesis"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.garden~, tap.bloom, tap.808.cowbell~, tap.808.cymbal~"};

    inlet<>  m_in{this, "(list) note <pitch> <velocity> [brightness] — strike a tube"};
    outlet<> m_out_left{this, "(signal) the rack, left", "signal"};
    outlet<> m_out_right{this, "(signal) the rack, right", "signal"};
    outlet<> m_out_info{this, "(list) reports: voices <n>"};

    attribute<number> attack{this, "attack", kernel::k_default_attack_s, setter{MIN_FUNCTION{
                                 const double v = std::max(1e-6, static_cast<double>(args[0]));
                                 m_rack.set_times(v, m_rack.decay_s());
                                 return {v};
                             }},
                             description{"Strike attack time in SECONDS — a clapper, not a bow. Applies to future "
                                         "strikes; a ringing chime keeps its envelope until retriggered."}};

    attribute<number> decay{this, "decay", kernel::k_default_decay_s, setter{MIN_FUNCTION{
                                const double v = std::max(1e-6, static_cast<double>(args[0]));
                                m_rack.set_times(m_rack.attack_s(), v);
                                return {v};
                            }},
                            description{"Ring time in SECONDS for the fundamental. The upper modes die much faster "
                                        "(radiation damping), and each strike scales this by sqrt(440/f) — small "
                                        "high tubes ring shorter."}};

    attribute<int> material{this,
                            "material",
                            kernel::material_chime,
                            setter{MIN_FUNCTION{
                                const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_materials - 1);
                                m_rack.set_material(v);
                                return {v};
                            }},
                            range{0, kernel::k_num_materials - 1},
                            description{"What the tubes are made of: 0 the free-free chime tube (1 : 2.756 : 5.404 "
                                        ": 8.933), 1 the tuned bar (double octaves, 1 : 4 : 10 : 20). A mode, not "
                                        "a fader — read at strike time, so every later strike re-voices."}};

    attribute<number> spread{this, "spread", kernel::k_default_spread, setter{MIN_FUNCTION{
                                 const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                 m_rack.set_spread(v);
                                 return {v};
                             }},
                             description{"The rack's stereo width, 0 to 1. Each tube's seat comes from its pitch, "
                                         "so the image is fixed rather than moving; 0 collapses the rack to centre "
                                         "mono, bitwise equal on both busses."}};

    attribute<number> brightness{
        this, "brightness", kernel::k_default_brightness,
        setter{MIN_FUNCTION{ return {std::clamp(static_cast<double>(args[0]), 0.0, 1.0)}; }},
        description{"Default upper-mode weight (0..1) for strikes that do not carry their "
                    "own. A soft strike is duller regardless: effective brightness scales "
                    "with velocity."}};

    message<> note{this, "note",
                   "note <pitch> <velocity> [brightness]: strike the tube at that MIDI pitch (fractional "
                   "accepted — fractional pitches are distinct tubes with their own flaws and their own seat). "
                   "Velocity 0..1; brightness defaults to the attribute.",
                   MIN_FUNCTION{
                       if (args.size() >= 2 && static_cast<double>(args[1]) > 0.0) {
                           const double b =
                               (args.size() >= 3) ? static_cast<double>(args[2]) : static_cast<double>(brightness);
                           m_rack.strike(args[0], args[1], b);
                       }
                       return {};
                   }};

    message<> hz{
        this, "hz", "hz <frequency> <velocity> [brightness]: strike at a raw frequency rather than a MIDI pitch.",
        MIN_FUNCTION{
            if (args.size() >= 2 && static_cast<double>(args[0]) > 0.0 && static_cast<double>(args[1]) > 0.0) {
                const double b = (args.size() >= 3) ? static_cast<double>(args[2]) : static_cast<double>(brightness);
                m_rack.strike_hz(args[0], args[1], b);
            }
            return {};
        }};

    message<> voices{this, "voices", "Report how many bells are currently ringing, out the right outlet.",
                     MIN_FUNCTION{
                         m_out_info.send("voices", m_rack.active_voices());
                         return {};
                     }};

    message<> clear{this, "clear", "Silence every bell at once. Parameters are kept.",
                    MIN_FUNCTION{
                        m_rack.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Re-prepare the rack when the sample rate changes.",
                       MIN_FUNCTION{
                           m_rack.prepare(samplerate());
                           return {};
                       }};

    chime(const atoms& args = {}) {
        if (!args.empty()) {
            material = args[0];
        }
        m_rack.prepare(samplerate());
    }

    samples<2> operator()() {
        double left  = 0.0;
        double right = 0.0;
        m_rack.process(left, right); // the kernel accumulates, so start from zero
        return {left, right};
    }
};

MIN_EXTERNAL(chime);
