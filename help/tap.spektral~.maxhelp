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
   910.0
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
     "text": "tap.spektral~"
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
      585.0,
      21.0
     ],
     "text": "spectral delay: per-band level, delay, and feedback across the spectrum"
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
      74.0
     ],
     "text": "A recreation of NI's Spektral Delay: an STFT splits the signal into log-spaced bands (default 160); every band gets its own level, delay (up to 12 s), and feedback, set as curves - send lists of ANY length (multislider-friendly, resampled to the band count) or poke single bands with [band value]. Freeze loops the delay memory losslessly. Rotate and smear process the spectrum; an LFO animates band shift / feedback / delay scale. Latency: one FFT frame (dry is delayed to match)."
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
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      30.0,
      145.0,
      560.0,
      19.0
     ],
     "text": "Curves: draw in the multisliders, or send any list / [band value] pokes"
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "multislider",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      30.0,
      167.0,
      170.0,
      60.0
     ],
     "outlettype": [
      "",
      ""
     ],
     "setminmax": [
      0.0,
      2.0
     ],
     "size": 16,
     "slidercolor": [
      0.4,
      0.6,
      0.9,
      1.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      104.0,
      232.0,
      100.0,
      19.0
     ],
     "text": "level 0..2"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "multislider",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      230.0,
      167.0,
      170.0,
      60.0
     ],
     "outlettype": [
      "",
      ""
     ],
     "setminmax": [
      0.0,
      4000.0
     ],
     "size": 16,
     "slidercolor": [
      0.9,
      0.6,
      0.4,
      1.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      308.0,
      232.0,
      110.0,
      19.0
     ],
     "text": "delay ms 0..12000"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "multislider",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      430.0,
      167.0,
      170.0,
      60.0
     ],
     "outlettype": [
      "",
      ""
     ],
     "setminmax": [
      0.0,
      0.99
     ],
     "size": 16,
     "slidercolor": [
      0.5,
      0.85,
      0.5,
      1.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      492.0,
      232.0,
      110.0,
      19.0
     ],
     "text": "feedback 0..0.99"
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      255.0,
      90.0,
      19.0
     ],
     "outlettype": [
      ""
     ],
     "text": "prepend gains"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      230.0,
      255.0,
      94.0,
      19.0
     ],
     "outlettype": [
      ""
     ],
     "text": "prepend delays"
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      430.0,
      255.0,
      80.0,
      19.0
     ],
     "outlettype": [
      ""
     ],
     "text": "prepend fbs"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      120.0,
      232.0,
      76.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "gains 40 0."
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      285.0,
      130.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "gains 1. 0.2 1. 0.2 1."
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      230.0,
      285.0,
      150.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "delays 0. 2000. 500. 4000."
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      430.0,
      285.0,
      96.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "fbs 0.2 0.8"
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
      320.0,
      260.0,
      19.0
     ],
     "text": "Masters"
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      30.0,
      342.0,
      50.0,
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
     "id": "obj-24",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      365.0,
      96.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "level $1"
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      84.0,
      342.0,
      40.0,
      19.0
     ],
     "text": "0..2"
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      150.0,
      342.0,
      50.0,
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
      150.0,
      365.0,
      96.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "fb $1"
    }
   },
   {
    "box": {
     "id": "obj-28",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      204.0,
      342.0,
      40.0,
      19.0
     ],
     "text": "0..2"
    }
   },
   {
    "box": {
     "id": "obj-29",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      270.0,
      342.0,
      50.0,
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
     "id": "obj-30",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      270.0,
      365.0,
      96.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "delayscale $1"
    }
   },
   {
    "box": {
     "id": "obj-31",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      324.0,
      342.0,
      40.0,
      19.0
     ],
     "text": "0..2"
    }
   },
   {
    "box": {
     "id": "obj-32",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      420.0,
      342.0,
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
     "id": "obj-33",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      420.0,
      365.0,
      70.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "freeze $1"
    }
   },
   {
    "box": {
     "id": "obj-34",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      448.0,
      342.0,
      150.0,
      19.0
     ],
     "text": "FREEZE: loop forever"
    }
   },
   {
    "box": {
     "id": "obj-35",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      30.0,
      400.0,
      400.0,
      19.0
     ],
     "text": "Mod FX + LFO"
    }
   },
   {
    "box": {
     "id": "obj-36",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      30.0,
      422.0,
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
     "id": "obj-37",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      445.0,
      68.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "rotate $1"
    }
   },
   {
    "box": {
     "id": "obj-38",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      106.0,
      422.0,
      150.0,
      19.0
     ],
     "text": "bins/sec, bipolar"
    }
   },
   {
    "box": {
     "id": "obj-39",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      230.0,
      422.0,
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
     "id": "obj-40",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      230.0,
      445.0,
      66.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "smear $1"
    }
   },
   {
    "box": {
     "id": "obj-41",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      292.0,
      422.0,
      150.0,
      19.0
     ],
     "text": "0..100: diffusion"
    }
   },
   {
    "box": {
     "id": "obj-42",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      30.0,
      470.0,
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
     "id": "obj-43",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      493.0,
      74.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "lforate $1"
    }
   },
   {
    "box": {
     "id": "obj-44",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      130.0,
      470.0,
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
     "id": "obj-45",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      130.0,
      493.0,
      82.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "lfodepth $1"
    }
   },
   {
    "box": {
     "id": "obj-46",
     "maxclass": "umenu",
     "numinlets": 1,
     "numoutlets": 3,
     "patching_rect": [
      240.0,
      470.0,
      130.0,
      19.0
     ],
     "outlettype": [
      "int",
      "",
      ""
     ],
     "items": [
      "off",
      ",",
      "bandshift",
      ",",
      "feedback",
      ",",
      "delayscale"
     ]
    }
   },
   {
    "box": {
     "id": "obj-47",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      240.0,
      493.0,
      76.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "lfodest $1"
    }
   },
   {
    "box": {
     "id": "obj-48",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      430.0,
      422.0,
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
     "id": "obj-49",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      430.0,
      445.0,
      56.0,
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
     "id": "obj-50",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      492.0,
      422.0,
      110.0,
      19.0
     ],
     "text": "dry/wet 0..100"
    }
   },
   {
    "box": {
     "id": "obj-51",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      30.0,
      525.0,
      560.0,
      19.0
     ],
     "text": "Presets: snapshots include the full curves; recall morphs over the interp time (ms)"
    }
   },
   {
    "box": {
     "id": "obj-52",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      547.0,
      50.0,
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
     "id": "obj-53",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      86.0,
      547.0,
      50.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "store 2"
    }
   },
   {
    "box": {
     "id": "obj-54",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      142.0,
      547.0,
      54.0,
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
     "id": "obj-55",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      202.0,
      547.0,
      54.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "recall 2"
    }
   },
   {
    "box": {
     "id": "obj-56",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      262.0,
      547.0,
      88.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "recall 2 8000"
    }
   },
   {
    "box": {
     "id": "obj-57",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      358.0,
      547.0,
      40.0,
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
     "id": "obj-58",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      410.0,
      547.0,
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
     "id": "obj-59",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      410.0,
      570.0,
      70.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "interp $1"
    }
   },
   {
    "box": {
     "id": "obj-60",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      30.0,
      605.0,
      400.0,
      19.0
     ],
     "text": "Percussive input shows the per-band delays best"
    }
   },
   {
    "box": {
     "id": "obj-61",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      627.0,
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
     "id": "obj-62",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      673.0,
      64.0,
      19.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.noise~"
    }
   },
   {
    "box": {
     "id": "obj-63",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      697.0,
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
     "id": "obj-64",
     "maxclass": "button",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      120.0,
      627.0,
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
     "id": "obj-65",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      120.0,
      673.0,
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
     "id": "obj-66",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      721.0,
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
     "id": "obj-67",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      753.0,
      130.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.spektral~ 2048"
    }
   },
   {
    "box": {
     "id": "obj-68",
     "maxclass": "scope~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      300.0,
      753.0,
      130.0,
      90.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-69",
     "maxclass": "ezdac~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      30.0,
      795.0,
      45.0,
      45.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-70",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      85.0,
      807.0,
      160.0,
      19.0
     ],
     "text": "start audio here"
    }
   },
   {
    "box": {
     "id": "obj-71",
     "maxclass": "bpatcher",
     "name": "tap.badge.maxpat",
     "numinlets": 0,
     "numoutlets": 0,
     "patching_rect": [
      430.0,
      605.0,
      180.0,
      67.0
     ]
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-15",
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
      "obj-16",
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
      "obj-17",
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
      "obj-32",
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
      "obj-36",
      0
     ],
     "destination": [
      "obj-37",
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
      "obj-40",
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
      "obj-46",
      0
     ],
     "destination": [
      "obj-47",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-48",
      0
     ],
     "destination": [
      "obj-49",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-58",
      0
     ],
     "destination": [
      "obj-59",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-61",
      0
     ],
     "destination": [
      "obj-63",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-62",
      0
     ],
     "destination": [
      "obj-63",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-63",
      0
     ],
     "destination": [
      "obj-66",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-64",
      0
     ],
     "destination": [
      "obj-65",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-65",
      0
     ],
     "destination": [
      "obj-66",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-66",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-67",
      0
     ],
     "destination": [
      "obj-69",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-67",
      0
     ],
     "destination": [
      "obj-69",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-67",
      0
     ],
     "destination": [
      "obj-68",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-15",
      0
     ],
     "destination": [
      "obj-67",
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
      "obj-67",
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
      "obj-67",
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
      "obj-67",
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
      "obj-67",
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
      "obj-67",
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
      "obj-67",
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
      "obj-67",
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
      "obj-67",
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
      "obj-67",
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
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-37",
      0
     ],
     "destination": [
      "obj-67",
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
      "obj-67",
      0
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
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-45",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-47",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-49",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-52",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-53",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-54",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-55",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-56",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-57",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-59",
      0
     ],
     "destination": [
      "obj-67",
      0
     ]
    }
   }
  ],
  "dependency_cache": [],
  "autosave": 0
 }
}