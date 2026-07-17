{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 8,
   "minor": 5,
   "revision": 0,
   "architecture": "x64",
   "modernui": 1
  },
  "rect": [
   80.0,
   80.0,
   520.0,
   560.0
  ],
  "bglocked": 0,
  "openinpresentation": 0,
  "default_fontsize": 11.0,
  "default_fontface": 0,
  "default_fontname": "Verdana",
  "gridonopen": 1,
  "gridsize": [
   5.0,
   5.0
  ],
  "gridsnaponopen": 1,
  "statusbarvisible": 2,
  "toolbarvisible": 1,
  "description": "tap.808.rim~ help.",
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      16.0,
      240.0,
      27.0
     ],
     "text": "tap.808.rim~",
     "fontname": "Verdana",
     "fontsize": 18.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      44.0,
      460.0,
      18.0
     ],
     "text": "TR-808-style rimshot/claves voice, synthesized from the analog circuit",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      66.0,
      460.0,
      72.0
     ],
     "text": "The rimshot: ~1667 + ~455 Hz resonators cut to a ~10 ms crack by the harmonic-generating VCA. @model claves: the same channel's pure ~2500 Hz woody tick. Edge amplitude 0..1 = accent; fully deterministic (no noise in this channel).",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-4",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      150.0,
      180.0,
      18.0
     ],
     "text": "trigger from the scheduler:",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      20.0,
      172.0,
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
     "id": "obj-6",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      20.0,
      198.0,
      70.0,
      20.0
     ],
     "outlettype": [
      "bang"
     ],
     "text": "metro 400",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "button",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      20.0,
      226.0,
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
     "id": "obj-8",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      60.0,
      226.0,
      70.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 0.3",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      140.0,
      226.0,
      66.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 1.",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      212.0,
      228.0,
      200.0,
      18.0
     ],
     "text": "<- accent: soft vs full hit",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      262.0,
      380.0,
      18.0
     ],
     "text": "or sample-accurately, with a signal edge (amplitude = accent):",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "button",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      20.0,
      284.0,
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
     "id": "obj-13",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      20.0,
      310.0,
      48.0,
      20.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "click~",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      20.0,
      356.0,
      120.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.808.rim~",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      260.0,
      150.0,
      140.0,
      18.0
     ],
     "text": "controls:",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "attrui",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      260.0,
      172.0,
      190.0,
      22.0
     ],
     "outlettype": [
      ""
     ],
     "attr": "model",
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "attrui",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      260.0,
      198.0,
      190.0,
      22.0
     ],
     "outlettype": [
      ""
     ],
     "attr": "level",
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "gain~",
     "numinlets": 2,
     "numoutlets": 2,
     "patching_rect": [
      20.0,
      396.0,
      30.0,
      60.0
     ],
     "outlettype": [
      "signal",
      "int"
     ]
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "ezdac~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      470.0,
      40.0,
      40.0
     ],
     "local": 1
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      80.0,
      490.0,
      400.0,
      18.0
     ],
     "text": "the crack cuts through anything; claves for the woody tick.",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "obj-5",
      0
     ],
     "destination": [
      "obj-6",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-7",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-7",
      0
     ],
     "destination": [
      "obj-14",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-8",
      0
     ],
     "destination": [
      "obj-14",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-9",
      0
     ],
     "destination": [
      "obj-14",
      0
     ],
     "hidden": 0,
     "disabled": 0
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
     ],
     "hidden": 0,
     "disabled": 0
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
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-16",
      0
     ],
     "destination": [
      "obj-14",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-17",
      0
     ],
     "destination": [
      "obj-14",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-14",
      0
     ],
     "destination": [
      "obj-18",
      0
     ],
     "hidden": 0,
     "disabled": 0
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
     ],
     "hidden": 0,
     "disabled": 0
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
      1
     ],
     "hidden": 0,
     "disabled": 0
    }
   }
  ]
 }
}
