/// @file
/// @brief      Unit tests for tap.biquadcalc.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2004-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.biquadcalc.cpp" // include the object source so we can instantiate it

using namespace c74;

// Read the most recent 5-coefficient list (a0 a1 a2 b1 b2) emitted on the left outlet.
static std::vector<double> last_coeffs(max::t_sequence* seq) {
    REQUIRE(!seq->empty());
    const auto& msg = seq->back();
    REQUIRE(msg.size() == 5);
    std::vector<double> out;
    for (const auto& a : msg) {
        out.push_back(static_cast<double>(a));
    }
    return out;
}

SCENARIO("tap.biquadcalc instantiates with the expected defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<biquadcalc> an_instance;
        biquadcalc&              my_object = an_instance;

        THEN("type defaults to lowpass") {
            REQUIRE(my_object.filter_type == symbol{"lowpass"});
        }
        THEN("cf defaults to 1000") {
            REQUIRE(static_cast<double>(my_object.cf) == APPROX(1000.0));
        }
        THEN("gain defaults to 0") {
            REQUIRE(static_cast<double>(my_object.gain) == APPROX(0.0));
        }
        THEN("q defaults to 1") {
            REQUIRE(static_cast<double>(my_object.q) == APPROX(1.0));
        }
        THEN("trigger defaults to 4 (recalc on any of cf/gain/q)") {
            REQUIRE(static_cast<int>(my_object.trigger) == 4);
        }
        THEN("sr defaults to the mock kernel sample rate of 44100") {
            REQUIRE(static_cast<int>(my_object.sr) == 44100);
        }
    }
}

// All expected coefficients below are RBJ Audio EQ Cookbook values computed for
// Fs = 44100, f0 = 1000, gain = 0 dB, Q = 1, matching this object's formulas.
SCENARIO("tap.biquadcalc computes lowpass coefficients for cf=1000 Q=1 sr=44100") {
    ext_main(nullptr);

    GIVEN("a default (lowpass) instance") {
        test_wrapper<biquadcalc> an_instance;
        biquadcalc&              my_object = an_instance;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("a bang triggers the calculation") {
            output->clear();
            my_object.bang();

            THEN("the five coefficients match the known reference values") {
                auto c = last_coeffs(output);
                REQUIRE(c[0] == APPROX(0.0047304174129274516)); // a0
                REQUIRE(c[1] == APPROX(0.009460834825854903));  // a1
                REQUIRE(c[2] == APPROX(0.0047304174129274516)); // a2
                REQUIRE(c[3] == APPROX(-1.8484969161333196));   // b1
                REQUIRE(c[4] == APPROX(0.8674185857850293));    // b2
            }

            AND_THEN("the lowpass has unity DC gain: (a0+a1+a2)/(1+b1+b2) == 1") {
                auto         c  = last_coeffs(output);
                const double dc = (c[0] + c[1] + c[2]) / (1.0 + c[3] + c[4]);
                REQUIRE(dc == APPROX(1.0));
            }
        }
    }
}

SCENARIO("tap.biquadcalc computes highpass coefficients for cf=1000 Q=1 sr=44100") {
    ext_main(nullptr);

    GIVEN("a highpass instance") {
        test_wrapper<biquadcalc> an_instance;
        biquadcalc&              my_object = an_instance;
        my_object.filter_type              = "highpass";

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("a bang triggers the calculation") {
            output->clear();
            my_object.bang();

            THEN("the five coefficients match the known reference values") {
                auto c = last_coeffs(output);
                REQUIRE(c[0] == APPROX(0.9289788754795872));  // a0
                REQUIRE(c[1] == APPROX(-1.8579577509591745)); // a1
                REQUIRE(c[2] == APPROX(0.9289788754795872));  // a2
                REQUIRE(c[3] == APPROX(-1.8484969161333196)); // b1
                REQUIRE(c[4] == APPROX(0.8674185857850293));  // b2
            }
        }
    }
}

SCENARIO("tap.biquadcalc computes bandpass and allpass coefficients") {
    ext_main(nullptr);

    GIVEN("a bandpass instance") {
        test_wrapper<biquadcalc> an_instance;
        biquadcalc&              my_object = an_instance;
        my_object.filter_type              = "bandpass";

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("a bang triggers the calculation") {
            output->clear();
            my_object.bang();

            THEN("the bandpass coefficients match the reference (a1 == 0)") {
                auto c = last_coeffs(output);
                REQUIRE(c[0] == APPROX(0.06629070710748529));  // a0
                REQUIRE(c[1] == APPROX(0.0));                  // a1
                REQUIRE(c[2] == APPROX(-0.06629070710748529)); // a2
                REQUIRE(c[3] == APPROX(-1.8484969161333196));  // b1
                REQUIRE(c[4] == APPROX(0.8674185857850293));   // b2
            }
        }
    }

    GIVEN("an allpass instance") {
        test_wrapper<biquadcalc> an_instance;
        biquadcalc&              my_object = an_instance;
        my_object.filter_type              = "allpass";

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("a bang triggers the calculation") {
            output->clear();
            my_object.bang();

            THEN("the allpass coefficients match the reference (a2 == 1)") {
                auto c = last_coeffs(output);
                REQUIRE(c[0] == APPROX(0.8674185857850293));  // a0
                REQUIRE(c[1] == APPROX(-1.8484969161333196)); // a1
                REQUIRE(c[2] == APPROX(1.0));                 // a2
                REQUIRE(c[3] == APPROX(-1.8484969161333196)); // b1
                REQUIRE(c[4] == APPROX(0.8674185857850293));  // b2
            }
        }
    }
}

SCENARIO("tap.biquadcalc recalculates automatically when cf changes (trigger==4)") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<biquadcalc> an_instance;
        biquadcalc&              my_object = an_instance;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("the cf attribute is set") {
            output->clear();
            my_object.cf = 2000.0;

            THEN("a coefficient list is emitted without an explicit bang") {
                REQUIRE(!output->empty());
                REQUIRE(output->back().size() == 5);
            }
        }
    }
}
