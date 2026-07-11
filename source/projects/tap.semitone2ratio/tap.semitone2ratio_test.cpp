/// @file
/// @brief      Unit tests for tap.semitone2ratio.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min_unittest.h"
#include "tap.semitone2ratio.cpp"

#include <cmath>

using namespace c74;

// Read the numeric values of the most recent outlet message. The mock logs may or may not
// prepend a selector symbol depending on the send overload, so keep only the numeric atoms.
static std::vector<double> last_values(max::t_sequence* seq) {
    REQUIRE(!seq->empty());
    const auto&         msg = seq->back();
    std::vector<double> v;
    for (const auto& a : msg) {
        const auto t = a.type();
        if (t == c74::min::message_type::float_argument || t == c74::min::message_type::int_argument)
            v.push_back(static_cast<double>(a));
    }
    REQUIRE(!v.empty());
    return v;
}

SCENARIO("tap.semitone2ratio converts semitones to frequency ratios") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<semitone2ratio> an_instance;
        semitone2ratio&              my_object = an_instance;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        THEN("the octave points are exact") {
            struct { double in, out; } cases[] = {
                { 0.0, 1.0 }, { 12.0, 2.0 }, { -12.0, 0.5 }, { 24.0, 4.0 }, { -24.0, 0.25 },
            };
            for (auto& c : cases) {
                output->clear();
                my_object.m_number(atoms { c.in });
                const auto v = last_values(output);
                REQUIRE(v.size() == 1);
                REQUIRE(std::abs(v[0] - c.out) < 1e-12);
            }
        }
        THEN("fractional semitones convert (a fifth is ~1.4983)") {
            output->clear();
            my_object.m_number(atoms { 7.0 });
            REQUIRE(std::abs(last_values(output)[0] - 1.4983070768766815) < 1e-12);
        }
        THEN("lists convert element-wise") {
            output->clear();
            my_object.list(atoms { 0.0, 12.0, -12.0 });
            const auto v = last_values(output);
            REQUIRE(v.size() == 3);
            REQUIRE(std::abs(v[0] - 1.0) < 1e-12);
            REQUIRE(std::abs(v[1] - 2.0) < 1e-12);
            REQUIRE(std::abs(v[2] - 0.5) < 1e-12);
        }
        THEN("bang re-outputs the last conversion") {
            my_object.m_number(atoms { 12.0 });
            output->clear();
            my_object.bang();
            REQUIRE(std::abs(last_values(output)[0] - 2.0) < 1e-12);
        }
    }
}
