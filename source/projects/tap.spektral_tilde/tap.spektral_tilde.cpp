/// @file
/// @brief      tap.spektral~ — spectral delay with per-band level/delay/feedback, after NI's
///             Spektral Delay.
/// @details    A thin Min wrapper around the portable kernel in spektral.h (all DSP lives there —
///             see its header comment for the algorithm and provenance). The signal is STFT-split
///             into log-spaced bands; every band has its own level, delay (up to 12 s), and
///             feedback, set by drawing/sending curves as lists (any length — resampled to the
///             band count) or poking single bands. Freeze recirculates the delay matrix
///             losslessly. Mod FX (rotate, smear) and an assignable LFO animate the spectrum.
///             Latency is one FFT frame (the dry path is delayed to match, so mix stays aligned).
///             Single-channel: wrap in mc. for stereo (the original's independent L/R curves are
///             two instances).
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min.h"
#include "spektral.h"

#include <vector>

using namespace c74::min;
namespace kernel = taptools::spektral;


class spektral : public object<spektral>, public vector_operator<> {
private:
    // Constructed before the attributes below so attribute defaults can forward into it.
    kernel::spektral_engine m_engine;

public:
    MIN_DESCRIPTION { "A spectral delay modeled on NI's Spektral Delay: the signal is split into "
                      "log-spaced frequency bands (STFT), and every band has its own level, delay "
                      "(up to 12 seconds), and feedback, set as curves. Freeze recirculates the "
                      "delay matrix losslessly; rotate and smear process the spectrum before the "
                      "matrix; an LFO can animate band shift, feedback, or delay scale. All "
                      "parameters and curves interpolate smoothly, and sixteen preset slots can "
                      "be stored and morphed between. Latency is one FFT frame (dry is delayed "
                      "to match)." };
    MIN_TAGS    { "processors, fft" };
    MIN_AUTHOR  { "Timothy Place" };
    MIN_RELATED { "tap.nr~, tap.spectra~, tap.5comb~, tap.pitchaccum~, pfft~" };

    inlet<>  m_in  { this, "(signal) audio input" };
    outlet<> m_out { this, "(signal) spectral delay output", "signal" };

    // First argument = FFT size (a power of two, 256..8192; default 2048).
    spektral(const atoms& args = {})
        : m_engine { args.empty() ? kernel::k_default_fftsize : static_cast<int>(args[0]) } {
        m_engine.prepare(samplerate());
    }

#define TAP_SPEKTRAL_ATTR(NAME, INDEX, DEFAULT, DESC)                                              \
    attribute<number> NAME { this, #NAME, DEFAULT,                                                 \
        setter { MIN_FUNCTION {                                                                    \
            const double v = kernel::clamp_param(INDEX, args[0]);                                  \
            m_engine.set_param(INDEX, v);                                                          \
            return { v };                                                                          \
        }},                                                                                        \
        description { DESC }                                                                       \
    };

    TAP_SPEKTRAL_ATTR(level, kernel::p_level_master, 1.0,
        "Master multiplier (0..2) on the per-band level curve.")
    TAP_SPEKTRAL_ATTR(fb, kernel::p_fb_master, 1.0,
        "Master multiplier (0..2) on the per-band feedback curve (per-bin feedback is capped at 0.99).")
    TAP_SPEKTRAL_ATTR(delayscale, kernel::p_delayscale, 1.0,
        "Master multiplier (0..2) on the per-band delay curve.")
    TAP_SPEKTRAL_ATTR(rotate, kernel::p_rotate, 0.0,
        "Cyclic spectral rotation in bins per second (-500..500). 0 is off (neutral).")
    TAP_SPEKTRAL_ATTR(smear, kernel::p_smear, 0.0,
        "Amplitude smear (0..100): per-bin peak-hold diffusion of the spectrum over time, from "
        "off (0) through reverb-like (mid) to near-freezing (100).")
    TAP_SPEKTRAL_ATTR(lforate, kernel::p_lforate, 1.0,
        "LFO rate in Hz (0..20).")
    TAP_SPEKTRAL_ATTR(lfodepth, kernel::p_lfodepth, 0.0,
        "LFO depth (0..100): bins of shift for the bandshift destination; percent for the "
        "feedback and delayscale destinations.")
    TAP_SPEKTRAL_ATTR(mix, kernel::p_mix, 50.0,
        "Dry/wet crossfade (0 = dry .. 100 = wet), equal-power. The dry path is delayed by one "
        "FFT frame so it stays time-aligned with the wet path.")
    TAP_SPEKTRAL_ATTR(gain, kernel::p_gain, 0.0,
        "Output gain in dB.")

#undef TAP_SPEKTRAL_ATTR

