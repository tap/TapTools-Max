/// @file
/// @brief      tap.jit.sum — sum the values in a Jitter matrix.
/// @details    Receives a `jit_matrix` message, reads the named matrix, and outputs the sum of all
///             of its cell values (across every plane). Faithful port of the original's default
///             mode; rebuilt as a plain Min object that reads the matrix through the Jitter API
///             (`c74::max`), since this is a matrix→value reduction rather than a matrix→matrix
///             operator.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;
namespace mx = c74::max;

class jit_sum : public object<jit_sum> {
  public:
    MIN_DESCRIPTION{"Sum the values in a Jitter matrix. On a jit_matrix message, outputs the total "
                    "of all cell values across all planes."};
    MIN_TAGS{"jitter"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"jit.3m, jit.findbounds, jit.iter"};

    inlet<>  m_in{this, "(matrix) jit_matrix to analyze"};
    outlet<> m_out{this, "(int/float) sum of the matrix values"};
    outlet<> m_dump{this, "(anything) dumpout"};

    message<> jit_matrix{
        this, "jit_matrix", "Sum the named matrix and output the result.",
        MIN_FUNCTION{
            if (args.empty()) {
                return {};
            }

            auto* matrix = mx::jit_object_findregistered(args[0]);
            if (!matrix || !mx::jit_object_method(matrix, mx::gensym("class_jit_matrix"))) {
                return {};
            }

            const long savelock =
                reinterpret_cast<long>(mx::jit_object_method(matrix, mx::gensym("lock"), reinterpret_cast<void*>(1)));

            mx::t_jit_matrix_info info{};
            mx::jit_object_method(matrix, mx::gensym("getinfo"), &info);
            char* data = nullptr;
            mx::jit_object_method(matrix, mx::gensym("getdata"), &data);

            if (data) {
                const double sum     = sum_matrix(info, data);
                const bool   integer = (info.type == mx::gensym("char") || info.type == mx::gensym("long"));
                if (integer) {
                    m_out.send(static_cast<int>(sum));
                }
                else {
                    m_out.send(sum);
                }
            }

            mx::jit_object_method(matrix, mx::gensym("lock"), reinterpret_cast<void*>(savelock));
            m_dump.send("done");
            return {};
        }};

  private:
    static double sum_matrix(const mx::t_jit_matrix_info& info, char* data) {
        const long width   = info.dim[0];
        const long height  = (info.dimcount > 1) ? info.dim[1] : 1;
        const long planes  = info.planecount;
        const long rstride = info.dimstride[1];
        const long cstride = info.dimstride[0];

        double sum = 0.0;
        for (long y = 0; y < height; ++y) {
            char* row = data + y * rstride;
            for (long x = 0; x < width; ++x) {
                char* cell = row + x * cstride;
                for (long p = 0; p < planes; ++p) {
                    if (info.type == mx::gensym("char")) {
                        sum += reinterpret_cast<unsigned char*>(cell)[p];
                    }
                    else if (info.type == mx::gensym("long")) {
                        sum += reinterpret_cast<mx::t_int32*>(cell)[p];
                    }
                    else if (info.type == mx::gensym("float32")) {
                        sum += reinterpret_cast<float*>(cell)[p];
                    }
                    else if (info.type == mx::gensym("float64")) {
                        sum += reinterpret_cast<double*>(cell)[p];
                    }
                }
            }
        }
        return sum;
    }
};

MIN_EXTERNAL(jit_sum);
