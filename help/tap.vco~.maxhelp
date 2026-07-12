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
     "text": "tap.vco~"
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
     "text": "virtual-analog oscillator"
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
     "text": "PolyBLEP alias-suppressed saw / pulse (with PWM) / triangle / sine, morphed by one continuous shape parameter (0..3). Hard sync on the right inlet, through-zero linear FM (in Hz) on the middle inlet, and analog character: slow deterministic pitch drift plus static detune. Pair it with tap.ladder~ for a complete VA voice."
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
     "text": "Waveform"
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
      160.0,
      19.0
     ],
     "text": "Hz, 0.01..20000"
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
      66.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "shape $1"
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
     "text": "0 sine - 1 tri - 2 saw - 3 pulse (morphs continuously)"
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
      248.0,
      82.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "waveform sine"
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      118.0,
      248.0,
      82.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "waveform triangle"
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      206.0,
      248.0,
      82.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "waveform saw"
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      294.0,
      248.0,
      82.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "waveform pulse"
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
      274.0,
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
     "id": "obj-17",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      297.0,
      50.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "pw $1"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      128.0,
      274.0,
      220.0,
      19.0
     ],
     "text": "pulse width % (try sweeping it)"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      400.0,
      132.0,
      200.0,
      19.0
     ],
     "text": "Analog character"
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      400.0,
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
     "id": "obj-21",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      400.0,
      177.0,
      62.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "drift $1"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      468.0,
      154.0,
      130.0,
      19.0
     ],
     "text": "cents of slow drift"
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      400.0,
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
     "id": "obj-24",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      400.0,
      224.0,
      70.0,
      17.0
     ],
     "outlettype": [
      ""
     ],
     "text": "detune $1"
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      478.0,
      201.0,
      120.0,
      19.0
     ],
     "text": "cents, static"
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      400.0,
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
     "id": "obj-27",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      400.0,
      271.0,
      60.0,
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
     "id": "obj-28",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      468.0,
      248.0,
      140.0,
      19.0
     ],
     "text": "decorrelate instances"
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
      335.0,
      560.0,
      19.0
     ],
     "text": "Presets morph over the interp time; sweep shape+pw+frequency between snapshots"
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
      357.0,
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
     "id": "obj-31",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      86.0,
      357.0,
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
     "id": "obj-32",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      142.0,
      357.0,
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
     "id": "obj-33",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      202.0,
      357.0,
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
     "id": "obj-34",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      262.0,
      357.0,
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
     "id": "obj-35",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      358.0,
      357.0,
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
     "id": "obj-36",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      410.0,
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
     "id": "obj-37",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      410.0,
      380.0,
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
     "id": "obj-38",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      30.0,
      415.0,
      560.0,
      19.0
     ],
     "text": "FM (middle inlet, in Hz) and hard sync (right inlet)"
    }
   },
   {
    "box": {
     "id": "obj-39",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      30.0,
      437.0,
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
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      461.0,
      70.0,
      19.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "cycle~ 110."
    }
   },
   {
    "box": {
     "id": "obj-41",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      485.0,
      46.0,
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
     "id": "obj-42",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      110.0,
      437.0,
      220.0,
      19.0
     ],
     "text": "FM depth in Hz (push past the pitch!)"
    }
   },
   {
    "box": {
     "id": "obj-43",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      250.0,
      461.0,
      78.0,
      19.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "cycle~ 82.41"
    }
   },
   {
    "box": {
     "id": "obj-44",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      250.0,
      437.0,
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
     "id": "obj-45",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [
      250.0,
      485.0,
      46.0,
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
     "id": "obj-46",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      330.0,
      437.0,
      240.0,
      19.0
     ],
     "text": "toggle hard sync from a master oscillator"
    }
   },
   {
    "box": {
     "id": "obj-47",
     "maxclass": "newobj",
     "numinlets": 3,
     "numoutlets": 1,
     "patching_rect": [
      30.0,
      530.0,
      130.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.vco~ @shape 2"
    }
   },
   {
    "box": {
     "id": "obj-48",
     "maxclass": "scope~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      300.0,
      530.0,
      130.0,
      90.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-49",
     "maxclass": "gain~",
     "numinlets": 2,
     "numoutlets": 2,
     "patching_rect": [
      30.0,
      565.0,
      26.0,
      70.0
     ],
     "outlettype": [
      "signal",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-50",
     "maxclass": "ezdac~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      30.0,
      645.0,
      45.0,
      45.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-51",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      85.0,
      657.0,
      200.0,
      19.0
     ],
     "text": "start audio, raise the fader"
    }
   },
   {
    "box": {
     "id": "obj-52",
     "maxclass": "bpatcher",
     "name": "tap.badge.maxpat",
     "numinlets": 0,
     "numoutlets": 0,
     "patching_rect": [
      430.0,
      620.0,
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
      "obj-20",
      0
     ],
     "destination": [
      "obj-21",
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
      "obj-41",
      1
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
      "obj-45",
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
      "obj-41",
      0
     ],
     "destination": [
      "obj-47",
      1
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
      "obj-47",
      2
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
      "obj-49",
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
      "obj-50",
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
      "obj-50",
      1
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
      "obj-7",
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
      "obj-10",
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
      "obj-12",
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
      "obj-13",
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
      "obj-14",
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
      "obj-15",
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
      "obj-17",
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
      "obj-21",
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
      "obj-24",
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
      "obj-27",
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
      "obj-30",
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
      "obj-31",
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
      "obj-32",
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
      "obj-33",
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
      "obj-34",
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
      "obj-35",
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
      "obj-37",
      0
     ],
     "destination": [
      "obj-47",
      0
     ]
    }
   }
  ],
  "dependency_cache": [],
  "autosave": 0
 }
}