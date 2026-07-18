/// @file
/// @brief      Unit tests for tap.303~ (Min-level behavior).
/// @details    DSP correctness for the voice kernel lives in the kernel repo
///             (submodules/taptools/tests/tb303_voice_test.cpp — pitch/slide semantics,
///             envelopes, accent scope, waveforms). These scenarios cover the wrapper:
///             attribute defaults and clamping, the note/bang/release message plumbing into
///             the kernel, and a short audio smoke through the voice. The harness pins this
///             translation unit to C++17.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cmath>
#include <cstddef>
#include <vector>

#include "c74_min_unittest.h"
#include "tap.303_tilde.cpp"

namespace ktb = taptools::tb303;

namespace {

    constexpr double k_c_sr = 48000.0;

    double rms(const std::vector<double>& y, size_t lo, size_t hi) {
        double acc = 0.0;
        size_t n   = 0;
        for (size_t i = lo; i < hi && i < y.size(); ++i, ++n) {
            acc += y[i] * y[i];
        }
        return n ? std::sqrt(acc / n) : 0.0;
    }

} // namespace

SCENARIO("the Min wrapper instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("an instance of tap.303~") {
        test_wrapper<voice303> an_instance;
        voice303&              my_object = an_instance;

        THEN("attribute defaults match the spec") {
            REQUIRE(static_cast<double>(my_object.cutoff) == 1000.0);
            REQUIRE(static_cast<double>(my_object.resonance) == 0.5);
            REQUIRE(static_cast<double>(my_object.envmod) == 0.5);
            REQUIRE(static_cast<double>(my_object.decay) == 400.0);
            REQUIRE(static_cast<double>(my_object.accent) == 0.5);
            REQUIRE(static_cast<double>(my_object.tuning) == 0.0);
            REQUIRE(static_cast<double>(my_object.gain) == 0.0);
            REQUIRE(my_object.waveform == symbol("saw"));
            REQUIRE(static_cast<int>(my_object.oversample) == 2);
        }
        THEN("out-of-range values are clamped") {
            my_object.cutoff = 20000.0;
            REQUIRE(static_cast<double>(my_object.cutoff) == ktb::k_cutoff_max);
            my_object.cutoff = 1.0;
            REQUIRE(static_cast<double>(my_object.cutoff) == ktb::k_cutoff_min);
            my_object.resonance = 9.0;
            REQUIRE(static_cast<double>(my_object.resonance) == taptools::diode::k_res_max);
            my_object.envmod = 7.0;
            REQUIRE(static_cast<double>(my_object.envmod) == 1.0);
            my_object.decay = 1.0;
            REQUIRE(static_cast<double>(my_object.decay) == ktb::k_decay_min_ms);
            my_object.decay = 99999.0;
            REQUIRE(static_cast<double>(my_object.decay) == ktb::k_decay_max_ms);
            my_object.tuning = 5000.0;
            REQUIRE(static_cast<double>(my_object.tuning) == ktb::k_tuning_range);
            my_object.oversample = 3;
            REQUIRE(static_cast<int>(my_object.oversample) == 2);
        }
        THEN("attribute changes reach the kernel") {
            my_object.cutoff = 800.0;
            REQUIRE(my_object.voice().snap_targets().v[ktb::p_cutoff] == 800.0);
            my_object.envmod = 0.9;
            REQUIRE(my_object.voice().snap_targets().v[ktb::p_envmod] == 0.9);
        }
        THEN("the waveform symbol maps to the kernel's enum") {
            my_object.waveform = symbol("square");
            REQUIRE(my_object.voice().wave() == ktb::wave_square);
            my_object.waveform = symbol("saw");
            REQUIRE(my_object.voice().wave() == ktb::wave_saw);
        }
        THEN("the slice-4 bends default to stock and clamp to their ranges") {
            REQUIRE(static_cast<double>(my_object.slide) == 60.0);
            REQUIRE(static_cast<double>(my_object.attack) == 3.0);
            REQUIRE(static_cast<double>(my_object.accdecay) == 200.0);
            REQUIRE(static_cast<double>(my_object.drive) == 0.0);
            REQUIRE(static_cast<double>(my_object.tolerance) == 0.0);
            my_object.slide = 9999.0;
            REQUIRE(static_cast<double>(my_object.slide) == ktb::k_slide_max_ms);
            my_object.attack = 0.0;
            REQUIRE(static_cast<double>(my_object.attack) == ktb::k_attack_min_ms);
            my_object.accdecay = 1.0;
            REQUIRE(static_cast<double>(my_object.accdecay) == ktb::k_accdecay_min_ms);
            my_object.drive = 99.0;
            REQUIRE(static_cast<double>(my_object.drive) == ktb::k_drive_range_db);
            my_object.tolerance = 5.0;
            REQUIRE(static_cast<double>(my_object.tolerance) == 1.0);
            my_object.seed = 42;
            REQUIRE(my_object.voice().seed() == 42u);
        }
        THEN("the phase-2 VCA circuit defaults to clean and maps its symbols") {
            REQUIRE(my_object.vca == symbol("clean"));
            REQUIRE(my_object.voice().vca() == ktb::vca_clean);
            my_object.vca = symbol("warm");
            REQUIRE(my_object.voice().vca() == ktb::vca_warm);
            my_object.vca = symbol("clean");
            REQUIRE(my_object.voice().vca() == ktb::vca_clean);
        }
        THEN("factory presets are recallable out of the box") {
            my_object.recall(atoms{2, 0.0}); // deep sub
            REQUIRE(my_object.voice().snap_targets().v[ktb::p_cutoff] == 250.0);
            REQUIRE(my_object.voice().snap_targets().v[ktb::p_waveform] == 1.0);
        }
    }
}

