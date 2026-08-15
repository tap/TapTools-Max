#!/usr/bin/env python3
"""Generate `.maxtest.maxpat` runtime-test patchers for TapTools objects.

These patchers are consumed by Cycling '74's max-test harness (vendored as the
`max-test` submodule). The topology mirrors the harness's own reference example
(`max-test/patchers/2087-bitxor~.maxtest.maxpat`), so a generated patcher is a
faithful, structurally-valid starting point — but because this generator runs
headless (no Max), each new patcher should be opened once in Max to confirm the
timing/assert wiring before trusting it in CI. See runtime-tests/README.md.

Three topologies are supported:

  audio_test(...)   sig~ IN -> OBJECT -> round~ tol -> test.sample~
                    -> test.equals EXPECTED -> test.assert NAME
                    (loadbang starts DSP via a [1(->dac~; the sample bangs
                     test.terminate when done)

  control_test(...) loadbang -> delay -> trigger MSG -> OBJECT
                    -> test.equals EXPECTED -> test.assert NAME -> test.terminate

  null_test(...)    sig~ IN -> MONOLITH ------------------\
                    sig~ IN -> COMPONENT chain -> +~ -> -~ -> abs~ -> ... -> assert 0
                    For the objects that were split into components: proves the patch
                    and the object are the same machine by subtracting one from the
                    other. Audio-clock objects only — see the note on the garden below.

Usage:  python3 make_maxtest.py        # (re)generates the bundled examples
        # or import and call audio_test()/control_test()/null_test() from your own script.
"""

import json
import os

HEADER = {
    "fileversion": 1,
    "appversion": {"major": 7, "minor": 0, "revision": 4, "architecture": "x64", "modernui": 1},
    "rect": [66.0, 79.0, 1070.0, 480.0],
    "bglocked": 0, "openinpresentation": 0,
    "default_fontsize": 12.0, "default_fontface": 0,
    "default_fontname": "Helvetica Neue Light",
    "gridonopen": 1, "gridsize": [5.0, 5.0], "gridsnaponopen": 1,
    "statusbarvisible": 2, "toolbarvisible": 1,
}

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "patchers")


class _Builder:
    def __init__(self):
        self.boxes = []
        self.lines = []
        self._n = 0

    def box(self, text, maxclass="newobj", numinlets=1, numoutlets=1,
            outlettype=None, x=0.0, y=0.0, w=120.0, h=22.0):
        self._n += 1
        bid = f"obj-{self._n}"
        b = {
            "id": bid, "maxclass": maxclass,
            "numinlets": numinlets, "numoutlets": numoutlets,
            "outlettype": outlettype if outlettype is not None else [""] * numoutlets,
            "patching_rect": [x, y, w, h],
        }
        if maxclass in ("newobj", "message", "comment"):
            b["text"] = text
        self.boxes.append({"box": b})
        return bid

    def link(self, src, src_out, dst, dst_in):
        self.lines.append({"patchline": {
            "source": [src, src_out], "destination": [dst, dst_in],
            "hidden": 0, "disabled": 0,
        }})

    def patcher(self, description=""):
        p = dict(HEADER)
        p["description"] = description
        p["boxes"] = self.boxes
        p["lines"] = self.lines
        return {"patcher": p}


def _write(name, doc):
    os.makedirs(OUT_DIR, exist_ok=True)
    path = os.path.join(OUT_DIR, name)
    with open(path, "w") as f:
        json.dump(doc, f, indent=1)
        f.write("\n")
    # Validate it round-trips as JSON (structural sanity; not a Max-load check).
    json.load(open(path))
    print(f"wrote {path}")
    return path


