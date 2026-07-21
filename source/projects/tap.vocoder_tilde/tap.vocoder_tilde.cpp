/// @file
/// @brief      tap.vocoder~ — a basic 24-band channel vocoder.
/// @details    Reconstructed from its surviving reference documentation (no source survived the
///             revival). A classic channel vocoder: a bank of 24 log-spaced bandpass filters
///             analyses the modulator (left inlet, e.g. a voice); a per-band envelope follower
///             tracks each band's amplitude; an identical filter bank splits the carrier (right
///             inlet, e.g. a synth) and each carrier band is multiplied by the matching modulator
///             envelope. The bands are summed to the output, imposing the modulator's spectral
///             envelope onto the carrier.
///
///             The documented attributes are honoured: `q` (the Q factor shared by every resonant
///             filter) and `response_interval` (the envelope-follower analysis period, in ms). The
///             original registered both as `symbol`; here they are `number` attributes, which is
///             how they actually behave (a Q value and a millisecond time). A practical `gain`
///             (linear makeup) attribute is added for level staging.
///
///             The DSP lives in the portable, Min-free kernel `tap::tools::vocoder::bank` (vocoder.h)
///             — RBJ constant-0 dB-peak bandpass biquads with per-band envelope followers. This file
///             is the Min wrapper.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <taptools/vocoder.h>

#include "c74_min.h"

using namespace c74::min;

class vocoder : public object<vocoder>, public sample_operator<2, 1> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    tap::tools::vocoder::bank m_bank;

  public:
    MIN_DESCRIPTION{"A basic 24-band channel vocoder. The modulator (left inlet) imposes its "
                    "spectral envelope onto the carrier (right inlet) through a bank of bandpass "
                    "filters and per-band envelope followers."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.svf~, tap.comb~, vocoder~, fffb~"};

    inlet<>  m_in_mod{this, "(signal) modulator — the spectral envelope source (e.g. a voice)"};
    inlet<>  m_in_car{this, "(signal) carrier — the signal to be shaped (e.g. a synth)"};
    outlet<> m_out{this, "(signal) the vocoded output", "signal"};

    vocoder(const atoms& = {}) { m_bank.prepare(samplerate()); }

    attribute<number> q{this,
                        "q",
                        20.0,
                        range{0.5, 200.0},
                        setter{MIN_FUNCTION{
                            m_bank.set_q(args[0]);
                            return {args[0]};
                        }},
                        description{"The Q factor (resonance) shared by all of the bandpass filters. Higher values "
                                    "give narrower, more 'robotic' bands."}};

    attribute<number> response_interval{
        this,
        "response_interval",
        20.0,
        range{0.1, 1000.0},
        setter{MIN_FUNCTION{
            m_bank.set_response_ms(args[0]);
            return {args[0]};
        }},
        description{"The envelope-follower analysis period, in milliseconds. Shorter values track "
                    "the modulator more sharply; longer values smooth it."}};

    attribute<number> gain{this,
                           "gain",
                           1.0,
                           range{0.0, 100.0},
                           setter{MIN_FUNCTION{
                               m_bank.set_gain(args[0]);
                               return {args[0]};
                           }},
                           description{"Linear makeup gain applied to the summed output."}};

    message<> clear{this, "clear", "Reset all filter and envelope-follower state.",
                    MIN_FUNCTION{
                        m_bank.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Recompute coefficients when the DSP chain starts.",
                       MIN_FUNCTION{
                           m_bank.prepare(samplerate());
                           return {};
                       }};

    sample operator()(sample modulator, sample carrier) { return m_bank.process(modulator, carrier); }
};

MIN_EXTERNAL(vocoder);
