/// @file
/// @brief      Unit tests for tap.delay~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 1999-2026 Timothy Place.

#include <vector>

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.delay_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    // Feed an impulse followed by silence and return the whole output.
    // dt is the per-sample delay-time signal on the right inlet (0 = "use the attribute").
    std::vector<double> impulse_response(delay& object, size_t n, double dt) {
        std::vector<double> out(n, 0.0);
        for (size_t i = 0; i < n; ++i) {
            out[i] = object(i == 0 ? 1.0 : 0.0, dt);
        }
        return out;
    }

    // Index of the single non-zero sample, or -1 if there is not exactly one.
    long sole_tap(const std::vector<double>& x) {
        long found = -1;
        for (size_t i = 0; i < x.size(); ++i) {
            if (x[i] != 0.0) {
                if (found >= 0) {
                    return -1; // more than one
                }
                found = static_cast<long>(i);
            }
        }
        return found;
    }

} // namespace

SCENARIO("tap.delay~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;

        THEN("the buffer is 1000 ms and the delay is zero") {
            REQUIRE(static_cast<double>(my_object.buffersize) == 1000.0);
            REQUIRE(static_cast<double>(my_object.delaytime) == 0.0);
        }
    }
}

SCENARIO("tap.delay~ passes the signal straight through at zero delay") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;

        THEN("each sample comes out on the sample it went in") {
            REQUIRE(my_object(0.5, 0.0) == 0.5);
            REQUIRE(my_object(-0.25, 0.0) == -0.25);
            REQUIRE(my_object(1.0, 0.0) == 1.0);
        }
    }
}

SCENARIO("tap.delay~ delays by the attribute value") {
    ext_main(nullptr);

    GIVEN("a 10 ms delay") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;
        my_object.delaytime           = 10.0;

        WHEN("an impulse is processed") {
            const std::vector<double> out = impulse_response(my_object, 2048, 0.0);

            THEN("exactly one tap comes out, 441 samples later") {
                REQUIRE(sole_tap(out) == static_cast<long>(10.0 * k_mock_sr * 0.001));
                REQUIRE(out[441] == 1.0);
            }
        }
    }
}

SCENARIO("tap.delay~ clamps the delay time to the documented bounds") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;

        THEN("a negative delay is floored at zero and echoed back") {
            my_object.delaytime = -50.0;
            REQUIRE(static_cast<double>(my_object.delaytime) == 0.0);
        }
        THEN("a delay past the buffer size is accepted but bounded by the buffer") {
            my_object.delaytime = 5000.0; // buffer is 1000 ms
            REQUIRE(static_cast<double>(my_object.delaytime) == 5000.0);
            const std::vector<double> out = impulse_response(my_object, 44100, 0.0);
            // Clamped to buffersize - 1 samples, so the tap lands at the very end of the buffer.
            REQUIRE(sole_tap(out) == static_cast<long>(1000.0 * k_mock_sr * 0.001) - 1);
        }
    }
}

SCENARIO("tap.delay~ lets a non-zero delay-time signal override the attribute") {
    ext_main(nullptr);

    GIVEN("an attribute delay of 10 ms") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;
        my_object.delaytime           = 10.0;

        WHEN("a 20 ms delay-time signal is present on the right inlet") {
            const std::vector<double> out = impulse_response(my_object, 4096, 20.0);

            THEN("the signal wins — the tap lands at 882 samples, not 441") {
                REQUIRE(sole_tap(out) == static_cast<long>(20.0 * k_mock_sr * 0.001));
            }
        }
    }
}

SCENARIO("tap.delay~ clears its buffer on demand") {
    ext_main(nullptr);

    GIVEN("a 10 ms delay holding an impulse") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;
        my_object.delaytime           = 10.0;
        my_object(1.0, 0.0); // write the impulse
        for (int i = 0; i < 100; ++i) {
            my_object(0.0, 0.0);
        }

        WHEN("clear is sent before the tap comes due") {
            my_object.clear();

            THEN("the buffered impulse never emerges") {
                for (int i = 0; i < 2048; ++i) {
                    REQUIRE(my_object(0.0, 0.0) == 0.0);
                }
            }
        }
    }
}
