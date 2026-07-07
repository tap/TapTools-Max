/// @file
/// @brief      tap.delay — delay lists, symbols, numbers, etc. by N milliseconds.
/// @details    Send any message (int, float, list, or symbol) into the left inlet and it is held,
///             then sent out the outlet after the delay time has elapsed. The right inlet (or the
///             optional creation argument) sets the delay time in milliseconds. This generalizes
///             Max's `delay` — which only delays bangs — to any message type.
///
///             Like Max's `delay`, the object holds a single pending event: a new input restarts
///             the timer, carrying the most recent message. The original `tap.delay` shipped as an
///             abstraction wrapping Max's `delay` object (a single, retriggerable pending event), so
///             this single-pending model is the faithful behavior.
///
///             This object was RECONSTRUCTED from its surviving maxref documentation
///             (docs/tap.delay.maxref.xml), help patcher, and the original abstraction — the C++
///             source never existed (it was a .maxpat abstraction). The maxref's attribute list
///             (clip / coefficients / gain) is boilerplate erroneously copied from a filter object
///             and is intentionally NOT implemented; the help patcher and abstraction document the
///             actual behavior (2 inlets, 1 outlet, ms delay) reproduced here.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 1999-2026 Timothy Place.

#include <algorithm>
#include <limits>

#include "c74_min.h"

using namespace c74::min;

class delay : public object<delay> {
  public:
    MIN_DESCRIPTION{"Delay lists, symbols, numbers, etc. Send any message (int, float, list, or "
                    "symbol) into the left inlet and it is sent out after the delay time has "
                    "elapsed. The right inlet sets the delay time in milliseconds. Like Max's "
                    "<o>delay</o>, but works for any message, not just bangs."};
    MIN_TAGS{"timing"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"delay, pipe, defer, tap.change"};

    inlet<>  m_in_input{this, "(anything) message to be delayed"};
    inlet<>  m_in_time{this, "(float) delay time in milliseconds"};
    outlet<> m_out{this, "(anything) the delayed message"};

    attribute<number> delaytime{this, "delaytime", 0.0, range{0.0, std::numeric_limits<double>::max()},
                                description{"Delay time in milliseconds."}};

    delay(const atoms& args = {}) {
        if (!args.empty()) {
            delaytime = static_cast<double>(args[0]);
        }
    }

    // Set the delay time (ms) when a number arrives in the right inlet; schedule the pending
    // message when input arrives in the left inlet.
    message<> m_int{this, "int", "Delay an integer, or (right inlet) set the delay time.",
                    MIN_FUNCTION{if (inlet == 1){delaytime = static_cast<double>(args[0]);
} else {
    schedule({static_cast<long>(args[0])});
}
return {};
}
}
;

message<> m_float{this, "float", "Delay a float, or (right inlet) set the delay time.",
                  MIN_FUNCTION{if (inlet == 1){delaytime = static_cast<double>(args[0]);
}
else {
    schedule({static_cast<double>(args[0])});
}
return {};
}
}
;

message<> m_list{this, "list", "Delay a list.", MIN_FUNCTION{if (inlet == 0){schedule(args);
}
return {};
}
}
;

// Min prepends the selector for an 'anything' message, so args is the full message — exactly
// the form needed to re-emit it faithfully.
message<> m_anything{this, "anything", "Delay a symbol or arbitrary message.",
                     MIN_FUNCTION{if (inlet == 0){schedule(args);
}
return {};
}
}
;

message<> stop{this, "stop", "Cancel the pending delayed message.", MIN_FUNCTION{m_timer.stop();
return {};
}
}
;

private:
atoms m_pending{};

// A new input replaces any previously pending message and restarts the clock — matching the
// single-pending, retriggerable behavior of Max's delay.
void schedule(const atoms& message) {
    m_pending = message;
    m_timer.delay(std::max(0.0, static_cast<double>(delaytime)));
}

timer<> m_timer{this, MIN_FUNCTION{m_out.send(m_pending);
return {};
}
}
;
}
;

MIN_EXTERNAL(delay);
