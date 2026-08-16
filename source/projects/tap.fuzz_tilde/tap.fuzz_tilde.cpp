/// @file
/// @brief      tap.fuzz~ — the two-stage tone-stacked fuzz on the portable kernel.
/// @details    Wraps tap::tools::fuzz::pedal (taptools/fuzz.h): two cascaded clipping stages and
///             a @bass / @contrast / @treble voicing section — the harder, more scooped school
///             that sits beside tap.overdrive~ rather than replacing it (that object is a
///             feedback soft-clipper chasing the TS lineage).
///
///             The architecture is the simplified cascade of Yeh, Abel & Smith (DAFx-07):
///             conditioning filter, memoryless nonlinearity, equalization filter — twice. @gain
///             sweeps the first stage's drive, @edge sharpens the second stage's knee from a
///             soft limiter toward a hard corner, and @asymmetry buys the even harmonics an
///             odd-only curve structurally cannot make. It is a recreation of a circuit *class*:
///             no component value or corner here is claimed as measured from any pedal, and the
///             control names follow that class's conventional layout rather than asserting what
///             any particular unit does.
///
///             @oversample is 1, 2, 4 or 8 and defaults to **2** — which is not a typo and not
///             laziness. Measured through this kernel, every factor beats no oversampling by
///             orders of magnitude, but the sequence is not monotone and 2 comes out best; the
///             kernel header carries the numbers and what is and is not known about why. Reach
///             for a higher factor only if a specific patch measures better there.
///
///             All DSP lives in the Min-free kernel; this file is only the Max plumbing. For
///             multichannel use, wrap the object in an mc. operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/fuzz.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::fuzz;

class fuzz : public object<fuzz>, public sample_operator<1, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::pedal m_pedal;

  public:
    MIN_DESCRIPTION{"A two-stage, tone-stacked fuzz. Two cascaded clipping stages with a bass / "
                    "contrast / treble voicing section — the harder, more scooped school beside "
                    "tap.overdrive~'s feedback soft-clipper. gain sweeps the first stage's "
                    "drive, edge sharpens the second stage's knee toward a hard corner, and "
                    "asymmetry brings in the even harmonics a symmetric curve cannot make. Built "
                    "on the Yeh/Abel/Smith DAFx-07 simplified cascade; a recreation of a circuit "
                    "class, not a model of any one pedal. For multichannel signals, wrap this "
                    "object in an mc. operator."};
    MIN_TAGS{"effects"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.overdrive~, tap.tapecho~, tap.filter~, overdrive~"};

    inlet<>  m_in{this, "(signal) audio input"};
    outlet<> m_out{this, "(signal) the distorted output", "signal"};

    attribute<number> gain{this, "gain", 0.5, setter{MIN_FUNCTION{
                               const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                               m_pedal.set_gain(v);
                               return {v};
                           }},
                           description{"Drive into the first clipping stage (0..1). 0 is a lit-up clean boost, 1 is "
                                       "the pedal wide open. The floor sits below unity on purpose — the second "
                                       "stage carries its own small-signal gain, and a hotter floor would arrive "
                                       "there already saturated."}};

    attribute<number> edge{this, "edge", 0.5, setter{MIN_FUNCTION{
                               const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                               m_pedal.set_edge(v);
                               return {v};
                           }},
                           description{"Knee sharpness of the second stage (0..1): a soft limiter at 0, close to a "
                                       "hard corner at 1. High settings are where a static curve aliases worst — if "
                                       "it sounds gritty in the wrong way, try oversample before the tone controls."}};

    attribute<number> asymmetry{this, "asymmetry", 0.0, setter{MIN_FUNCTION{
                                    const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                    m_pedal.set_asymmetry(v);
                                    return {v};
                                }},
                                description{"Clipping asymmetry (0..1) — the even-harmonic control. A symmetric "
                                            "curve is an odd function and produces odd harmonics only; a real "
                                            "op-amp stage clips lopsided, which is where a pedal's even harmonics "
                                            "come from. Costs no DC: silence stays exactly silent."}};

    attribute<number> bass{this, "bass", 0.0, setter{MIN_FUNCTION{
                               const double v = std::clamp(static_cast<double>(args[0]), -1.0, 1.0);
                               m_pedal.set_bass(v);
                               return {v};
                           }},
                           description{"Low shelf, -1 to 1 (full travel is 12 dB either way). Linear, and entirely "
                                       "outside the nonlinearity."}};

    attribute<number> treble{this, "treble", 0.0, setter{MIN_FUNCTION{
                                 const double v = std::clamp(static_cast<double>(args[0]), -1.0, 1.0);
                                 m_pedal.set_treble(v);
                                 return {v};
                             }},
                             description{"High shelf, -1 to 1 (full travel is 12 dB either way)."}};

    attribute<number> contrast{this, "contrast", 0.35, setter{MIN_FUNCTION{
                                   const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                   m_pedal.set_contrast(v);
                                   return {v};
                               }},
                               description{"Mid-scoop depth (0..1, up to 14 dB at 620 Hz). The scoop is this "
                                           "object's own curve; the control name is the pedal class's."}};

    attribute<number> level{this, "level", 0.0, setter{MIN_FUNCTION{
                                const double v = std::clamp(static_cast<double>(args[0]), -24.0, 24.0);
                                m_pedal.set_level_db(v);
                                return {v};
                            }},
                            description{"Output level in dB (-24..24)."}};

    attribute<int> oversample{this, "oversample", 2, setter{MIN_FUNCTION{
                                  const int in = static_cast<int>(args[0]);
                                  const int v  = (in >= 8) ? 8 : (in >= 4) ? 4 : (in >= 2) ? 2 : 1;
                                  m_pedal.set_oversample(v);
                                  return {v};
                              }},
                              description{"Oversampling factor for the clipper pair: 1, 2, 4 or 8. Default 2, which "
                                          "measures best — every factor beats 1 by orders of magnitude, but the "
                                          "sequence is not monotone here, so bigger is not automatically better. "
                                          "Changing this reconfigures the filters and is not real-time-safe."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{MIN_FUNCTION{
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_pedal.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper ramp time for the setters, in ms (0 = instant)."}};

    message<> clear{this, "clear", "Flush the filters and the oversampling chain. Parameters are kept.",
                    MIN_FUNCTION{
                        m_pedal.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Prepare the pedal for the current sample rate.",
                       MIN_FUNCTION{
                           if (!m_pedal.prepared() || samplerate() != m_pedal.samplerate()) {
                               m_pedal.prepare(samplerate());
                           }
                           return {};
                       }};

    fuzz(const atoms& args = {}) { m_pedal.prepare(samplerate()); }

    sample operator()(sample x) { return m_pedal.process(x); }
};

MIN_EXTERNAL(fuzz);