def null_test(filename, monolith, components, assert_name, input_value="0.5",
              setup_messages=(), settle_ms=1500, tolerance=0.000001, description="",
              monolith_outlets=3, component_outlets=3):
    """A null test: drive a monolith and a hand-wired chain of its components from one
    source, subtract the two left busses, and assert the difference is zero.

    This is how "the patch IS the object" stops being a claim and becomes a measurement.
    It only applies where both sides run on the audio clock — see the note in
    runtime-tests/README.md about why there is no garden equivalent.

    `setup_messages` are (message_text, [target_indices]) pairs sent BEFORE DSP starts,
    where index 0 is the monolith and 1..n are the components. Sending them with the DSP
    chain off is what keeps the two sides sample-aligned: no audio has been processed, so
    every head is still at zero when the gates open.
    """
    b = _Builder()
    loadbang = b.box("loadbang", x=40, y=20, w=70)

    # Phase 1 (immediately on load, DSP still off): open the record gates.
    setup_trigger = b.box("t b b", numoutlets=2, outlettype=["bang", "bang"], x=40, y=60, w=70)

    src = b.box(f"sig~ {input_value}", numoutlets=1, outlettype=["signal"], x=300, y=100, w=110)
    mono = b.box(monolith, numinlets=1, numoutlets=monolith_outlets,
                 outlettype=["signal"] * (monolith_outlets - 1) + [""], x=300, y=150, w=380)

    comp_ids = []
    for i, text in enumerate(components):
        comp_ids.append(b.box(text, numinlets=1, numoutlets=component_outlets,
                              outlettype=["signal"] * (component_outlets - 1) + [""],
                              x=300, y=200 + 30 * i, w=380))

    # Sum the component lanes, then subtract the monolith's left bus from the sum.
    summed = b.box("+~", numinlets=2, numoutlets=1, outlettype=["signal"],
                   x=300, y=210 + 30 * len(components), w=60)
    diff = b.box("-~", numinlets=2, numoutlets=1, outlettype=["signal"],
                 x=300, y=250 + 30 * len(components), w=60)
    mag = b.box("abs~", numoutlets=1, outlettype=["signal"], x=300, y=290 + 30 * len(components), w=60)
    rnd = b.box(f"round~ {tolerance}", numoutlets=1, outlettype=["signal"],
                x=300, y=330 + 30 * len(components), w=110)
    sample = b.box("test.sample~", numoutlets=1, outlettype=[""], x=300, y=370 + 30 * len(components), w=90)
    equals = b.box("test.equals 0.", x=300, y=410 + 30 * len(components), w=150)
    assert_ = b.box(f"test.assert {assert_name}", x=300, y=450 + 30 * len(components), w=220)

    # Phase 2 (after the gates are open): start DSP, let it settle, then sample once.
    delay_dsp = b.box("delay 20", numoutlets=1, outlettype=["bang"], x=40, y=110, w=70)
    msg_on = b.box("1", maxclass="message", x=40, y=150, w=30)
    dac = b.box("dac~", numinlets=2, numoutlets=0, outlettype=[], x=40, y=190, w=45)
    delay_settle = b.box(f"delay {settle_ms}", numoutlets=1, outlettype=["bang"], x=120, y=150, w=90)
    terminate = b.box("test.terminate", x=120, y=230, w=110)

    b.link(loadbang, 0, setup_trigger, 0)
    # right outlet fires first in a trigger: the gates open before DSP is asked for
    for text, targets in setup_messages:
        msg = b.box(text, maxclass="message", x=700, y=100 + 30 * len(b.boxes) % 300, w=110)
        b.link(setup_trigger, 1, msg, 0)
        for t in targets:
            b.link(msg, 0, mono if t == 0 else comp_ids[t - 1], 0)

    b.link(setup_trigger, 0, delay_dsp, 0)
    b.link(delay_dsp, 0, msg_on, 0)
    b.link(msg_on, 0, dac, 0)
    b.link(delay_dsp, 0, delay_settle, 0)
    b.link(delay_settle, 0, sample, 0)

    b.link(src, 0, mono, 0)
    for c in comp_ids:
        b.link(src, 0, c, 0)
        b.link(c, 0, summed, 0)
    b.link(mono, 0, diff, 1)
    b.link(summed, 0, diff, 0)
    b.link(diff, 0, mag, 0)
    b.link(mag, 0, rnd, 0)
    b.link(rnd, 0, sample, 0)
    b.link(sample, 0, equals, 0)
    b.link(equals, 0, assert_, 0)
    b.link(assert_, 0, terminate, 0)
    return _write(filename, b.patcher(description))


