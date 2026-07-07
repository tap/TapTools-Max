/// @file
/// @brief      tap.rotate — perform 3D rotations on sets of coordinates.
/// @details    Holds parallel lists of x, y, and z coordinates and rotates each (x,y,z) triple about
///             the origin by the Euler angles supplied (in degrees) to the rotation inlet. A list in
///             the x inlet triggers output. Pure data object — no Jamoma dependency; the rotation
///             algorithm (provided by Stephan Moore) is a faithful port of the original.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2007-2026 Timothy Place.

#include <array>
#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class rotate : public object<rotate> {
  public:
    MIN_DESCRIPTION{"Perform 3D rotations on sets of coordinates. Rotates parallel lists of x/y/z "
                    "coordinates about the origin by Euler angles given in degrees."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"cartopol, poltocar, jit.gl.handle"};

    inlet<>  m_in_x{this, "(list) x coordinates (triggers output)"};
    inlet<>  m_in_y{this, "(list) y coordinates"};
    inlet<>  m_in_z{this, "(list) z coordinates"};
    inlet<>  m_in_rot{this, "(list) x y z angles (degrees) defining the rotation"};
    outlet<> m_out_x{this, "(list) rotated x coordinates"};
    outlet<> m_out_y{this, "(list) rotated y coordinates"};
    outlet<> m_out_z{this, "(list) rotated z coordinates"};
    outlet<> m_dump{this, "(anything) dumpout"};

    message<> list_msg{
        this, "list", "Set coordinates (x/y/z inlets) or the rotation (4th inlet).",
        MIN_FUNCTION{switch (inlet){case 0 : // x coordinates — store and trigger output
                                        m_numsets = std::min(static_cast<int>(args.size()), k_max_numsets);
    for (int i = 0; i < m_numsets; ++i) {
        m_x[i] = args[i];
    }
    apply();
    break;
case 1:
    for (int i = 0; i < static_cast<int>(args.size()) && i < k_max_numsets; ++i) {
        m_y[i] = args[i];
    }
    break;
case 2:
    for (int i = 0; i < static_cast<int>(args.size()) && i < k_max_numsets; ++i) {
        m_z[i] = args[i];
    }
    break;
case 3: // rotation angles, in degrees
    if (args.size() != 3) {
        cerr << "wrong number of list elements (expected x y z)" << endl;
        return {};
    }
    m_rot_x_rad = (static_cast<double>(args[0]) / 180.0) * k_pi;
    m_rot_y_rad = (static_cast<double>(args[1]) / 180.0) * k_pi;
    m_rot_z_rad = (static_cast<double>(args[2]) / 180.0) * k_pi;
    break;
} return {};
}
}
;

message<> number{this, "number", "Set a single coordinate (x inlet triggers output).",
                 MIN_FUNCTION{const double v = args[0];
if (inlet == 0) {
    m_numsets = 1;
    m_x[0]    = v;
    apply();
}
else if (inlet == 1) {
    m_y[0] = v;
}
else if (inlet == 2) {
    m_z[0] = v;
}
return {};
}
}
;

message<> bang{this, "bang", "Re-apply the rotation and output.", MIN_FUNCTION{apply();
return {};
}
}
;

private:
static constexpr int    k_max_numsets{128};
static constexpr double k_pi{3.1415926535897932};

std::array<double, k_max_numsets> m_x{};
std::array<double, k_max_numsets> m_y{};
std::array<double, k_max_numsets> m_z{};
int                               m_numsets{1};
double                            m_rot_x_rad{0.0};
double                            m_rot_y_rad{0.0};
double                            m_rot_z_rad{0.0};

static void cart_to_pol(double real, double imaginary, double& magnitude, double& phase) {
    magnitude = std::sqrt((real * real) + (imaginary * imaginary));
    if (real == 0.0) {
        real = 0.000001; // prevent divide by zero
    }
    phase = std::atan(imaginary / real);
    if ((real < 0) && (imaginary < 0)) {
        phase -= k_pi;
    }
    else if ((real < 0) && (imaginary >= 0)) {
        phase += k_pi;
    }
}

static void pol_to_cart(double magnitude, double phase, double& real, double& imaginary) {
    real      = magnitude * std::cos(phase);
    imaginary = magnitude * std::sin(phase);
}

void apply() {
    atoms ax, ay, az;
    ax.reserve(m_numsets);
    ay.reserve(m_numsets);
    az.reserve(m_numsets);

    for (int i = 0; i < m_numsets; ++i) {
        double x = m_x[i], y = m_y[i], z = m_z[i];
        double a, b;

        // z-rotate
        cart_to_pol(x, y, a, b);
        b += m_rot_z_rad;
        pol_to_cart(a, b, x, y);

        // y-rotate
        cart_to_pol(z, x, a, b);
        b += m_rot_y_rad;
        pol_to_cart(a, b, z, x);

        // x-rotate
        cart_to_pol(y, z, a, b);
        b += m_rot_x_rad;
        pol_to_cart(a, b, y, z);

        ax.push_back(x);
        ay.push_back(y);
        az.push_back(z);
    }

    m_out_z.send(az);
    m_out_y.send(ay);
    m_out_x.send(ax);
}
}
;

MIN_EXTERNAL(rotate);
