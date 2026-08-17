/// @file
/// @brief      tap.triode~ — one triode gain stage, from a published tube model.
/// @details    Wraps tap::tools::ondes::triode (taptools/ondes.h). A common-cathode valve stage,
///             solved on its load line: conditioning highpass, drive into the grid, the tube's own
///             transfer curve, equalization lowpass. The architecture is Yeh, Abel & Smith's
///             DAFx-07 simplified cascade, the same one <o>tap.fuzz~</o> uses — but where that
///             object reaches for a tanh, this one solves an actual valve.
///
///             **Almost nothing here is voiced by ear.** The tube is the enhanced Norman Koren
///             model (Koren, Glass Audio 8(5), 1996; grid-current branch from Cohen & Hélie, AES
///             129, 2010), and the parameter sets are fitted to the valves in ondes Martenot
///             No. 169 by Najnudel, Hélie, Roze & Boutin (IEEE/ACM TASLP 28, 2020, Table II),
///             along with the supply voltage, cathode resistor and plate load of every stage.
///             @tube picks the valve; @stage picks one of the three published operating points.
///
///             Two properties worth knowing before patching it. The stage **inverts**, because a
///             real common-cathode stage does — and that is not cosmetic here, since the valve's
///             asymmetry acts on whichever side of the waveform reaches its grid. And it is
///             **strongly asymmetric**: at the demodulator's operating point, equal grid swings
///             either way give plate swings in a 2.17 : 1 ratio, which is where a triode's even
///             harmonics come from.
///
///             @drive is normalized out of the level, so turning it up changes the distortion and
///             not the volume. @tube and @stage rebuild the stage's curve table, so they are
///             modes to set rather than knobs to sweep at audio rate.
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing. For
///             multichannel use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/ondes.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::ondes;

class triode : public object<triode>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::triode m_stage;

    static kernel::operating_point op_for(int stage) {
        switch (stage) {
        case 1:
            return kernel::k_op_preamp;
        case 2:
            return kernel::k_op_power;
        default:
            return kernel::k_op_demod;
        }
    }

  public:
    MIN_DESCRIPTION{"One triode gain stage, solved on its load line from a published tube model. "
                    "The enhanced Norman Koren law with parameter sets fitted to the valves in "
                    "ondes Martenot No. 169 (6F5, 6C5, 2A3) and their published operating points "
                    "— supply, cathode resistor and plate load. The stage inverts and is strongly "
                    "asymmetric, which is where a triode's even harmonics come from. drive is "
                    "normalized out of the level, so it changes the distortion rather than the "
                    "volume. For multichannel signals, wrap this object in an mc. operator."};
    MIN_TAGS{"effects"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.ondes~, tap.fuzz~, tap.overdrive~, tap.vca~"};

    inlet<>  m_in{this, "(signal) audio input — the grid"};
    outlet<> m_out{this, "(signal) the plate, inverted and normalized", "signal"};

    attribute<int> tube{this, "tube", kernel::tube_6c5, setter{MIN_FUNCTION{
                            const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_tubes - 1);
                            m_stage.set_tube(v);
                            return {v};
                        }},
                        description{"Which valve: 0 the 6F5 (the oscillators'), 1 the 6C5 (the demodulator's and "
                                    "preamplifier's), 2 the 2A3 (the power amplifier's). A mode, not a fader — it "
                                    "rebuilds the stage's curve table, which is sub-millisecond but not something "
                                    "to automate at audio rate."}};

    attribute<int> stage{this, "stage", 0, setter{MIN_FUNCTION{
                             const int v = std::clamp(static_cast<int>(args[0]), 0, 2);
                             m_stage.set_operating_point(op_for(v));
                             return {v};
                         }},
                         description{"Which published operating point: 0 demodulator (100 V supply, 1 k cathode, "
                                     "4 k plate load), 1 preamplifier (180 V), 2 power amplifier (230 V, 750 R, "
                                     "1.5 k). Also a mode. The pairings the instrument uses are tube 1 with stages "
                                     "0 and 1, and tube 2 with stage 2 — but any combination will solve, and some "
                                     "of the wrong ones are interesting."}};

    attribute<number> drive{this, "drive", kernel::k_default_drive_v, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), kernel::k_min_drive_v,
                                                            kernel::k_max_drive_v);
                                m_stage.set_drive(v);
                                return {v};
                            }},
                            description{"Peak grid volts for a full-scale input. This is the distortion control, "
                                        "and it is normalized out of the level: the small-signal gain stays at "
                                        "unity whatever you set, so turning it up gets dirtier rather than "
                                        "louder."}};

    attribute<number> highpass{this, "highpass", 20.0, setter{MIN_FUNCTION{
                                   const double v = std::max(0.0, static_cast<double>(args[0]));
                                   m_stage.set_corners(v, static_cast<double>(lowpass));
                                   return {v};
                               }},
                               description{"Conditioning highpass in Hz — the coupling capacitor and grid leak. "
                                           "The first filter of the DAFx-07 cascade."}};

    attribute<number> lowpass{this, "lowpass", 12000.0, setter{MIN_FUNCTION{
                                  const double v = std::max(1.0, static_cast<double>(args[0]));
                                  m_stage.set_corners(static_cast<double>(highpass), v);
                                  return {v};
                              }},
                              description{"Equalization lowpass in Hz — Miller capacitance and the plate load's "
                                          "own pole. The last filter of the cascade."}};

    message<> clear{this, "clear", "Reset the stage's filters. Parameters are kept.",
                    MIN_FUNCTION{
                        m_stage.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Prepare the stage for the current sample rate.",
                       MIN_FUNCTION{
                           m_stage.prepare(samplerate());
                           m_stage.set_tube(static_cast<int>(tube));
                           m_stage.set_operating_point(op_for(static_cast<int>(stage)));
                           m_stage.set_drive(static_cast<double>(drive));
                           m_stage.set_corners(static_cast<double>(highpass), static_cast<double>(lowpass));
                           return {};
                       }};

    triode(const atoms& args = {}) { m_stage.prepare(samplerate()); }

    sample operator()(sample x) { return m_stage.process(x); }
};

MIN_EXTERNAL(triode);
