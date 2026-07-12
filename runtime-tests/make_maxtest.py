#!/usr/bin/env python3
"""Generate `.maxtest.maxpat` runtime-test patchers for TapTools objects.

These patchers are consumed by Cycling '74's max-test harness (vendored as the
`max-test` submodule). The topology mirrors the harness's own reference example
(`max-test/patchers/2087-bitxor~.maxtest.maxpat`), so a generated patcher is a
faithful, structurally-valid starting point — but because this generator runs
headless (no Max), each new patcher should be opened once in Max to confirm the
timing/assert wiring before trusting it in CI. See runtime-tests/README.md.

Two topologies are supported:

  audio_test(...)   sig~ IN -> OBJECT -> round~ tol -> test.sample~
                    -> test.equals EXPECTED -> test.assert NAME
                    (loadbang starts DSP via a [1(->dac~; the sample bangs
                     test.terminate when done)

  control_test(...) loadbang -> delay -> trigger MSG -> OBJECT
                    -> test.equals EXPECTED -> test.assert NAME -> test.terminate

Usage:  python3 make_maxtest.py        # (re)generates the bundled examples
        # or import and call audio_test()/control_test() from your own script.
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


def audio_test(filename, object_text, input_value, expected, assert_name,
               tolerance=0.000001, description="", numinlets=1):
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
    obj = b.box(object_text, numinlets=numinlets, numoutlets=1, outlettype=["signal"],
                x=300, y=100, w=160)
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