    // LFO destination as an int attribute with named index constants — not attribute<enum class>,
    // per the tap.crossfade~/tap.pan~ GCC lesson (REVIVAL.md 9.5).
    attribute<int> lfodest { this, "lfodest", kernel::dest_off,
        setter { MIN_FUNCTION {
            const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_dests - 1);
            m_engine.set_lfodest(v);
            return { v };
        }},
        description { "LFO destination: 0 = off, 1 = bandshift (oscillating spectral shift), "
                      "2 = feedback master, 3 = delay scale." }
    };

    attribute<int> bands { this, "bands", kernel::k_default_bands,
        setter { MIN_FUNCTION {
            const int v = std::clamp(static_cast<int>(args[0]), kernel::k_min_bands, kernel::k_max_bands);
            m_engine.set_bands(v);
            return { v };
        }},
        description { "Number of editable log-spaced bands (8..512, default 160). Curves keep "
                      "their indices when this changes; the band frequencies shift." }
    };

    attribute<bool> freeze { this, "freeze", false,
        setter { MIN_FUNCTION {
            m_engine.set_freeze(args[0]);
            return args;
        }},
        description { "Freeze: mute the matrix input and recirculate the delay memory losslessly "
                      "— the contents loop indefinitely until freeze is released." }
    };

    attribute<number> interp { this, "interp", 500.0,
        setter { MIN_FUNCTION {
            return { std::max(0.0, static_cast<double>(args[0])) };
        }},
        description { "Default preset-morph time in ms used by the 'recall' message." }
    };

    attribute<number> smooth { this, "smooth", kernel::k_default_smooth_ms,
        setter { MIN_FUNCTION {
            const double v = std::max(0.0, static_cast<double>(args[0]));
            m_engine.set_smooth_ms(v);
            return { v };
        }},
        description { "Interpolation time in ms applied to direct parameter and curve changes." }
    };

    attribute<bool> bypass { this, "bypass", false,
        description { "Pass the input through unprocessed." }
    };

    attribute<bool> mute { this, "mute", false,
        description { "Silence the output." }
    };

    // -- band curves --------------------------------------------------------------------------------

    message<> gains { this, "gains",
        "Set the per-band level curve (0..2): either a list of any length (resampled to the band "
        "count) or [band value] with an integer band index (0-based).",
        MIN_FUNCTION { curve_message(kernel::curve_level, args); return {}; }
    };

    message<> delays { this, "delays",
        "Set the per-band delay curve in ms (0..12000): a list of any length (resampled) or "
        "[band value].",
        MIN_FUNCTION { curve_message(kernel::curve_delay, args); return {}; }
    };

    message<> fbs { this, "fbs",
        "Set the per-band feedback curve (0..0.99): a list of any length (resampled) or "
        "[band value].",
        MIN_FUNCTION { curve_message(kernel::curve_fb, args); return {}; }
    };

    // -- presets ------------------------------------------------------------------------------------

    message<> store { this, "store", "Store the current parameters and curves in a preset slot (1..16).",
        MIN_FUNCTION {
            if (!args.empty())
                m_engine.store_preset(static_cast<int>(args[0]) - 1);
            return {};
        }
    };

    message<> recall { this, "recall",
        "Morph all parameters and curves to a stored preset (1..16). An optional second argument "
        "overrides the morph time in ms (default: the interp attribute).",
        MIN_FUNCTION {
            if (!args.empty()) {
                const double ms = (args.size() > 1) ? static_cast<double>(args[1])
                                                    : static_cast<double>(interp);
                m_engine.recall_preset(static_cast<int>(args[0]) - 1, ms * 0.001);
            }
            return {};
        }
    };

    message<> clear { this, "clear", "Clear the delay matrix and all STFT state.",
        MIN_FUNCTION {
            m_engine.clear();
            return {};
        }
    };

    message<> dspsetup { this, "dspsetup", "Allocate and reset when the DSP chain starts.",
        MIN_FUNCTION {
            m_engine.prepare(samplerate());
            return {};
        }
    };

    // -- perform ------------------------------------------------------------------------------------

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
            m_engine.process(in, out, static_cast<size_t>(n));
        }
    }

    // Exposed for unit tests.
    kernel::spektral_engine& engine() { return m_engine; }

private:
    // [int-band value] pokes one band; any other list is resampled across all bands.
    void curve_message(int which, const atoms& args) {
        if (args.empty())
            return;
        if (args.size() == 2 && args[0].type() == message_type::int_argument) {
            m_engine.set_curve_point(which, static_cast<int>(args[0]), args[1]);
            return;
        }
        std::vector<double> values;
        values.reserve(args.size());
        for (const auto& a : args)
            values.push_back(static_cast<double>(a));
        m_engine.set_curve(which, values.data(), values.size());
    }
};


MIN_EXTERNAL(spektral);
