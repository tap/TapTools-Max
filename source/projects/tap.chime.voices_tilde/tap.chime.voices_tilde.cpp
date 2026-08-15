/// @file
/// @brief      tap.chime.voices~ — the wind-chime rack with every bell on its own outlet.
/// @details    The same tap::tools::garden::rack that tap.chime~ wraps, tapped per voice instead
///             of summed to stereo: sixteen mono signal outlets, one per bell, carrying each tube
///             RAW — before its seat in the stereo image is applied. Put them back through the
///             seats (which `voice` reports) and you have tap.chime~ again to the bit, pinned in
///             the kernel by "the per-voice taps summed through their seats are the stereo rack".
///             The point, of course, is that you do not have to: place, filter, gate, or send each
///             tube wherever you like.
///
///             Why a separate object rather than a mode on tap.chime~: outlet count is fixed when
///             a Min object is constructed, so it cannot be switched at runtime. And why sixteen
///             discrete outlets rather than one multichannel outlet: min-api's mc support is
///             inlet-side only — it sets Z_MC_INLETS and provides no multichanneloutputs, which is
///             the method Max requires before an external may declare a variable-channel mc outlet.
///             If that lands in a later Min, this object is the natural place to adopt it.
///
///             The pool reassigns bells as it steals, so slot 3 is whatever tube was last put
///             there, not a fixed pitch. `voice <n>` reports which tube a slot is holding, how
///             loudly, and the seat gains tap.chime~ would have used — enough to follow a strike
///             to its outlet, or to rebuild the rack image downstream.
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/garden.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::garden;

