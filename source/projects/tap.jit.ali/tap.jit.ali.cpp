/// @file
/// @brief      tap.jit.ali — proximity-weighted interpolation across a set of data points.
/// @details    Given a 2D control coordinate, looks up a precomputed per-point weight field
///             (`space_matrix`, a 3D float32 matrix indexed by [x][y][point]), normalizes the
///             weights, and uses them to interpolate the rows of a `data_matrix` into an output
///             vector. Outputs the interpolated data and the weights. Interpolation algorithm by
///             Ali Momeni. Rebuilt as a plain Min object reading the matrices through the Jitter API
///             (`c74::max`).
/// @note       Only the default list-output mode is ported; the original's optional jit.matrix output
///             (`output_type matrix`) is not included.
/// @author     Timothy Place, Ali Momeni
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <algorithm>
#include <vector>

#include "c74_min.h"

using namespace c74::min;
namespace mx = c74::max;

class jit_ali : public object<jit_ali> {
  public:
    MIN_DESCRIPTION{"Proximity-weighted interpolation across a set of data points. Given a control "
                    "coordinate, looks up per-point weights from a space matrix and interpolates the "
                    "rows of a data matrix. Interpolation algorithm by Ali Momeni."};
    MIN_TAGS{"jitter"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.jit.proximity, nodes, jit.gen"};

    inlet<>  m_in{this, "(coords x y) control-space coordinate"};
    outlet<> m_out{this, "(list) interpolated data"};
    outlet<> m_weights{this, "(list) the per-point weights"};
    outlet<> m_dump{this, "(anything) dumpout"};

    attribute<symbol> space_matrix{this, "space_matrix", "",
                                   description{"Name of the 3D weight-field matrix ([x][y][point])."}};
    attribute<symbol> data_matrix{this, "data_matrix", "",
                                  description{"Name of the data matrix whose rows are interpolated."}};
    attribute<bool>   data_interpolation{this, "data_interpolation", true,
                                       description{"When off, output only the weights (no data interpolation)."}};
    attribute<int>    data_depthclip{this, "data_depthclip", 0,
                                  description{"Number of points to use (0 = all rows of the data matrix)."}};
    attribute<int>    data_widthclip{this, "data_widthclip", 0,
                                  description{"Width of the interpolated output (0 = full data-matrix width)."}};

    message<> coords{this, "coords", "Interpolate at the control coordinate (x, y).",
                     MIN_FUNCTION{if (args.size() < 2){return {};
} const double xin = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
const double   yin = std::clamp(static_cast<double>(args[1]), 0.0, 1.0);

const symbol space_name = space_matrix;
const symbol data_name  = data_matrix;
auto*        space      = mx::jit_object_findregistered(static_cast<mx::t_symbol*>(space_name));
auto*        data       = mx::jit_object_findregistered(static_cast<mx::t_symbol*>(data_name));
if (!is_matrix(space) || !is_matrix(data)) {
    return {};
}

const long slock = lock(space);
const long dlock = lock(data);

mx::t_jit_matrix_info sinfo{}, dinfo{};
char*                 sbp = nullptr;
char*                 dbp = nullptr;
mx::jit_object_method(space, mx::gensym("getinfo"), &sinfo);
mx::jit_object_method(space, mx::gensym("getdata"), &sbp);
mx::jit_object_method(data, mx::gensym("getinfo"), &dinfo);
mx::jit_object_method(data, mx::gensym("getdata"), &dbp);

if (sbp && dbp) {
    const long cx    = static_cast<long>(xin * (sinfo.dim[0] - 1));
    const long cy    = static_cast<long>(yin * (sinfo.dim[1] - 1));
    const long depth = clamp_dim(data_depthclip, dinfo.dim[1]);
    const long width = clamp_dim(data_widthclip, dinfo.dim[0]);

    std::vector<float> w(depth);
    double             sum = 0.0;
    for (long i = 0; i < depth; ++i) {
        w[i] = get_3d(sbp, sinfo, cx, cy, i);
        sum += w[i];
    }

    if (sum != 0.0) {
        const double inv = 1.0 / sum;
        atoms        wout;
        wout.reserve(depth);
        for (long i = 0; i < depth; ++i) {
            w[i] = static_cast<float>(w[i] * inv);
            wout.push_back(w[i]);
        }
        m_weights.send(wout);

        if (data_interpolation) {
            std::vector<float> out(width, 0.0f);
            for (long i = 0; i < depth; ++i) {
                for (long j = 0; j < width; ++j) {
                    out[j] += w[i] * get_2d(dbp, dinfo, j, i);
                }
            }

            atoms dout;
            dout.reserve(width);
            for (long j = 0; j < width; ++j) {
                dout.push_back(out[j]);
            }
            m_out.send(dout);
        }
    }
}

mx::jit_object_method(space, mx::gensym("lock"), reinterpret_cast<void*>(slock));
mx::jit_object_method(data, mx::gensym("lock"), reinterpret_cast<void*>(dlock));
return {};
}
}
;

private:
static bool is_matrix(void* m) {
    return m && mx::jit_object_method(m, mx::gensym("class_jit_matrix"));
}
static long lock(void* m) {
    return reinterpret_cast<long>(mx::jit_object_method(m, mx::gensym("lock"), reinterpret_cast<void*>(1)));
}
static long clamp_dim(int clip, long full) {
    return (clip == 0) ? full : std::clamp(static_cast<long>(clip), 1L, full);
}
static float get_2d(char* bp, const mx::t_jit_matrix_info& info, long x, long y) {
    return reinterpret_cast<float*>(bp + y * info.dimstride[1] + x * info.dimstride[0])[0];
}
static float get_3d(char* bp, const mx::t_jit_matrix_info& info, long x, long y, long z) {
    return reinterpret_cast<float*>(bp + z * info.dimstride[2] + y * info.dimstride[1] + x * info.dimstride[0])[0];
}
}
;

MIN_EXTERNAL(jit_ali);
