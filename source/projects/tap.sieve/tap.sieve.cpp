/// @file
/// @brief      tap.sieve — only allow a matching value to pass through.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

class sieve : public object<sieve> {
  public:
    MIN_DESCRIPTION{"Only allow a matching value to pass through. When the integer received in "
                    "the left inlet equals the stored value, it is passed to the outlet; "
                    "otherwise nothing is output."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"select, route, gate, change"};

    inlet<>  m_in{this, "(int) value to test"};
    inlet<>  m_in_set{this, "(int) set the value to match"};
    outlet<> m_out{this, "(int) the matched value"};
    outlet<> m_dump{this, "(anything) dumpout"};

    attribute<int> value{this, "value", 0, description{"The value that input must match in order to pass through."}};

    argument<int> value_arg{this, "value", "Initial value to match (same as the value attribute).",
                            MIN_ARGUMENT_FUNCTION{ value = static_cast<int>(arg); }};

    message<> number{this, "int",
                     "Left inlet: pass the value through if it matches. Right inlet: set the value to match.",
                     MIN_FUNCTION{
                         const int input = static_cast<int>(args[0]);
                         if (inlet == 0) {
                             if (value == input) {
                                 m_out.send(input);
                             }
                         }
                         else {
                             value = input;
                         }
                         return {};
                     }};
};

MIN_EXTERNAL(sieve);
