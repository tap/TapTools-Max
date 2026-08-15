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
  "description": "The entry quantizer behind tap.garden~'s promise that anything you plant sounds consonant. A float in gives the snapped pitch out; a pitch/velocity pair passes the velocity through untouched, so it drops straight into a note chain.",
  "digest": "Snap a MIDI pitch to the nearest degree of a root and scale",
  "tags": "TapTools",
  "style": "",
  "subpatcher_template": "",
  "assistshowspatchername": 0,
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "panel",
     "patching_rect": [
      20.0,
      15.0,
      640.0,
      95.0
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
     "bgfillcolor_angle": 270.0
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "comment",
     "patching_rect": [
      30.0,
      20.0,
      400.0,
      36.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "tap.scale",
     "fontsize": 24.0,
     "fontname": "Lato Light",
     "textcolor": [
      1.0,
      1.0,
      1.0,
      1.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "comment",
     "patching_rect": [
      30.0,
      55.0,
      620.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Snap a MIDI pitch to the nearest degree of a root and scale",
     "fontsize": 13.0,
     "fontname": "Lato Light",
     "textcolor": [
      0.92,
      0.92,
      0.95,
      1.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-4",
     "maxclass": "bpatcher",
     "patching_rect": [
      680.0,
      20.0,
      225.0,
      67.0
     ],
     "numinlets": 0,
     "numoutlets": 0,
     "name": "tap.badge.maxpat"
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      120.0,
      900.0,
      34.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "The entry quantizer behind tap.garden~'s promise that anything you plant sounds consonant. A float in gives the snapped pitch out; a pitch/velocity pair passes the velocity through untouched, so it drops straight into a note chain."
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      170.0,
      300.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "drag through the chromatic scale:"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "flonum",
     "patching_rect": [
      20.0,
      195.0,
      60.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "bang"
     ]
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      230.0,
      200.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "tap.scale @root 0 @scale 3",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "flonum",
     "patching_rect": [
      20.0,
      265.0,
      60.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "bang"
     ]
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "comment",
     "patching_rect": [
      95.0,
      265.0,
      400.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "\u2014 only the degrees of the scale ever come out"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "patching_rect": [
      260.0,
      170.0,
      300.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the field:"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "message",
     "patching_rect": [
      260.0,
      195.0,
      70.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "root 0",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "message",
     "patching_rect": [
      335.0,
      195.0,
      70.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "root 7",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "message",
     "patching_rect": [
      260.0,
      222.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "scale 0",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "message",
     "patching_rect": [
      355.0,
      222.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "scale 3",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "message",
     "patching_rect": [
      260.0,
      249.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "scale 4",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "comment",
     "patching_rect": [
      360.0,
      249.0,
      320.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "0 chromatic, 1 major, 2 minor,"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "comment",
     "patching_rect": [
      360.0,
      269.0,
      320.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "3 major pentatonic, 4 minor pentatonic"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      320.0,
      420.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "in a note chain the velocity rides through untouched:"
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "message",
     "patching_rect": [
      20.0,
      345.0,
      130.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "63.5 0.8",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      380.0,
      200.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "tap.scale @root 2 @scale 4",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      415.0,
      130.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "print tap.scale",
     "outlettype": []
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      470.0,
      900.0,
      50.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Quantizing happens at entry, which is the same contract tap.garden~ keeps: changing the root or the scale re-pitches nothing already planted downstream, only what passes through next. Put this between tap.gardener and tap.bloom to turn raw wind into music."
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
   },
   {
    "patchline": {
     "source": [
      "obj-15",
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
      "obj-16",
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
      "obj-21",
      0
     ],
     "destination": [
      "obj-22",
      0
     ]
    }
   }
  ]
 }
}
