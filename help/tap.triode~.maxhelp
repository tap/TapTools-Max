{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 8,
   "minor": 5,
   "revision": 5,
   "architecture": "x64",
   "modernui": 1
  },
  "classnamespace": "box",
  "rect": [
   59.0,
   106.0,
   980.0,
   760.0
  ],
  "bglocked": 0,
  "openinpresentation": 0,
  "default_fontsize": 12.0,
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
  "description": "A common-cathode valve stage solved on its load line from the enhanced Norman Koren model, with parameters fitted to the valves in ondes Martenot No. 169. It inverts, it is strongly asymmetric, and drive changes the dirt rather than the level.",
  "digest": "One triode gain stage, from a published tube model",
  "tags": "TapTools",
  "style": "",
  "subpatcher_template": "",
  "assistshowspatchername": 0,
  "boxes": [
   {
    "box": {
     "maxclass": "panel",
     "patching_rect": [
      20,
      15,
      640,
      95
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "mode": 0,
     "proportion": 0.5,
     "bgmode": 0,
     "bgfillcolor_type": "gradient",
     "bgfillcolor_color1": [
      0.29,
      0.31,
      0.373,
      1.0
     ],
     "bgfillcolor_color2": [
      0.2,
      0.2,
      0.26,
      1.0
     ],
     "bgfillcolor_angle": 270.0,
     "id": "obj-1"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      30,
      20,
      400,
      36
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "tap.triode~",
     "fontsize": 24.0,
     "fontname": "Lato Light",
     "textcolor": [
      1.0,
      1.0,
      1.0,
      1.0
     ],
     "id": "obj-2"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      30,
      55,
      620,
      22
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "One triode gain stage, from a published tube model",
     "fontsize": 13.0,
     "fontname": "Lato Light",
     "textcolor": [
      0.92,
      0.92,
      0.95,
      1.0
     ],
     "id": "obj-3"
    }
   },
   {
    "box": {
     "maxclass": "bpatcher",
     "patching_rect": [
      680,
      20,
      225.0,
      67.0
     ],
     "numinlets": 0,
     "numoutlets": 0,
     "name": "tap.badge.maxpat",
     "id": "obj-4"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      20,
      120,
      920,
      58
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Not a tanh with a bias on it. The tube is the enhanced Norman Koren model and the parameter sets are fitted to the valves in ondes Martenot No. 169 \u2014 6F5, 6C5, 2A3 \u2014 together with each stage's supply voltage, cathode resistor and plate load. The stage is then the static solution of its load line, which is a memoryless nonlinearity in the DAFx-07 sense, so tabulating it is not an approximation of the model: it IS the model.",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-5"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      20,
      190,
      380,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "something to push through it:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-6"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      215,
      200,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "cycle~ 220",
     "outlettype": [
      "signal"
     ],
     "id": "obj-7"
    }
   },
   {
    "box": {
     "maxclass": "toggle",
     "patching_rect": [
      230,
      215,
      24,
      24
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ],
     "parameter_enable": 0,
     "id": "obj-8"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      260,
      218,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "audio on",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-9"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      300,
      400,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "tap.triode~ @tube 1 @stage 0 @drive 1.",
     "outlettype": [
      "signal"
     ],
     "id": "obj-10"
    }
   },
   {
    "box": {
     "maxclass": "gain~",
     "patching_rect": [
      20,
      350,
      30,
      60
     ],
     "numinlets": 2,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      ""
     ],
     "parameter_enable": 0,
     "id": "obj-11"
    }
   },
   {
    "box": {
     "maxclass": "ezdac~",
     "patching_rect": [
      20,
      430,
      45,
      45
     ],
     "numinlets": 2,
     "numoutlets": 0,
     "outlettype": [],
     "id": "obj-12"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      80,
      350,
      220,
      130
     ],
     "numinlets": 2,
     "numoutlets": 0,
     "text": "scope~ @range -1. 1.",
     "id": "obj-13"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      80,
      490,
      260,
      34
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "watch the waveform go asymmetric as drive comes up \u2014 that asymmetry is the even harmonics",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-14"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      470,
      190,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the valve and its operating point:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-15"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      470,
      215,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "tube",
     "id": "obj-16"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      470,
      240,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "stage",
     "id": "obj-17"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      470,
      270,
      250,
      62
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "tube: 0 = 6F5 (the oscillators'), 1 = 6C5 (demodulator and preamp), 2 = 2A3 (power). stage: 0/1/2 for the demodulator, preamplifier and power operating points. Both rebuild the curve \u2014 set them, do not sweep them.",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-18"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      760,
      190,
      200,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "and the knobs:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-19"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      760,
      215,
      200,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "drive",
     "id": "obj-20"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      760,
      240,
      200,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "highpass",
     "id": "obj-21"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      760,
      265,
      200,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "lowpass",
     "id": "obj-22"
    }
   },
   {
    "box": {
     "maxclass": "message",
     "patching_rect": [
      760,
      300,
      100,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "clear",
     "id": "obj-23"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      470,
      350,
      480,
      120
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: leave drive at 1 and sweep it up to 20. The level barely moves \u2014 the stage is normalized by its own small-signal gain, so drive is a distortion control and not a volume control. Then switch tube to 2 (the 2A3 power triode) at stage 2 and hear how much more headroom it has before it does anything at all. The wrong pairings solve too: a 6F5 at the power amplifier's operating point is not a thing that ever existed, and it sounds like it.",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-24"
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "obj-8",
      0
     ],
     "destination": [
      "obj-7",
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
      "obj-10",
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
      "obj-10",
      0
     ],
     "destination": [
      "obj-13",
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
      "obj-12",
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
      "obj-12",
      1
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-21",
      0
     ],
     "destination": [
      "obj-10",
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
      "obj-10",
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
      "obj-10",
      0
     ]
    }
   }
  ]
 }
}