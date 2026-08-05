{
  "patcher": {
    "fileversion": 1,
    "appversion": {
      "major": 8,
      "minor": 6,
      "revision": 2,
      "architecture": "x64",
      "modernui": 1
    },
    "classnamespace": "box",
    "rect": [
      155.0,
      44.0,
      661.0,
      770.0
    ],
    "bglocked": 0,
    "openinpresentation": 0,
    "default_fontsize": 11.0,
    "default_fontface": 0,
    "default_fontname": "Arial",
    "gridonopen": 1,
    "gridsize": [
      15.0,
      15.0
    ],
    "gridsnaponopen": 1,
    "objectsnaponopen": 1,
    "statusbarvisible": 2,
    "toolbarvisible": 1,
    "lefttoolbarpinned": 0,
    "toptoolbarpinned": 0,
    "righttoolbarpinned": 0,
    "bottomtoolbarpinned": 0,
    "toolbars_unpinned_last_save": 0,
    "tallnewobj": 0,
    "boxanimatetime": 200,
    "enablehscroll": 1,
    "enablevscroll": 1,
    "devicewidth": 0.0,
    "description": "",
    "digest": "",
    "tags": "",
    "style": "",
    "subpatcher_template": "",
    "assistshowspatchername": 0,
    "boxes": [
      {
        "box": {
          "id": "obj-1",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            10.0,
            8.0,
            485.0,
            30.0
          ],
          "text": "tap.delay~"
        }
      },
      {
        "box": {
          "id": "obj-2",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            10.0,
            36.0,
            485.0,
            21.0
          ],
          "text": "a feedback delay line with fractional (Hermite) interpolation"
        }
      },
      {
        "box": {
          "id": "obj-3",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            10.0,
            57.0,
            585.0,
            80.0
          ],
          "text": "The input is delayed by a number of milliseconds set via the right inlet (signal or float) or the delay attribute, with DC-blocked feedback, an equal-power dry/wet mix, and a choice of fractional Hermite reads (interp 1, default) or the legacy bit-compatible integer-sample truncation (interp 0). The argument sets the buffer size in ms, which caps the delay time. Breaking change from the 1999 lineage: a signal connected to the right inlet ALWAYS drives the time - including at 0.0."
        }
      },
      {
        "box": {
          "id": "obj-4",
          "maxclass": "panel",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            5.0,
            5.0,
            605.0,
            52.0
          ]
        }
      },
      {
        "box": {
          "id": "obj-5",
          "maxclass": "bpatcher",
          "numinlets": 0,
          "numoutlets": 0,
          "patching_rect": [
            385.0,
            100.0,
            225.0,
            67.0
          ],
          "name": "tap.badge.maxpat"
        }
      },
      {
        "box": {
          "id": "obj-6",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30.0,
            185.0,
            300.0,
            19.0
          ],
          "text": "Controls (every change is slewed - no zippers)"
        }
      },
      {
        "box": {
          "id": "obj-7",
          "maxclass": "flonum",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            30.0,
            207.0,
            54.0,
            19.0
          ],
          "outlettype": [
            "",
            "bang"
          ],
          "parameter_enable": 0
        }
      },
      {
        "box": {
          "id": "obj-8",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            230.0,
            71.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "delay $1"
        }
      },
      {
        "box": {
          "id": "obj-9",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            100.0,
            207.0,
            310.0,
            19.0
          ],
          "text": "delay time in ms (clamped to the buffer size)"
        }
      },
      {
        "box": {
          "id": "obj-10",
          "maxclass": "flonum",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            30.0,
            257.0,
            54.0,
            19.0
          ],
          "outlettype": [
            "",
            "bang"
          ],
          "parameter_enable": 0
        }
      },
      {
        "box": {
          "id": "obj-11",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            280.0,
            92.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "feedback $1"
        }
      },
      {
        "box": {
          "id": "obj-12",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            100.0,
            257.0,
            310.0,
            19.0
          ],
          "text": "0 - 0.99: the loop is DC-blocked and always decays"
        }
      },
      {
        "box": {
          "id": "obj-13",
          "maxclass": "flonum",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            30.0,
            307.0,
            54.0,
            19.0
          ],
          "outlettype": [
            "",
            "bang"
          ],
          "parameter_enable": 0
        }
      },
      {
        "box": {
          "id": "obj-14",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            330.0,
            57.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "mix $1"
        }
      },
      {
        "box": {
          "id": "obj-15",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            100.0,
            307.0,
            400.0,
            19.0
          ],
          "text": "0 dry - 100 wet, equal-power (default 100 = wet-only, like the old object)"
        }
      },
      {
        "box": {
          "id": "obj-16",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            357.0,
            71.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "interp 1"
        }
      },
      {
        "box": {
          "id": "obj-17",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            116.0,
            357.0,
            420.0,
            19.0
          ],
          "text": "Hermite fractional reads (default): modulating the time stays clean"
        }
      },
      {
        "box": {
          "id": "obj-18",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            382.0,
            71.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "interp 0"
        }
      },
      {
        "box": {
          "id": "obj-19",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            116.0,
            382.0,
            420.0,
            19.0
          ],
          "text": "legacy integer-sample truncation, bit-compatible with the Jamoma object"
        }
      },
      {
        "box": {
          "id": "obj-20",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            407.0,
            50.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "clear"
        }
      },
      {
        "box": {
          "id": "obj-21",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            95.0,
            407.0,
            420.0,
            19.0
          ],
          "text": "flush the buffer and the feedback-path filter state"
        }
      },
      {
        "box": {
          "id": "obj-22",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30.0,
            445.0,
            570.0,
            45.0
          ],
          "text": "To modulate the time, patch a signal into the right inlet - the signal always wins, even at 0.0 (the old wrapper treated 0.0 as 'use the attribute'; that trap is gone). Try the chain below."
        }
      },
      {
        "box": {
          "id": "obj-23",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            420.0,
            495.0,
            72.0,
            19.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "cycle~ 0.3"
        }
      },
      {
        "box": {
          "id": "obj-24",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            420.0,
            519.0,
            48.0,
            19.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "*~ 100"
        }
      },
      {
        "box": {
          "id": "obj-25",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            420.0,
            543.0,
            48.0,
            19.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "+~ 150"
        }
      },
      {
        "box": {
          "id": "obj-26",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            480.0,
            543.0,
            130.0,
            19.0
          ],
          "text": "a 0.3 Hz time wobble"
        }
      },
      {
        "box": {
          "id": "obj-27",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30.0,
            500.0,
            200.0,
            19.0
          ],
          "text": "Clicks make echoes easy to hear"
        }
      },
      {
        "box": {
          "id": "obj-28",
          "maxclass": "button",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            522.0,
            20.0,
            20.0
          ],
          "outlettype": [
            "bang"
          ]
        }
      },
      {
        "box": {
          "id": "obj-29",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            550.0,
            44.0,
            19.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "click~"
        }
      },
      {
        "box": {
          "id": "obj-30",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            610.0,
            220.0,
            21.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "tap.delay~ 2000 @feedback 0.5 @mix 50"
        }
      },
      {
        "box": {
          "id": "obj-31",
          "maxclass": "scope~",
          "numinlets": 2,
          "numoutlets": 0,
          "patching_rect": [
            250.0,
            650.0,
            130.0,
            90.0
          ]
        }
      },
      {
        "box": {
          "id": "obj-32",
          "maxclass": "ezdac~",
          "numinlets": 2,
          "numoutlets": 0,
          "patching_rect": [
            30.0,
            690.0,
            45.0,
            45.0
          ]
        }
      },
      {
        "box": {
          "id": "obj-33",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            85.0,
            702.0,
            160.0,
            19.0
          ],
          "text": "start audio here"
        }
      }
    ],
    "lines": [
      {
        "patchline": {
          "source": [
            "obj-7",
            0
          ],
          "destination": [
            "obj-8",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-10",
            0
          ],
          "destination": [
            "obj-11",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-13",
            0
          ],
          "destination": [
            "obj-14",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-23",
            0
          ],
          "destination": [
            "obj-24",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-24",
            0
          ],
          "destination": [
            "obj-25",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-28",
            0
          ],
          "destination": [
            "obj-29",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-29",
            0
          ],
          "destination": [
            "obj-30",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-8",
            0
          ],
          "destination": [
            "obj-30",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-11",
            0
          ],
          "destination": [
            "obj-30",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-14",
            0
          ],
          "destination": [
            "obj-30",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-16",
            0
          ],
          "destination": [
            "obj-30",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-18",
            0
          ],
          "destination": [
            "obj-30",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-20",
            0
          ],
          "destination": [
            "obj-30",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-30",
            0
          ],
          "destination": [
            "obj-31",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-30",
            0
          ],
          "destination": [
            "obj-32",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-30",
            0
          ],
          "destination": [
            "obj-32",
            1
          ]
        }
      }
    ],
    "dependency_cache": [],
    "autosave": 0
  }
}
