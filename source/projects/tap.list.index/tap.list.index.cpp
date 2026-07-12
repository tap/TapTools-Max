/// @file
/// @brief      tap.list.index — construct or decompose lists by element.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2005-2026 Timothy Place.

#include <algorithm>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class list_index : public object<list_index> {
  public:
    MIN_DESCRIPTION{"Construct or decompose lists by element. In list2indexed mode each element "
                    "of the incoming list is output as an [index value] pair. In indexed2list "
                    "mode incoming [index value] pairs are collected and the assembled list is "
                    "output."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"zl, listfunnel, spray"};

    inlet<>  m_in{this, "(list) a list, or an [index value] pair"};
    outlet<> m_out{this, "(list) the result"};
    outlet<> m_dump{this, "(anything) dumpout"};

    attribute<symbol> mode{this, "mode", "list2indexed", range{"indexed2list", "list2indexed"},
                           description{"list2indexed: emit one [index value] pair per element. "
                                       "indexed2list: collect [index value] pairs into a list."}};

    attribute<int> offset{this, "offset", 0,
                          description{"A constant added to (list2indexed) or removed from (indexed2list) each index."}};

    attribute<bool> onebased{this, "onebased", false, description{"Use one-based indices instead of zero-based."}};

    message<> list_msg{this, "list", "Process a list.",
                       MIN_FUNCTION{
                           process(args);
                           return {};
                       }};

    // For an 'anything' message Min prepends the selector, so `args` is the full message — exactly
    // what we want to treat as the incoming list (this handles lists that begin with a symbol).
    message<> anything_msg{this, "anything", "Process a list that begins with a symbol.",
                           MIN_FUNCTION{
                               process(args);
                               return {};
                           }};

  private:
    static constexpr int k_max_length{512};
    std::vector<atom>    m_store = std::vector<atom>(k_max_length, atom{0});
    int                  m_length{0};

    void process(const atoms& args) {
        const int    bias = static_cast<int>(offset) + (onebased ? 1 : 0);
        const symbol m    = mode;

        if (m == "indexed2list") {
            if (args.size() > 1) {
                int idx      = static_cast<int>(args[0]) - bias;
                idx          = std::clamp(idx, 0, k_max_length - 1);
                m_store[idx] = args[1];
                if (idx + 1 > m_length) {
                    m_length = idx + 1;
                }
            }
            m_out.send(atoms(m_store.begin(), m_store.begin() + m_length));
        }
        else { // list2indexed
            for (auto i = 0; i < static_cast<int>(args.size()); ++i) {
                m_out.send(atoms{i + bias, args[i]});
            }
            m_dump.send("done");
        }
    }
};

MIN_EXTERNAL(list_index);
