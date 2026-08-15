/// @file
/// @brief      tap.scale — snap a MIDI pitch to the nearest degree of a root and scale.
/// @details    Wrapping tap::tools::garden::scale_quantizer (taptools/garden.h): the entry
///             quantizer that makes tap.garden~'s promise — anything you plant sounds consonant —
///             work. It is the nearest-allowed search (any non-empty scale has a degree within a
///             tritone, so nothing is ever left unsnapped), and it looks UP before it looks down
///             when a pitch sits exactly between two degrees.
///
///             In a patched garden it goes between tap.gardener and tap.bloom, because the wind
///             emits raw pitches on purpose: the scale is the patch's field, not the wind's. On
///             its own it is simply a pitch quantizer, and it will happily sit in front of any
///             note source.
///
///             Quantizing is at ENTRY, which is the same contract the monolith keeps: changing
///             @root or @scale re-pitches nothing already planted downstream, only what passes
///             through next.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>

#include <taptools/garden.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::garden;

// Not named `scale`: Min already has a scale() utility, and the collision makes the whole
// class template-invalid. The Max-side name comes from the project folder, not from this.
class scalesnap : public object<scalesnap> {
  private:
    // Constructed before the attributes below so their defaults can forward into it.
    kernel::scale_quantizer m_quantizer;

  public:
    MIN_DESCRIPTION{"Snap a MIDI pitch to the nearest degree of a root and scale — the entry "
                    "quantizer behind tap.garden~'s promise that anything you plant sounds "
                    "consonant. A float in gives the snapped pitch out; a pitch/velocity pair "
                    "passes the velocity through untouched, so it drops straight into a note "
                    "chain. Between tap.gardener and tap.bloom it is what turns raw wind into "
                    "music; on its own it is a pitch quantizer for any note source."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.garden~, tap.bloom, tap.gardener, tap.tune~, tap.semitone2ratio"};

    inlet<>  m_in{this, "(float) a MIDI pitch, or a (list) <pitch> <velocity> pair"};
    outlet<> m_out{this, "(float/list) the snapped pitch, velocity passed through"};

    attribute<int> root{this,
                        "root",
                        0,
                        setter{MIN_FUNCTION{
                            const int v = ((static_cast<int>(args[0]) % 12) + 12) % 12;
                            m_quantizer.set_root(v);
                            return {v};
                        }},
                        range{0, 11},
                        description{"Root pitch class, 0..11 (0 = C). Values outside the range wrap."}};

    attribute<int> scalemode{this,
                             "scale",
                             kernel::scale_major_pentatonic,
                             setter{MIN_FUNCTION{
                                 const int v = std::clamp(static_cast<int>(args[0]), 0, kernel::k_num_scales - 1);
                                 m_quantizer.set_scale(v);
                                 return {v};
                             }},
                             range{0, kernel::k_num_scales - 1},
                             description{"Scale: 0 chromatic, 1 major, 2 minor, 3 major pentatonic (the default — "
                                         "it is the one where nothing can sound wrong), 4 minor pentatonic. "
                                         "Public-domain scale theory, not any app's preset list."}};

    message<> number{this, "number", "A MIDI pitch (fractional accepted) — the snapped pitch comes out.",
                     MIN_FUNCTION{
                         if (!args.empty()) {
                             m_out.send(m_quantizer.quantize(args[0]));
                         }
                         return {};
                     }};

    message<> list{this, "list",
                   "<pitch> <velocity>: the pitch is snapped, the velocity (and anything after it) passes "
                   "through untouched, so this drops into a note chain without rebuilding the message.",
                   MIN_FUNCTION{
                       if (args.empty()) {
                           return {};
                       }
                       atoms out{m_quantizer.quantize(args[0])};
                       for (size_t i = 1; i < args.size(); ++i) {
                           out.push_back(args[i]);
                       }
                       m_out.send(out);
                       return {};
                   }};

    scalesnap(const atoms& args = {}) {
        if (!args.empty()) {
            root = args[0];
        }
        if (args.size() > 1) {
            scalemode = args[1];
        }
    }
};

MIN_EXTERNAL(scalesnap);