def audio_test(filename, object_text, input_value, expected, assert_name,
               tolerance=0.000001, description="", numinlets=1, numoutlets=1):
    """A signal-rate test: feed `sig~ input_value` into `object_text`, sample the
    output, and assert it equals `expected` (within `tolerance`)."""
    b = _Builder()
    loadbang = b.box("loadbang", x=40, y=40, w=70)
    button = b.box("", maxclass="button", numinlets=1, numoutlets=1,
                   outlettype=["bang"], x=40, y=90, w=24, h=24)
    msg_on = b.box("1", maxclass="message", x=40, y=140, w=30)
    dac = b.box("dac~", numinlets=2, numoutlets=0, outlettype=[], x=40, y=190, w=45)
    terminate = b.box("test.terminate", x=120, y=140, w=110)

    sig = b.box(f"sig~ {input_value}", numoutlets=1, outlettype=["signal"],
                x=300, y=40, w=110)
    obj = b.box(object_text, numinlets=numinlets, numoutlets=numoutlets,
                outlettype=["signal"] * numoutlets, x=300, y=100, w=160)
    rnd = b.box(f"round~ {tolerance}", numoutlets=1, outlettype=["signal"],
                x=300, y=160, w=110)
    sample = b.box("test.sample~", numoutlets=1, outlettype=[""], x=300, y=220, w=90)
    equals = b.box(f"test.equals {expected}", x=300, y=280, w=150)
    assert_ = b.box(f"test.assert {assert_name}", x=300, y=340, w=180)

    b.link(loadbang, 0, button, 0)
    b.link(button, 0, msg_on, 0)
    b.link(msg_on, 0, dac, 0)
    b.link(button, 0, terminate, 0)
    b.link(sig, 0, obj, 0)
    b.link(obj, 0, rnd, 0)
    b.link(rnd, 0, sample, 0)
    b.link(sample, 0, button, 0)     # after the sample is taken, drive terminate
    b.link(sample, 0, equals, 0)
    b.link(equals, 0, assert_, 0)
    return _write(filename, b.patcher(description))


def control_test(filename, object_text, trigger, expected, assert_name,
                 delay_ms=200, description=""):
    """A control-rate test: after load, send `trigger` (e.g. "bang" or "5") into
    `object_text` and assert its output equals `expected`."""
    b = _Builder()
    loadbang = b.box("loadbang", x=40, y=40, w=70)
    delay = b.box(f"delay {delay_ms}", numoutlets=1, outlettype=["bang"], x=40, y=90, w=70)
    trig = b.box(trigger, maxclass="message", x=40, y=140, w=60)
    obj = b.box(object_text, numinlets=1, numoutlets=1, x=40, y=200, w=160)
    equals = b.box(f"test.equals {expected}", x=40, y=260, w=150)
    assert_ = b.box(f"test.assert {assert_name}", x=40, y=320, w=180)
    terminate = b.box("test.terminate", x=300, y=320, w=110)

    b.link(loadbang, 0, delay, 0)
    b.link(delay, 0, trig, 0)
    b.link(trig, 0, obj, 0)
    b.link(obj, 0, equals, 0)
    b.link(equals, 0, assert_, 0)
    b.link(assert_, 0, terminate, 0)
    return _write(filename, b.patcher(description))


