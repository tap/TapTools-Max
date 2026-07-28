/// @file
/// @brief      Unit tests for tap.multitap~.
/// @details    The list-valued attributes are assigned as `atoms`, not as `std::vector<number>`:
///             min-api's `attribute<std::vector<T>>::operator=(const T)` wraps the whole vector in a
///             single atom via the catch-all `atom(T)` template, which recurses until the stack runs
///             out. The `atoms` overload takes the same path Max does.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"     // required unit-test header (defines main via Catch)
#include "tap.multitap_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    long samples_for(double ms) {
        return static_cast<long>(ms * k_mock_sr * 0.001);
    }

    // Feed an impulse followed by silence and return the whole output.
    std::vector<double> impulse_response(multitap& object, size_t n) {
        std::vector<double> out(n, 0.0);
        for (size_t i = 0; i < n; ++i) {
            out[i] = object(i == 0 ? 1.0 : 0.0);
        }
        return out;
    }

    size_t count_nonzero(const std::vector<double>& x) {
        size_t n = 0;
        for (double s : x) {
            if (s != 0.0) {
                ++n;
            }
        }
        return n;
    }

} // namespace

SCENARIO("tap.multitap~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<multitap> an_instance;
        multitap&              my_object = an_instance;

        THEN("one tap, a 1000 ms buffer, no delay and unity gain") {
            REQUIRE(static_cast<double>(my_object.buffersize) == 1000.0);
            REQUIRE(static_cast<int>(my_object.taps) == 1);
            const std::vector<number>& delays = my_object.delay;
            const std::vector<number>& gains  = my_object.gain;
            REQUIRE(delays.size() == 1);
            REQUIRE(delays[0] == 0.0);
            REQUIRE(gains.size() == 1);
            REQUIRE(gains[0] == 0.0); // 0 dB
        }
    }
}

SCENARIO("tap.multitap~ clamps the tap count to its documented range") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<multitap> an_instance;
        multitap&              my_object = an_instance;

        THEN("zero and negative counts are floored at one") {
            my_object.taps = 0;
            REQUIRE(static_cast<int>(my_object.taps) == 1);
            my_object.taps = -5;
            REQUIRE(static_cast<int>(my_object.taps) == 1);
        }
        THEN("a count past the internal maximum is capped at 99") {
            my_object.taps = 500;
            REQUIRE(static_cast<int>(my_object.taps) == 99);
        }
    }
}

SCENARIO("tap.multitap~ places a single tap at its delay time") {
    ext_main(nullptr);

    GIVEN("one tap at 10 ms and 0 dB") {
        test_wrapper<multitap> an_instance;
        multitap&              my_object = an_instance;
        my_object.delay                  = atoms{10.0};

        WHEN("an impulse is processed") {
            const std::vector<double> out = impulse_response(my_object, 2048);

            THEN("exactly one tap comes out, at unity, 441 samples later") {
                REQUIRE(count_nonzero(out) == 1);
                REQUIRE(out[samples_for(10.0)] == 1.0);
            }
        }
    }
}

SCENARIO("tap.multitap~ sums several taps, each with its own time and gain") {
    ext_main(nullptr);

    GIVEN("three taps at 10/20/30 ms and 0/-6/-12 dB") {
        test_wrapper<multitap> an_instance;
        multitap&              my_object = an_instance;
        my_object.taps                   = 3;
        my_object.delay                  = atoms{10.0, 20.0, 30.0};
        my_object.gain                   = atoms{0.0, -6.0, -12.0};

        WHEN("an impulse is processed") {
            const std::vector<double> out = impulse_response(my_object, 4096);

            THEN("all three taps appear, at their own times and levels") {
                REQUIRE(count_nonzero(out) == 3);
                REQUIRE(std::abs(out[samples_for(10.0)] - 1.0) < 1e-12);
                REQUIRE(std::abs(out[samples_for(20.0)] - std::pow(10.0, -6.0 * 0.05)) < 1e-12);
                REQUIRE(std::abs(out[samples_for(30.0)] - std::pow(10.0, -12.0 * 0.05)) < 1e-12);
            }
        }
    }

    GIVEN("three tap times configured but only two taps active") {
        test_wrapper<multitap> an_instance;
        multitap&              my_object = an_instance;
        my_object.taps                   = 2;
        my_object.delay                  = atoms{10.0, 20.0, 30.0};
        my_object.gain                   = atoms{0.0, 0.0, 0.0};

        WHEN("an impulse is processed") {
            const std::vector<double> out = impulse_response(my_object, 4096);

            THEN("the third tap is inactive — taps bounds the loop, not the table") {
                REQUIRE(count_nonzero(out) == 2);
                REQUIRE(out[samples_for(30.0)] == 0.0);
            }
        }
    }

    GIVEN("two taps at the same delay time") {
        test_wrapper<multitap> an_instance;
        multitap&              my_object = an_instance;
        my_object.taps                   = 2;
        my_object.delay                  = atoms{10.0, 10.0};
        my_object.gain                   = atoms{0.0, 0.0};

        WHEN("an impulse is processed") {
            const std::vector<double> out = impulse_response(my_object, 2048);

            THEN("the taps sum") {
                REQUIRE(std::abs(out[samples_for(10.0)] - 2.0) < 1e-12);
            }
        }
    }
}

SCENARIO("tap.multitap~ bounds a tap by its buffer") {
    ext_main(nullptr);

    GIVEN("one tap asked to delay well past the 1000 ms buffer") {
        test_wrapper<multitap> an_instance;
        multitap&              my_object = an_instance;
        my_object.delay                  = atoms{5000.0};

        WHEN("an impulse is processed") {
            const std::vector<double> out = impulse_response(my_object, 44100);

            THEN("the tap lands at the far end of the buffer instead of wrapping") {
                REQUIRE(count_nonzero(out) == 1);
                REQUIRE(out[samples_for(1000.0) - 1] == 1.0);
            }
        }
    }
}

SCENARIO("tap.multitap~ clears its buffer on demand") {
    ext_main(nullptr);

    GIVEN("a 10 ms tap holding an impulse") {
        test_wrapper<multitap> an_instance;
        multitap&              my_object = an_instance;
        my_object.delay                  = atoms{10.0};
        my_object(1.0);
        for (int i = 0; i < 100; ++i) {
            my_object(0.0);
        }

        WHEN("clear is sent before the tap comes due") {
            my_object.clear();

            THEN("the buffered impulse never emerges") {
                for (int i = 0; i < 2048; ++i) {
                    REQUIRE(my_object(0.0) == 0.0);
                }
            }
        }
    }
}
