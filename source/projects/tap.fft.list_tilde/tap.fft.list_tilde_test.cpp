/// @file
/// @brief      Unit tests for tap.fft.list~.
/// @details    The autopoll path emits through a `queue`, and the mock kernel's `qelem_set` is a
///             no-op, so the frame is polled here with an explicit bang — which runs the same emit()
///             synchronously. Verifying that a completed frame auto-polls needs Max's main-thread
///             queue and belongs in runtime-tests/.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include <vector>

#include "c74_min_unittest.h"     // required unit-test header (defines main via Catch)
#include "tap.fft.list_tilde.cpp" // include the object source so we can instantiate it

using namespace c74;

namespace {

    // Push one frame of `values` through the object, presenting bin indices 0..n-1 alongside them.
    void push_frame(fft_list& object, const std::vector<double>& values) {
        std::vector<double> data = values;
        std::vector<double> index(values.size());
        for (size_t i = 0; i < index.size(); ++i) {
            index[i] = static_cast<double>(i);
        }
        std::vector<double> unused(values.size(), 0.0);
        double*             inp[2]  = {data.data(), index.data()};
        double*             outp[1] = {unused.data()};
        audio_bundle        ina{inp, 2, static_cast<long>(values.size())};
        audio_bundle        outa{outp, 1, static_cast<long>(values.size())};
        object(ina, outa);
    }

    // A frame of `n` bins whose value is the bin number, so a position error is visible.
    std::vector<double> ramp_frame(size_t n) {
        std::vector<double> v(n);
        for (size_t i = 0; i < n; ++i) {
            v[i] = static_cast<double>(i);
        }
        return v;
    }

} // namespace

SCENARIO("tap.fft.list~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<fft_list> an_instance;
        fft_list&              my_object = an_instance;

        THEN("attribute defaults match the reference page") {
            REQUIRE(static_cast<double>(my_object.mult) == 1.0);
            REQUIRE(static_cast<bool>(my_object.nyquist) == true);
            REQUIRE(static_cast<bool>(my_object.autopoll) == true);
        }
    }
}

SCENARIO("tap.fft.list~ emits the gathered frame as a list on bang") {
    ext_main(nullptr);

    GIVEN("a frame of 512 bins gathered from the signal inlets") {
        test_wrapper<fft_list> an_instance;
        fft_list&              my_object = an_instance;
        push_frame(my_object, ramp_frame(512));

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("bang is sent") {
            output->clear();
            my_object.bang();

            THEN("one list is emitted, truncated at Nyquist (fftsize/2 values)") {
                REQUIRE(output->size() == 1);
                const auto& list = (*output)[0];
                REQUIRE(list.size() == 256);
                REQUIRE(static_cast<double>(list[0]) == 0.0);
                REQUIRE(static_cast<double>(list[1]) == 1.0);
                REQUIRE(static_cast<double>(list[255]) == 255.0);
            }
        }
    }
}

SCENARIO("tap.fft.list~ outputs the whole frame when nyquist truncation is off") {
    ext_main(nullptr);

    GIVEN("a gathered frame with nyquist disabled") {
        test_wrapper<fft_list> an_instance;
        fft_list&              my_object = an_instance;
        my_object.nyquist                = false;
        push_frame(my_object, ramp_frame(512));

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("bang is sent") {
            output->clear();
            my_object.bang();

            THEN("all 512 values are emitted") {
                REQUIRE(output->size() == 1);
                const auto& list = (*output)[0];
                REQUIRE(list.size() == 512);
                REQUIRE(static_cast<double>(list[511]) == 511.0);
            }
        }
    }
}

SCENARIO("tap.fft.list~ scales the gathered values by mult") {
    ext_main(nullptr);

    GIVEN("mult set to 0.5") {
        test_wrapper<fft_list> an_instance;
        fft_list&              my_object = an_instance;
        my_object.mult                   = 0.5;
        push_frame(my_object, ramp_frame(512));

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("bang is sent") {
            output->clear();
            my_object.bang();

            THEN("every value is halved") {
                const auto& list = (*output)[0];
                REQUIRE(static_cast<double>(list[2]) == 1.0);
                REQUIRE(static_cast<double>(list[10]) == 5.0);
            }
        }
    }
}

SCENARIO("tap.fft.list~ holds the last gathered frame between bangs") {
    ext_main(nullptr);

    GIVEN("two successive frames") {
        test_wrapper<fft_list> an_instance;
        fft_list&              my_object = an_instance;
        push_frame(my_object, std::vector<double>(512, 1.0));
        push_frame(my_object, std::vector<double>(512, 2.0));

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("bang is sent twice") {
            output->clear();
            my_object.bang();
            my_object.bang();

            THEN("both lists report the most recent frame") {
                REQUIRE(output->size() == 2);
                REQUIRE(static_cast<double>((*output)[0][0]) == 2.0);
                REQUIRE(static_cast<double>((*output)[1][0]) == 2.0);
            }
        }
    }
}
