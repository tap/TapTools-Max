/// @file
/// @brief      Unit tests for tap.adsr~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.adsr_tilde.cpp" // include the object source so we can instantiate it

// The mock kernel's sys_getsr() returns 44100, so samplerate() is 44100 here.
static constexpr double k_sr = 44100.0;

SCENARIO("tap.adsr~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<adsr> an_instance;
        adsr&              my_object = an_instance;

        THEN("attack 50, decay 100, sustain -6, release 500, mode hybrid, trigger off") {
            REQUIRE(static_cast<double>(my_object.attack) == 50.0);
            REQUIRE(static_cast<double>(my_object.decay) == 100.0);
            REQUIRE(static_cast<double>(my_object.sustain) == -6.0);
            REQUIRE(static_cast<double>(my_object.release) == 500.0);
            const symbol m = my_object.mode;
            REQUIRE(std::string(m.c_str()) == "hybrid");
            REQUIRE(static_cast<bool>(my_object.trigger) == false);
        }
    }
}

SCENARIO("tap.adsr~ stays silent while untriggered") {
    ext_main(nullptr);

    GIVEN("a default instance with no trigger") {
        test_wrapper<adsr> an_instance;
        adsr&              my_object = an_instance;

        THEN("the output stays at zero (inactive state)") {
            REQUIRE(static_cast<double>(my_object(0.0)) == 0.0);
            REQUIRE(static_cast<double>(my_object(0.0)) == 0.0);
        }
    }
}

SCENARIO("tap.adsr~ linear attack rises by a fixed step per sample") {
    ext_main(nullptr);

    GIVEN("a triggered instance in linear mode, attack 10 ms") {
        test_wrapper<adsr> an_instance;
        adsr&              my_object = an_instance;
        my_object.mode               = symbol("linear");
        my_object.attack             = 10.0; // setter recomputes the step sizes
        my_object.trigger            = true; // open the envelope

        // attack_samples = (10/1000) * 44100 = 441; step = 1/441 per sample.
        const long   attack_samples = static_cast<long>((10.0 / 1000.0) * k_sr);
        const double step           = 1.0 / attack_samples;

        THEN("the first few samples climb linearly from the step size") {
            REQUIRE(static_cast<double>(my_object(0.0)) == APPROX(1.0 * step));
            REQUIRE(static_cast<double>(my_object(0.0)) == APPROX(2.0 * step));
            REQUIRE(static_cast<double>(my_object(0.0)) == APPROX(3.0 * step));
        }
    }
}

SCENARIO("tap.adsr~ linear attack reaches 1.0 then enters decay toward the sustain level") {
    ext_main(nullptr);

    GIVEN("a triggered instance in linear mode, attack 10 ms, sustain -6 dB") {
        test_wrapper<adsr> an_instance;
        adsr&              my_object = an_instance;
        my_object.mode               = symbol("linear");
        my_object.attack             = 10.0;
        my_object.sustain            = -6.0;
        my_object.trigger            = true;

        const long attack_samples = static_cast<long>((10.0 / 1000.0) * k_sr); // 441

        THEN("the peak hits exactly 1.0 at the end of the attack") {
            double last = 0.0;
            for (long i = 0; i < attack_samples; ++i) {
                last = static_cast<double>(my_object(0.0));
            }
            REQUIRE(last == APPROX(1.0));
        }

        AND_THEN("the envelope then decays below 1.0 toward the sustain amplitude") {
            // Run the attack to completion.
            for (long i = 0; i < attack_samples; ++i) {
                my_object(0.0);
            }
            const double after_peak = static_cast<double>(my_object(0.0));
            // First decay sample steps down from 1.0.
            REQUIRE(after_peak < 1.0);
            // The sustain amplitude is 10^(-6/20) ~= 0.50119; the envelope never drops below it
            // during decay/sustain.
            const double sustain_amp = std::pow(10.0, -6.0 * 0.05);
            REQUIRE(after_peak >= sustain_amp);
        }
    }
}

SCENARIO("tap.adsr~ settles at the sustain amplitude and holds there") {
    ext_main(nullptr);

    GIVEN("a triggered instance in linear mode, short attack and decay, sustain -6 dB") {
        test_wrapper<adsr> an_instance;
        adsr&              my_object = an_instance;
        my_object.mode               = symbol("linear");
        my_object.attack             = 5.0;
        my_object.decay              = 5.0;
        my_object.sustain            = -6.0;
        my_object.trigger            = true;

        const double sustain_amp = std::pow(10.0, -6.0 * 0.05);

        THEN("after enough samples the output equals the sustain amplitude and stays put") {
            double v = 0.0;
            for (long i = 0; i < 2000; ++i) { // well past attack+decay (~5ms+5ms = ~441 samples)
                v = static_cast<double>(my_object(0.0));
            }
            REQUIRE(v == APPROX(sustain_amp));
            // Hold: sustain state outputs a constant.
            REQUIRE(static_cast<double>(my_object(0.0)) == APPROX(sustain_amp));
        }
    }
}

SCENARIO("tap.adsr~ releases toward zero after the trigger is cleared") {
    ext_main(nullptr);

    GIVEN("a triggered instance in linear mode that has reached sustain") {
        test_wrapper<adsr> an_instance;
        adsr&              my_object = an_instance;
        my_object.mode               = symbol("linear");
        my_object.attack             = 5.0;
        my_object.decay              = 5.0;
        my_object.release            = 10.0;
        my_object.sustain            = -6.0;
        my_object.trigger            = true;

        const double sustain_amp = std::pow(10.0, -6.0 * 0.05);
        for (long i = 0; i < 2000; ++i) {
            my_object(0.0); // settle to sustain
        }

        WHEN("the trigger is cleared") {
            my_object.trigger = false;

            THEN("the next sample steps down from the sustain amplitude") {
                const double v = static_cast<double>(my_object(0.0));
                REQUIRE(v < sustain_amp);
            }

            AND_THEN("after the full release the output reaches zero and stays inactive") {
                double v = 1.0;
                for (long i = 0; i < 4000; ++i) { // > release of 10 ms (~441 samples)
                    v = static_cast<double>(my_object(0.0));
                }
                REQUIRE(v == APPROX(0.0));
            }
        }
    }
}
