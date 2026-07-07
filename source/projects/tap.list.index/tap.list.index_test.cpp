/// @file
/// @brief      Unit tests for tap.list.index.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2005-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.list.index.cpp" // include the object source so we can instantiate it

using namespace c74;

SCENARIO("tap.list.index instantiates with the expected defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<list_index> an_instance;
        list_index&              my_object = an_instance;

        THEN("mode defaults to list2indexed") {
            REQUIRE(my_object.mode == symbol{"list2indexed"});
        }
        THEN("offset defaults to 0") {
            REQUIRE(static_cast<int>(my_object.offset) == 0);
        }
        THEN("onebased defaults to false") {
            REQUIRE(static_cast<bool>(my_object.onebased) == false);
        }
    }
}

SCENARIO("tap.list.index decomposes a list into [index value] pairs (list2indexed)") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<list_index> an_instance;
        list_index&              my_object = an_instance;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("the list {10, 20, 30} is received") {
            output->clear();
            my_object.list_msg({10, 20, 30});

            THEN("one [index value] pair is emitted per element, zero-based") {
                REQUIRE(output->size() == 3);
                REQUIRE(static_cast<long>((*output)[0][0]) == 0);
                REQUIRE(static_cast<long>((*output)[0][1]) == 10);
                REQUIRE(static_cast<long>((*output)[1][0]) == 1);
                REQUIRE(static_cast<long>((*output)[1][1]) == 20);
                REQUIRE(static_cast<long>((*output)[2][0]) == 2);
                REQUIRE(static_cast<long>((*output)[2][1]) == 30);
            }
        }
    }
}

SCENARIO("tap.list.index honors offset and onebased in list2indexed mode") {
    ext_main(nullptr);

    GIVEN("an instance with offset 10 and one-based indexing") {
        test_wrapper<list_index> an_instance;
        list_index&              my_object = an_instance;
        my_object.offset                   = 10;
        my_object.onebased                 = true;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("the list {7, 8} is received") {
            output->clear();
            my_object.list_msg({7, 8});

            THEN("indices start at offset + 1 (bias = 11)") {
                REQUIRE(output->size() == 2);
                REQUIRE(static_cast<long>((*output)[0][0]) == 11);
                REQUIRE(static_cast<long>((*output)[0][1]) == 7);
                REQUIRE(static_cast<long>((*output)[1][0]) == 12);
                REQUIRE(static_cast<long>((*output)[1][1]) == 8);
            }
        }
    }
}

SCENARIO("tap.list.index assembles a list from [index value] pairs (indexed2list)") {
    ext_main(nullptr);

    GIVEN("an instance in indexed2list mode") {
        test_wrapper<list_index> an_instance;
        list_index&              my_object = an_instance;
        my_object.mode                     = "indexed2list";

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("the pair [2 99] is received") {
            output->clear();
            my_object.list_msg({2, 99});

            THEN("a 3-element list is emitted, padded with the default 0 values") {
                REQUIRE(output->size() == 1);
                const auto& list = (*output)[0];
                REQUIRE(list.size() == 3);
                REQUIRE(static_cast<long>(list[0]) == 0);
                REQUIRE(static_cast<long>(list[1]) == 0);
                REQUIRE(static_cast<long>(list[2]) == 99);
            }

            AND_WHEN("a further pair [0 7] is received") {
                output->clear();
                my_object.list_msg({0, 7});

                THEN("the assembled list grows/updates in place") {
                    const auto& list = (*output)[0];
                    REQUIRE(list.size() == 3);
                    REQUIRE(static_cast<long>(list[0]) == 7);
                    REQUIRE(static_cast<long>(list[1]) == 0);
                    REQUIRE(static_cast<long>(list[2]) == 99);
                }
            }
        }
    }
}
