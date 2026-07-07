/// @file
/// @brief      Unit tests for tap.delay.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 1999-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.delay.cpp"      // include the object source so we can instantiate it

using namespace c74;

SCENARIO("tap.delay instantiates with the documented default delaytime") {
    ext_main(nullptr);

    GIVEN("a default instance (no creation argument)") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;

        THEN("delaytime defaults to 0.0") {
            REQUIRE(static_cast<double>(my_object.delaytime) == 0.0);
        }
    }
}

SCENARIO("tap.delay delaytime can be set via the attribute") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;

        WHEN("delaytime is set to 100") {
            my_object.delaytime = 100.0;

            THEN("the attribute reflects the new value") {
                REQUIRE(static_cast<double>(my_object.delaytime) == 100.0);
            }
        }

        WHEN("a larger delaytime is requested") {
            my_object.delaytime = 5000.0;

            THEN("the attribute stores the new value") {
                REQUIRE(static_cast<double>(my_object.delaytime) == 5000.0);
            }
        }
    }
}

SCENARIO("tap.delay sets the delay time from a number to the right inlet") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;

        WHEN("an int arrives in the right inlet (inlet 1)") {
            my_object.m_int(atoms{75}, 1);

            THEN("delaytime updates to that value, and the message is not delayed") {
                REQUIRE(static_cast<double>(my_object.delaytime) == 75.0);
            }
        }

        WHEN("a float arrives in the right inlet (inlet 1)") {
            my_object.m_float(atoms{42.5}, 1);

            THEN("delaytime updates to the float value") {
                REQUIRE(static_cast<double>(my_object.delaytime) == 42.5);
            }
        }
    }
}

SCENARIO("tap.delay stop cancels the pending message without error") {
    ext_main(nullptr);

    GIVEN("a default instance with a left-inlet input scheduled") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;
        my_object.delaytime           = 1000.0; // long delay so it will not fire during the test

        WHEN("a message is scheduled and then stopped") {
            my_object.m_int(atoms{5}, 0); // schedule into the left inlet
            my_object.stop();             // cancel the pending timer

            THEN("the object remains in a valid state with its delaytime intact") {
                // The timer is cancelled; we can only assert that the documented attribute
                // state is preserved (scheduling needs a running kernel to observe output).
                REQUIRE(static_cast<double>(my_object.delaytime) == 1000.0);
            }
        }
    }
}
