/// @file
/// @brief      tap.semitone2ratio — convert semitones to a frequency ratio.
/// @details    Resurrection of the legacy TapTools abstraction of the same name (recovered from
///             the legacy branch), now a native object. The abstraction computed
///             mtof(60 + semitones) / mtof(60), which is exactly 2^(semitones/12) — implemented
///             directly here. Accepts ints, floats, and lists; a bang re-outputs the last result.
/// @author     Timothy Place
/// @copyright  Copyright 2003-2026 Timothy Place. Distributed under the New BSD License.

#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class semitone2ratio : public object<semitone2ratio> {
  public:
    MIN_DESCRIPTION{"Convert units from semitones to a frequency ratio: ratio = 2^(semitones/12). "
                    "Accepts a number or a list of numbers; bang re-outputs the last result."};
    MIN_TAGS{"math"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.shift~, tap.pitchaccum~, mtof, expr"};

    inlet<>  m_in{this, "(number/list) semitones to convert"};
    outlet<> m_out{this, "(number/list) frequency ratio(s)"};

    message<> m_number{this, "number", "Convert semitones to a frequency ratio.",
                       MIN_FUNCTION {
                           m_last = {convert(args[0])};
                           m_out.send(m_last);
                           return {};
                       }};

    message<> list{this, "list", "Convert a list of semitone values to frequency ratios.",
                   MIN_FUNCTION {
                       atoms out;
                       out.reserve(args.size());
                       for (const auto& a : args) {
                           out.push_back(convert(a));
                       }
                       m_last = out;
                       m_out.send(m_last);
                       return {};
                   }};

    message<> bang{this, "bang", "Re-output the last conversion.",
                   MIN_FUNCTION {
                       if (!m_last.empty()) {
                           m_out.send(m_last);
                       }
                       return {};
                   }};

  private:
    atoms m_last;

    static double convert(double semitones) { return std::exp2(semitones / 12.0); }
};

MIN_EXTERNAL(semitone2ratio);
