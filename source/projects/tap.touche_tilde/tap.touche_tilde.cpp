/// @file
/// @brief      tap.touche~ — the Ondes Martenot intensity key as a gain law.
/// @details    Wraps tap::tools::touche::key (taptools/touche.h). The *touche d'intensité* is
///             the pressure key the Ondes Martenot player's left hand rides — Messiaen called it
///             the instrument's greatest invention — and underneath the mechanism it is a gain
///             curve of unusual quality: 50 dB of range in about 4.5 mm of travel, shaped so a
///             player can place a crescendo where they want it.
///
///             **The curve here is not modelled. It is a published measurement.** Quartier,
///             Meurisse, Colmars, Frelat & Vaiedelich, "Intensity Key of the Ondes Martenot: An
///             Early Mechanical Haptic Device" (Acta Acustica united with Acustica 101(2),
///             421–428, 2015) measured force, displacement and sound simultaneously on
///             instrument No. 320 and published the boundaries of the six musical nuances across
///             the key's travel. This object interpolates those seven points with monotone cubic
///             segments and does not wander between them.
///
///             @position spans the *physical* throw (0 to 9.5 mm), not the measured band inside
///             it — so roughly the first 45 % is silent. That dead zone is the key's own first
///             phase, bending before it reaches the powder bag, and it is why the instrument can
///             be played with such sharp attacks: the useful 50 dB is packed into the 4.5 mm
///             right after it. Drive @position from a signal in the right inlet for a pedal or a
///             sensor, or set it as an attribute.
///
///             It is the first piece of the Ondes Martenot to land, and it is here on its own
///             because a measured expressive gain law is useful on anything — try it as a
///             swell pedal on a guitar, or as the VCA at the end of any chain.
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing. For
///             multichannel use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/touche.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::touche;

class touche : public object<touche>, public sample_operator<2, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::key m_key;

  public:
    MIN_DESCRIPTION{"The Ondes Martenot intensity key as a gain law. Not a model — the published "
                    "measurement (Quartier et al., Acta Acustica 2015): 50 dB of range across "
                    "4.5 mm of key travel, interpolated through all seven measured points. "
                    "position spans the physical 9.5 mm throw, so the bottom 45% is silent, "
                    "which is the key's own first phase before it reaches the powder bag. Drive "
                    "position from a signal in the right inlet for a pedal or sensor. Useful on "
                    "anything that wants an expressive swell, not just on an ondes. For "
                    "multichannel signals, wrap this object in an mc. operator."};
    MIN_TAGS{"effects"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.vca~, tap.adsr~, tap.elixir~, gain~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_pos{this, "(signal) key position, 0 to 1 of the travel"};
    outlet<> m_out{this, "(signal) the input, scaled by the key", "signal"};

    attribute<number> position{this, "position", 0.0, setter{MIN_FUNCTION{
                                   const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                   m_key.set_position(v);
                                   return {v};
                               }},
                               description{"Key position, 0 to 1 over the physical 9.5 mm travel. Roughly the first "
                                           "45% is silent — that is the key bending before it compresses the powder "
                                           "bag, not a dead spot in the object. Ignored while a signal is connected "
                                           "to the right inlet."}};

    attribute<number> mm{this, "mm", 0.0, setter{MIN_FUNCTION{
                             const double v = std::max(0.0, static_cast<double>(args[0]));
                             m_key.set_position_mm(v);
                             return {v};
                         }},
                         description{"The same control in the published unit: millimetres of key travel. The "
                                     "measured band runs 4.3 mm (the instrument's noise floor) to 8.8 mm (full "
                                     "press)."}};

    attribute<int> mode{this, "mode", 0, setter{MIN_FUNCTION{
                            const int v = (static_cast<int>(args[0]) == 1) ? 1 : 0;
                            m_key.set_mode(v);
                            return {v};
                        }},
                        description{"Which measured column drives the gain: 0 displacement (the primary map, and "
                                    "what the paper says the sound actually depends on), 1 finger force. The force "
                                    "axis is far from uniform — six of the seven measured points sit below 1.4 N "
                                    "and the seventh is at 9.6 N — so it is offered for completeness rather than "
                                    "as a good controller."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_key.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for attribute-driven position moves, in ms (0 = "
                                         "instant). The signal inlet is not ramped — a control signal is assumed "
                                         "smooth already."}};

    message<> clear{this, "clear", "Return the key to rest (silent).",
                    MIN_FUNCTION{
                        m_key.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Prepare the key for the current sample rate.",
                       MIN_FUNCTION{
                           m_key.prepare(samplerate());
                           return {};
                       }};

    touche(const atoms& args = {}) { m_key.prepare(samplerate()); }

    sample operator()(sample x, sample pos) {
        // A signal on the right inlet drives the key directly; otherwise the attribute does.
        if (m_in_pos.has_signal_connection()) {
            return m_key.process(x, pos);
        }
        return m_key.process(x);
    }
};

MIN_EXTERNAL(touche);
