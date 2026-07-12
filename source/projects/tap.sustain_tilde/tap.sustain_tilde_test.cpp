/// @file
/// @brief      Unit tests for tap.sustain~ (defaults, multi-voice allocation, rise).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2019-2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.sustain_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_pi = 3.14159265358979323846;

    // Push a sine wave through the object one sample at a time. The object records into its master
    // ring buffer on every audio frame, so this fills it with capturable material (real
    // zero-crossings, non-zero RMS) and services any pending bang() capture.
    void feed_sine(sustain& obj, int samples, double freq_hz = 220.0, double sr = 44100.0) {
        static double phase = 0.0;
        const double  inc   = 2.0 * k_pi * freq_hz / sr;
        auto&         vop   = static_cast<c74::min::vector_operator<>&>(obj); // reach the single-sample helper
        for (int i = 0; i < samples; ++i) {
            vop(static_cast<c74::min::sample>(std::sin(phase)));
            phase += inc;
            if (phase > 2.0 * k_pi) {
                phase -= 2.0 * k_pi;
            }
        }
    }

    int count_active(const sustain& obj) {
        int n = 0;
        for (const auto& v : obj.voice_bank()) {
            if (v.active()) {
                ++n;
            }
        }
        return n;
    }

} // anonymous namespace

SCENARIO("tap.sustain~ instantiates with the expected defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<sustain> an_instance;
        sustain&              my_object = an_instance;

        THEN("length defaults to 1000 ms") {
            REQUIRE(static_cast<double>(my_object.length) == 1000.0);
        }
        THEN("fade defaults to 100 ms") {
            REQUIRE(static_cast<double>(my_object.fade) == 100.0);
        }
        THEN("rise defaults to 100 ms") {
            REQUIRE(static_cast<double>(my_object.rise) == 100.0);
        }
        THEN("voices defaults to the full bank of 5") {
            REQUIRE(static_cast<int>(my_object.voices) == 5);
        }
        THEN("the voice bank has 5 voices, all initially silent") {
            REQUIRE(my_object.voice_bank().size() == 5);
            REQUIRE(count_active(my_object) == 0);
        }
        THEN("each voice's rise is derived from the rise attribute (100 ms @ 44.1 kHz = 4410 samples)") {
            REQUIRE(my_object.voice_bank()[0].rise() == 4410);
        }
    }
}

SCENARIO("multiple bangs engage multiple voices round-robin") {
    ext_main(nullptr);

    GIVEN("a primed default instance") {
        test_wrapper<sustain> an_instance;
        sustain&              my_object = an_instance;

        // Prime the master ring buffer with a full window of audio so captures are non-empty.
        feed_sine(my_object, 40000);

        WHEN("a single bang is captured") {
            my_object.bang();
            feed_sine(my_object, 64); // one audio block services the pending capture

            THEN("exactly one voice becomes active") {
                REQUIRE(count_active(my_object) == 1);
            }
            THEN("the round-robin cursor advanced to the second voice") {
                REQUIRE(my_object.next_voice() == 1);
            }
        }

        WHEN("three bangs are captured in succession") {
            for (int b = 0; b < 3; ++b) {
                my_object.bang();
                feed_sine(my_object, 64);
            }

            THEN("three distinct voices are active") {
                REQUIRE(count_active(my_object) == 3);
            }
            THEN("the cursor wrapped to the fourth voice") {
                REQUIRE(my_object.next_voice() == 3);
            }
        }

        WHEN("more bangs than voices are captured") {
            for (int b = 0; b < 7; ++b) { // 7 bangs into a 5-voice bank
                my_object.bang();
                feed_sine(my_object, 64);
            }

            THEN("all five voices are active (oldest recycled, none lost)") {
                REQUIRE(count_active(my_object) == 5);
            }
            THEN("the cursor wrapped round-robin back into the bank") {
                REQUIRE(my_object.next_voice() == (7 % 5));
            }
        }

        WHEN("clear is sent after capturing") {
            my_object.bang();
            feed_sine(my_object, 64);
            my_object.clear();

            THEN("all voices go silent and the cursor resets") {
                REQUIRE(count_active(my_object) == 0);
                REQUIRE(my_object.next_voice() == 0);
            }
        }
    }
}

SCENARIO("rise applies a one-shot per-voice fade-in") {
    ext_main(nullptr);

    GIVEN("a primed instance with a known rise time") {
        test_wrapper<sustain> an_instance;
        sustain&              my_object = an_instance;

        my_object.rise = 50.0; // 50 ms @ 44.1 kHz = 2205 samples
        feed_sine(my_object, 40000);

        THEN("the rise attribute propagated to every voice in samples") {
            for (const auto& v : my_object.voice_bank()) {
                REQUIRE(v.rise() == 2205);
            }
        }

        WHEN("a voice is captured and then played for fewer samples than the rise length") {
            my_object.bang();
            feed_sine(my_object, 64); // services the capture; voice begins at rise_pos 0
            const auto& v = my_object.voice_bank()[0];

            // capture() arms rise_pos to 0; only the first audio block (64 frames) has elapsed.
            THEN("the voice is mid-rise (rise position advanced but not yet complete)") {
                REQUIRE(v.rise_position() > 0);
                REQUIRE(v.rise_position() < v.rise());
            }

            AND_WHEN("it is played long enough to finish rising") {
                feed_sine(my_object, 4000); // well past 2205 samples
                THEN("the rise envelope has reached full power") {
                    REQUIRE(v.rise_position() == v.rise());
                }
            }
        }
    }
}
