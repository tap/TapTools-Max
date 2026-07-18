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
   120.0,
   80.0,
   900.0,
   720.0
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
  "statusbarvisible": 2,
  "toolbarvisible": 1,
  "description": "tap.303.seq~: the acid line driving tap.303~ sample-accurately \u2014 accent, slide (gate-hold legato), transpose, swing, slots.",
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      8.0,
      300.0,
      20.0
     ],
     "text": "tap.303.seq~ \u2014 a step-sequenced acid line"
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      32.0,
      860.0,
      60.0
     ],
     "text": "The row emits tap.303~'s inlet pair: pitch (MIDI note) and a gate whose edge amplitude encodes accent (1 plain, 2 accented). A step with the slide flag is approached legato \u2014 the gate holds through the boundary and the voice glides (~60 ms) without retriggering, exactly the hardware. This pattern is the tap.303~-pattern.maxpat demo line, now sample-accurate."
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      10.0,
      108.0,
      110.0,
      22.0
     ],
     "text": "phasor~ 0.4845"
    }
   },
   {
    "box": {
     "id": "obj-4",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      130.0,
      108.0,
      380.0,
      20.0
     ],
     "text": "<- one ramp per pattern; the same phasor can drive tap.808.seq~ rows in lock"
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      150.0,
      850.0,
      20.0
     ],
     "text": "pitches 33 33 45 33 33 36 33 40 38 33 33 45 33 36 35 31 \\, gates 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 \\, accents 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 \\, slides 0 0 1 0 0 0 0 1 0 0 0 0 0 1 0 0"
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ],
     "patching_rect": [
      870.0,
      108.0,
      70.0,
      22.0
     ],
     "text": "loadbang"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      178.0,
      640.0,
      20.0
     ],
     "text": "^ the whole line as lists \u2014 or per step: step <n> <pitch> [accent] [slide], rest <n>"
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      206.0,
      130.0,
      20.0
     ],
     "text": "step 3 45 0 1"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      150.0,
      206.0,
      50.0,
      20.0
     ],
     "text": "rest 3"
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      220.0,
      206.0,
      90.0,
      20.0
     ],
     "text": "transpose 12"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      320.0,
      206.0,
      90.0,
      20.0
     ],
     "text": "transpose 0"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      420.0,
      206.0,
      80.0,
      20.0
     ],
     "text": "swing 0.61"
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      510.0,
      206.0,
      60.0,
      20.0
     ],
     "text": "store 2"
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      580.0,
      206.0,
      60.0,
      20.0
     ],
     "text": "recall 2"
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 3,
     "outlettype": [
      "signal",
      "signal",
      ""
     ],
     "patching_rect": [
      10.0,
      250.0,
      200.0,
      22.0
     ],
     "text": "tap.303.seq~"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "number",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "bang"
     ],
     "patching_rect": [
      230.0,
      250.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      287.0,
      250.0,
      200.0,
      20.0
     ],
     "text": "<- current step, for displays"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      10.0,
      300.0,
      430.0,
      22.0
     ],
     "text": "tap.303~ @cutoff 500 @resonance 0.95 @envmod 0.65 @decay 300 @accent 0.9"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      330.0,
      560.0,
      20.0
     ],
     "text": "^ pitch -> left inlet, gate -> right inlet; the voice's accent knob decides how hard accents hit; its gate edge can also trigger any tap.808.* voice directly"
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      450.0,
      268.0,
      56.0,
      20.0
     ],
     "text": "recall 1"
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      514.0,
      268.0,
      56.0,
      20.0
     ],
     "text": "recall 3"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      578.0,
      268.0,
      56.0,
      20.0
     ],
     "text": "recall 6"
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      640.0,
      268.0,
      260.0,
      20.0
     ],
     "text": "<- voice factory presets morph under the line"
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      10.0,
      366.0,
      40.0,
      22.0
     ],
     "text": "*~ 0.8"
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "ezdac~",
     "numinlets": 2,
     "numoutlets": 0,
     "outlettype": [],
     "patching_rect": [
      10.0,
      408.0,
      45.0,
      45.0
     ]
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "obj-3",
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
      "obj-6",
      0
     ],
     "destination": [
      "obj-5",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-5",
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
      "obj-8",
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
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
      "obj-18",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-15",
      1
     ],
     "destination": [
      "obj-18",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-15",
      2
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
      "obj-20",
      0
     ],
     "destination": [
      "obj-18",
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
      "obj-18",
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
      "obj-18",
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
      "obj-24",
      0
     ],
     "destination": [
      "obj-25",
      1
     ]
    }
   }
  ],
  "dependency_cache": [],
  "autosave": 0
 }
}
