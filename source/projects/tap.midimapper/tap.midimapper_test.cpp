/// @file
/// @brief      Unit tests for tap.midimapper.
/// @details    `includes` is assigned as `atoms`, not as `std::vector<int>`: min-api's
///             `attribute<std::vector<T>>::operator=(const T)` wraps the whole vector in a single
///             atom via the catch-all `atom(T)` template, which recurses until the stack runs out.
///             The `atoms` overload takes the same path Max does.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <vector>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.midimapper.cpp" // include the object source so we can instantiate it

using namespace c74;

namespace {

    // Inlet indices, in the order the object declares them.
    constexpr int k_in_note    = 0;
    constexpr int k_in_poly    = 1;
    constexpr int k_in_control = 2;
    constexpr int k_in_program = 3;
    constexpr int k_in_touch   = 4;
    constexpr int k_in_bend    = 5;
    constexpr int k_in_channel = 6;

} // namespace

SCENARIO("tap.midimapper instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<midimapper> an_instance;
        midimapper&              my_object = an_instance;

        THEN("it responds to nothing until a type is chosen") {
            REQUIRE(my_object.type == symbol{"none"});
        }
        THEN("the channel and value filters default to zero") {
            REQUIRE(static_cast<int>(my_object.channel) == 0);
            REQUIRE(static_cast<int>(my_object.match1) == 0);
            REQUIRE(static_cast<int>(my_object.match2) == 0);
        }
        THEN("no incoming items are appended to the output") {
            const std::vector<int>& flags = my_object.includes;
            REQUIRE(flags.size() == 3);
            REQUIRE(flags[0] == 0);
            REQUIRE(flags[1] == 0);
            REQUIRE(flags[2] == 0);
        }
    }
}

SCENARIO("tap.midimapper stays silent while its type is none") {
    ext_main(nullptr);

    GIVEN("a default instance with a mapto template") {
        test_wrapper<midimapper> an_instance;
        midimapper&              my_object = an_instance;
        my_object.mapto(atoms{symbol("target")});

        auto* output = max::object_getoutput(my_object.maxobj(), 0);
        output->clear();

        THEN("neither a note nor a program change produces output") {
            my_object.list_msg(atoms{0, 0}, k_in_note);
            my_object.int_msg(atoms{0}, k_in_program);
            REQUIRE(output->empty());
        }
    }
}

SCENARIO("tap.midimapper emits its template when a note matches") {
    ext_main(nullptr);

    GIVEN("type note, wildcard value filters, and a two-word template") {
        test_wrapper<midimapper> an_instance;
        midimapper&              my_object = an_instance;
        my_object.type                     = "note";
        my_object.match1                   = -1; // any pitch
        my_object.match2                   = -1; // any velocity
        my_object.mapto(atoms{symbol("synth"), symbol("gate")});

        auto* output = max::object_getoutput(my_object.maxobj(), 0);
        output->clear();

        WHEN("a note arrives at the note inlet") {
            my_object.list_msg(atoms{60, 100}, k_in_note);

            THEN("the template comes out on its own — nothing is appended by default") {
                REQUIRE(output->size() == 1);
                const auto& out = (*output)[0];
                REQUIRE(out.size() == 2);
                REQUIRE(symbol(out[0]) == symbol{"synth"});
                REQUIRE(symbol(out[1]) == symbol{"gate"});
            }
        }

        WHEN("a note arrives at the control-change inlet instead") {
            my_object.list_msg(atoms{60, 100}, k_in_control);

            THEN("nothing is emitted — the type selects the inlet that is live") {
                REQUIRE(output->empty());
            }
        }
    }
}

SCENARIO("tap.midimapper appends the incoming values when asked") {
    ext_main(nullptr);

    GIVEN("type control with both value flags set") {
        test_wrapper<midimapper> an_instance;
        midimapper&              my_object = an_instance;
        my_object.type                     = "control";
        my_object.match1                   = -1;
        my_object.match2                   = -1;
        my_object.includes                 = atoms{0, 1, 1};
        my_object.mapto(atoms{symbol("cutoff")});

        auto* output = max::object_getoutput(my_object.maxobj(), 0);
        output->clear();

        WHEN("a control change arrives") {
            my_object.list_msg(atoms{74, 42}, k_in_control);

            THEN("both incoming values follow the template") {
                REQUIRE(output->size() == 1);
                const auto& out = (*output)[0];
                REQUIRE(out.size() == 3);
                REQUIRE(symbol(out[0]) == symbol{"cutoff"});
                REQUIRE(static_cast<int>(out[1]) == 74);
                REQUIRE(static_cast<int>(out[2]) == 42);
            }
        }
    }

    GIVEN("type control with only the channel flag set") {
        test_wrapper<midimapper> an_instance;
        midimapper&              my_object = an_instance;
        my_object.type                     = "control";
        my_object.match1                   = -1;
        my_object.match2                   = -1;
        my_object.channel                  = -1; // any channel
        my_object.includes                 = atoms{1, 0, 0};
        my_object.mapto(atoms{symbol("cutoff")});

        auto* output = max::object_getoutput(my_object.maxobj(), 0);
        output->clear();

        WHEN("the channel inlet is set to 5 and a control change arrives") {
            my_object.int_msg(atoms{5}, k_in_channel);
            my_object.list_msg(atoms{74, 42}, k_in_control);

            THEN("the incoming channel follows the template") {
                REQUIRE(output->size() == 1);
                const auto& out = (*output)[0];
                REQUIRE(out.size() == 2);
                REQUIRE(static_cast<int>(out[1]) == 5);
            }
        }
    }
}

