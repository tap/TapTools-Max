/// @file
/// @brief      tap.crossfade~ — crossfade between two signals.
/// @details    Equal-power (out = A·cos(θ) + B·sin(θ), θ = position·π/2) or linear
///             (out = A·(1−position) + B·position) crossfade, with `position` running 0 (all A)
///             to 1 (all B). Faithful to Jamoma's TTCrossfade "calculate" formulas. Position may
///             be driven by the third (signal) inlet, or set via the `position` attribute / a float.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class crossfade : public object<crossfade>, public sample_operator<3, 1> {
  public:
    // Named indices for the integer-valued shape/mode attributes. These are stored as plain
    // `int` (not `enum class`) attributes: the help patcher's umenus send the menu index, and an
    // `attribute<int>` accepts that directly. (An `attribute<enum class>` would be cleaner in the
    // inspector but does not compile under GCC — min-api's `atom::operator==` has no enum overload.)
    enum shapes : int { equalpower, linear };
    enum modes : int { lookuptable, calculate };

    MIN_DESCRIPTION{"Crossfade between two signals. The position runs from 0 (input A) to 1 "
                    "(input B), using either an equal-power or a linear curve."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.pan~, matrix~, *~, +~"};

    inlet<>  m_in_a{this, "(signal) input A"};
    inlet<>  m_in_b{this, "(signal) input B"};
    inlet<>  m_in_pos{this, "(signal/float) crossfade position, 0 (A) to 1 (B)"};
    outlet<> m_out{this, "(signal) the crossfaded output", "signal"};

    attribute<int> shape{this, "shape", equalpower, range{equalpower, linear},
                         description{"Crossfade curve: 0 = equal-power, 1 = linear."}};

    attribute<int> mode{this, "mode", lookuptable, range{lookuptable, calculate},
                        description{"Computation method (0 = lookup table, 1 = calculate). Provided for "
                                    "compatibility with the original object; both settings produce identical "
                                    "results in this implementation, which always computes the curve directly."}};

    attribute<number> position{this, "position", 0.5, range{0.0, 1.0},
                               description{"Crossfade position, from 0 (input A) to 1 (input B). "
                                           "Overridden by a signal connected to the right inlet."}};

    message<threadsafe::yes> m_number{this, "number", "Set the crossfade position (0..1).",
                                      MIN_FUNCTION{position = args;
    return {};
}
}
;

sample operator()(sample a, sample b, sample pos = 0.5) {
    double p = m_in_pos.has_signal_connection() ? static_cast<double>(pos) : static_cast<double>(position);
    p        = MIN_CLAMP(p, 0.0, 1.0);

    double wa, wb;
    if (shape == equalpower) {
        const double rad = p * k_half_pi;
        wa               = std::cos(rad);
        wb               = std::sin(rad);
    }
    else {
        wa = 1.0 - p;
        wb = p;
    }
    return a * wa + b * wb;
}

private:
static constexpr double k_half_pi{1.57079632679489661923};
}
;

MIN_EXTERNAL(crossfade);
