/// @file
/// @brief      tap.midimapper — map MIDI input to a user-defined output message.
/// @details    Watches one MIDI message type (note, poly-pressure, control, program, aftertouch, or
///             pitch-bend) on its dedicated inlet and, when the channel and value filters match,
///             emits the `mapto` template — optionally appending the channel and the incoming
///             value(s) per the `includes` flags. Faithful port of the original; pure control logic,
///             no Max MIDI API.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <vector>

#include "c74_min.h"

using namespace c74::min;

class midimapper : public object<midimapper> {
  public:
    MIN_DESCRIPTION{"Map MIDI input to a user-defined output. When the incoming MIDI message "
                    "matches the configured type, channel, and values, the mapto template is sent "
                    "out, optionally with the channel and incoming values appended."};
    MIN_TAGS{"midi"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"midiin, midiparse, router, tap.route"};

    inlet<>  m_in_note{this, "(list) note [pitch velocity]"};
    inlet<>  m_in_poly{this, "(list) poly pressure [note pressure]"};
    inlet<>  m_in_control{this, "(list) control change [controller value]"};
    inlet<>  m_in_program{this, "(int) program change"};
    inlet<>  m_in_touch{this, "(int) channel aftertouch"};
    inlet<>  m_in_bend{this, "(int) pitch bend"};
    inlet<>  m_in_channel{this, "(int) incoming MIDI channel"};
    outlet<> m_out{this, "(list) the mapped output"};
    outlet<> m_dump{this, "(anything) dumpout"};

    attribute<symbol> type{this, "type", "none",
                           range{"none", "note", "polypressure", "control", "program", "touch", "bend", "any"},
                           description{"Which MIDI message type to respond to."}};

    attribute<int> channel{this, "channel", 0, description{"MIDI channel to match (negative = any channel)."}};

    attribute<int> match1{this, "match1", 0,
                          description{"First value to match — pitch/controller/etc. (negative = any)."}};

    attribute<int> match2{this, "match2", 0,
                          description{"Second value to match — velocity/value/etc. (negative = any)."}};

    attribute<std::vector<int>> includes{
        this,
        "includes",
        {0, 0, 0},
        description{"Flags [channel, value1, value2] for which incoming items to append to the output."}};

    message<> mapto{this, "mapto", "Set the output message template.", MIN_FUNCTION{m_mapto = args;
    return {};
}
}
;

message<>    int_msg{this, "int", "Program (inlet 4), aftertouch (5), bend (6), or channel (7).",
                  MIN_FUNCTION{const long val = args[0];
const symbol t = type;
switch (inlet) {
case 3:
    if (t == "program" || t == "any") {
        map_one(val);
    }
    break;
case 4:
    if (t == "touch" || t == "any") {
        map_one(val);
    }
    break;
case 5:
    if (t == "bend" || t == "any") {
        map_one(val);
    }
    break;
case 6:
    m_inchannel = val;
    break;
}
return {};
}
}
;

message<> list_msg{this, "list", "Note (inlet 1), poly pressure (2), or control change (3).",
                   MIN_FUNCTION{if (args.size() < 2){return {};
}
const long   v1 = args[0];
const long   v2 = args[1];
const symbol t  = type;
switch (inlet) {
case 0:
    if (t == "note" || t == "any") {
        map_two(v1, v2);
    }
    break;
case 1:
    if (t == "polypressure" || t == "any") {
        map_two(v1, v2);
    }
    break;
case 2:
    if (t == "control" || t == "any") {
        map_two(v1, v2);
    }
    break;
}
return {};
}
}
;

private:
atoms m_mapto;
long  m_inchannel{0};

bool channel_matches() const {
    const int ch = channel;
    return (ch == m_inchannel) || (ch < 0);
}

int include_flag(size_t i) const {
    const std::vector<int>& f = includes;
    return (i < f.size()) ? f[i] : 0;
}

void map_one(long val) {
    if (!channel_matches()) {
        return;
    }
    if (static_cast<int>(match1) != static_cast<int>(val) && static_cast<int>(match1) >= 0) {
        return;
    }

    atoms out = m_mapto;
    if (include_flag(0)) {
        out.push_back(static_cast<int>(m_inchannel));
    }
    if (include_flag(2)) {
        out.push_back(static_cast<int>(val));
    }
    m_out.send(out);
}

void map_two(long val1, long val2) {
    if (!channel_matches()) {
        return;
    }
    const bool m1 = (static_cast<int>(match1) == static_cast<int>(val1)) || (static_cast<int>(match1) < 0);
    const bool m2 = (static_cast<int>(match2) == static_cast<int>(val2)) || (static_cast<int>(match2) < 0);
    if (!m1 || !m2) {
        return;
    }

    atoms out = m_mapto;
    if (include_flag(0)) {
        out.push_back(static_cast<int>(m_inchannel));
    }
    if (include_flag(1)) {
        out.push_back(static_cast<int>(val1));
    }
    if (include_flag(2)) {
        out.push_back(static_cast<int>(val2));
    }
    m_out.send(out);
}
}
;

MIN_EXTERNAL(midimapper);
