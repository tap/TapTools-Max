/// @file
/// @brief      Unit tests for tap.tune~ (Min-level behavior; the DSP is tested in the kernel repo).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.tune_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.tune~ instantiates with the expected defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<tune> an_instance;
        tune&              my_object = an_instance;

        THEN("the attributes have the documented defaults") {
            REQUIRE(my_object.key == symbol("c"));
            REQUIRE(my_object.scale == symbol("chromatic"));
            REQUIRE(my_object.mode == symbol("scale"));
            REQUIRE(static_cast<double>(my_object.speed) == tap::tools::tune::k_default_speed_ms);
            REQUIRE(static_cast<double>(my_object.amount) == 100.0);
            REQUIRE(static_cast<double>(my_object.minfreq) == tap::tools::tune::k_default_min_hz);
            REQUIRE(static_cast<double>(my_object.maxfreq) == tap::tools::tune::k_default_max_hz);
            REQUIRE(static_cast<bool>(my_object.bypass) == false);
            REQUIRE(static_cast<bool>(my_object.mute) == false);
        }
        THEN("the engine is prepared and starts fully chromatic") {
            REQUIRE(my_object.engine().prepared());
            REQUIRE(my_object.engine().notes() == tap::tools::tune::k_scale_chromatic);
        }
    }
}

SCENARIO("attributes forward into the kernel") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<tune> an_instance;
        tune&              my_object = an_instance;

        WHEN("the key and scale are set") {
            my_object.key   = "d";
            my_object.scale = "major";

            THEN("the kernel note mask is D major, rotated to absolute pitch classes") {
                REQUIRE(my_object.engine().key() == 2);
                REQUIRE(my_object.engine().scale() == tap::tools::tune::k_scale_major);
                unsigned expected = 0u;
                for (const int degree : {0, 2, 4, 5, 7, 9, 11}) {
                    expected |= 1u << ((degree + 2) % 12);
                }
                REQUIRE(my_object.engine().notes() == expected);
            }
        }

        WHEN("speed and amount are set") {
            my_object.speed  = 250.0;
            my_object.amount = 50.0;

            THEN("the kernel follows") {
                REQUIRE(my_object.engine().speed() == 250.0);
                REQUIRE(my_object.engine().amount() == 50.0);
            }
        }

        WHEN("mode is set to midi") {
            my_object.mode = "midi";

            THEN("the kernel switches target mode") {
                REQUIRE(my_object.engine().target_mode() == tap::tools::tune::mode::midi);
            }
        }

        WHEN("the backend is switched") {
            THEN("it defaults to the grain engine") {
                REQUIRE(my_object.backend == symbol("grain"));
                REQUIRE(my_object.engine().resynth_backend() == tap::tools::tune::backend::grain);
            }

            my_object.backend = "psola";
            THEN("psola forwards into the kernel") {
                REQUIRE(my_object.engine().resynth_backend() == tap::tools::tune::backend::psola);
            }

            my_object.backend = "pvoc";
            THEN("pvoc forwards into the kernel") {
                REQUIRE(my_object.engine().resynth_backend() == tap::tools::tune::backend::pvoc);
            }
        }

        WHEN("auto-key learning is toggled") {
            THEN("it defaults off, with reporting at 50 ms") {
                REQUIRE(static_cast<bool>(my_object.autokey) == false);
                REQUIRE(my_object.engine().autokey() == false);
                REQUIRE(static_cast<double>(my_object.interval) == 50.0);
            }

            my_object.autokey = true;
            THEN("the flag forwards into the kernel") {
                REQUIRE(my_object.engine().autokey() == true);
            }
        }

        WHEN("the key estimate is queried before anything was heard") {
            my_object.autokey = true;
            my_object.getkey();

            THEN("no estimate exists yet (the message must not crash or fabricate one)") {
                REQUIRE_FALSE(my_object.engine().autokey_estimate().valid());
            }
        }

        WHEN("applykey runs without an estimate") {
            my_object.applykey();

            THEN("the key and scale attributes stay untouched") {
                REQUIRE(my_object.key == symbol("c"));
                REQUIRE(my_object.scale == symbol("chromatic"));
            }
        }

        WHEN("formant preservation is enabled") {
            THEN("it defaults off") {
                REQUIRE(static_cast<bool>(my_object.formant) == false);
                REQUIRE(my_object.engine().formant() == false);
            }

            my_object.formant = true;
            THEN("the flag forwards into the kernel") {
                REQUIRE(my_object.engine().formant() == true);
            }
        }
    }
}

SCENARIO("the notes message sets the mask in both accepted forms") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<tune> an_instance;
        tune&              my_object = an_instance;

        WHEN("twelve 0/1 toggles are sent") {
            my_object.notes(atoms{1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}); // C, E, G

            THEN("the mask holds exactly those pitch classes") {
                REQUIRE(my_object.engine().notes() == ((1u << 0) | (1u << 4) | (1u << 7)));
            }
        }

        WHEN("a shorter list of pitch classes is sent") {
            my_object.notes(atoms{9, 11, 2}); // A, B, D

            THEN("the mask holds exactly those pitch classes") {
                REQUIRE(my_object.engine().notes() == ((1u << 9) | (1u << 11) | (1u << 2)));
            }
        }
    }
}

SCENARIO("processing produces finite output and bypass passes through") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<tune> an_instance;
        tune&              my_object = an_instance;

        WHEN("a burst of samples runs through") {
            bool all_finite = true;
            for (int t = 0; t < 4096; ++t) {
                const auto y = my_object(std::sin(t * 0.05));
                if (!std::isfinite(static_cast<double>(y))) {
                    all_finite = false;
                }
            }
            THEN("the output stays finite") {
                REQUIRE(all_finite);
            }
        }

        WHEN("bypass is on") {
            my_object.bypass = true;
            THEN("the input passes through untouched") {
                REQUIRE(static_cast<double>(my_object(0.5)) == 0.5);
            }
        }

        WHEN("mute is on") {
            my_object.mute = true;
            THEN("the output is silent") {
                REQUIRE(static_cast<double>(my_object(0.5)) == 0.0);
            }
        }
    }
}
