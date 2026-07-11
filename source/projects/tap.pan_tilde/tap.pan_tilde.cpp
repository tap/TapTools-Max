/// @file
/// @brief      tap.pan~ — stereo panner.
/// @details    Pans a mono signal between left and right outputs. The position runs from -1 (hard
///             left) through 0 (center) to 1 (hard right); internally this maps to 0..1 and drives
///             an equal-power (cos/sin), linear, or square-root gain curve. Faithful to Jamoma's
///             TTPanorama "calculate" formulas. Position may be driven by the second (signal)
///             inlet, or set via the `position` attribute / a float.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class pan : public object<pan>, public sample_operator<2, 2> {
  public:
    // Named indices for the integer-valued shape/mode attributes (order matches the original
    // object: equalpower(0), linear(1), squareroot(2)). Stored as plain `int` (not `enum class`)
    // attributes: the help patcher's umenus send the menu index, which an `attribute<int>` takes
    // directly. (An `attribute<enum class>` is cleaner in the inspector but does not compile under
    // GCC — min-api's `atom::operator==` has no enum overload.)
    enum shapes : int { equalpower, linear, squareroot };
    enum modes : int { lookuptable, calculate };

    MIN_DESCRIPTION{"Stereo panner. Pan a mono signal between the left and right outputs using a "
                    "position from -1 (left) through 0 (center) to 1 (right)."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.crossfade~, matrix~, *~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_pos{this, "(signal/float) pan position, -1 (left) to 1 (right)"};
    outlet<> m_out_l{this, "(signal) left output", "signal"};
    outlet<> m_out_r{this, "(signal) right output", "signal"};

    attribute<int> shape{this, "shape", equalpower, range{equalpower, squareroot},
                         description{"Panning curve: 0 = equalpower, 1 = linear, 2 = squareroot."}};

    attribute<int> mode{this, "mode", lookuptable, range{lookuptable, calculate},
                        description{"Computation method (0 = lookup table, 1 = calculate). Provided for "
                                    "compatibility with the original object; both settings produce identical "
                                    "results in this implementation, which always computes the curve directly."}};

    attribute<number> position{this, "position", 0.0, range{-1.0, 1.0},
                               description{"Pan position, from -1 (left) through 0 (center) to 1 (right). "
                                           "Overridden by a signal connected to the right inlet."}};

    message<threadsafe::yes> m_number{this, "number", "Set the pan position (-1..1).",
                                      MIN_FUNCTION{
                                          position = args;
                                          return {};
                                      }};

    samples<2> operator()(sample x, sample pos = 0.0) {
        double p = m_in_pos.has_signal_connection() ? static_cast<double>(pos) : static_cast<double>(position);
        p        = MIN_CLAMP(p, -1.0, 1.0);
        const double scaled = 0.5 * (p + 1.0); // map -1..1 to 0..1

        double    wl, wr;
        const int s = shape;
        switch (s) {
        case linear:
            wl = 1.0 - scaled;
            wr = scaled;
            break;
        case squareroot:
            wl = std::sqrt(1.0 - scaled);
            wr = std::sqrt(scaled);
            break;
        case equalpower:
        default: {
            const double rad = scaled * k_half_pi;
            wl               = std::cos(rad);
            wr               = std::sin(rad);
            break;
        }
        }
        return {x * wl, x * wr};
    }

  private:
    static constexpr double k_half_pi{1.57079632679489661923};
};

MIN_EXTERNAL(pan);
