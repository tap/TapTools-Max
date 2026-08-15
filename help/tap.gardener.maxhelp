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
  "description": "After idle seconds with nobody planting, this starts planting for you \u2014 gusts of up to five neighbouring tubes, then calms, at about one strike per loop pass. Below is the whole of tap.garden~, patched: wind into scale into ring into rack.",
  "digest": "The idle wind that plants the garden when you stop",
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
     "text": "tap.gardener",
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
     "text": "The idle wind that plants the garden when you stop",
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
     "text": "After idle seconds with nobody planting, this starts planting for you \u2014 gusts of up to five neighbouring tubes, then calms, at about one strike per loop pass. Below is the whole of tap.garden~, patched: wind into scale into ring into rack."
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      170.0,
      480.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the wind \u2014 raw pitches, because the scale is the patch's field, not the wind's:"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      200.0,
      300.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "tap.gardener @idle 3. @gust 0.5 @loop 8.",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "comment",
     "patching_rect": [
      340.0,
      200.0,
      340.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "seed it and the same wind blows every run;"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "comment",
     "patching_rect": [
      340.0,
      220.0,
      340.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "set idle 0 and the seed cannot matter at all"
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "message",
     "patching_rect": [
      20.0,
      240.0,
      70.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "idle 3.",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "message",
     "patching_rect": [
      95.0,
      240.0,
      70.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "idle 0.",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "message",
     "patching_rect": [
      170.0,
      240.0,
      80.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "gust 0.",
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
      255.0,
      240.0,
      80.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "gust 1.",
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
      20.0,
      267.0,
      80.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "seed 12345",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      310.0,
      420.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "give the wind a field to blow across:"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      335.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 2,
     "text": "route plant",
     "outlettype": [
      "",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      365.0,
      220.0,
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
     "id": "obj-18",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      400.0,
      420.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "recirculate what it plants:"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      425.0,
      300.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "text": "tap.bloom @loop 8. @decay 0.85",
     "outlettype": [
      "",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      460.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 2,
     "text": "route note",
     "outlettype": [
      "",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "comment",
     "patching_rect": [
      340.0,
      425.0,
      400.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "anything you plant by hand closes the idle gate:"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "message",
     "patching_rect": [
      340.0,
      450.0,
      110.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "plant 60 0.9",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      495.0,
      420.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "and sound it:"
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      520.0,
      330.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 3,
     "text": "tap.chime~ @decay 5. @spread 0.7",
     "outlettype": [
      "signal",
      "signal",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "gain~",
     "patching_rect": [
      20.0,
      560.0,
      30.0,
      60.0
     ],
     "numinlets": 2,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "gain~",
     "patching_rect": [
      60.0,
      560.0,
      30.0,
      60.0
     ],
     "numinlets": 2,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-27",
     "maxclass": "ezdac~",
     "patching_rect": [
      20.0,
      640.0,
      45.0,
      45.0
     ],
     "numinlets": 2,
     "numoutlets": 0,
     "outlettype": []
    }
   },
   {
    "box": {
     "id": "obj-28",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      700.0,
      900.0,
      60.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "This chain is tap.garden~ with the lid off. Leave it alone for three seconds and the wind starts; plant by hand and it drops and waits again. One difference worth knowing: here the ring runs on Max's scheduler, so returns land within a millisecond of the grid rather than exactly on it, as they do inside the monolith."
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "obj-10",
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
      "obj-11",
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
      "obj-12",
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
      "obj-13",
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
      "obj-14",
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
      "obj-16",
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
      "obj-17",
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
      "obj-22",
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
      "obj-22",
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
      "obj-20",
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
      1
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
      "obj-25",
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
      0
     ],
     "destination": [
      "obj-27",
      1
     ]
    }
   }
  ]
 }
}
