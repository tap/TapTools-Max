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
  "description": "A multi-head tape echo: one record head, a span of moving tape, and up to four playback heads along it. span is the motor and moves every head together, bending pitch as it goes. Past 1.0 the regeneration self-oscillates, bounded by the drive saturator rather than by a feedback cap.",
  "digest": "A multi-head tape echo",
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
     "text": "tap.tapecho~",
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
     "text": "A multi-head tape echo",
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
      920,
      44
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "One record head, a span of moving tape, and up to four playback heads along it. span is the motor \u2014 the delay of the head at the far end \u2014 so moving it moves every head together and bends pitch on the way, which is what a tape speed change is. Past 1.0, regen self-oscillates: the drive saturator is what bounds it, not a feedback cap.",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "comment",
     "patching_rect": [
      20,
      175,
      330,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "a source (clicks show the head layout plainly):",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      200,
      120,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "text": "train~ 2000"
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "gain~",
     "patching_rect": [
      20,
      230,
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
     "maxclass": "comment",
     "patching_rect": [
      60,
      245,
      300,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the send fader \u2014 or ride the input attribute",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      330,
      370,
      22
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      "signal"
     ],
     "text": "tap.tapecho~ 4. @span 400 @regen 0.5 @mix 50"
    }
   },
   {
    "box": {
     "id": "obj-11",
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
     "id": "obj-12",
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
     "id": "obj-13",
     "maxclass": "comment",
     "patching_rect": [
      100,
      395,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "left / right \u2014 the heads are panned",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-14",
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
     "id": "obj-15",
     "maxclass": "comment",
     "patching_rect": [
      420,
      175,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the machine:",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "attrui",
     "patching_rect": [
      420,
      200,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "span"
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "attrui",
     "patching_rect": [
      420,
      225,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "heads"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "attrui",
     "patching_rect": [
      420,
      250,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "regen"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "attrui",
     "patching_rect": [
      420,
      275,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "mix"
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "comment",
     "patching_rect": [
      420,
      320,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the head layout (ratios of span):",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "message",
     "patching_rect": [
      420,
      345,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "ratios 0.25 0.5 0.75 1., heads 4"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "message",
     "patching_rect": [
      420,
      370,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "ratios 0.333 0.667 1., heads 3"
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "message",
     "patching_rect": [
      420,
      395,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "pans -0.7 0.5 -0.35 0.8"
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "message",
     "patching_rect": [
      420,
      420,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "pans 0. 0. 0. 0."
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "comment",
     "patching_rect": [
      690,
      175,
      250,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the wear (the stabilizer):",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "attrui",
     "patching_rect": [
      690,
      200,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "darken"
    }
   },
   {
    "box": {
     "id": "obj-27",
     "maxclass": "attrui",
     "patching_rect": [
      690,
      225,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "drive"
    }
   },
   {
    "box": {
     "id": "obj-28",
     "maxclass": "comment",
     "patching_rect": [
      690,
      255,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the transport (depth ms, rate Hz):",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-29",
     "maxclass": "message",
     "patching_rect": [
      690,
      280,
      220,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "wow 1. 0.9, flutter 0.06 13."
    }
   },
   {
    "box": {
     "id": "obj-30",
     "maxclass": "message",
     "patching_rect": [
      690,
      305,
      220,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "wow 0. 0., flutter 0. 0."
    }
   },
   {
    "box": {
     "id": "obj-31",
     "maxclass": "comment",
     "patching_rect": [
      690,
      345,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "past unity \u2014 sound-on-sound:",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-32",
     "maxclass": "message",
     "patching_rect": [
      690,
      370,
      220,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "drive 0.7, regen 1.35"
    }
   },
   {
    "box": {
     "id": "obj-33",
     "maxclass": "message",
     "patching_rect": [
      690,
      395,
      220,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "input 0."
    }
   },
   {
    "box": {
     "id": "obj-34",
     "maxclass": "message",
     "patching_rect": [
      690,
      420,
      220,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "regen 0.5, input 1."
    }
   },
   {
    "box": {
     "id": "obj-35",
     "maxclass": "message",
     "patching_rect": [
      690,
      445,
      220,
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
     "id": "obj-36",
     "maxclass": "comment",
     "patching_rect": [
      20,
      525,
      920,
      60
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: start the clicks and set the head layout \u2014 four evenly spaced heads, then the Copicat-style three. Then push past unity (drive 0.7, regen 1.35) and pull your hands off with input 0.: the loop keeps building on what you already gave it, bounded by the saturator rather than by a gain cap. Ride darken while it howls, then bring it home \u2014 or hit clear, the eject button. Drop drive to 0 mid-howl and the effective regen falls back to 1.0: the attribute keeps its value and returns when drive does.",
     "fontsize": 12.0,
     "fontname": "Arial"
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-10",
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
      "obj-11",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-10",
      1
     ],
     "destination": [
      "obj-12",
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
      "obj-14",
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
      "obj-14",
      1
     ]
    }
   }
  ]
 }
}
