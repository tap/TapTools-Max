/// @file
/// @brief      tap.jit.proximity — find the point in a matrix nearest to an input coordinate.
/// @details    The named float32 matrix holds a list of 2D points (column 0 = x, column 1 = y, one
///             point per row). Given an input (x, y) via the `coords` message, this finds the point
///             with the smallest Manhattan distance and outputs its 1-based index and coordinates.
///             Faithful port — rebuilt as a plain Min object reading the matrix through the Jitter
///             API (`c74::max`).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <cmath>

#include "c74_min.h"

using namespace c74::min;
namespace mx = c74::max;

class jit_proximity : public object<jit_proximity> {
  public:
    MIN_DESCRIPTION{"Find the point in a matrix nearest to an input coordinate. The matrix holds 2D "
                    "points (column 0 = x, column 1 = y, one per row); coords outputs the closest "
                    "point's index and value."};
    MIN_TAGS{"jitter"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"jit.findbounds, tap.jit.ali, nodes"};

    inlet<>  m_in{this, "(coords x y) input coordinate to match"};
    outlet<> m_out{this, "(list) nearest point: index x y"};
    outlet<> m_dump{this, "(anything) dumpout"};

    attribute<symbol> name{this, "name", "", description{"Name of the registered jit.matrix holding the points."}};

    message<> coords{this, "coords", "Find the matrix point nearest to (x, y).",
                     MIN_FUNCTION{
                         if (args.size() < 2) {
                             return {};
                         }
                         const double xin = args[0];
                         const double yin = args[1];

                         const symbol matrix_name = name;
                         auto*        matrix = mx::jit_object_findregistered(static_cast<mx::t_symbol*>(matrix_name));
                         if (!matrix || !mx::jit_object_method(matrix, mx::gensym("class_jit_matrix"))) {
                             return {};
                         }

                         const long savelock = reinterpret_cast<long>(
                             mx::jit_object_method(matrix, mx::gensym("lock"), reinterpret_cast<void*>(1)));

                         mx::t_jit_matrix_info info{};
                         mx::jit_object_method(matrix, mx::gensym("getinfo"), &info);
                         char* bp = nullptr;
                         mx::jit_object_method(matrix, mx::gensym("getdata"), &bp);

                         if (bp) {
                             const long rows   = (info.dimcount > 1) ? info.dim[1] : 1;
                             double     best   = 1.0e30;
                             long       best_i = 0;
                             float      best_x = 0.0f, best_y = 0.0f;

                             for (long i = 0; i < rows; ++i) {
                                 const float  px = get_f32(bp, info, 0, i); // column 0
                                 const float  py = get_f32(bp, info, 1, i); // column 1
                                 const double d  = std::abs(xin - px) + std::abs(yin - py);
                                 if (d < best) {
                                     best   = d;
                                     best_i = i;
                                     best_x = px;
                                     best_y = py;
                                 }
                             }
                             m_out.send(static_cast<int>(best_i + 1), best_x, best_y);
                         }

                         mx::jit_object_method(matrix, mx::gensym("lock"), reinterpret_cast<void*>(savelock));
                         return {};
                     }};

  private:
    static float get_f32(char* bp, const mx::t_jit_matrix_info& info, long col, long row) {
        char* p = bp + row * info.dimstride[1] + col * info.dimstride[0];
        return reinterpret_cast<float*>(p)[0]; // plane 0
    }
};

MIN_EXTERNAL(jit_proximity);
