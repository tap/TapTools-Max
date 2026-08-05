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
      760.0
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
          "text": "tap.multitap~"
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
          "text": "multiple delay taps from one buffer, each with its own gain and stereo pan"
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
          "text": "One delay buffer, up to 99 read taps. The delay / gain / pan attributes take lists - one value per tap: times in ms (slewed, Hermite-interpolated), gains in dB, and an equal-power pan (-1 hard left .. 1 hard right) into the stereo outlet pair. A center-panned tap feeds both outlets equally; the old mono output is the mix of the two. The argument sets the buffer size in ms, which caps the tap times."
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
          "text": "Controls (lists address the taps in order)"
        }
      },
      {
        "box": {
          "id": "obj-7",
          "maxclass": "number",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            30.0,
            207.0,
            44.0,
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
            64.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "taps $1"
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
          "text": "active taps (1 - 99)"
        }
      },
      {
        "box": {
          "id": "obj-10",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            257.0,
            162.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "delay 125 250 375 500"
        }
      },
      {
        "box": {
          "id": "obj-11",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            207.0,
            257.0,
            340.0,
            19.0
          ],
          "text": "per-tap times in ms - a quarter-note echo ladder"
        }
      },
      {
        "box": {
          "id": "obj-12",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            282.0,
            120.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "gain 0 -3 -6 -9"
        }
      },
      {
        "box": {
          "id": "obj-13",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            165.0,
            282.0,
            340.0,
            19.0
          ],
          "text": "per-tap gains in dB - each repeat softer"
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
            307.0,
            148.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "pan -1 -0.33 0.33 1"
        }
      },
      {
        "box": {
          "id": "obj-15",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            193.0,
            307.0,
            340.0,
            19.0
          ],
          "text": "per-tap pan: the ladder walks left to right"
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
            332.0,
            148.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "delay 40 95 145 210"
        }
      },
      {
        "box": {
          "id": "obj-17",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            193.0,
            332.0,
            340.0,
            19.0
          ],
          "text": "short spread: one click becomes a stereo cluster"
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
            369.0,
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
          "id": "obj-19",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            116.0,
            369.0,
            340.0,
            19.0
          ],
          "text": "Hermite fractional reads (default)"
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
            394.0,
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
          "id": "obj-21",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            116.0,
            394.0,
            340.0,
            19.0
          ],
          "text": "legacy integer-sample truncation, bit-compatible"
        }
      },
      {
        "box": {
          "id": "obj-22",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            419.0,
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
          "id": "obj-23",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            95.0,
            419.0,
            340.0,
            19.0
          ],
          "text": "flush the buffer"
        }
      },
      {
        "box": {
          "id": "obj-24",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30.0,
            470.0,
            200.0,
            19.0
          ],
          "text": "Clicks show each tap distinctly"
        }
      },
      {
        "box": {
          "id": "obj-25",
          "maxclass": "button",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            492.0,
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
          "id": "obj-26",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            520.0,
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
          "id": "obj-27",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            30.0,
            575.0,
            420.0,
            21.0
          ],
          "outlettype": [
            "signal",
            "signal"
          ],
          "text": "tap.multitap~ 2000 @taps 4 @delay 125 250 375 500 @gain 0 -3 -6 -9 @pan -1 -0.33 0.33 1"
        }
      },
      {
        "box": {
          "id": "obj-28",
          "maxclass": "scope~",
          "numinlets": 2,
          "numoutlets": 0,
          "patching_rect": [
            250.0,
            640.0,
            130.0,
            90.0
          ]
        }
      },
      {
        "box": {
          "id": "obj-29",
          "maxclass": "ezdac~",
          "numinlets": 2,
          "numoutlets": 0,
          "patching_rect": [
            30.0,
            660.0,
            45.0,
            45.0
          ]
        }
      },
      {
        "box": {
          "id": "obj-30",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            85.0,
            672.0,
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
            "obj-25",
            0
          ],
          "destination": [
            "obj-26",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-26",
            0
          ],
          "destination": [
            "obj-27",
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
            "obj-27",
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
            "obj-27",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-12",
            0
          ],
          "destination": [
            "obj-27",
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
            "obj-27",
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
            "obj-27",
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
            "obj-27",
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
            "obj-27",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-22",
            0
          ],
          "destination": [
            "obj-27",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-27",
            0
          ],
          "destination": [
            "obj-28",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-27",
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
            "obj-27",
            1
          ],
          "destination": [
            "obj-29",
            1
          ]
        }
      }
    ],
    "dependency_cache": [],
    "autosave": 0
  }
}
