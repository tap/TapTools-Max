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
  "description": "Input is recorded onto tape, spools loop seconds to the playback machine, and returns through a wear path (darken, drive, DC blocker) into the record head \u2014 so regen legally reaches 1.0 and sustains, bounded, because every pass is degraded. Play a phrase, then fade the input send: the piece continues without you.",
  "digest": "The Discreet Music two-machine tape loop",
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
     "text": "tap.discreet~",
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
     "text": "The Discreet Music two-machine tape loop",
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
     "text": "Input is recorded onto tape, spools loop seconds to the playback machine, and returns through a wear path (darken, drive, DC blocker) into the record head \u2014 so regen legally reaches 1.0 and sustains, bounded, because every pass is degraded. Play a phrase, then fade the input send: the piece continues without you.",
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
      300,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "a phrase to feed the machine:"
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
     "text": "cycle~ 220"
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
     "maxclass": "comment",
     "patching_rect": [
      60,
      240,
      250,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "ride this like the send fader (or use @input)"
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      320,
      330,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "text": "tap.discreet~ 12. @loop 5. @regen 0.95 @mix 60"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "patching_rect": [
      380,
      170,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the machine:"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "attrui",
     "patching_rect": [
      380,
      195,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "loop"
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "attrui",
     "patching_rect": [
      380,
      220,
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
     "id": "obj-14",
     "maxclass": "attrui",
     "patching_rect": [
      380,
      245,
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
     "id": "obj-15",
     "maxclass": "comment",
     "patching_rect": [
      620,
      170,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the wear (the stabilizer):"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "attrui",
     "patching_rect": [
      620,
      195,
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
     "id": "obj-17",
     "maxclass": "attrui",
     "patching_rect": [
      620,
      220,
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
     "id": "obj-18",
     "maxclass": "comment",
     "patching_rect": [
      620,
      250,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the transport (depth ms, rate Hz):"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "message",
     "patching_rect": [
      620,
      275,
      120,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "wow 2. 0.5"
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "message",
     "patching_rect": [
      750,
      275,
      120,
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
     "id": "obj-21",
     "maxclass": "comment",
     "patching_rect": [
      380,
      275,
      220,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the eject button:"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "message",
     "patching_rect": [
      380,
      300,
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
      370,
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
     "maxclass": "ezdac~",
     "patching_rect": [
      20,
      450,
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
     "id": "obj-25",
     "maxclass": "comment",
     "patching_rect": [
      20,
      510,
      900,
      50
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: start the phrase, let the loop fill, then pull the input gain to zero \u2014 regen 1.0 sustains the wash forever, worn a shade darker every pass. Loop changes glide as tape-speed doppler (they bend pitch, by design; smooth sets how fast the transport re-spools)."
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
      "obj-12",
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
      "obj-13",
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
      "obj-14",
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
      "obj-20",
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
      "obj-10",
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
      0
     ],
     "destination": [
      "obj-24",
      1
     ]
    }
   }
  ]
 }
}
