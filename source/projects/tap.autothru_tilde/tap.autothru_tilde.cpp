/// @file
/// @brief      tap.autothru~ — automatic signal pass-through.
/// @details    Passes the first inlet's signal straight through, unless a signal is connected to the
///             second inlet — in which case the second inlet is passed instead, cutting off the
///             first. Useful for building optional insert points. Faithful port — plain C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

class autothru : public object<autothru>, public sample_operator<2, 1> {
  public:
    MIN_DESCRIPTION{"Automatically pass a signal through. The first inlet passes through unless a "
                    "signal is connected to the second inlet, which then takes over."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"selector~, gate~, matrix~"};

    inlet<>  m_in1{this, "(signal) passes through if inlet 2 is not connected"};
    inlet<>  m_in2{this, "(signal) cuts off inlet 1 and passes through if connected"};
    outlet<> m_out{this, "(signal) inlet 2 if connected, otherwise inlet 1", "signal"};

    sample operator()(sample in1, sample in2) { return m_in2.has_signal_connection() ? in2 : in1; }
};

MIN_EXTERNAL(autothru);
