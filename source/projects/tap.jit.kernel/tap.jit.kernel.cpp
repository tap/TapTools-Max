/// @file
/// @brief      tap.jit.kernel — generate a radial (gaussian-style) kernel matrix.
/// @details    A matrix generator (no matrix input, one matrix output). For every cell it computes
///             a radial falloff value from a configurable center, per-axis size, and overall weight,
///             producing a soft circular "blob". Faithful port of the original Jitter object, rebuilt
///             on Min's matrix_operator (MOP) path — since the object owns no matrix input, the SDK
///             treats it as a generator and drives calc_cell() for each output cell.
///
///             Provenance / faithful quirks reproduced from the 2003 original
///             (source/tap.jit.kernel/tap.jit.kernel.cpp on the `legacy` branch):
///               * The center[] and size[] components are intentionally swapped before use
///                 ("Not sure why, but these need to be reversed" — original comment), so attribute
///                 component 0 drives the y math and component 1 drives the x math. Preserved.
///               * size is applied as its reciprocal (1.0 / size), and weight as its reciprocal
///                 (1.0 / weight) — larger size/weight values therefore *shrink* the blob. Preserved.
///               * The horizontal axis is scaled by width and the vertical by height, but the
///                 original crosses these (row index against width, column index against height).
///                 Preserved exactly.
///               * The value is written only to plane 1; other planes are left untouched (for a
///                 freshly-allocated generator matrix that means 0). Preserved.
///               * BUG FIX: the original wrote each value to column (j-1), shifting the whole kernel
///                 one cell to the left and writing column 0's value to x = -1 (out of bounds — a
///                 stray write into the previous row / before the buffer). The MOP path writes the
///                 cell at its real coordinate, which removes the off-by-one and the OOB write. This
///                 is the one intentional behavioral change from the legacy code.
///
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

// Portable clip, replacing Jamoma's TTClip<float>() used by the original.
template <typename T>
static T clip(T value, T low, T high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

class jit_kernel : public object<jit_kernel>, public matrix_operator<> {
  public:
    MIN_DESCRIPTION{"Generate a radial (gaussian-style) kernel matrix. With no matrix input this "
                    "object acts as a generator, filling the output matrix with a soft circular "
                    "blob whose location, per-axis size, and overall weight are set by attributes."};
    MIN_TAGS{"jitter, generators"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"jit.gradient, jit.bfg, tap.jit.thresh"};

    // Generator MOP: declare ONLY a matrix outlet and no matrix inlet. The SDK keys off the absence
    // of a matrix inlet to wire this up as a generator (it owns its own outputmatrix / jit_matrix).
    outlet<> output{this, "(matrix) the generated kernel", "matrix"};

    attribute<std::vector<double>> center{
        this,
        "center",
        {0.5, 0.5},
        description{"Coordinates (x y, normalized 0..1) for the center of the kernel within the matrix."}};

    attribute<std::vector<double>> size{
        this,
        "size",
        {1.0, 1.0},
        description{"Horizontal and vertical size of the kernel (larger values produce a tighter blob)."}};

    attribute<double> weight{
        this, "weight", 1.0,
        description{"Overall weight applied to the kernel (larger values produce a tighter blob)."}};

    // Called once per output cell by the MOP engine.
    template <typename MatrixType, size_t Planecount>
    cell<MatrixType, Planecount> calc_cell(cell<MatrixType, Planecount> input, const matrix_info& info,
                                           matrix_coord& position) {
        // Start from the (zeroed, for a generator) input cell so only plane 1 is overwritten —
        // matching the original, which set plane 1 only and left the rest as-is.
        cell<MatrixType, Planecount> out = input;

        const long width  = info.width();
        const long height = info.height();

        // The original swaps the center/size components and takes the reciprocal of size/weight.
        const double center0 = center.get()[1]; // reversed (see header note)
        const double center1 = center.get()[0];
        const double size0   = 1.0 / size.get()[1];
        const double size1   = 1.0 / size.get()[0];
        const double w       = 1.0 / weight;

        // position.x() == column (j), position.y() == row (i)
        const long i = position.y();
        const long j = position.x();

        // Reproduce the original per-cell math exactly (including the crossed width/height usage).
        float temp1 = static_cast<float>(i - (width * center0));
        float temp2 = static_cast<float>(j - (height * center1));

        temp1 = temp1 / static_cast<float>(width * 0.5);  // scale down
        temp2 = temp2 / static_cast<float>(height * 0.5); // scale down
        temp1 = temp1 * static_cast<float>(size0);        // size: x
        temp2 = temp2 * static_cast<float>(size1);        // size: y

        float temp3 = std::sqrt((temp1 * temp1) + (temp2 * temp2)); // pythagorean (radius)
        temp3       = temp3 * static_cast<float>(w);                // weight

        temp3 -= 1.0f;
        temp3 *= -1.0f;
        temp3 = clip<float>(temp3, 0.0001f, 1.0f);

        // Original wrote float32 plane 1; here we cast to the matrix's element type for any output
        // type. Plane 1 is the only plane written (other planes pass through untouched).
        if (Planecount > 1) {
            out[1] = static_cast<MatrixType>(temp3);
        }

        return out;
    }
};

MIN_EXTERNAL(jit_kernel);
