/// @file
/// @brief      tap.jit.colortrack — track up to four colors in a Jitter matrix and report their
///             bounds, center, and size.
/// @details    On a `jit_matrix` message the named 4-plane (ARGB) char matrix is read through the
///             Jitter API (`c74::max`). For each of four independent trackers, every pixel is
///             converted RGB→HSL and tested against that tracker's hue/saturation/brightness range;
///             the bounding box of all matching pixels is accumulated and, depending on the
///             tracker's output flags, emitted as a `bounds` / `center` / `size` list prefixed with
///             the 1-based tracker number. Faithful port of the 2003 object — the integer RGB→HSL
///             conversion, the hue color-wheel wrap test, the TTClip/onewrap bound math, the
///             0x7FFFFFFF/-1 sentinel handling, and the list output formats are all preserved.
///             Rebuilt as a plain Min object since this is a matrix→list analysis op (no matrix out).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;
namespace mx = c74::max;

// Plane indices into the per-pixel HSL test (ARGB source order; alpha plane 0 ignored).
static constexpr int k_hue        = 1;
static constexpr int k_saturation = 2;
static constexpr int k_lightness  = 3; // a.k.a. BRIGHTNESS / LUMINANCE in the original

static constexpr int k_num_trackers = 4;
static constexpr int k_num_planes   = 4;

