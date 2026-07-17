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
   730.0
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
     "text": "tap.diode~"
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
     "text": "virtual-analog diode-ladder filter - the TB-303 topology"
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
      60.0
     ],
     "text": "A zero-delay-feedback 4-stage diode ladder modeled from Stinchcombe's TB-303 circuit analysis: mutually-loading stages (shallow knee into a 24 dB/oct slope), saturating diode pairs, and a high-passed resonance feedback path - so like a stock 303, resonance up to 1 squelches but never quite self-oscillates. Above 1 (and the fbhp attribute) are the documented bends. The right inlet takes a signal for per-sample cutoff modulation. tap.ladder~ is the buffered Moog-style sibling."
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
      132.0,
      260.0,
      19.0
     ],
     "text": "The essentials"
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      30.0,
      154.0,
      60.0,
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
     "id": "obj-7",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      177.0,
      90.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "frequency $1"
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      128.0,
      154.0,
      200.0,
      19.0
     ],
     "text": "cutoff Hz (10..20000)"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      30.0,
      201.0,
      60.0,
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
     "id": "obj-10",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      224.0,
      92.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "resonance $1"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      128.0,
      201.0,
      300.0,
      19.0
     ],
     "text": "0..1.5 - up to 1 is stock-303 squelch (no self-osc); above 1 it sings"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      30.0,
      248.0,
      60.0,
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
     "id": "obj-13",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      271.0,
      64.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "drive $1"
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      128.0,
      248.0,
      280.0,
      19.0
     ],
     "text": "dB into the saturating ladder (-24..24)"
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      330.0,
      154.0,
      60.0,
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
     "id": "obj-16",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      330.0,
      177.0,
      62.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "fbhp $1"
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      400.0,
      154.0,
      200.0,
      19.0
     ],
     "text": "feedback HPF corner Hz (0..2000; 150 = hardware)"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      330.0,
      201.0,
      60.0,
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
     "id": "obj-19",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      330.0,
      224.0,
      60.0,
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
     "id": "obj-20",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      400.0,
      201.0,
      130.0,
      19.0
     ],
     "text": "output gain dB"
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      330.0,
      248.0,
      96.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "resonance 1.45"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      434.0,
      248.0,
      48.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "fbhp 0"
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      330.0,
      271.0,
      280.0,
      19.0
     ],
     "text": "the bends: extended res sings; fbhp 0 = exact-tuned self-osc"
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
      305.0,
      400.0,
      19.0
     ],
     "text": "Oversampling and smoothing"
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "umenu",
     "numinlets": 1,
     "numoutlets": 3,
     "patching_rect": [
      30.0,
      327.0,
      60.0,
      19.0
     ],
     "outlettype": [
      "int",
      "",
      ""
     ],
     "items": [
      "1",
      ",",
      "2",
      ",",
      "4"
     ]
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 4,
     "patching_rect": [
      98.0,
      327.0,
      76.0,
      19.0
     ],
     "outlettype": [
      "bang",
      "bang",
      "bang",
      ""
     ],
     "text": "sel 0 1 2"
    }
   },
   {
    "box": {
     "id": "obj-27",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      98.0,
      350.0,
      84.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "oversample 1"
    }
   },
   {
    "box": {
     "id": "obj-28",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      190.0,
      350.0,
      84.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "oversample 2"
    }
   },
   {
    "box": {
     "id": "obj-29",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      282.0,
      350.0,
      84.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "oversample 4"
    }
   },
   {
    "box": {
     "id": "obj-30",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      400.0,
      327.0,
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
     "id": "obj-31",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      400.0,
      350.0,
      72.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "smooth $1"
    }
   },
   {
    "box": {
     "id": "obj-32",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      480.0,
      327.0,
      120.0,
      19.0
     ],
     "text": "glide time, ms"
    }
   },
   {
    "box": {
     "id": "obj-33",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      30.0,
      385.0,
      560.0,
      19.0
     ],
     "text": "Presets morph over the interp time - store two extremes and ride recall"
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
      407.0,
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
     "id": "obj-35",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      86.0,
      407.0,
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
     "id": "obj-36",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      142.0,
      407.0,
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
     "id": "obj-37",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      202.0,
      407.0,
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
     "id": "obj-38",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      262.0,
      407.0,
      88.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "recall 2 4000"
    }
   },
   {
    "box": {
     "id": "obj-39",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      358.0,
      407.0,
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
     "id": "obj-40",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      410.0,
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
     "id": "obj-41",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      410.0,
      430.0,
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
     "id": "obj-42",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      30.0,
      465.0,
      400.0,
      19.0
     ],
     "text": "A saw wave is traditional"
    }
   },
   {
    "box": {
     "id": "obj-43",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      487.0,
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
     "id": "obj-44",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      120.0,
      487.0,
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
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      120.0,
      511.0,
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
     "id": "obj-46",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      535.0,
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
     "id": "obj-47",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      250.0,
      487.0,
      78.0,
      19.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "cycle~ 0.25"
    }
   },
   {
    "box": {
     "id": "obj-48",
     "maxclass": "newobj",
     "numinlets": 5,
     "numoutlets": 1,
     "patching_rect": [
      250.0,
      511.0,
      110.0,
      19.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "scale~ -1. 1. 200. 5000."
    }
   },
   {
    "box": {
     "id": "obj-49",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      250.0,
      465.0,
      330.0,
      19.0
     ],
     "text": "or patch an LFO into the right inlet: per-sample cutoff"
    }
   },
   {
    "box": {
     "id": "obj-50",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      570.0,
      210.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.diode~ @resonance 0.85 @frequency 500"
    }
   },
   {
    "box": {
     "id": "obj-51",
     "maxclass": "scope~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      300.0,
      570.0,
      130.0,
      90.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-52",
     "maxclass": "ezdac~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      30.0,
      612.0,
      45.0,
      45.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-53",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      85.0,
      624.0,
      160.0,
      19.0
     ],
     "text": "start audio here"
    }
   },
   {
    "box": {
     "id": "obj-54",
     "maxclass": "bpatcher",
     "name": "tap.badge.maxpat",
     "numinlets": 0,
     "numoutlets": 0,
     "patching_rect": [
      430.0,
      465.0,
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
      "obj-7",
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
      "obj-10",
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
      "obj-13",
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
      "obj-16",
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
      "obj-19",
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
      "obj-26",
      1
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
      "obj-26",
      2
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
      "obj-40",
      0
     ],
     "destination": [
      "obj-41",
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
      "obj-45",
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
      "obj-46",
      1
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
      "obj-50",
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
      "obj-48",
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
      "obj-50",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-50",
      0
     ],
     "destination": [
      "obj-52",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-50",
      0
     ],
     "destination": [
      "obj-52",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-50",
      0
     ],
     "destination": [
      "obj-51",
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
      "obj-50",
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
      "obj-50",
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
      "obj-50",
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
      "obj-50",
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
      "obj-50",
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
      "obj-50",
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
      "obj-50",
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
      "obj-50",
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
      "obj-50",
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
      "obj-50",
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
      "obj-50",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-35",
      0
     ],
     "destination": [
      "obj-50",
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
      "obj-50",
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
      "obj-50",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-38",
      0
     ],
     "destination": [
      "obj-50",
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
      "obj-50",
      0
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
      "obj-50",
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
      "obj-50",
      0
     ]
    }
   }
  ],
  "dependency_cache": [],
  "autosave": 0
 }
}
