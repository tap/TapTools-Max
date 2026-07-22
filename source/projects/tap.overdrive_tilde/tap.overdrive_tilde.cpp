/// @file
/// @brief      tap.overdrive~ — voiced feedback overdrive / saturation.
/// @details    A thin Min wrapper around the portable kernel in overdrive.h (all DSP lives there —
///             see its header comment for the model). The spiritual successor to the old Jamoma
///             TTOverdrive-based tap.overdrive~, not a port: instead of a memoryless waveshaping
///             curve applied to the full spectrum, the clipper sits inside a lowpass feedback loop
///             (solved zero-delay), so low frequencies stay tight and nearly clean while mids and
///             highs take the full drive and break up first — the frequency-dependent behavior of
///             TS-lineage feedback pedals (voiced against the Mad Professor Little Green Wonder).
///             An asymmetry control adds even-harmonic warmth, a unity clean path keeps the
///             transfer from ever flattening, the body control voices the lows/mids tilt, and the
///             clipping stage runs oversampled with an always-on DC blocker behind it.
///             Single-channel: wrap in mc. for multichannel (the kernel itself is N-channel for
///             reuse outside Max).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2004-2026 Timothy Place.

#include <taptools/overdrive.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::od;

class overdrive : public object<overdrive>, public vector_operator<> {
  private:
    // Constructed before the attributes below so attribute defaults can forward into it.
    kernel::overdrive m_od;

  public:
    MIN_DESCRIPTION{"Voiced feedback overdrive. The clipper sits inside a lowpass feedback loop, "
                    "so bass stays tight and nearly clean at any drive while mids and highs break "
                    "up first — the frequency-dependent gain of a TS-lineage feedback pedal, "
                    "which a static waveshaper cannot do. Asymmetry adds even-harmonic warmth, a "
                    "unity clean path keeps the transfer from ever flattening into a hard "
                    "plateau, and the body control voices the low/mid balance (CCW fuller lows "
                    "with a slight treble lift, CW tighter lows with an upper-mid push). The "
                    "clipping stage runs oversampled (default 4x) with an always-on DC blocker "
                    "behind it. Single-channel: wrap in mc. for multichannel."};
    MIN_TAGS{"audio, distortion"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"overdrive~, tap.vca~, tap.svf~, tap.dcblock~"};

    inlet<>  m_in{this, "(signal) audio input"};
    outlet<> m_out{this, "(signal) overdriven output", "signal"};

    overdrive(const atoms& args = {}) { m_od.prepare(samplerate(), 1); }

#define TAP_OD_ATTR(NAME, INDEX, DEFAULT, DESC)                                                                        \
    attribute<number> NAME{this, #NAME, DEFAULT,                                                                       \
                           setter{MIN_FUNCTION{const double v = kernel::clamp_param(INDEX, args[0]);                   \
    m_od.set_param(INDEX, v);                                                                                          \
    return {v};                                                                                                        \
    }                                                                                                                  \
    }                                                                                                                  \
    , description { DESC }                                                                                             \
    }                                                                                                                  \
    ;

    TAP_OD_ATTR(drive, kernel::p_drive, 0.35,
                "Drive (0..1). 0 is edge-of-breakup — a pedal's gain knob full counterclockwise, still "
                "warm — and 1 is heavily saturated. The sweep is perceptual (dB of gain into the "
                "clipper); use bypass for a true clean signal.")
    TAP_OD_ATTR(body, kernel::p_body, 0.0,
                "Body voicing (-1..+1, 0 neutral). Toward -1: fuller lows into the clipper and a "
                "slight treble lift. Toward +1: thinner, tighter lows and an upper-mid push. This is "
                "linear pre/post voicing around the clipper, not part of the nonlinearity.")
    TAP_OD_ATTR(asymmetry, kernel::p_asymmetry, 0.15,
                "Asymmetry (0..1): even-harmonic amount. 0 is a symmetric clip (odd harmonics only); "
                "raising it biases the clipper for even-order warmth. The DC this generates is removed "
                "by the built-in DC blocker.")
    TAP_OD_ATTR(preamp, kernel::p_preamp, 0.0, "Input gain in dB (-24..24), applied before the clipper.")
    TAP_OD_ATTR(output, kernel::p_output, 0.0, "Makeup gain in dB (-24..24), applied at the output.")

#undef TAP_OD_ATTR

    attribute<int> oversample{this, "oversample", 4, setter{ MIN_FUNCTION {
                                  const int req = static_cast<int>(args[0]);
                                  const int v   = (req >= 8) ? 8 : (req >= 4) ? 4 : (req >= 2) ? 2 : 1;
                                  m_od.set_oversample(v);
                                  return {v};
                              }},
                              description{"Internal oversampling factor for the clipping stage (1, 2, 4, or 8; "
                                          "default 4). Reduces aliasing from the clipper's harmonics."}};

    attribute<number> smooth{this, "smooth", kernel::k_default_smooth_ms, setter{ MIN_FUNCTION {
                                 const double v = std::max(0.0, static_cast<double>(args[0]));
                                 m_od.set_smooth_ms(v);
                                 return {v};
                             }},
                             description{"Anti-zipper interpolation time in ms applied to parameter changes."}};

    attribute<bool> bypass{this, "bypass", false, description{"Pass the input through unprocessed."}};

    attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

    // -- messages -------------------------------------------------------------------------------

    message<> clear{this, "clear", "Reset the filter, feedback, and resampler state.",
                    MIN_FUNCTION {
                        m_od.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Reconfigure for the sample rate when DSP starts.",
                       MIN_FUNCTION {
                           m_od.prepare(samplerate(), 1);
                           return {};
                       }};

    // -- perform --------------------------------------------------------------------------------

    void operator()(audio_bundle input, audio_bundle output) override {
        const long    n   = input.frame_count();
        const double* in  = input.samples(0);
        double*       out = output.samples(0);

        if (mute) {
            std::fill_n(out, n, 0.0);
        }
        else if (bypass) {
            std::copy_n(in, n, out);
        }
        else {
            m_od.process(in, out, static_cast<size_t>(n));
        }
    }

    // Exposed for unit tests.
    kernel::overdrive& od() { return m_od; }
};

MIN_EXTERNAL(overdrive);
