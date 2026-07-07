/// @file
/// @brief      Unit tests for tap.bits.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.bits.cpp"       // include the object source so we can instantiate it

using namespace c74;

SCENARIO("tap.bits instantiates with the expected defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<bits> an_instance;
        bits&              my_object = an_instance;

        THEN("mode defaults to bits2ints") {
            REQUIRE(my_object.mode == symbol{"bits2ints"});
        }
        THEN("matrix_width defaults to 8") {
            REQUIRE(static_cast<int>(my_object.matrix_width) == 8);
        }
    }
}

SCENARIO("tap.bits packs a list of bits into an integer (bits2ints)") {
    ext_main(nullptr);

    GIVEN("a default instance (bits2ints mode)") {
        test_wrapper<bits> an_instance;
        bits&              my_object = an_instance;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        THEN("most-significant-bit-first lists pack correctly") {
            // {1,0,1} -> 0b101 = 5
            output->clear();
            my_object.list_msg({1, 0, 1});
            REQUIRE(output->size() == 1);
            REQUIRE(static_cast<long>((*output)[0][1]) == 5);

            // {1,1,1,1} -> 0b1111 = 15
            output->clear();
            my_object.list_msg({1, 1, 1, 1});
            REQUIRE(static_cast<long>((*output)[0][1]) == 15);

            // {1,0,0,0,0,0,0,0} -> 0b10000000 = 128
            output->clear();
            my_object.list_msg({1, 0, 0, 0, 0, 0, 0, 0});
            REQUIRE(static_cast<long>((*output)[0][1]) == 128);

            // all zeros -> 0
            output->clear();
            my_object.list_msg({0, 0, 0});
            REQUIRE(static_cast<long>((*output)[0][1]) == 0);
        }
    }
}

SCENARIO("tap.bits explodes an integer into a 32-bit list (ints2bits)") {
    ext_main(nullptr);

    GIVEN("an instance in ints2bits mode") {
        test_wrapper<bits> an_instance;
        bits&              my_object = an_instance;
        my_object.mode               = "ints2bits";

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("the integer 5 is received") {
            output->clear();
            my_object.number(atoms{5});

            THEN("a 32-element list is produced, MSB first, with 5 = ...00000101") {
                REQUIRE(output->size() == 1);
                const auto& bitlist = (*output)[0];
                REQUIRE(bitlist.size() == 32);

                // 5 occupies the three least-significant bits: positions 29,30,31 -> 1,0,1
                for (int i = 0; i < 32; ++i) {
                    long expected = 0;
                    if (i == 29) {
                        expected = 1; // bit value 4
                    }
                    if (i == 31) {
                        expected = 1; // bit value 1
                    }
                    INFO("bit position " << i);
                    REQUIRE(static_cast<long>(bitlist[i]) == expected);
                }
            }
        }

        WHEN("the integer 1 is received") {
            output->clear();
            my_object.number(atoms{1});

            THEN("only the least-significant (last) bit is set") {
                const auto& bitlist = (*output)[0];
                REQUIRE(bitlist.size() == 32);
                REQUIRE(static_cast<long>(bitlist[31]) == 1);
                REQUIRE(static_cast<long>(bitlist[30]) == 0);
            }
        }
    }
}

SCENARIO("tap.bits round-trips an integer through ints2bits and bits2ints") {
    ext_main(nullptr);

    GIVEN("two instances, one per mode") {
        test_wrapper<bits> exploder_w;
        bits&              exploder = exploder_w;
        exploder.mode               = "ints2bits";

        test_wrapper<bits> packer_w;
        bits&              packer = packer_w; // default bits2ints

        auto* explode_out = max::object_getoutput(exploder.maxobj(), 0);
        auto* pack_out    = max::object_getoutput(packer.maxobj(), 0);

        THEN("exploding 42 and re-packing yields 42") {
            explode_out->clear();
            exploder.number(atoms{42});
            const atoms bitlist = (*explode_out)[0]; // 32 bits

            pack_out->clear();
            packer.list_msg(bitlist);
            REQUIRE(static_cast<long>((*pack_out)[0][1]) == 42);
        }
    }
}

SCENARIO("tap.bits formats integers for a matrixctrl (ints2matrixctrl)") {
    ext_main(nullptr);

    GIVEN("an instance in ints2matrixctrl mode with width 4") {
        test_wrapper<bits> an_instance;
        bits&              my_object = an_instance;
        my_object.mode               = "ints2matrixctrl";
        my_object.matrix_width       = 4;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("a single integer 5 is received") {
            output->clear();
            my_object.list_msg(atoms{5});

            THEN("width [column row state] triples are emitted, LSB first") {
                // 5 = 0b0101. For column j=0, bits 0..3 = 1,0,1,0.
                REQUIRE(output->size() == 4);
                const long expected_state[4] = {1, 0, 1, 0};
                for (int col = 0; col < 4; ++col) {
                    const auto& triple = (*output)[col]; // [col, row, state]
                    REQUIRE(triple.size() == 3);
                    REQUIRE(static_cast<long>(triple[0]) == col); // column index
                    REQUIRE(static_cast<long>(triple[1]) == 0);   // row index (input #0)
                    INFO("column " << col);
                    REQUIRE(static_cast<long>(triple[2]) == expected_state[col]);
                }
            }
        }
    }
}
