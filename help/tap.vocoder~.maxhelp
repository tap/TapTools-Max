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
      790.0
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
          "text": "tap.vocoder~"
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
          "text": "a 24-band channel vocoder with sibilance and an equal-power carrier mix"
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
            90.0
          ],
          "text": "The modulator (left inlet - a voice) imposes its spectral envelope onto the carrier (right inlet - a synth) through 24 bandpass filters and per-band envelope followers. sibilance blends deterministic seeded noise into the carrier path of the bands above ~4 kHz so unvoiced consonants come through (the modulator still gates them); at 0, the default, the output is identical to the noise-free vocoder. mix crossfades equal-power between the dry carrier and the vocoded signal."
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
            195.0,
            300.0,
            19.0
          ],
          "text": "Controls"
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
            217.0,
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
            240.0,
            43.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "q $1"
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
            217.0,
            310.0,
            19.0
          ],
          "text": "0.5 - 200: band narrowness; higher = more robotic"
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
            267.0,
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
            290.0,
            140.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "response_interval $1"
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
            267.0,
            380.0,
            19.0
          ],
          "text": "envelope-follower period in ms: short = sharp, long = smeared"
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
            317.0,
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
            340.0,
            99.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "sibilance $1"
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
            317.0,
            400.0,
            19.0
          ],
          "text": "0 - 1: seeded noise for the bands above ~4 kHz - intelligible s/t/k"
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
            367.0,
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
            390.0,
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
          "id": "obj-18",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            100.0,
            367.0,
            310.0,
            19.0
          ],
          "text": "0 = dry carrier - 100 = fully vocoded, equal-power"
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
            417.0,
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
            440.0,
            64.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "gain $1"
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
            417.0,
            310.0,
            19.0
          ],
          "text": "linear makeup gain on the summed output"
        }
      },
      {
        "box": {
          "id": "obj-22",
          "maxclass": "number",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            30.0,
            467.0,
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
          "id": "obj-23",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            490.0,
            64.0,
            17.0
          ],
          "outlettype": [
            ""
          ],
          "text": "seed $1"
        }
      },
      {
        "box": {
          "id": "obj-24",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            100.0,
            467.0,
            400.0,
            19.0
          ],
          "text": "sibilance noise seed (>= 1): deterministic; vary per mc. instance"
        }
      },
      {
        "box": {
          "id": "obj-25",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            517.0,
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
          "id": "obj-26",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            95.0,
            517.0,
            400.0,
            19.0
          ],
          "text": "reset all filter and envelope-follower state"
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
            555.0,
            260.0,
            19.0
          ],
          "text": "Modulator: speak or sing into the mic"
        }
      },
      {
        "box": {
          "id": "obj-28",
          "maxclass": "ezadc~",
          "numinlets": 0,
          "numoutlets": 2,
          "patching_rect": [
            30.0,
            577.0,
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
          "id": "obj-29",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            330.0,
            555.0,
            280.0,
            19.0
          ],
          "text": "Carrier: a detuned saw pair (the classic bed)"
        }
      },
      {
        "box": {
          "id": "obj-30",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            330.0,
            577.0,
            66.0,
            19.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "saw~ 82.41"
        }
      },
      {
        "box": {
          "id": "obj-31",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            410.0,
            577.0,
            66.0,
            19.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "saw~ 82.61"
        }
      },
      {
        "box": {
          "id": "obj-32",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            330.0,
            603.0,
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
          "id": "obj-33",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            30.0,
            650.0,
            130.0,
            21.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "tap.vocoder~ @q 20"
        }
      },
      {
        "box": {
          "id": "obj-34",
          "maxclass": "scope~",
          "numinlets": 2,
          "numoutlets": 0,
          "patching_rect": [
            250.0,
            690.0,
            130.0,
            90.0
          ]
        }
      },
      {
        "box": {
          "id": "obj-35",
          "maxclass": "ezdac~",
          "numinlets": 2,
          "numoutlets": 0,
          "patching_rect": [
            30.0,
            700.0,
            45.0,
            45.0
          ]
        }
      },
      {
        "box": {
          "id": "obj-36",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            85.0,
            712.0,
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
            "obj-22",
            0
          ],
          "destination": [
            "obj-23",
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
            "obj-31",
            0
          ],
          "destination": [
            "obj-32",
            1
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
            "obj-33",
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
            "obj-33",
            1
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
            "obj-33",
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
            "obj-33",
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
            "obj-33",
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
            "obj-33",
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
            "obj-33",
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
            "obj-33",
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
            "obj-33",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-33",
            0
          ],
          "destination": [
            "obj-34",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-33",
            0
          ],
          "destination": [
            "obj-35",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-33",
            0
          ],
          "destination": [
            "obj-35",
            1
          ]
        }
      }
    ],
    "dependency_cache": [],
    "autosave": 0
  }
}
