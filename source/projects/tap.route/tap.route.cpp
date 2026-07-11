/// @file
/// @brief      tap.route — a more flexible route.
/// @details    Routes a whole message to the "matched" outlet when a token at one of the watched
///             positions matches the search string, otherwise to the "unmatched" outlet. Unlike
///             Max's route, the entire incoming message is passed through unchanged. Faithful port
///             of the original TapTools object (no Jamoma dependency — this was always pure logic).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <string>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class route : public object<route> {
  public:
    MIN_DESCRIPTION{"A more flexible route. When a token at one of the watched positions matches "
                    "the search string, the entire incoming message is sent out the left (matched) "
                    "outlet; otherwise it is sent out the right (unmatched) outlet."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"route, select, tap.sieve"};

    inlet<>  m_in{this, "(anything) message to route"};
    outlet<> m_matched{this, "(anything) the message, if it matched"};
    outlet<> m_unmatched{this, "(anything) the message, if it did not match"};
    outlet<> m_dump{this, "(anything) dumpout"};

    attribute<symbol> searchstring{this, "searchstring", "default",
                                   description{"The string to look for at the watched positions."}};

    attribute<std::vector<int>> searchpositions{
        this,
        "searchpositions",
        {1},
        description{"Which positions in the incoming message to test (1 = the first token / the "
                    "leading selector)."}};

    attribute<bool> partialmatch{
        this, "partialmatch", false,
        description{"Match when the input begins with the search string, rather than requiring an "
                    "exact match."}};

    message<> list_msg{this, "list", "Route a list.",
                       MIN_FUNCTION{
                           route_message(args);
                           return {};
                       }};

    // Min prepends the selector for an 'anything' message, so args is the full message. The leading
    // selector is treated as position 1 — exactly matching the original object's behavior.
    message<> anything_msg{this, "anything", "Route a message that begins with a symbol.",
                           MIN_FUNCTION{
                               route_message(args);
                               return {};
                           }};

  private:
    std::string search_text() const {
        const symbol s = searchstring;
        // Go through c_str(): symbol has both operator std::string() and operator const char*(),
        // which makes a direct std::string conversion ambiguous under MSVC.
        return std::string(s.c_str());
    }

    // Render an incoming atom as the string we will compare against the search text.
    static std::string atom_text(const atom& a) {
        switch (a.type()) {
        case message_type::int_argument:
            return std::to_string(static_cast<int>(a));
        case message_type::float_argument:
            return std::to_string(static_cast<double>(a));
        case message_type::symbol_argument:
            return std::string(static_cast<symbol>(a).c_str());
        default:
            return {};
        }
    }

    bool position_is_watched(int position) const {
        for (int p : static_cast<const std::vector<int>&>(searchpositions)) {
            if (p == position) {
                return true;
            }
        }
        return false;
    }

    bool matches(const std::string& input, const std::string& search) const {
        if (partialmatch) {
            if (input.size() < search.size()) {
                return false;
            }
            return input.compare(0, search.size(), search) == 0;
        }
        return input == search;
    }

    void route_message(const atoms& args) {
        const std::string search  = search_text();
        bool              matched = false;

        for (auto i = 0u; i < args.size(); ++i) {
            const int position = static_cast<int>(i) + 1; // element 0 is position 1
            if (position_is_watched(position) && matches(atom_text(args[i]), search)) {
                matched = true;
                break;
            }
        }

        if (matched) {
            m_matched.send(args);
        }
        else {
            m_unmatched.send(args);
        }
    }
};

MIN_EXTERNAL(route);