class chimevoices : public object<chimevoices>, public sample_operator<0, kernel::k_voices> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::rack m_rack;

  public:
    MIN_DESCRIPTION{"The tap.chime~ rack with every bell on its own outlet: sixteen mono signals, "
                    "one per voice, each carrying its tube dry — before the seat in the stereo "
                    "image is applied. Place, filter, or gate each one yourself. The pool "
                    "reassigns bells as it steals, so a slot holds whatever tube was last put "
                    "there; voice <n> reports which, how loudly, and the seat gains tap.chime~ "
                    "would have used."};
    MIN_TAGS{"synthesis"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.chime~, tap.garden~, tap.bloom, poly~"};

    inlet<>  m_in{this, "(list) note <pitch> <velocity> [brightness] — strike a tube"};
    outlet<> m_out_0{this, "(signal) voice 0 — the bell in pool slot 0, dry", "signal"};
    outlet<> m_out_1{this, "(signal) voice 1 — the bell in pool slot 1, dry", "signal"};
    outlet<> m_out_2{this, "(signal) voice 2 — the bell in pool slot 2, dry", "signal"};
    outlet<> m_out_3{this, "(signal) voice 3 — the bell in pool slot 3, dry", "signal"};
    outlet<> m_out_4{this, "(signal) voice 4 — the bell in pool slot 4, dry", "signal"};
    outlet<> m_out_5{this, "(signal) voice 5 — the bell in pool slot 5, dry", "signal"};
    outlet<> m_out_6{this, "(signal) voice 6 — the bell in pool slot 6, dry", "signal"};
    outlet<> m_out_7{this, "(signal) voice 7 — the bell in pool slot 7, dry", "signal"};
    outlet<> m_out_8{this, "(signal) voice 8 — the bell in pool slot 8, dry", "signal"};
    outlet<> m_out_9{this, "(signal) voice 9 — the bell in pool slot 9, dry", "signal"};
    outlet<> m_out_10{this, "(signal) voice 10 — the bell in pool slot 10, dry", "signal"};
    outlet<> m_out_11{this, "(signal) voice 11 — the bell in pool slot 11, dry", "signal"};
    outlet<> m_out_12{this, "(signal) voice 12 — the bell in pool slot 12, dry", "signal"};
    outlet<> m_out_13{this, "(signal) voice 13 — the bell in pool slot 13, dry", "signal"};
    outlet<> m_out_14{this, "(signal) voice 14 — the bell in pool slot 14, dry", "signal"};
    outlet<> m_out_15{this, "(signal) voice 15 — the bell in pool slot 15, dry", "signal"};
    outlet<> m_out_info{this, "(list) reports: voices <n>, voice <slot> <hz> <level> <gainL> <gainR>"};

    attribute<number> attack{this, "attack", kernel::k_default_attack_s, setter{MIN_FUNCTION{
                                 const double v = std::max(1e-6, static_cast<double>(args[0]));
                                 m_rack.set_times(v, m_rack.decay_s());
                                 return {v};
                             }},
                             description{"Strike attack time in SECONDS — a clapper, not a bow."}};

    attribute<number> decay{this, "decay", kernel::k_default_decay_s, setter{MIN_FUNCTION{
                                const double v = std::max(1e-6, static_cast<double>(args[0]));
                                m_rack.set_times(m_rack.attack_s(), v);
                                return {v};
                            }},
                            description{"Ring time in SECONDS for the fundamental. Each strike scales this by "
                                        "sqrt(440/f) — small high tubes ring shorter."}};

    attribute<int> material{this,
                            "material",
                            kernel::material_chime,
                            setter{MIN_FUNCTION{
                                const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_materials - 1);
                                m_rack.set_material(v);
                                return {v};
                            }},
                            range{0, kernel::k_num_materials - 1},
                            description{"0 the free-free chime tube, 1 the tuned bar. Read at strike time, so "
                                        "every later strike re-voices."}};

    attribute<number> spread{this, "spread", kernel::k_default_spread, setter{MIN_FUNCTION{
                                 const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                 m_rack.set_spread(v);
                                 return {v};
                             }},
                             description{"The rack's stereo width, 0 to 1. It does NOT affect these outlets, which "
                                         "are always dry — it only sets the seat gains that `voice` reports, so a "
                                         "patch rebuilding the image downstream can honour it."}};

    attribute<number> brightness{
        this, "brightness", kernel::k_default_brightness,
        setter{MIN_FUNCTION{ return {std::clamp(static_cast<double>(args[0]), 0.0, 1.0)}; }},
        description{"Default upper-mode weight (0..1) for strikes that do not carry their own."}};

    message<> note{this, "note",
                   "note <pitch> <velocity> [brightness]: strike the tube at that MIDI pitch. Which outlet it "
                   "lands on is the pool's choice — an idle bell if there is one, else the quietest is stolen.",
                   MIN_FUNCTION{
                       if (args.size() >= 2 && static_cast<double>(args[1]) > 0.0) {
                           const double b =
                               (args.size() >= 3) ? static_cast<double>(args[2]) : static_cast<double>(brightness);
                           m_rack.strike(args[0], args[1], b);
                       }
                       return {};
                   }};

    message<> list{this, "list", "A bare <pitch> <velocity> [brightness] list is the same as note.",
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

    message<> voice{this, "voice",
                    "voice <slot>: report that outlet's tube — hz, level, and the seat gains tap.chime~ would "
                    "have used — out the right outlet. With no argument, reports every slot in turn.",
                    MIN_FUNCTION{
                        if (args.empty()) {
                            for (int i = 0; i < kernel::k_voices; ++i) {
                                report(i);
                            }
                        }
                        else {
                            report(static_cast<int>(args[0]));
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

    chimevoices(const atoms& args = {}) {
        if (!args.empty()) {
            material = args[0];
        }
        m_rack.prepare(samplerate());
    }

    samples<kernel::k_voices> operator()() {
        samples<kernel::k_voices> v{}; // samples<N> is std::array<double, N>, so this is the buffer
        m_rack.process_voices(v.data(), kernel::k_voices);
        return v;
    }

  private:
    void report(int slot) {
        if (slot < 0 || slot >= kernel::k_voices) {
            return;
        }
        m_out_info.send("voice", slot, m_rack.voice_hz(slot), m_rack.voice_level(slot), m_rack.voice_gain_left(slot),
                        m_rack.voice_gain_right(slot));
    }
};

MIN_EXTERNAL(chimevoices);
