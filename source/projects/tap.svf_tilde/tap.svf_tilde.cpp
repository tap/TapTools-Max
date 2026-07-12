/// @file
/// @brief      tap.svf~ — Simper state-variable filter (morphing, multi-order, clean or driven).
/// @details    A thin Min wrapper around the portable kernel in svf.h (all DSP lives there — see
///             its header comment for the model). A trapezoidal-integration (zero-delay-feedback)
///             state-variable filter after Andy Simper's Cytomic papers — the lineage of the
///             Cytomic-designed filters in Ableton Live. Discrete responses (lowpass, highpass,
///             bandpass, notch, peak, allpass), a continuous morph mode that sweeps
///             LP -> BP -> HP -> notch -> LP like Live's Morph filter, and parametric-EQ modes
///             (bell, low shelf, high shelf) with a boost/cut gain. Orders 2/4/8 (12/24/48 dB per
///             octave, Butterworth-flat at resonance 0), clean (linear) or driven (tanh-saturated,
///             oversampled, self-oscillating) circuits. The right inlet takes a signal for true
///             per-sample cutoff modulation (or a float to set the frequency attribute).
///             Single-channel: wrap in mc. for multichannel (the kernel itself is N-channel for
///             reuse outside Max).
///
///             This replaces the earlier Jamoma-faithful port (stereo Chamberlin SVF + built-in
///             LFO + portamento, from ttblue tt_svf/tt_lfo/tt_ramp) — patch an LFO into the right
///             inlet and use the smooth attribute where the old object used portamento.
/// @author     Timothy Place
/// @copyright  Copyright 2004-2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min.h"
#include "svf.h"

using namespace c74::min;
namespace kernel = taptools::svf;


class svf : public object<svf>, public vector_operator<> {
private:
    // Constructed before the attributes below so attribute defaults can forward into it.
    kernel::svf_filter m_filter;

    // Effective order for Q conversions: the EQ modes always run a single 2nd-order section.
    int effective_order() const {
        return kernel::is_eq_mode(m_filter.mode()) ? 2 : m_filter.order();
    }

public:
    MIN_DESCRIPTION { "A state-variable filter built on Andy Simper's trapezoidal-integration "
                      "(zero-delay-feedback) SVF — the design behind the Cytomic filters in "
                      "Ableton Live. Discrete types (lowpass, highpass, bandpass, notch, peak, "
                      "allpass), a morph type that sweeps continuously around "
                      "lowpass -> bandpass -> highpass -> notch -> lowpass, and parametric-EQ "
                      "types (bell, lowshelf, highshelf) with a boost/cut gain. Orders 2, 4, or 8 "
                      "(12/24/48 dB per octave; maximally flat at resonance 0, one clean resonant "
                      "peak as resonance rises). The clean circuit is pure linear; the driven "
                      "circuit adds drive into a tanh limiter on each section's band node, runs "
                      "oversampled, and self-oscillates at resonance 1 (ping it to start). "
                      "Tuning is exact to the top octaves at any sample rate, and a signal in the "
                      "right inlet modulates the cutoff per sample. Single-channel: wrap in mc. "
                      "for multichannel." };
    MIN_TAGS    { "filters" };
    MIN_AUTHOR  { "Timothy Place" };
    MIN_RELATED { "tap.ladder~, tap.fourpole~, tap.filter~, svf~, biquad~" };

    inlet<>  m_in      { this, "(signal) audio input" };
    inlet<>  m_in_freq { this, "(signal/float) cutoff / center frequency in Hz" };
    outlet<> m_out     { this, "(signal) filtered output", "signal" };

    svf(const atoms& args = {}) {
        m_filter.prepare(samplerate(), 1);
    }

    attribute<symbol> type { this, "type", "lowpass",
        range { "lowpass", "highpass", "bandpass", "notch", "peak", "allpass",
                "morph", "bell", "lowshelf", "highshelf" },
        setter { MIN_FUNCTION {
            int m = kernel::mode_lowpass;
            if (args[0] == "highpass")       m = kernel::mode_highpass;
            else if (args[0] == "bandpass")  m = kernel::mode_bandpass;
            else if (args[0] == "notch")     m = kernel::mode_notch;
            else if (args[0] == "peak")      m = kernel::mode_peak;
            else if (args[0] == "allpass")   m = kernel::mode_allpass;
            else if (args[0] == "morph")     m = kernel::mode_morph;
            else if (args[0] == "bell")      m = kernel::mode_bell;
            else if (args[0] == "lowshelf")  m = kernel::mode_lowshelf;
            else if (args[0] == "highshelf") m = kernel::mode_highshelf;
            m_filter.set_mode(m);
            return args;
        }},
        description { "Filter response. The discrete types plus 'morph' (continuous sweep around "
                      "lowpass -> bandpass -> highpass -> notch -> lowpass, positioned by the "
                      "morph attribute) and the parametric-EQ types 'bell', 'lowshelf', and "
                      "'highshelf' (boost/cut set by the gain attribute; always 2nd-order)." }
    };

#define TAP_SVF_ATTR(NAME, INDEX, DEFAULT, DESC)                                                   \
    attribute<number> NAME { this, #NAME, DEFAULT,                                                 \
        setter { MIN_FUNCTION {                                                                    \
            const double v = kernel::clamp_param(INDEX, args[0]);                                  \
            m_filter.set_param(INDEX, v);                                                          \
            return { v };                                                                          \
        }},                                                                                        \
        description { DESC }                                                                       \
    };

