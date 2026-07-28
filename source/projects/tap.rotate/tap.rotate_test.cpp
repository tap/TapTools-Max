/// @file
/// @brief      Unit tests for tap.rotate.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2007-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.rotate.cpp"     // include the object source so we can instantiate it

using namespace c74;

namespace {

    // Inlet and outlet indices, in the order the object declares them.
    constexpr int k_in_x   = 0;
    constexpr int k_in_y   = 1;
    constexpr int k_in_z   = 2;
    constexpr int k_in_rot = 3;
    constexpr int k_out_x  = 0;
    constexpr int k_out_y  = 1;
    constexpr int k_out_z  = 2;

    struct point {
        double x;
        double y;
        double z;
    };

    // Load one coordinate triple, set the Euler angles, and read the rotated result back.
    // The x inlet is what triggers the output, so it goes last.
    point rotate_one(rotate& object, point p, double ax, double ay, double az) {
        object.list_msg(atoms{p.y}, k_in_y);
        object.list_msg(atoms{p.z}, k_in_z);
        object.list_msg(atoms{ax, ay, az}, k_in_rot);

        auto* out_x = max::object_getoutput(object.maxobj(), k_out_x);
        auto* out_y = max::object_getoutput(object.maxobj(), k_out_y);
        auto* out_z = max::object_getoutput(object.maxobj(), k_out_z);
        out_x->clear();
        out_y->clear();
        out_z->clear();

        object.list_msg(atoms{p.x}, k_in_x);

        return {static_cast<double>((*out_x)[0][0]), static_cast<double>((*out_y)[0][0]),
                static_cast<double>((*out_z)[0][0])};
    }

} // namespace

SCENARIO("tap.rotate leaves coordinates alone at zero rotation") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<rotate> an_instance;
        rotate&              my_object = an_instance;

        THEN("an arbitrary point comes back unchanged") {
            const point p = rotate_one(my_object, {1.0, 2.0, 3.0}, 0.0, 0.0, 0.0);
            REQUIRE(std::abs(p.x - 1.0) < 1e-9);
            REQUIRE(std::abs(p.y - 2.0) < 1e-9);
            REQUIRE(std::abs(p.z - 3.0) < 1e-9);
        }
        THEN("a full turn about every axis is also the identity") {
            const point p = rotate_one(my_object, {1.0, 2.0, 3.0}, 360.0, 360.0, 360.0);
            REQUIRE(std::abs(p.x - 1.0) < 1e-9);
            REQUIRE(std::abs(p.y - 2.0) < 1e-9);
            REQUIRE(std::abs(p.z - 3.0) < 1e-9);
        }
    }
}

SCENARIO("tap.rotate turns a point about each axis") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<rotate> an_instance;
        rotate&              my_object = an_instance;

        // Coordinates that are exactly zero go through cart_to_pol's documented divide-by-zero
        // guard, which substitutes 0.000001 for a zero real part — so an axis-aligned point carries
        // an absolute error on the order of 1e-6. That floor is a property of the ported algorithm,
        // not of these assertions, hence the wider tolerance here than in the identity scenario.
        constexpr double k_axis_tol = 1e-5;

        THEN("90 degrees about z carries +x onto +y") {
            const point p = rotate_one(my_object, {1.0, 0.0, 0.0}, 0.0, 0.0, 90.0);
            REQUIRE(std::abs(p.x) < k_axis_tol);
            REQUIRE(std::abs(p.y - 1.0) < k_axis_tol);
            REQUIRE(std::abs(p.z) < k_axis_tol);
        }
        THEN("90 degrees about x carries +y onto +z") {
            const point p = rotate_one(my_object, {0.0, 1.0, 0.0}, 90.0, 0.0, 0.0);
            REQUIRE(std::abs(p.x) < k_axis_tol);
            REQUIRE(std::abs(p.y) < k_axis_tol);
            REQUIRE(std::abs(p.z - 1.0) < k_axis_tol);
        }
        THEN("90 degrees about y carries +z onto +x") {
            const point p = rotate_one(my_object, {0.0, 0.0, 1.0}, 0.0, 90.0, 0.0);
            REQUIRE(std::abs(p.x - 1.0) < k_axis_tol);
            REQUIRE(std::abs(p.y) < k_axis_tol);
            REQUIRE(std::abs(p.z) < k_axis_tol);
        }
    }
}

SCENARIO("tap.rotate preserves length") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<rotate> an_instance;
        rotate&              my_object = an_instance;

        THEN("an arbitrary rotation is rigid") {
            const point  p   = rotate_one(my_object, {1.0, -2.0, 3.0}, 37.0, -113.0, 250.0);
            const double len = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
            REQUIRE(std::abs(len - std::sqrt(1.0 + 4.0 + 9.0)) < 1e-9);
        }
    }
}

