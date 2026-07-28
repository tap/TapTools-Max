/// @file
/// @brief      Unit tests for tap.elixir~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"   // required unit-test header (defines main via Catch)
#include "tap.elixir_tilde.cpp" // include the object source so we can instantiate it

namespace {

    // The output is scaled by a fixed 0.95 after the equal-share mix — the original's headroom trim.
    constexpr double k_master = 0.95;

    // Run one block through the two default channels, holding each channel at a constant value.
    std::vector<double> mix(elixir& object, double ch0, double ch1, size_t n = 16) {
        std::vector<double> a(n, ch0);
        std::vector<double> b(n, ch1);
        std::vector<double> output(n, 0.0);
        double*             inp[2]  = {a.data(), b.data()};
        double*             outp[1] = {output.data()};
        audio_bundle        ina{inp, 2, static_cast<long>(n)};
        audio_bundle        outa{outp, 1, static_cast<long>(n)};
        object(ina, outa);
        return output;
    }

} // namespace

SCENARIO("tap.elixir~ mutes every channel until one is toggled on") {
    ext_main(nullptr);

    GIVEN("a default (2-inlet) instance") {
        test_wrapper<elixir> an_instance;
        elixir&              my_object = an_instance;

        THEN("nothing passes — every channel's gain starts at zero") {
            for (double s : mix(my_object, 1.0, 1.0)) {
                REQUIRE(s == 0.0);
            }
        }
    }
}

SCENARIO("tap.elixir~ gives a single active channel the whole gain") {
    ext_main(nullptr);

    GIVEN("channel 0 toggled on with no slew") {
        test_wrapper<elixir> an_instance;
        elixir&              my_object = an_instance;
        my_object.m_int(atoms{1}, 0);

        THEN("channel 0 passes at unity (times the 0.95 trim) and channel 1 is silent") {
            for (double s : mix(my_object, 0.5, 1.0)) {
                REQUIRE(std::abs(s - 0.5 * k_master) < 1e-12);
            }
        }
    }
}

SCENARIO("tap.elixir~ shares the gain equally between active channels") {
    ext_main(nullptr);

    GIVEN("both channels toggled on with no slew") {
        test_wrapper<elixir> an_instance;
        elixir&              my_object = an_instance;
        my_object.m_int(atoms{1}, 0);
        my_object.m_int(atoms{1}, 1);

        THEN("each contributes half — the sum of the gains is still one") {
            for (double s : mix(my_object, 1.0, 0.5)) {
                REQUIRE(std::abs(s - (1.0 * 0.5 + 0.5 * 0.5) * k_master) < 1e-12);
            }
        }
    }

    GIVEN("both on, then channel 1 toggled back off") {
        test_wrapper<elixir> an_instance;
        elixir&              my_object = an_instance;
        my_object.m_int(atoms{1}, 0);
        my_object.m_int(atoms{1}, 1);
        my_object.m_int(atoms{0}, 1);

        THEN("channel 0 is back to the full share") {
            for (double s : mix(my_object, 1.0, 1.0)) {
                REQUIRE(std::abs(s - 1.0 * k_master) < 1e-12);
            }
        }
    }
}

SCENARIO("tap.elixir~ slews a gain change instead of stepping it") {
    ext_main(nullptr);

    GIVEN("a 10 ms global slew time") {
        test_wrapper<elixir> an_instance;
        elixir&              my_object = an_instance;
        my_object.gtime(atoms{10.0});

        WHEN("channel 0 is toggled on") {
            my_object.m_int(atoms{1}, 0);
            const std::vector<double> out = mix(my_object, 1.0, 0.0, 64);

            THEN("the gain ramps up rather than jumping to its target") {
                REQUIRE(out[0] > 0.0);
                REQUIRE(out[0] < 0.05 * k_master); // 10 ms is 441 samples; 64 in is still early
                for (size_t i = 1; i < out.size(); ++i) {
                    REQUIRE(out[i] > out[i - 1]);
                }
            }
            THEN("it reaches the full share once the slew completes") {
                mix(my_object, 1.0, 0.0, 1024);
                const std::vector<double> settled = mix(my_object, 1.0, 0.0, 16);
                REQUIRE(std::abs(settled.back() - 1.0 * k_master) < 1e-9);
            }
        }
    }
}

SCENARIO("tap.elixir~ takes a per-channel slew time from a list") {
    ext_main(nullptr);

    GIVEN("channel 0 toggled on with an explicit 0 ms slew via the list message") {
        test_wrapper<elixir> an_instance;
        elixir&              my_object = an_instance;
        my_object.gtime(atoms{50.0}); // a global slew that the list should override
        my_object.m_list(atoms{1, 0.0}, 0);

        THEN("the change is immediate — the per-channel time wins") {
            for (double s : mix(my_object, 1.0, 0.0)) {
                REQUIRE(std::abs(s - 1.0 * k_master) < 1e-12);
            }
        }
    }
}
