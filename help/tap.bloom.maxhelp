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
  "description": "Every pass a bloom returns a step quieter and a step purer, until it drops below the floor and retires \u2014 so the population converges however fast you plant. It emits notes, not sound, so it will drive tap.chime~, makenote, poly~, or MIDI out just as happily.",
  "digest": "The generative event loop \u2014 plant a note and it comes back",
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
     "text": "tap.bloom",
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
     "text": "The generative event loop \u2014 plant a note and it comes back",
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
     "text": "Every pass a bloom returns a step quieter and a step purer, until it drops below the floor and retires \u2014 so the population converges however fast you plant. It emits notes, not sound, so it will drive tap.chime~, makenote, poly~, or MIDI out just as happily."
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      170.0,
      320.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "plant a note \u2014 pitch and velocity:"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "message",
     "patching_rect": [
      20.0,
      195.0,
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
     "id": "obj-8",
     "maxclass": "message",
     "patching_rect": [
      20.0,
      222.0,
      110.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "plant 67 0.8",
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
      110.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "plant 72 0.7",
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
      145.0,
      195.0,
      340.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "each one returns at its own place on the loop,"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "patching_rect": [
      145.0,
      215.0,
      340.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "every pass, until it fades below the floor"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      290.0,
      340.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the shape of the fading:"
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "message",
     "patching_rect": [
      20.0,
      315.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "decay 0.85",
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
      115.0,
      315.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "decay 0.5",
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
      20.0,
      342.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "soften 0.9",
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
      115.0,
      342.0,
      90.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "loop 4.",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "message",
     "patching_rect": [
      20.0,
      372.0,
      70.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "events",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "message",
     "patching_rect": [
      95.0,
      372.0,
      60.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "clear",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      410.0,
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
      340.0,
      440.0,
      150.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "print tap.bloom",
     "outlettype": []
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      445.0,
      340.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the notes come out here \u2014 send them anywhere:"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      470.0,
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
     "id": "obj-23",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      505.0,
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
     "id": "obj-24",
     "maxclass": "gain~",
     "patching_rect": [
      20.0,
      545.0,
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
     "id": "obj-25",
     "maxclass": "gain~",
     "patching_rect": [
      60.0,
      545.0,
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
     "maxclass": "ezdac~",
     "patching_rect": [
      20.0,
      625.0,
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
     "id": "obj-27",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      690.0,
      900.0,
      50.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: plant all three, then set decay 0.5 and watch the texture thin out fast; set decay 0.85 and it lingers. The arithmetic is exact \u2014 a plant at velocity v under decay d gets ceil(log(floor/v)/log(d)) strikes and not one more."
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
      "obj-19",
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
      "obj-19",
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
      "obj-19",
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
      "obj-19",
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
      "obj-19",
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
      "obj-19",
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
      "obj-19",
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
      "obj-19",
      0
     ],
     "destination": [
      "obj-22",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-19",
      1
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
      "obj-22",
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
      "obj-23",
      1
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
      "obj-26",
      1
     ]
    }
   }
  ]
 }
}