SCENARIO("tap.rotate rotates parallel lists of coordinates in one pass") {
    ext_main(nullptr);

    GIVEN("three points loaded as parallel x/y/z lists") {
        test_wrapper<rotate> an_instance;
        rotate&              my_object = an_instance;

        my_object.list_msg(atoms{0.0, 0.0, 0.0}, k_in_y);
        my_object.list_msg(atoms{0.0, 0.0, 0.0}, k_in_z);
        my_object.list_msg(atoms{0.0, 0.0, 90.0}, k_in_rot);

        auto* out_x = max::object_getoutput(my_object.maxobj(), k_out_x);
        auto* out_y = max::object_getoutput(my_object.maxobj(), k_out_y);
        out_x->clear();
        out_y->clear();

        WHEN("the x list arrives") {
            my_object.list_msg(atoms{1.0, 2.0, 3.0}, k_in_x);

            THEN("one list per axis is emitted, with a value per input coordinate") {
                REQUIRE(out_x->size() == 1);
                REQUIRE((*out_x)[0].size() == 3);
                REQUIRE(out_y->size() == 1);
                REQUIRE((*out_y)[0].size() == 3);
            }
            THEN("each point is turned by the same 90 degrees about z") {
                const auto& ys = (*out_y)[0];
                REQUIRE(std::abs(static_cast<double>(ys[0]) - 1.0) < 1e-5);
                REQUIRE(std::abs(static_cast<double>(ys[1]) - 2.0) < 1e-5);
                REQUIRE(std::abs(static_cast<double>(ys[2]) - 3.0) < 1e-5);
                for (size_t i = 0; i < 3; ++i) {
                    REQUIRE(std::abs(static_cast<double>((*out_x)[0][i])) < 1e-5);
                }
            }
        }
    }
}

SCENARIO("tap.rotate re-emits on bang without new coordinates") {
    ext_main(nullptr);

    GIVEN("a point already loaded and rotated once") {
        test_wrapper<rotate> an_instance;
        rotate&              my_object = an_instance;
        const point          first     = rotate_one(my_object, {1.0, 2.0, 3.0}, 10.0, 20.0, 30.0);

        auto* out_x = max::object_getoutput(my_object.maxobj(), k_out_x);
        out_x->clear();

        WHEN("bang is sent") {
            my_object.bang();

            THEN("the same result is emitted again") {
                REQUIRE(out_x->size() == 1);
                REQUIRE(std::abs(static_cast<double>((*out_x)[0][0]) - first.x) < 1e-12);
            }
        }
    }
}

SCENARIO("tap.rotate rejects a rotation that is not three values") {
    ext_main(nullptr);

    GIVEN("a point rotated 90 degrees about z") {
        test_wrapper<rotate> an_instance;
        rotate&              my_object = an_instance;
        const point          before    = rotate_one(my_object, {1.0, 0.0, 0.0}, 0.0, 0.0, 90.0);

        WHEN("a two-element rotation list arrives") {
            my_object.list_msg(atoms{45.0, 45.0}, k_in_rot);

            auto* out_y = max::object_getoutput(my_object.maxobj(), k_out_y);
            out_y->clear();
            my_object.list_msg(atoms{1.0}, k_in_x);

            THEN("the previous rotation is still in force") {
                REQUIRE(std::abs(static_cast<double>((*out_y)[0][0]) - before.y) < 1e-12);
            }
        }
    }
}

SCENARIO("tap.rotate accepts a bare number as a single coordinate") {
    ext_main(nullptr);

    GIVEN("a default instance with a 90 degree z rotation") {
        test_wrapper<rotate> an_instance;
        rotate&              my_object = an_instance;
        my_object.list_msg(atoms{0.0, 0.0, 90.0}, k_in_rot);
        my_object.number(atoms{0.0}, k_in_y);
        my_object.number(atoms{0.0}, k_in_z);

        auto* out_x = max::object_getoutput(my_object.maxobj(), k_out_x);
        auto* out_y = max::object_getoutput(my_object.maxobj(), k_out_y);
        out_x->clear();
        out_y->clear();

        WHEN("a number arrives at the x inlet") {
            my_object.number(atoms{2.0}, k_in_x);

            THEN("a single-element result is emitted") {
                REQUIRE(out_x->size() == 1);
                REQUIRE((*out_x)[0].size() == 1);
                REQUIRE(std::abs(static_cast<double>((*out_y)[0][0]) - 2.0) < 1e-5);
            }
        }
    }
}