SCENARIO("note messages drive the gate, slide, and release plumbing") {
    ext_main(nullptr);

    GIVEN("an instance of tap.303~") {
        test_wrapper<voice303> an_instance;
        voice303&              my_object = an_instance;

        THEN("note starts the gate and release ends it") {
            REQUIRE(!my_object.voice().gate());
            my_object.note(atoms{45});
            REQUIRE(my_object.voice().gate());
            my_object.release(atoms{});
            REQUIRE(!my_object.voice().gate());
        }
        THEN("a slid note keeps the gate held instead of retriggering") {
            my_object.note(atoms{45});
            my_object.note(atoms{52, 0, 1}); // slide flag
            REQUIRE(my_object.voice().gate());
            my_object.release(atoms{});
        }
        THEN("bang retriggers the last note") {
            my_object.note(atoms{40});
            my_object.release(atoms{});
            my_object.bang(atoms{});
            REQUIRE(my_object.voice().gate());
            my_object.release(atoms{});
        }
        THEN("preset and clear messages are callable") {
            my_object.store(atoms{1});
            my_object.recall(atoms{1});
            my_object.recall(atoms{1, 250.0});
            my_object.clear(atoms{});
            REQUIRE(!my_object.voice().gate());
        }
    }
}

SCENARIO("the wrapped voice makes acid: audible when gated, silent after release") {
    ext_main(nullptr);

    GIVEN("a triggered voice") {
        test_wrapper<voice303> an_instance;
        voice303&              my_object = an_instance;
        auto&                  v         = my_object.voice();
        v.prepare(k_c_sr);
        v.set_smooth_ms(0.0);

        my_object.note(atoms{45, 1});
        std::vector<double> held(static_cast<size_t>(0.5 * k_c_sr));
        v.process(held.data(), held.size());
        THEN("it sounds") {
            REQUIRE(rms(held, held.size() / 10, held.size()) > 0.01);
        }

        my_object.release(atoms{});
        std::vector<double> off(static_cast<size_t>(0.25 * k_c_sr));
        v.process(off.data(), off.size());
        THEN("release chops it") {
            REQUIRE(rms(off, off.size() / 5, off.size()) < 1e-4);
        }
    }
}
