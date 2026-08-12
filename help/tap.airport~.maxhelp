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
   720.0
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
  "description": "Up to eight free-running tape loops of unequal lengths, each with one head that plays and records. Nothing ever resets a phase \u2014 the free-run is the piece. Punch a phrase onto each loop, freeze it, and let the lengths do the composing: the coincidences never repeat.",
  "digest": "The Music for Airports incommensurate loop bank",
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
     "bgfillcolor_angle": 270.0
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "comment",
     "patching_rect": [
      30,
      20,
      400,
      36
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "tap.airport~",
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
      30,
      55,
      620,
      22
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "The Music for Airports incommensurate loop bank",
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
      680,
      20,
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
      20,
      120,
      900,
      34
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Up to eight free-running tape loops of unequal lengths, each with one head that plays and records. Nothing ever resets a phase \u2014 the free-run is the piece. Punch a phrase onto each loop, freeze it, and let the lengths do the composing: the coincidences never repeat.",
     "fontsize": 12.0
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "comment",
     "patching_rect": [
      20,
      170,
      320,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "a phrase source to punch onto the loops:"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      195,
      110,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "text": "cycle~ 330"
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "gain~",
     "patching_rect": [
      20,
      225,
      30,
      60
     ],
     "numinlets": 2,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      ""
     ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      330,
      430,
      22
     ],
     "numinlets": 1,
     "numoutlets": 3,
     "outlettype": [
      "signal",
      "signal",
      ""
     ],
     "text": "tap.airport~ 12. @loops 3 @lengths 6.3 7.7 9.1 @pans -0.7 0. 0.7"
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "comment",
     "patching_rect": [
      380,
      170,
      280,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the record gates (loop, on/off):"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "message",
     "patching_rect": [
      380,
      195,
      90,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "record 0 $1"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "toggle",
     "patching_rect": [
      476,
      195,
      22,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "message",
     "patching_rect": [
      380,
      222,
      90,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "record 1 $1"
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "toggle",
     "patching_rect": [
      476,
      222,
      22,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "message",
     "patching_rect": [
      380,
      249,
      90,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "record 2 $1"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "toggle",
     "patching_rect": [
      476,
      249,
      22,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "comment",
     "patching_rect": [
      510,
      170,
      200,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "recording starts wherever the head is"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "comment",
     "patching_rect": [
      620,
      220,
      280,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the score is the lengths; ask the arithmetic:"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "message",
     "patching_rect": [
      620,
      245,
      60,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "period"
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "newobj",
     "patching_rect": [
      690,
      245,
      130,
      22
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "outlettype": [],
     "text": "print tap.airport~"
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "comment",
     "patching_rect": [
      380,
      280,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "a full restart of the piece:"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "message",
     "patching_rect": [
      380,
      303,
      60,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "clear"
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "gain~",
     "patching_rect": [
      20,
      380,
      30,
      60
     ],
     "numinlets": 2,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      ""
     ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "gain~",
     "patching_rect": [
      60,
      380,
      30,
      60
     ],
     "numinlets": 2,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      ""
     ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "ezdac~",
     "patching_rect": [
      20,
      460,
      45,
      45
     ],
     "numinlets": 2,
     "numoutlets": 0,
     "outlettype": []
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "comment",
     "patching_rect": [
      20,
      520,
      900,
      50
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: open record 0, play a phrase, close it; repeat on loops 1 and 2 with different material. Then stop playing. With incommensurate lengths the composite period (the period message) runs to hours \u2014 or to inf, which is the point. Run sources through tap.discreet~ first for tape breath."
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
      "obj-11",
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
      "obj-9",
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
      "obj-13",
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
      "obj-9",
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
      "obj-9",
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
      "obj-9",
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
      "obj-9",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-9",
      2
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
      "obj-9",
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
      "obj-9",
      1
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
      "obj-23",
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
  ]
 }
}
