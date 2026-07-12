/// @file
/// @brief      tap.random — generate a random floating-point number.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <cstdint>
#include <cstdlib>

#include "c74_min.h"

using namespace c74::min;

class randomgen : public object<randomgen> {
  public:
    MIN_DESCRIPTION{"Generate a random floating-point number in the range -1 to 1. "
                    "Send a bang to generate the next value."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"random, drunk, decide"};

    inlet<>  m_in{this, "(bang) generate a random number"};
    outlet<> m_out{this, "(float) random number in [-1, 1)"};
    outlet<> m_dump{this, "(anything) dumpout"};

    message<> bang{this, "bang", "Generate a random number.",
                   MIN_FUNCTION{ // Faithful port of the original tap.random: seed from rand(), advance one LCG step,
                       // take 15 bits, and scale into [-1, 1). (uint32_t keeps the 32-bit wrap well-defined.)
                       uint32_t randx     = static_cast<uint32_t>(std::rand());
                       randx              = randx * 1103515245u + 12345u;
                       const int    i     = static_cast<int>((randx >> 16) & 077777);
                       const double value = static_cast<double>(i) / 16384.0 - 1.0;
                       m_out.send(value);
                       return {};
                   }};
};

MIN_EXTERNAL(randomgen);