    TAP_SVF_ATTR(frequency, kernel::p_frequency, 1000.0,
        "Cutoff / center frequency in Hz (1..20000). A signal in the right inlet overrides this "
        "per sample.")
    TAP_SVF_ATTR(resonance, kernel::p_resonance, 0.0,
        "Resonance (0..1). 0 is maximally flat (Butterworth); rising values sharpen a single "
        "resonant peak. At 1 the clean circuit reaches a very high finite Q; the driven circuit "
        "self-oscillates at the cutoff (ping it to start). Use the q message to set this from a "
        "Q value.")
    TAP_SVF_ATTR(morph, kernel::p_morph, 0.0,
        "Position around the response circle when type is morph: 0 = lowpass, 0.25 = bandpass, "
        "0.5 = highpass, 0.75 = notch, 1 wraps back to lowpass.")
    TAP_SVF_ATTR(gain, kernel::p_gain, 0.0,
        "Boost/cut in dB (-24..24) for the parametric-EQ types (bell, lowshelf, highshelf); "
        "ignored by the other types.")
    TAP_SVF_ATTR(drive, kernel::p_drive, 0.0,
        "Input drive in dB (-24..24) into the driven circuit's saturation; ignored by the clean "
        "circuit.")

#undef TAP_SVF_ATTR

    attribute<symbol> circuit { this, "circuit", "clean",
        range { "clean", "driven" },
        setter { MIN_FUNCTION {
            m_filter.set_circuit(args[0] == "driven" ? kernel::circuit_driven
                                                     : kernel::circuit_clean);
            return args;
        }},
        description { "clean: the pure linear SVF (transparent, cheap, no oversampling). "
                      "driven: adds drive into a tanh limiter on each section's band node — "
                      "saturation color, bounded resonance, true self-oscillation at resonance 1 "
                      "— and runs oversampled (see the oversample attribute)." }
    };

    attribute<int> order { this, "order", 2,
        setter { MIN_FUNCTION {
            const int v = (static_cast<int>(args[0]) >= 8) ? 8
                        : (static_cast<int>(args[0]) >= 4) ? 4 : 2;
            m_filter.set_order(v);
            return { v };
        }},
        description { "Filter order: 2, 4, or 8 (12/24/48 dB per octave), as a cascade with the "
                      "Butterworth Q spread — resonance 0 stays maximally flat at every order. "
                      "The EQ types (bell/shelves) always run 2nd-order." }
    };

    attribute<int> oversample { this, "oversample", 2,
        setter { MIN_FUNCTION {
            const int req = static_cast<int>(args[0]);
            const int v   = (req >= 4) ? 4 : (req >= 2) ? 2 : 1;
            m_filter.set_oversample(v);
            return { v };
        }},
        description { "Internal oversampling factor for the driven circuit (1, 2, or 4; default "
                      "2). Reduces aliasing from the saturation. The clean circuit is linear and "
                      "always runs 1x." }
    };

    attribute<number> smooth { this, "smooth", kernel::k_default_smooth_ms,
        setter { MIN_FUNCTION {
            const double v = std::max(0.0, static_cast<double>(args[0]));
            m_filter.set_smooth_ms(v);
            return { v };
        }},
        description { "Anti-zipper interpolation time in ms applied to parameter changes." }
    };

    attribute<bool> bypass { this, "bypass", false,
        description { "Pass the input through unprocessed." }
    };

    attribute<bool> mute { this, "mute", false,
        description { "Silence the output." }
    };

    // -- messages -------------------------------------------------------------------------------

    message<> m_number { this, "number", "A float in the right inlet sets the cutoff frequency.",
        MIN_FUNCTION {
            if (inlet == 1)
                frequency = args[0];
            return {};
        }
    };

    message<> q { this, "q",
        "Set the resonance from a Q value (of the resonant section; the Butterworth base Q of "
        "the current order is the minimum). Without an argument, posts the current equivalent Q "
        "to the console.",
        MIN_FUNCTION {
            if (args.empty()) {
                cout << "q: "
                     << kernel::q_from_resonance(m_filter.param(kernel::p_resonance),
                                                 effective_order())
                     << endl;
            }
            else {
                resonance = kernel::resonance_from_q(args[0], effective_order());
            }
            return {};
        }
    };

    message<> clear { this, "clear", "Reset the filter state (stops self-oscillation).",
        MIN_FUNCTION {
            m_filter.clear();
            return {};
        }
    };

    message<> dspsetup { this, "dspsetup", "Reconfigure for the sample rate when DSP starts.",
        MIN_FUNCTION {
            m_filter.prepare(samplerate(), 1);
            return {};
        }
    };

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
        else if (m_in_freq.has_signal_connection()) {
            const double* freq = input.samples(1);
            for (long i = 0; i < n; ++i)
                out[i] = m_filter.process(in[i], freq[i]);   // true per-sample cutoff modulation
        }
        else {
            m_filter.process(in, out, static_cast<size_t>(n));
        }
    }

    // Exposed for unit tests.
    kernel::svf_filter& filter() { return m_filter; }
};


MIN_EXTERNAL(svf);
