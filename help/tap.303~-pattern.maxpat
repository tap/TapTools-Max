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
   640.0,
   480.0
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
  "description": "tap.303~ driven by a Max-patched step sequencer (phase-3 dry run)",
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
      24.0
     ],
     "text": "tap.303~ pattern demo"
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
      34.0,
      560.0,
      48.0
     ],
     "text": "a Max-patched 16-step acid sequencer driving the note message - the dry run for the phase-3 tap.303.seq~ interface. Steps live in the coll as: index, pitch accent slide."
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ],
     "patching_rect": [
      30.0,
      100.0,
      24.0,
      24.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-4",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ],
     "patching_rect": [
      30.0,
      140.0,
      70.0,
      22.0
     ],
     "text": "metro 115"
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "newobj",
     "numinlets": 5,
     "numoutlets": 4,
     "outlettype": [
      "int",
      "",
      "",
      "int"
     ],
     "patching_rect": [
      30.0,
      180.0,
      90.0,
      22.0
     ],
     "text": "counter 1 16"
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "",
      "",
      ""
     ],
     "patching_rect": [
      30.0,
      220.0,
      120.0,
      22.0
     ],
     "text": "coll tap303.steps",
     "coll_data": {
      "count": 16,
      "data": [
       {
        "key": 1,
        "value": [
         33,
         1,
         0
        ]
       },
       {
        "key": 2,
        "value": [
         33,
         0,
         0
        ]
       },
       {
        "key": 3,
        "value": [
         45,
         0,
         1
        ]
       },
       {
        "key": 4,
        "value": [
         33,
         0,
         0
        ]
       },
       {
        "key": 5,
        "value": [
         33,
         0,
         0
        ]
       },
       {
        "key": 6,
        "value": [
         36,
         1,
         0
        ]
       },
       {
        "key": 7,
        "value": [
         33,
         0,
         0
        ]
       },
       {
        "key": 8,
        "value": [
         40,
         0,
         1
        ]
       },
       {
        "key": 9,
        "value": [
         38,
         0,
         0
        ]
       },
       {
        "key": 10,
        "value": [
         33,
         0,
         0
        ]
       },
       {
        "key": 11,
        "value": [
         33,
         1,
         0
        ]
       },
       {
        "key": 12,
        "value": [
         45,
         0,
         0
        ]
       },
       {
        "key": 13,
        "value": [
         33,
         0,
         0
        ]
       },
       {
        "key": 14,
        "value": [
         36,
         0,
         1
        ]
       },
       {
        "key": 15,
        "value": [
         35,
         0,
         0
        ]
       },
       {
        "key": 16,
        "value": [
         31,
         1,
         0
        ]
       }
      ]
     },
     "saved_object_attributes": {
      "embed": 1,
      "precision": 6
     }
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      30.0,
      260.0,
      90.0,
      22.0
     ],
     "text": "prepend note"
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      30.0,
      310.0,
      430.0,
      22.0
     ],
     "text": "tap.303~ @cutoff 500 @resonance 0.95 @envmod 0.65 @decay 300 @accent 0.9"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      30.0,
      350.0,
      50.0,
      22.0
     ],
     "text": "*~ 0.8"
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "ezdac~",
     "numinlets": 2,
     "numoutlets": 0,
     "outlettype": [],
     "patching_rect": [
      30.0,
      395.0,
      45.0,
      45.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      70.0,
      104.0,
      340.0,
      20.0
     ],
     "text": "toggle starts the line; edit the coll to write your own"
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
      250.0,
      140.0,
      56.0,
      20.0
     ],
     "text": "recall 1"
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
      314.0,
      140.0,
      56.0,
      20.0
     ],
     "text": "recall 3"
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
      378.0,
      140.0,
      56.0,
      20.0
     ],
     "text": "recall 6"
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      250.0,
      164.0,
      300.0,
      20.0
     ],
     "text": "factory presets morph under the running pattern"
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
      "obj-4",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-4",
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
      "obj-6",
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
      "obj-8",
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
      "obj-9",
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
      "obj-9",
      0
     ],
     "destination": [
      "obj-10",
      1
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
      "obj-8",
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
      "obj-8",
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
      "obj-8",
      0
     ]
    }
   }
  ],
  "dependency_cache": [],
  "autosave": 0
 }
}
