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
      810.0
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
          "text": "tap.harmony~"
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
          "text": "formant-preserving multi-voice harmonizer"
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
          "text": "Up to four pitch-shifted copies of a monophonic input hold a chord around the dry voice: intervals in semitones (-24..24, fractional allowed), per-voice levels (0 disables), LPC formant preservation so shifted voices keep the singer's envelope, and glide between intervals. Sixteen preset slots morph via store / recall. Latency is one FFT frame (fftsize, default 1024). Wants monophonic, pitched input - sing one note at a time."
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
            400.0,
            19.0
          ],
          "text": "Chords: one message sets the intervals and enables those voices"
        }
      },
      {
        "box": {
          "id": "obj-7",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            207.0,
            99.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "chord 4 7 12"
        }
      },
      {
        "box": {
          "id": "obj-8",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            144.0,
            207.0,
            340.0,
            19.0
          ],
          "text": "major triad + octave above the dry voice"
        }
      },
      {
        "box": {
          "id": "obj-9",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            232.0,
            106.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "chord -12 3 7"
        }
      },
      {
        "box": {
          "id": "obj-10",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            151.0,
            232.0,
            340.0,
            19.0
          ],
          "text": "octave below + minor stack"
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
            257.0,
            85.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "chord 12.1"
        }
      },
      {
        "box": {
          "id": "obj-12",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            130.0,
            257.0,
            340.0,
            19.0
          ],
          "text": "one voice, a fractionally-wide octave (chorus shimmer)"
        }
      },
      {
        "box": {
          "id": "obj-13",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            282.0,
            50.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "chord"
        }
      },
      {
        "box": {
          "id": "obj-14",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            95.0,
            282.0,
            340.0,
            19.0
          ],
          "text": "no arguments: all voices off - dry passes through"
        }
      },
      {
        "box": {
          "id": "obj-15",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30.0,
            320.0,
            560.0,
            32.0
          ],
          "text": "Per-voice control underneath: interval1-4 (semitones, fractional) and level1-4 (0-2; 0 disables the voice). dry sets the (latency-aligned) dry level."
        }
      },
      {
        "box": {
          "id": "obj-16",
          "maxclass": "flonum",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            30.0,
            357.0,
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
          "id": "obj-17",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            380.0,
            99.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "interval1 $1"
        }
      },
      {
        "box": {
          "id": "obj-18",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            100.0,
            357.0,
            380.0,
            19.0
          ],
          "text": "voice 1 interval, live - glide smooths the sweep"
        }
      },
      {
        "box": {
          "id": "obj-19",
          "maxclass": "flonum",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            30.0,
            407.0,
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
          "id": "obj-20",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            430.0,
            57.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "dry $1"
        }
      },
      {
        "box": {
          "id": "obj-21",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            100.0,
            407.0,
            310.0,
            19.0
          ],
          "text": "dry level, 0 - 2"
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
            457.0,
            78.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "formant 1"
        }
      },
      {
        "box": {
          "id": "obj-23",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            123.0,
            457.0,
            380.0,
            19.0
          ],
          "text": "keep the singer's formants (default)"
        }
      },
      {
        "box": {
          "id": "obj-24",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            482.0,
            78.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "formant 0"
        }
      },
      {
        "box": {
          "id": "obj-25",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            123.0,
            482.0,
            380.0,
            19.0
          ],
          "text": "formants shift with the pitch - the chipmunk chorus"
        }
      },
      {
        "box": {
          "id": "obj-26",
          "maxclass": "flonum",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            30.0,
            507.0,
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
          "id": "obj-27",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            530.0,
            71.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "glide $1"
        }
      },
      {
        "box": {
          "id": "obj-28",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            100.0,
            507.0,
            400.0,
            19.0
          ],
          "text": "interval glide in ms: click-free at 15, portamento in the hundreds"
        }
      },
      {
        "box": {
          "id": "obj-29",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30.0,
            557.0,
            400.0,
            19.0
          ],
          "text": "Presets morph every voice at once"
        }
      },
      {
        "box": {
          "id": "obj-30",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            579.0,
            64.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "store 1"
        }
      },
      {
        "box": {
          "id": "obj-31",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            109.0,
            579.0,
            340.0,
            19.0
          ],
          "text": "snapshot the current chord into slot 1"
        }
      },
      {
        "box": {
          "id": "obj-32",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            604.0,
            71.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "recall 1"
        }
      },
      {
        "box": {
          "id": "obj-33",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            116.0,
            604.0,
            340.0,
            19.0
          ],
          "text": "morph back over the interp time"
        }
      },
      {
        "box": {
          "id": "obj-34",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            629.0,
            106.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "recall 2 3000"
        }
      },
      {
        "box": {
          "id": "obj-35",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            151.0,
            629.0,
            340.0,
            19.0
          ],
          "text": "slot 2, three-second morph"
        }
      },
      {
        "box": {
          "id": "obj-36",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            654.0,
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
          "id": "obj-37",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            95.0,
            654.0,
            340.0,
            19.0
          ],
          "text": "zero the running state (delay lines, phases, slews)"
        }
      },
      {
        "box": {
          "id": "obj-38",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            420.0,
            185.0,
            190.0,
            19.0
          ],
          "text": "Sing into the mic, or toggle the saw for a hands-free test"
        }
      },
      {
        "box": {
          "id": "obj-39",
          "maxclass": "ezadc~",
          "numinlets": 0,
          "numoutlets": 2,
          "patching_rect": [
            420.0,
            232.0,
            45.0,
            45.0
          ],
          "outlettype": [
            "signal",
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-40",
          "maxclass": "toggle",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            490.0,
            232.0,
            20.0,
            20.0
          ],
          "outlettype": [
            "int"
          ]
        }
      },
      {
        "box": {
          "id": "obj-41",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            490.0,
            260.0,
            60.0,
            19.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "saw~ 110"
        }
      },
      {
        "box": {
          "id": "obj-42",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            490.0,
            284.0,
            42.0,
            19.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "*~ 0."
        }
      },
      {
        "box": {
          "id": "obj-43",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            420.0,
            310.0,
            34.0,
            19.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "+~"
        }
      },
      {
        "box": {
          "id": "obj-44",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            700.0,
            190.0,
            21.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "tap.harmony~ @interval1 7 @level1 1"
        }
      },
      {
        "box": {
          "id": "obj-45",
          "maxclass": "scope~",
          "numinlets": 2,
          "numoutlets": 0,
          "patching_rect": [
            250.0,
            730.0,
            130.0,
            70.0
          ]
        }
      },
      {
        "box": {
          "id": "obj-46",
          "maxclass": "ezdac~",
          "numinlets": 2,
          "numoutlets": 0,
          "patching_rect": [
            30.0,
            740.0,
            45.0,
            45.0
          ]
        }
      },
      {
        "box": {
          "id": "obj-47",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            85.0,
            752.0,
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
            "obj-16",
            0
          ],
          "destination": [
            "obj-17",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-19",
            0
          ],
          "destination": [
            "obj-20",
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
            "obj-40",
            0
          ],
          "destination": [
            "obj-42",
            1
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-41",
            0
          ],
          "destination": [
            "obj-42",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-39",
            0
          ],
          "destination": [
            "obj-43",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-42",
            0
          ],
          "destination": [
            "obj-43",
            1
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-43",
            0
          ],
          "destination": [
            "obj-44",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-7",
            0
          ],
          "destination": [
            "obj-44",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-9",
            0
          ],
          "destination": [
            "obj-44",
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
            "obj-44",
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
            "obj-44",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-17",
            0
          ],
          "destination": [
            "obj-44",
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
            "obj-44",
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
            "obj-44",
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
            "obj-44",
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
            "obj-44",
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
            "obj-44",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-32",
            0
          ],
          "destination": [
            "obj-44",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-34",
            0
          ],
          "destination": [
            "obj-44",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-36",
            0
          ],
          "destination": [
            "obj-44",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-44",
            0
          ],
          "destination": [
            "obj-45",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-44",
            0
          ],
          "destination": [
            "obj-46",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-44",
            0
          ],
          "destination": [
            "obj-46",
            1
          ]
        }
      }
    ],
    "dependency_cache": [],
    "autosave": 0
  }
}
