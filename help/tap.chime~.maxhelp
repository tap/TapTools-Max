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
  "description": "Four decaying mode doublets per strike at the transverse ratios of the chosen material. Every pitch is its own tube: same flaws, same seat in the stereo field, every time. Voice stealing takes the quietest bell and re-aims it, so a steal glides rather than cuts.",
  "digest": "A rack of sixteen wind chimes \u2014 tap.garden~'s voice",
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
     "text": "tap.chime~",
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
     "text": "A rack of sixteen wind chimes \u2014 tap.garden~'s voice",
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
     "text": "Four decaying mode doublets per strike at the transverse ratios of the chosen material. Every pitch is its own tube: same flaws, same seat in the stereo field, every time. Voice stealing takes the quietest bell and re-aims it, so a steal glides rather than cuts."
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
     "text": "strike a tube \u2014 pitch, velocity, brightness:"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "message",
     "patching_rect": [
      20.0,
      195.0,
      130.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "note 60 0.9 1.",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "message",
     "patching_rect": [
      20.0,
      222.0,
      130.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "note 67 0.6 0.7",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "message",
     "patching_rect": [
      20.0,
      249.0,
      130.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "note 74 0.4 0.4",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "comment",
     "patching_rect": [
      160.0,
      195.0,
      320.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "fractional pitches are distinct tubes,"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "patching_rect": [
      160.0,
      215.0,
      320.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "with their own scatter and their own seat"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      290.0,
      300.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "or drive it from anything that makes notes:"
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      315.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "metro 700",
     "outlettype": [
      "bang"
     ]
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      345.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "random 24",
     "outlettype": [
      "int"
     ]
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      375.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "+ 55",
     "outlettype": [
      "int"
     ]
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      405.0,
      160.0,
      22.0
     ],
     "numinlets": 4,
     "numoutlets": 1,
     "text": "pack note 60 0.7 0.8",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "toggle",
     "patching_rect": [
      130.0,
      315.0,
      22.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ]
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "comment",
     "patching_rect": [
      350.0,
      290.0,
      340.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "what it is made of \u2014 the tuned bar is the other material:"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "message",
     "patching_rect": [
      350.0,
      315.0,
      100.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "material 0",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "message",
     "patching_rect": [
      350.0,
      342.0,
      100.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "material 1",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "message",
     "patching_rect": [
      350.0,
      369.0,
      100.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "spread 0.",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "message",
     "patching_rect": [
      350.0,
      396.0,
      100.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "spread 1.",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "message",
     "patching_rect": [
      350.0,
      430.0,
      70.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "voices",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "newobj",
     "patching_rect": [
      430.0,
      430.0,
      150.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "print tap.chime~",
     "outlettype": []
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      460.0,
      330.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 3,
     "text": "tap.chime~ @decay 4. @spread 0.7",
     "outlettype": [
      "signal",
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
      20.0,
      500.0,
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
     "maxclass": "gain~",
     "patching_rect": [
      60.0,
      500.0,
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
     "id": "obj-28",
     "maxclass": "ezdac~",
     "patching_rect": [
      20.0,
      580.0,
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
     "id": "obj-29",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      650.0,
      900.0,
      50.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: hit the three note messages fast enough to run past sixteen ringing bells and listen to the steal \u2014 it re-aims a chime rather than cutting one off. Then set spread 0 and hear the rack collapse to the centre."
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
      "obj-25",
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
      "obj-25",
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
      "obj-25",
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
      "obj-25",
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
      "obj-25",
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
      "obj-25",
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
      "obj-25",
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
      "obj-17",
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
      "obj-14",
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
      "obj-16",
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
      "obj-25",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-25",
      2
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
      "obj-25",
      1
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
      "obj-28",
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
      "obj-28",
      1
     ]
    }
   }
  ]
 }
}