// Clamp utility, matching Jamoma's TTClip (input below low -> low, above high -> high).
static long ttclip(long value, long low, long high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

// onewrap utility, ported verbatim from tt_audio_base::onewrap — wraps a value into [low, high).
static long onewrap(long value, long low, long high) {
    if ((value >= low) && (value < high)) {
        return value;
    }
    else if (value >= high) {
        return (low - 1) + (value - high);
    }
    else {
        return (high + 1) - (low - value);
    }
}

// RGB-2-HSL — verbatim integer port of the original (hue 0-360, sat/light 0-255).
static void rgb2hsl(unsigned char red, unsigned char green, unsigned char blue, short* hue, short* saturation,
                    short* lightness) {
    short max, min;
    short L, S = 0;
    float H = 0;
    float delta;

    min = red;
    if (min > green) {
        min = green;
    }
    if (min > blue) {
        min = blue;
    }

    max = red;
    if (max < green) {
        max = green;
    }
    if (max < blue) {
        max = blue;
    }

    L = (max + min) / 2; // the most L could be is 255

    delta = max - min;

    if (delta) {
        if (L < 128) {
            S = static_cast<short>(255.0f * (delta / static_cast<float>(max + min)));
        }
        else {
            S = static_cast<short>(255.0f * (delta / static_cast<float>(511 - max - min)));
        }

        if (red == max) {
            H = (green - blue) / delta;
        }
        else if (green == max) {
            H = 2 + (blue - red) / delta;
        }
        else if (blue == max) {
            H = 4 + (red - green) / delta;
        }

        H *= 60;
        if (H < 0) {
            H += 360;
        }
    }

    *hue        = static_cast<short>(H); // range: 0 - 360
    *saturation = S;                     // range: 0 - 255
    *lightness  = L;                     // range: 0 - 255
}

class jit_colortrack : public object<jit_colortrack> {
  public:
    MIN_DESCRIPTION{"Match up to four colors in a Jitter matrix and report their bounds, center, "
                    "and size. On a jit_matrix message, each pixel is converted to HSL and tested "
                    "against each tracker's hue/saturation/brightness range; the bounding box of "
                    "matching pixels is output as a list prefixed with the tracker number."};
    MIN_TAGS{"jitter"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"jit.findbounds, tap.jit.proximity, tap.jit.sum"};

    inlet<>  m_in{this, "(matrix) jit_matrix to analyze"};
    outlet<> m_out{this, "(list) tracker results: index bounds/center/size ..."};
    outlet<> m_dump{this, "(anything) dumpout"};

    jit_colortrack(const atoms& args = {}) {
        // Recompute all derived bounds from the (default-zero) attributes.
        for (int t = 0; t < k_num_trackers; ++t) {
            recompute_hue(t);
            recompute_saturation(t);
            recompute_brightness(t);
        }
    }

    // ---- per-tracker output flags -------------------------------------------------------------
    attribute<bool> output_bounds_1{this, "output_bounds_1", false, setter{MIN_FUNCTION{return set_bounds(0, args);
}
}
, description {
    "When 1, output the bounds of color tracker 1."
}
}
;
attribute<bool> output_center_1{this, "output_center_1", false, setter{MIN_FUNCTION{return set_center(0, args);
}
}
, description {
    "When 1, output the center of color tracker 1."
}
}
;
attribute<bool> output_size_1{this, "output_size_1", false, setter{MIN_FUNCTION{return set_size(0, args);
}
}
, description {
    "When 1, output the size of color tracker 1."
}
}
;

attribute<bool> output_bounds_2{this, "output_bounds_2", false, setter{MIN_FUNCTION{return set_bounds(1, args);
}
}
, description {
    "When 1, output the bounds of color tracker 2."
}
}
;
attribute<bool> output_center_2{this, "output_center_2", false, setter{MIN_FUNCTION{return set_center(1, args);
}
}
, description {
    "When 1, output the center of color tracker 2."
}
}
;
attribute<bool> output_size_2{this, "output_size_2", false, setter{MIN_FUNCTION{return set_size(1, args);
}
}
, description {
    "When 1, output the size of color tracker 2."
}
}
;

attribute<bool> output_bounds_3{this, "output_bounds_3", false, setter{MIN_FUNCTION{return set_bounds(2, args);
}
}
, description {
    "When 1, output the bounds of color tracker 3."
}
}
;
attribute<bool> output_center_3{this, "output_center_3", false, setter{MIN_FUNCTION{return set_center(2, args);
}
}
, description {
    "When 1, output the center of color tracker 3."
}
}
;
attribute<bool> output_size_3{this, "output_size_3", false, setter{MIN_FUNCTION{return set_size(2, args);
}
}
, description {
    "When 1, output the size of color tracker 3."
}
}
;

attribute<bool> output_bounds_4{this, "output_bounds_4", false, setter{MIN_FUNCTION{return set_bounds(3, args);
}
}
, description {
    "When 1, output the bounds of color tracker 4."
}
}
;
attribute<bool> output_center_4{this, "output_center_4", false, setter{MIN_FUNCTION{return set_center(3, args);
}
}
, description {
    "When 1, output the center of color tracker 4."
}
}
;
attribute<bool> output_size_4{this, "output_size_4", false, setter{MIN_FUNCTION{return set_size(3, args);
}
}
, description {
    "When 1, output the size of color tracker 4."
}
}
;

// ---- per-tracker hue / saturation / brightness --------------------------------------------
attribute<int> hue_1{this, "hue_1", 0, setter{MIN_FUNCTION{return set_hue(0, args);
}
}
, description {
    "Hue to match for tracker 1 (degrees, 0-359)."
}
}
;
attribute<int> hue_variation_1{this, "hue_variation_1", 0, setter{MIN_FUNCTION{return set_hue_var(0, args);
}
}
, description {
    "Acceptable hue variation for tracker 1 (degrees)."
}
}
;
attribute<int> hue_2{this, "hue_2", 0, setter{MIN_FUNCTION{return set_hue(1, args);
}
}
, description {
    "Hue to match for tracker 2 (degrees, 0-359)."
}
}
;
attribute<int> hue_variation_2{this, "hue_variation_2", 0, setter{MIN_FUNCTION{return set_hue_var(1, args);
}
}
, description {
    "Acceptable hue variation for tracker 2 (degrees)."
}
}
;
attribute<int> hue_3{this, "hue_3", 0, setter{MIN_FUNCTION{return set_hue(2, args);
}
}
, description {
    "Hue to match for tracker 3 (degrees, 0-359)."
}
}
;
attribute<int> hue_variation_3{this, "hue_variation_3", 0, setter{MIN_FUNCTION{return set_hue_var(2, args);
}
}
, description {
    "Acceptable hue variation for tracker 3 (degrees)."
}
}
;
attribute<int> hue_4{this, "hue_4", 0, setter{MIN_FUNCTION{return set_hue(3, args);
}
}
, description {
    "Hue to match for tracker 4 (degrees, 0-359)."
}
}
;
attribute<int> hue_variation_4{this, "hue_variation_4", 0, setter{MIN_FUNCTION{return set_hue_var(3, args);
}
}
, description {
    "Acceptable hue variation for tracker 4 (degrees)."
}
}
;

attribute<int> saturation_1{this, "saturation_1", 0, setter{MIN_FUNCTION{return set_sat(0, args);
}
}
, description {
    "Saturation to match for tracker 1 (0-255)."
}
}
;
attribute<int> saturation_variation_1{this, "saturation_variation_1", 0,
                                      setter{MIN_FUNCTION{return set_sat_var(0, args);
}
}
, description {
    "Acceptable saturation variation for tracker 1 (0-255)."
}
}
;
attribute<int> saturation_2{this, "saturation_2", 0, setter{MIN_FUNCTION{return set_sat(1, args);
}
}
, description {
    "Saturation to match for tracker 2 (0-255)."
}
}
;
attribute<int> saturation_variation_2{this, "saturation_variation_2", 0,
                                      setter{MIN_FUNCTION{return set_sat_var(1, args);
}
}
, description {
    "Acceptable saturation variation for tracker 2 (0-255)."
}
}
;
attribute<int> saturation_3{this, "saturation_3", 0, setter{MIN_FUNCTION{return set_sat(2, args);
}
}
, description {
    "Saturation to match for tracker 3 (0-255)."
}
}
;
attribute<int> saturation_variation_3{this, "saturation_variation_3", 0,
                                      setter{MIN_FUNCTION{return set_sat_var(2, args);
}
}
, description {
    "Acceptable saturation variation for tracker 3 (0-255)."
}
}
;
attribute<int> saturation_4{this, "saturation_4", 0, setter{MIN_FUNCTION{return set_sat(3, args);
}
}
, description {
    "Saturation to match for tracker 4 (0-255)."
}
}
;
attribute<int> saturation_variation_4{this, "saturation_variation_4", 0,
                                      setter{MIN_FUNCTION{return set_sat_var(3, args);
}
}
, description {
    "Acceptable saturation variation for tracker 4 (0-255)."
}
}
;

attribute<int> brightness_1{this, "brightness_1", 0, setter{MIN_FUNCTION{return set_bri(0, args);
}
}
, description {
    "Brightness to match for tracker 1 (0-255)."
}
}
;
attribute<int> brightness_variation_1{this, "brightness_variation_1", 0,
                                      setter{MIN_FUNCTION{return set_bri_var(0, args);
}
}
, description {
    "Acceptable brightness variation for tracker 1 (0-255)."
}
}
;
attribute<int> brightness_2{this, "brightness_2", 0, setter{MIN_FUNCTION{return set_bri(1, args);
}
}
, description {
    "Brightness to match for tracker 2 (0-255)."
}
}
;
attribute<int> brightness_variation_2{this, "brightness_variation_2", 0,
                                      setter{MIN_FUNCTION{return set_bri_var(1, args);
}
}
, description {
    "Acceptable brightness variation for tracker 2 (0-255)."
}
}
;
attribute<int> brightness_3{this, "brightness_3", 0, setter{MIN_FUNCTION{return set_bri(2, args);
}
}
, description {
    "Brightness to match for tracker 3 (0-255)."
}
}
;
attribute<int> brightness_variation_3{this, "brightness_variation_3", 0,
                                      setter{MIN_FUNCTION{return set_bri_var(2, args);
}
}
, description {
    "Acceptable brightness variation for tracker 3 (0-255)."
}
}
;
attribute<int> brightness_4{this, "brightness_4", 0, setter{MIN_FUNCTION{return set_bri(3, args);
}
}
, description {
    "Brightness to match for tracker 4 (0-255)."
}
}
;
attribute<int> brightness_variation_4{this, "brightness_variation_4", 0,
                                      setter{MIN_FUNCTION{return set_bri_var(3, args);
}
}
, description {
    "Acceptable brightness variation for tracker 4 (0-255)."
}
}
;

message<> jit_matrix{this, "jit_matrix", "Analyze the named matrix and output tracker results.",
                     MIN_FUNCTION{if (args.empty()){return {};
}

auto* matrix = mx::jit_object_findregistered(args[0]);
if (!matrix || !mx::jit_object_method(matrix, mx::gensym("class_jit_matrix"))) {
    return {};
}

const long savelock =
    reinterpret_cast<long>(mx::jit_object_method(matrix, mx::gensym("lock"), reinterpret_cast<void*>(1)));

mx::t_jit_matrix_info info{};
mx::jit_object_method(matrix, mx::gensym("getinfo"), &info);
char* bp = nullptr;
mx::jit_object_method(matrix, mx::gensym("getdata"), &bp);

if (bp) {
    long dim[2];
    dim[0] = info.dim[0]; // width
    dim[1] = info.dim[1]; // height
    calculate(dim, info, bp);
}
else {
    cerr << "Invalid input" << endl;
}

mx::jit_object_method(matrix, mx::gensym("lock"), reinterpret_cast<void*>(savelock));
return {};
}
}
;

private:
// Used internally — based on the three output flags; true when any output is requested.
char m_use[k_num_trackers]{};
// Indicates that the hue match range crosses 0 on the color wheel.
char m_hue_check[k_num_trackers]{};
// Low/high bounds (HSL planes; alpha plane unused), derived from the public attributes.
long m_min[k_num_trackers][k_num_planes]{};
long m_max[k_num_trackers][k_num_planes]{};

// Stored attribute values (mirror of the public attributes, for the derivation math).
long m_hue[k_num_trackers]{}, m_hue_var[k_num_trackers]{};
long m_sat[k_num_trackers]{}, m_sat_var[k_num_trackers]{};
long m_bri[k_num_trackers]{}, m_bri_var[k_num_trackers]{};
bool m_bounds[k_num_trackers]{}, m_center[k_num_trackers]{}, m_size[k_num_trackers]{};

void recompute_hue(int t) {
    m_min[t][k_hue] = onewrap(m_hue[t] - m_hue_var[t], 0L, 359L);
    m_max[t][k_hue] = onewrap(m_hue[t] + m_hue_var[t], 0L, 359L);
    m_hue_check[t]  = (m_min[t][k_hue] > m_max[t][k_hue]) ? 1 : 0;
}
void recompute_saturation(int t) {
    m_min[t][k_saturation] = ttclip(m_sat[t] - m_sat_var[t], 0L, 255L);
    m_max[t][k_saturation] = ttclip(m_sat[t] + m_sat_var[t], 0L, 255L);
}
void recompute_brightness(int t) {
    m_min[t][k_lightness] = ttclip(m_bri[t] - m_bri_var[t], 0L, 255L);
    m_max[t][k_lightness] = ttclip(m_bri[t] + m_bri_var[t], 0L, 255L);
}
void recompute_use(int t) {
    m_use[t] = (m_bounds[t] || m_center[t] || m_size[t]) ? 1 : 0;
}

atoms set_hue(int t, const atoms& args) {
    m_hue[t] = ttclip(static_cast<long>(args[0]), 0L, 359L);
    recompute_hue(t);
    return {m_hue[t]};
}
atoms set_hue_var(int t, const atoms& args) {
    m_hue_var[t] = ttclip(static_cast<long>(args[0]), 0L, 359L);
    recompute_hue(t);
    return {m_hue_var[t]};
}
atoms set_sat(int t, const atoms& args) {
    m_sat[t] = ttclip(static_cast<long>(args[0]), 0L, 255L);
    recompute_saturation(t);
    return {m_sat[t]};
}
atoms set_sat_var(int t, const atoms& args) {
    m_sat_var[t] = ttclip(static_cast<long>(args[0]), 0L, 255L);
    recompute_saturation(t);
    return {m_sat_var[t]};
}
atoms set_bri(int t, const atoms& args) {
    m_bri[t] = ttclip(static_cast<long>(args[0]), 0L, 255L);
    recompute_brightness(t);
    return {m_bri[t]};
}
atoms set_bri_var(int t, const atoms& args) {
    m_bri_var[t] = ttclip(static_cast<long>(args[0]), 0L, 255L);
    recompute_brightness(t);
    return {m_bri_var[t]};
}
// Output-flag setters mirror their value into the bounds/center/size store and refresh m_use,
// matching the original's colortrack_setoutput{bounds,center,size} helpers.
atoms set_bounds(int t, const atoms& args) {
    m_bounds[t] = static_cast<bool>(args[0]);
    recompute_use(t);
    return args;
}
atoms set_center(int t, const atoms& args) {
    m_center[t] = static_cast<bool>(args[0]);
    recompute_use(t);
    return args;
}
atoms set_size(int t, const atoms& args) {
    m_size[t] = static_cast<bool>(args[0]);
    recompute_use(t);
    return args;
}

void calculate(long* dim, const mx::t_jit_matrix_info& info, char* bip) {
    const long n = dim[0];

    long min_x[k_num_trackers], min_y[k_num_trackers], max_x[k_num_trackers], max_y[k_num_trackers];
    for (int t = 0; t < k_num_trackers; ++t) {
        min_x[t] = min_y[t] = 0x7FFFFFFF; // a really high number — looking for a smaller pixel
        max_x[t] = max_y[t] = -1;
    }

    char inrange_flag_x[k_num_trackers];
    char inrange_flag_y[k_num_trackers];

    for (long i = 0; i < dim[1]; ++i) { // ITERATE THROUGH THE Y-AXIS
        unsigned char* ip = reinterpret_cast<unsigned char*>(bip + i * info.dimstride[1]);
        for (int t = 0; t < k_num_trackers; ++t) {
            inrange_flag_y[t] = 0;
        }

        for (long j = 0; j < n; ++j) { // ITERATE THROUGH THE X-AXIS
            ++ip;                      // skip the alpha channel
            unsigned char red   = *ip++;
            unsigned char green = *ip++;
            unsigned char blue  = *ip++;

            short hue, saturation, lightness;
            rgb2hsl(red, green, blue, &hue, &saturation, &lightness);

            for (int tracker = 0; tracker < k_num_trackers; ++tracker) {
                if (!m_use[tracker]) {
                    continue;
                }
                inrange_flag_x[tracker] = 1;

                if (m_hue_check[tracker]) { // HUE — range crosses 0 on the color wheel
                    if (hue > m_min[tracker][k_hue]) {
                        inrange_flag_x[tracker] = 0;
                    }
                    else if (hue < m_max[tracker][k_hue]) {
                        inrange_flag_x[tracker] = 0;
                    }
                }
                else { // HUE — normal
                    if (hue < m_min[tracker][k_hue]) {
                        inrange_flag_x[tracker] = 0;
                    }
                    else if (hue > m_max[tracker][k_hue]) {
                        inrange_flag_x[tracker] = 0;
                    }
                }

                if (saturation < m_min[tracker][k_saturation]) { // SATURATION
                    inrange_flag_x[tracker] = 0;
                }
                else if (saturation > m_max[tracker][k_saturation]) {
                    inrange_flag_x[tracker] = 0;
                }

                if (lightness < m_min[tracker][k_lightness]) { // LIGHTNESS
                    inrange_flag_x[tracker] = 0;
                }
                else if (lightness > m_max[tracker][k_lightness]) {
                    inrange_flag_x[tracker] = 0;
                }

                if (inrange_flag_x[tracker]) {   // ANY TIME A PIXEL IS TRUE
                    inrange_flag_y[tracker] = 1; // set the Y-axis flag to true
                    if (j < min_x[tracker]) {    // update min_x / max_x with X location
                        min_x[tracker] = j;
                    }
                    else if (j > max_x[tracker]) {
                        max_x[tracker] = j;
                    }
                }
            }
        }

        for (int tracker = 0; tracker < k_num_trackers; ++tracker) {
            if (m_use[tracker] && inrange_flag_y[tracker]) { // the y-axis flag was set in the x loop
                if (i < min_y[tracker]) {                    // update min_y / max_y with Y location
                    min_y[tracker] = i;
                }
                else if (i > max_y[tracker]) {
                    max_y[tracker] = i;
                }
            }
        }
    }

    // ***************
    // POST PROCESSING
    for (int tracker = 0; tracker < k_num_trackers; ++tracker) {
        if (!m_use[tracker]) {
            continue;
        }

        bool no_match = true;
        if (min_x[tracker] == 0x7FFFFFFF) {
            min_x[tracker] = -1;
        }
        else {
            no_match = false;
        }

        if (min_y[tracker] == 0x7FFFFFFF) {
            min_y[tracker] = -1;
        }
        else {
            no_match = false;
        }

        if (max_x[tracker] == -1) {
            max_x[tracker] = min_x[tracker];
        }
        else {
            no_match = false;
        }

        if (max_y[tracker] == -1) {
            max_y[tracker] = min_y[tracker];
        }
        else {
            no_match = false;
        }

        if (no_match) {
            continue;
        }

        // Normalize
        float normalized_bounds[4];
        normalized_bounds[0] = static_cast<float>(min_x[tracker]) / dim[0];
        normalized_bounds[1] = static_cast<float>(min_y[tracker]) / dim[1];
        normalized_bounds[2] = static_cast<float>(max_x[tracker]) / dim[0];
        normalized_bounds[3] = static_cast<float>(max_y[tracker]) / dim[1];

        if (m_bounds[tracker]) {
            m_out.send(tracker + 1, "bounds", normalized_bounds[0], normalized_bounds[1], normalized_bounds[2],
                       normalized_bounds[3]);
        }
        if (m_center[tracker]) {
            m_out.send(tracker + 1, "center", (normalized_bounds[2] + normalized_bounds[0]) * 0.5,
                       (normalized_bounds[3] + normalized_bounds[1]) * 0.5);
        }
        if (m_size[tracker]) {
            m_out.send(tracker + 1, "size",
                       (normalized_bounds[2] - normalized_bounds[0]) * (normalized_bounds[3] - normalized_bounds[1]));
        }
    }
}
}
;

MIN_EXTERNAL(jit_colortrack);