SCENARIO("tap.midimapper filters on the incoming channel") {
    ext_main(nullptr);

    GIVEN("type note restricted to channel 3") {
        test_wrapper<midimapper> an_instance;
        midimapper&              my_object = an_instance;
        my_object.type                     = "note";
        my_object.match1                   = -1;
        my_object.match2                   = -1;
        my_object.channel                  = 3;
        my_object.mapto(atoms{symbol("hit")});

        auto* output = max::object_getoutput(my_object.maxobj(), 0);
        output->clear();

        THEN("a note on channel 1 is rejected") {
            my_object.int_msg(atoms{1}, k_in_channel);
            my_object.list_msg(atoms{60, 100}, k_in_note);
            REQUIRE(output->empty());
        }
        THEN("a note on channel 3 is passed") {
            my_object.int_msg(atoms{3}, k_in_channel);
            my_object.list_msg(atoms{60, 100}, k_in_note);
            REQUIRE(output->size() == 1);
        }
    }
}

SCENARIO("tap.midimapper filters on the incoming values") {
    ext_main(nullptr);

    GIVEN("type note restricted to pitch 60, any velocity") {
        test_wrapper<midimapper> an_instance;
        midimapper&              my_object = an_instance;
        my_object.type                     = "note";
        my_object.match1                   = 60;
        my_object.match2                   = -1;
        my_object.mapto(atoms{symbol("hit")});

        auto* output = max::object_getoutput(my_object.maxobj(), 0);
        output->clear();

        THEN("pitch 60 matches at any velocity") {
            my_object.list_msg(atoms{60, 1}, k_in_note);
            my_object.list_msg(atoms{60, 127}, k_in_note);
            REQUIRE(output->size() == 2);
        }
        THEN("any other pitch is rejected") {
            my_object.list_msg(atoms{61, 100}, k_in_note);
            REQUIRE(output->empty());
        }
    }
}

SCENARIO("tap.midimapper maps the single-value MIDI types from their own inlets") {
    ext_main(nullptr);

    GIVEN("type bend with a wildcard value filter") {
        test_wrapper<midimapper> an_instance;
        midimapper&              my_object = an_instance;
        my_object.type                     = "bend";
        my_object.match1                   = -1;
        my_object.includes                 = atoms{0, 0, 1};
        my_object.mapto(atoms{symbol("bend")});

        auto* output = max::object_getoutput(my_object.maxobj(), 0);
        output->clear();

        THEN("the bend inlet maps, and the program/aftertouch inlets do not") {
            my_object.int_msg(atoms{8192}, k_in_bend);
            REQUIRE(output->size() == 1);
            REQUIRE(static_cast<int>((*output)[0][1]) == 8192);

            my_object.int_msg(atoms{5}, k_in_program);
            my_object.int_msg(atoms{5}, k_in_touch);
            REQUIRE(output->size() == 1);
        }
    }

    GIVEN("type any") {
        test_wrapper<midimapper> an_instance;
        midimapper&              my_object = an_instance;
        my_object.type                     = "any";
        my_object.match1                   = -1;
        my_object.match2                   = -1;
        my_object.mapto(atoms{symbol("anything")});

        auto* output = max::object_getoutput(my_object.maxobj(), 0);
        output->clear();

        THEN("every MIDI inlet maps") {
            my_object.list_msg(atoms{60, 100}, k_in_note);
            my_object.list_msg(atoms{60, 100}, k_in_poly);
            my_object.list_msg(atoms{74, 42}, k_in_control);
            my_object.int_msg(atoms{1}, k_in_program);
            my_object.int_msg(atoms{2}, k_in_touch);
            my_object.int_msg(atoms{3}, k_in_bend);
            REQUIRE(output->size() == 6);
        }
    }
}
