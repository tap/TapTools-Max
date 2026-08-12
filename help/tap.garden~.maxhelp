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
  "description": "note messages plant notes that snap to the scale, bloom on a soft FM bell, and return every loop pass a step quieter (decay) and purer (soften) until they retire. Left idle, a seeded gardener plants about one scale note per pass \u2014 same seed, same garden, bit for bit.",
  "digest": "A generative event loop \u2014 the garden that plays itself",
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
     "text": "tap.garden~",
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
     "text": "A generative event loop \u2014 the garden that plays itself",
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
     "text": "note messages plant notes that snap to the scale, bloom on a soft FM bell, and return every loop pass a step quieter (decay) and purer (soften) until they retire. Left idle, a seeded gardener plants about one scale note per pass \u2014 same seed, same garden, bit for bit.",
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
      200,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "plant notes (pitch, velocity):"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "message",
     "patching_rect": [
      20,
      195,
      90,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "note 69 0.7"
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "message",
     "patching_rect": [
      116,
      195,
      90,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "note 76 0.5"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "message",
     "patching_rect": [
      212,
      195,
      96,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "note 63.5 0.9"
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "comment",
     "patching_rect": [
      314,
      195,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "<- off-scale plants snap: wrong notes are unrepresentable"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "patching_rect": [
      20,
      225,
      200,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "uproot everything:"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "message",
     "patching_rect": [
      20,
      248,
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
     "id": "obj-13",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      330,
      400,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "text": "tap.garden~ @loop 6. @idle 4. @seed 2008 @level 0.4"
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "comment",
     "patching_rect": [
      450,
      170,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "tend the garden:"
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "attrui",
     "patching_rect": [
      450,
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
     "attr": "decay"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "attrui",
     "patching_rect": [
      450,
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
     "attr": "soften"
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "attrui",
     "patching_rect": [
      450,
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
     "attr": "idle"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "attrui",
     "patching_rect": [
      450,
      270,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "scale"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "comment",
     "patching_rect": [
      690,
      170,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the bell (attack s, decay s, brightness):"
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "message",
     "patching_rect": [
      690,
      195,
      130,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "bell 0.12 3. 0.9"
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "message",
     "patching_rect": [
      690,
      222,
      130,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "bell 0.005 0.8 1."
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "comment",
     "patching_rect": [
      826,
      222,
      130,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "<- the music box"
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
     "id": "obj-25",
     "maxclass": "comment",
     "patching_rect": [
      20,
      520,
      900,
      50
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: plant three or four notes, then stop. After idle seconds the gardener takes over \u2014 always on the scale, about one note per pass, bounded forever. Same seed next year, same garden. For a wider garden, run several instances on different seeds."
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
      "obj-13",
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
      "obj-13",
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
      "obj-13",
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
      "obj-13",
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
      "obj-13",
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
      "obj-13",
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
      "obj-13",
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
      "obj-13",
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
      "obj-18",
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
