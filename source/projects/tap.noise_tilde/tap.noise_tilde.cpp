/// @file
/// @brief      tap.noise~ — generate various colors of noise.
/// @details    White, pink, brown, blue, or gaussian noise. The white/pink/brown/blue generators
///             are faithful ports of Jamoma's TTNoise (LCG white source + the same colouring
///             filters); gaussian uses a standard normal distribution with mean/deviation. DSP is
///             portable C++ (no min-lib). Mono — wrap in an mc. operator for multichannel.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <cmath>
#include <random>

#include "c74_min.h"

using namespace c74::min;

class noise : public object<noise>, public sample_operator<0, 1> {
  private:
    enum noise_color { white = 0, pink = 1, brown = 2, blue = 3, gauss = 4 };

    // Cached state — declared before the attributes so it is initialized before their setters run.
    int    m_color{white};
    double m_gain{1.0}; // linear gain

    long   m_accum{0};                                                    // white-noise LCG state
    double m_b0{0}, m_b1{0}, m_b2{0}, m_b3{0}, m_b4{0}, m_b5{0}, m_b6{0}; // pink filter
    double m_last{0};                                                     // brown / blue filter state

    double                           m_mean{0.0};
    double                           m_deviation{1.0};
    std::mt19937                     m_rng{std::random_device{}()};
    std::normal_distribution<double> m_normal{0.0, 1.0};

  public:
    MIN_DESCRIPTION{"Generate various colors of noise: white, pink, brown, blue, or gaussian."};
    MIN_TAGS{"generators"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"noise~, pink~, rand~"};

    inlet<>  m_in{this, "(message) set attributes; 'clear' resets the filter memory"};
    outlet<> m_out{this, "(signal) the noise output", "signal"};

    attribute<symbol> mode{this,
                           "mode",
                           "white",
                           range{"white", "pink", "brown", "blue", "gauss"},
                           setter{MIN_FUNCTION{
                               if (args[0] == "pink") {
                                   m_color = pink;
                               }
                               else if (args[0] == "brown") {
                                   m_color = brown;
                               }
                               else if (args[0] == "blue") {
                                   m_color = blue;
                               }
                               else if (args[0] == "gauss") {
                                   m_color = gauss;
                               }
                               else {
                                   m_color = white;
                               }
                               return args;
                           }},
                           description{"The color of the noise."}};

    attribute<number> gain{this, "gain", 0.0, setter{MIN_FUNCTION{
                               m_gain = std::pow(10.0, static_cast<double>(args[0]) * 0.05); // dB -> linear
                               return args;
                           }},
                           description{"Output gain in decibels (does not apply to gaussian noise)."}};

    attribute<number> mean{this, "mean", 0.0, setter{MIN_FUNCTION{
                               m_mean = args[0];
                               rebuild_normal();
                               return args;
                           }},
                           description{"Mean of the gaussian distribution."}};

    attribute<number> deviation{this,
                                "deviation",
                                1.0,
                                range{0.0, 1.0},
                                setter{MIN_FUNCTION{
                                    m_deviation = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                                    rebuild_normal();
                                    return args;
                                }},
                                description{"Standard deviation of the gaussian distribution."}};

    message<> clear{this, "clear", "Reset the noise filter memory.",
                    MIN_FUNCTION{
                        m_b0 = m_b1 = m_b2 = m_b3 = m_b4 = m_b5 = m_b6 = 0.0;
                        m_last                                         = 0.0;
                        return {};
                    }};

    sample operator()() {
        if (m_color == gauss) {
            return m_normal(m_rng); // mean/deviation define the level; gain is not applied
        }

        // White-noise source via LCG, scaled to the range (-1, 1).
        m_accum  = (m_accum * 3877 + 29573) % 139968;
        double s = 1.0 - (2.0 * static_cast<double>(m_accum) / 139968.0);

        switch (m_color) {
        case pink: {
            m_b0 = 0.99886 * m_b0 + s * 0.0555179;
            m_b1 = 0.99332 * m_b1 + s * 0.0750759;
            m_b2 = 0.96900 * m_b2 + s * 0.1538520;
            m_b3 = 0.86650 * m_b3 + s * 0.3104856;
            m_b4 = 0.55000 * m_b4 + s * 0.5329522;
            m_b5 = -0.7616 * m_b5 - s * 0.0168980;
            m_b6 = s * 0.115926;
            return (m_b0 + m_b1 + m_b2 + m_b3 + m_b4 + m_b5 + m_b6 + s * 0.5362) * m_gain;
        }
        case brown: {
            s *= 0.1;
            s      = m_last + s; // 6 dB/oct lowpass
            s      = MIN_CLAMP(s, -1.0, 1.0);
            m_last = s;
            return s * 0.25 * m_gain;
        }
        case blue: {
            s -= m_last; // 6 dB/oct highpass
            s      = MIN_CLAMP(s, -1.0, 1.0);
            m_last = s;
            return s * m_gain;
        }
        case white:
        default:
            return s * m_gain;
        }
    }

  private:
    void rebuild_normal() { m_normal = std::normal_distribution<double>(m_mean, m_deviation); }
};

MIN_EXTERNAL(noise);
