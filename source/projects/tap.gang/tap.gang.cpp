/// @file
/// @brief      tap.gang — link several objects together.
/// @details    Four inlets and four outlets. A value (or list) arriving at any inlet is forwarded to
///             the three *other* outlets, deferred to the low-priority thread. Change-detection (a
///             value identical to the last one is ignored) breaks the feedback loop when several
///             tap.gang objects are wired into a ring, so editing any one updates all the others
///             without an infinite cycle. Faithful port of the original (always pure logic — no
///             Jamoma dependency).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2005-2026 Timothy Place.

#include <array>
#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class gang : public object<gang> {
  public:
    MIN_DESCRIPTION{"Gang several objects together so that updating any one of them updates the "
                    "others. A value or list received at any inlet is passed out the other three "
                    "outlets (deferred), while repeated values are filtered out to prevent "
                    "feedback when the objects are connected in a loop."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"send, receive, pattr, change"};

    inlet<> m_in0{this, "(int/float/list) input — forwarded to the other three outlets"};
    inlet<> m_in1{this, "(int/float/list) input — forwarded to the other three outlets"};
    inlet<> m_in2{this, "(int/float/list) input — forwarded to the other three outlets"};
    inlet<> m_in3{this, "(int/float/list) input — forwarded to the other three outlets"};

    outlet<> m_out0{this, "(int/float/list) ganged output"};
    outlet<> m_out1{this, "(int/float/list) ganged output"};
    outlet<> m_out2{this, "(int/float/list) ganged output"};
    outlet<> m_out3{this, "(int/float/list) ganged output"};

    message<> number{this, "number", "A number received at any inlet updates the other three outlets.",
                     MIN_FUNCTION{
                         const double v = bitsafe(static_cast<double>(args[0]));
                         if (v == m_value) { // change-detection: identical value is ignored (breaks feedback loops)
                             return {};
                         }
                         m_value        = v;
                         m_type_is_list = false;
                         fan_out(inlet);
                         return {};
                     }};

    message<> list_msg{this, "list", "A list received at any inlet updates the other three outlets.",
                       MIN_FUNCTION{
                           if (!list_changed(args)) {
                               return {};
                           }
                           m_list.clear();
                           m_list.reserve(args.size());
                           for (const auto& a : args) {
                               m_list.push_back(bitsafe(static_cast<double>(a)));
                           }
                           m_type_is_list = true;
                           fan_out(inlet);
                           return {};
                       }};

  private:
    bool                m_type_is_list{false};
    double              m_value{-2000000.0}; // matches the original's "impossible" initial value
    std::vector<double> m_list;

    std::array<outlet<>*, 4> m_outs{&m_out0, &m_out1, &m_out2, &m_out3};

    // One deferred queue per outlet. Each sends the current value/list to its outlet when serviced.
    queue<>                 m_q0{this,
                 MIN_FUNCTION{
                     send_to(0);
                     return {};
                 }};
    queue<>                 m_q1{this,
                 MIN_FUNCTION{
                     send_to(1);
                     return {};
                 }};
    queue<>                 m_q2{this,
                 MIN_FUNCTION{
                     send_to(2);
                     return {};
                 }};
    queue<>                 m_q3{this,
                 MIN_FUNCTION{
                     send_to(3);
                     return {};
                 }};
    std::array<queue<>*, 4> m_queues{&m_q0, &m_q1, &m_q2, &m_q3};

    static double bitsafe(double in) {
        return std::isnan(in) ? 0.0 : in; // filter out NaNs
    }

    bool list_changed(const atoms& args) const {
        if (args.size() != m_list.size()) {
            return true;
        }
        for (auto i = 0u; i < args.size(); ++i) {
            if (bitsafe(static_cast<double>(args[i])) != m_list[i]) {
                return true;
            }
        }
        return false;
    }

    void fan_out(int source_inlet) {
        for (int j = 0; j < 4; ++j) {
            if (j != source_inlet) {
                m_queues[j]->set();
            }
        }
    }

    void send_to(int j) {
        if (m_type_is_list) {
            atoms out;
            out.reserve(m_list.size());
            for (double v : m_list) {
                out.push_back(v);
            }
            m_outs[j]->send(out);
        }
        else {
            m_outs[j]->send(m_value);
        }
    }
};

MIN_EXTERNAL(gang);
