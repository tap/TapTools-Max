/// @file
/// @brief      Unit tests for tap.route.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.route.cpp"      // include the object source so we can instantiate it

using namespace c74;

// Outlet indices: 0 = matched, 1 = unmatched, 2 = dump.
static constexpr int k_out_matched   = 0;
static constexpr int k_out_unmatched = 1;

SCENARIO("tap.route instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<route> an_instance;
        route&              my_object = an_instance;

        THEN("searchstring defaults to \"default\"") {
            const symbol s = my_object.searchstring;
            REQUIRE(std::string(s.c_str()) == "default");
        }
        THEN("searchpositions defaults to { 1 }") {
            const std::vector<int>& p = my_object.searchpositions;
            REQUIRE(p.size() == 1);
            REQUIRE(p[0] == 1);
        }
        THEN("partialmatch defaults to false") {
            REQUIRE(static_cast<bool>(my_object.partialmatch) == false);
        }
    }
}

SCENARIO("tap.route sends matching messages out the matched outlet and others out unmatched") {
    ext_main(nullptr);

    GIVEN("an instance watching position 1 for the symbol \"foo\"") {
        test_wrapper<route> an_instance;
        route&              my_object = an_instance;
        my_object.searchstring        = symbol("foo");

        auto* matched   = max::object_getoutput(my_object.maxobj(), k_out_matched);
        auto* unmatched = max::object_getoutput(my_object.maxobj(), k_out_unmatched);

        WHEN("a message beginning with foo arrives") {
            matched->clear();
            unmatched->clear();
            my_object.anything_msg(atoms{symbol("foo"), 1, 2}, 0);

            THEN("the whole message goes out the matched outlet") {
                REQUIRE(matched->size() == 1);
                REQUIRE(unmatched->empty());
                // The 'anything' selector is the leading token: the mock prepends it, so the
                // logged message is [foo, 1, 2].
                const auto& msg = matched->back();
                REQUIRE(msg.size() == 3);
                REQUIRE(std::string(static_cast<symbol>(msg[0]).c_str()) == "foo");
                REQUIRE(static_cast<int>(msg[1]) == 1);
                REQUIRE(static_cast<int>(msg[2]) == 2);
            }
        }

        WHEN("a message beginning with a different symbol arrives") {
            matched->clear();
            unmatched->clear();
            my_object.anything_msg(atoms{symbol("bar"), 1, 2}, 0);

            THEN("the message falls through to the unmatched outlet") {
                REQUIRE(matched->empty());
                REQUIRE(unmatched->size() == 1);
            }
        }
    }
}

SCENARIO("tap.route only tests watched positions") {
    ext_main(nullptr);

    GIVEN("an instance watching position 2 for the symbol \"foo\"") {
        test_wrapper<route> an_instance;
        route&              my_object = an_instance;
        my_object.searchstring        = symbol("foo");
        my_object.searchpositions     = atoms{2};

        auto* matched   = max::object_getoutput(my_object.maxobj(), k_out_matched);
        auto* unmatched = max::object_getoutput(my_object.maxobj(), k_out_unmatched);

        WHEN("foo appears at position 2") {
            matched->clear();
            unmatched->clear();
            my_object.anything_msg(atoms{symbol("bar"), symbol("foo")}, 0);

            THEN("it matches") {
                REQUIRE(matched->size() == 1);
                REQUIRE(unmatched->empty());
            }
        }

        WHEN("foo appears at position 1 (unwatched)") {
            matched->clear();
            unmatched->clear();
            my_object.anything_msg(atoms{symbol("foo"), symbol("bar")}, 0);

            THEN("it does not match") {
                REQUIRE(matched->empty());
                REQUIRE(unmatched->size() == 1);
            }
        }
    }
}

SCENARIO("tap.route matches int and float tokens by their string rendering") {
    ext_main(nullptr);

    GIVEN("an instance watching position 1 for the symbol \"5\"") {
        test_wrapper<route> an_instance;
        route&              my_object = an_instance;
        my_object.searchstring        = symbol("5");

        auto* matched   = max::object_getoutput(my_object.maxobj(), k_out_matched);
        auto* unmatched = max::object_getoutput(my_object.maxobj(), k_out_unmatched);

        WHEN("an int 5 arrives as a list") {
            matched->clear();
            unmatched->clear();
            my_object.list_msg(atoms{5, 6}, 0);

            THEN("the int renders to \"5\" and matches") {
                REQUIRE(matched->size() == 1);
                REQUIRE(unmatched->empty());
            }
        }

        WHEN("an int 6 arrives as a list") {
            matched->clear();
            unmatched->clear();
            my_object.list_msg(atoms{6, 5}, 0);

            THEN("position 1 is 6, which does not match") {
                REQUIRE(matched->empty());
                REQUIRE(unmatched->size() == 1);
            }
        }
    }
}

SCENARIO("tap.route partialmatch matches a leading prefix") {
    ext_main(nullptr);

    GIVEN("an instance with partialmatch on, searching for \"foo\"") {
        test_wrapper<route> an_instance;
        route&              my_object = an_instance;
        my_object.searchstring        = symbol("foo");
        my_object.partialmatch        = true;

        auto* matched   = max::object_getoutput(my_object.maxobj(), k_out_matched);
        auto* unmatched = max::object_getoutput(my_object.maxobj(), k_out_unmatched);

        WHEN("a token begins with foo") {
            matched->clear();
            unmatched->clear();
            my_object.anything_msg(atoms{symbol("foobar")}, 0);

            THEN("it matches on the prefix") {
                REQUIRE(matched->size() == 1);
                REQUIRE(unmatched->empty());
            }
        }

        WHEN("a token is shorter than the search string") {
            matched->clear();
            unmatched->clear();
            my_object.anything_msg(atoms{symbol("fo")}, 0);

            THEN("it does not match") {
                REQUIRE(matched->empty());
                REQUIRE(unmatched->size() == 1);
            }
        }
    }
}
