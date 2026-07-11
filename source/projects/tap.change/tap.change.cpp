/// @file
/// @brief      tap.change — pass input through only when it differs from the previous input.
/// @details    A generalization of Max's `change`: where `change` works only on numbers,
///             tap.change filters repetitions of *any* message — int, float, list, or symbol.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

class change : public object<change> {
  public:
    MIN_DESCRIPTION{"Filter out repetitions. Like Max's <o>change</o>, but works for any "
                    "message: int, float, list, or symbol. The input is passed through the "
                    "left outlet only when it differs from the previous input; when it is "
                    "identical, a bang is sent from the middle outlet instead. Send to the "
                    "right inlet to set the stored value silently (no output is produced)."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"change, change~, !=, route"};

    inlet<>  m_inlet_input{this, "(anything) value to filter"};
    inlet<>  m_inlet_set{this, "(anything) set the stored value without producing output"};
    outlet<> m_outlet_main{this, "(anything) the input, when it differs from the previous input"};
    outlet<> m_outlet_bang{this, "(bang) sent when the input matches the previous input"};
    outlet<> m_outlet_dump{this, "(anything) dumpout"};

    message<> m_int{this, "int", "Filter an integer.",
                    MIN_FUNCTION{
                        if (inlet == 0) {
                            if (is_novel(args)) {
                                m_outlet_main.send(static_cast<long>(args[0])); // re-emit as a true int
                            }
                            else {
                                m_outlet_bang.send("bang");
                            }
                        }
                        remember(args);
                        return {};
                    }};

    message<> m_float{this, "float", "Filter a float.",
                      MIN_FUNCTION{
                          if (inlet == 0) {
                              if (is_novel(args)) {
                                  m_outlet_main.send(static_cast<double>(args[0])); // re-emit as a true float
                              }
                              else {
                                  m_outlet_bang.send("bang");
                              }
                          }
                          remember(args);
                          return {};
                      }};

    message<> m_list{this, "list", "Filter a list.",
                     MIN_FUNCTION{
                         if (inlet == 0) {
                             if (is_novel(args)) {
                                 m_outlet_main.send(args);
                             }
                             else {
                                 m_outlet_bang.send("bang");
                             }
                         }
                         remember(args);
                         return {};
                     }};

    // For an "anything" message Min prepends the selector, so args == { selector, contents... },
    // which is exactly the form needed to both compare and re-emit it faithfully.
    message<> m_anything{this, "anything", "Filter a symbol or arbitrary message.",
                         MIN_FUNCTION{
                             if (inlet == 0) {
                                 if (is_novel(args)) {
                                     m_outlet_main.send(args);
                                 }
                                 else {
                                     m_outlet_bang.send("bang");
                                 }
                             }
                             remember(args);
                             return {};
                         }};

  private:
    atoms m_previous{};        ///< the previous input, stored verbatim
    bool  m_have_input{false}; ///< false until the first input arrives (so it always passes)

    /// True if the input differs from the previous input (and should be passed through).
    bool is_novel(const atoms& input) const { return !m_have_input || !matches_previous(input); }

    /// Store the input as the new comparison reference (both inlets do this).
    void remember(const atoms& input) {
        m_previous   = input;
        m_have_input = true;
    }

    /// Compare two messages for equality. A difference in atom *type* (e.g. int 5 vs float 5.)
    /// counts as a change — matching the behavior of the original tap.change.
    bool matches_previous(const atoms& input) const {
        if (input.size() != m_previous.size()) {
            return false;
        }
        for (size_t i = 0; i < input.size(); ++i) {
            const c74::max::t_atom& a = input[i];
            const c74::max::t_atom& b = m_previous[i];
            if (a.a_type != b.a_type || a.a_w.w_obj != b.a_w.w_obj) {
                return false;
            }
        }
        return true;
    }
};

MIN_EXTERNAL(change);