if __name__ == "__main__":
    # Control-rate example: the first prime out of tap.prime on a bang is 2.
    control_test(
        "tap.prime.maxtest.maxpat",
        object_text="tap.prime",
        trigger="bang",
        expected="2",
        assert_name="tap.prime-first-is-2",
        description="tap.prime: first bang yields the prime 2.",
    )
    # Audio-rate example: tap.radians~ default mode converts Hz -> radians as
    # hz * pi / (sr/2). At sr=44100, sig~ 22050 (= Nyquist) -> pi.
    audio_test(
        "tap.radians~.maxtest.maxpat",
        object_text="tap.radians~",
        input_value="22050",
        expected="3.141593",
        assert_name="tap.radians~-nyquist-is-pi",
        description="tap.radians~ (Hz->radians): sig~ 22050 at 44.1k -> pi.",
    )
    # tap.svf~: a lowpass with the cutoff parked at the top of its range passes DC
    # at exactly unity gain (the Simper SVF's DC gain is 1 regardless of tuning).
    audio_test(
        "tap.svf~.maxtest.maxpat",
        object_text="tap.svf~ @type lowpass @frequency 20000",
        input_value="1.",
        expected="1.",
        assert_name="tap.svf~-lowpass-dc-unity",
        description="tap.svf~ lowpass @ 20 kHz: DC (sig~ 1) passes at unity.",
        numinlets=2,
    )
    # tap.autowah~: with sensitivity at the floor the envelope is off (the cocked-wah
    # manual mode) and the object is a fixed 2-pole lowpass parked at bias — whose DC
    # gain is exactly 1 (same Simper-SVF property the tap.svf~ test uses). This pins
    # both the cocked-wah behavior and the wet-only default mix in one assertion.
    audio_test(
        "tap.autowah~.maxtest.maxpat",
        object_text="tap.autowah~ @sensitivity -60 @bias 15000",
        input_value="1.",
        expected="1.",
        assert_name="tap.autowah~-cocked-lowpass-dc-unity",
        description="tap.autowah~ cocked mode (sensitivity -60): DC passes the parked lowpass at unity.",
        numinlets=2,
        numoutlets=2,
    )
    # tap.tune~: DC is unpitched — the detector reports unvoiced, the correction
    # relaxes to zero, and the period-locked two-tap grain engine passes the
    # signal at exactly unity (its complementary envelopes sum to 1 and the
    # frozen taps read a constant). Pins the unpitched-passthrough contract in
    # a real Max instance. (Outlet 2 is the pitch/key report outlet.)
    audio_test(
        "tap.tune~.maxtest.maxpat",
        object_text="tap.tune~",
        input_value="1.",
        expected="1.",
        assert_name="tap.tune~-unpitched-dc-unity",
        description="tap.tune~: unpitched DC passes the grain engine at unity (no correction applies).",
        numoutlets=2,
    )
    # tap.overdrive~: bypass is a pure passthrough regardless of the drive settings —
    # pins that the external loads and the signal path is wired (the DSP itself is
    # covered by the kernel repo's overdrive_test.cpp).
    audio_test(
        "tap.overdrive~.maxtest.maxpat",
        object_text="tap.overdrive~ @bypass 1 @drive 1.",
        input_value="0.5",
        expected="0.5",
        assert_name="tap.overdrive~-bypass-passthrough",
        description="tap.overdrive~ @bypass 1: sig~ 0.5 passes unprocessed.",
    )
    # The null test: three tap.reel~ summed must BE tap.airport~. The kernel suite
    # already pins this bitwise (airport_test.cpp, "standalone lanes summed are the
    # bank, bitwise"); this is the same claim made against the real externals loaded
    # in Max, which is where wrapper-level mistakes — a mis-forwarded attribute, a
    # dspsetup that re-prepares one side and not the other — would show up instead.
    #
    # All three lanes are hard-panned left so the left bus carries the whole sum, and
    # the record gates open while the DSP chain is still off, which is what keeps the
    # two sides sample-aligned: no audio has been processed, so every head is at zero.
    #
    # There is deliberately no tap.garden~ equivalent. tap.bloom and tap.gardener run
    # on Max's scheduler rather than the audio clock, so their returns land within a
    # tick of the grid instead of exactly on it — the patched garden is the same
    # machine but not the same sample stream, and a null test would be asserting
    # something untrue. That claim is pinned in the kernel instead (garden_test.cpp,
    # "the bed is exactly its components wired together, bitwise").
    null_test(
        "tap.reel~-is-airport.maxtest.maxpat",
        monolith=("tap.airport~ 12. @loops 3 @lengths 6.3 7.7 9.1 "
                  "@pans -1. -1. -1. @levels 1. 1. 1. @smooth 0."),
        components=[
            "tap.reel~ 12. @length 6.3 @pan -1. @level 1. @smooth 0.",
            "tap.reel~ 12. @length 7.7 @pan -1. @level 1. @smooth 0.",
            "tap.reel~ 12. @length 9.1 @pan -1. @level 1. @smooth 0.",
        ],
        assert_name="tap.reel~-sum-equals-tap.airport~",
        input_value="0.5",
        setup_messages=[
            ("record 0 1", [0]),
            ("record 1 1", [0]),
            ("record 2 1", [0]),
            ("record 1", [1, 2, 3]),
        ],
        description=("Null test: three tap.reel~ summed against tap.airport~ with the same "
                     "lengths. The difference must be zero — the patch IS the object."),
    )
