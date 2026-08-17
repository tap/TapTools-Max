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
  "description": "An electromagnet driving twelve metal strings on a soundboard. Whichever strings share partials with the input ring in sympathy and the rest stay quiet. Run a guitar through it.",
  "digest": "The Ondes Martenot's string diffuseur, as a driven resonator",
  "tags": "TapTools",
  "style": "",
  "subpatcher_template": "",
  "assistshowspatchername": 0,
  "boxes": [
   {
    "box": {
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
     "bgfillcolor_angle": 270.0,
     "id": "obj-1"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      30,
      20,
      400,
      36
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "tap.palme~",
     "fontsize": 24.0,
     "fontname": "Lato Light",
     "textcolor": [
      1.0,
      1.0,
      1.0,
      1.0
     ],
     "id": "obj-2"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      30,
      55,
      620,
      22
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "The Ondes Martenot's string diffuseur, as a driven resonator",
     "fontsize": 13.0,
     "fontname": "Lato Light",
     "textcolor": [
      0.92,
      0.92,
      0.95,
      1.0
     ],
     "id": "obj-3"
    }
   },
   {
    "box": {
     "maxclass": "bpatcher",
     "patching_rect": [
      680,
      20,
      225.0,
      67.0
     ],
     "numinlets": 0,
     "numoutlets": 0,
     "name": "tap.badge.maxpat",
     "id": "obj-4"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      20,
      120,
      920,
      58
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Twelve metal strings on a lyre-shaped soundboard, driven by an electromagnet. Anything sent through comes back with a halo, because the strings that share partials with the signal ring and the rest stay quiet. Twelve, per the peer-reviewed source \u2014 the widely copied figure of twenty-four is not followed. Their tuning is not published anywhere found, so it is a parameter rather than a reconstruction: chromatic answers every key, harmonic answers one.",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-5"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      20,
      190,
      340,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "play it something pitched:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-6"
    }
   },
   {
    "box": {
     "maxclass": "message",
     "patching_rect": [
      20,
      215,
      200,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "0, 1 5 0 400",
     "id": "obj-7"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      245,
      200,
      22
     ],
     "numinlets": 2,
     "numoutlets": 2,
     "text": "line~",
     "outlettype": [
      "signal",
      "bang"
     ],
     "id": "obj-8"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      275,
      200,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "cycle~ 130.81",
     "outlettype": [
      "signal"
     ],
     "id": "obj-9"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      305,
      200,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "*~",
     "outlettype": [
      "signal"
     ],
     "id": "obj-10"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      230,
      218,
      300,
      34
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "\u2190 a short note on D3, which IS one of the strings at root 110",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-11"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      350,
      420,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "tap.palme~ @root 110 @tuning 0 @decay 5 @mix 55 @level 0.5",
     "outlettype": [
      "signal"
     ],
     "id": "obj-12"
    }
   },
   {
    "box": {
     "maxclass": "gain~",
     "patching_rect": [
      20,
      400,
      30,
      60
     ],
     "numinlets": 2,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      ""
     ],
     "parameter_enable": 0,
     "id": "obj-13"
    }
   },
   {
    "box": {
     "maxclass": "ezdac~",
     "patching_rect": [
      20,
      480,
      45,
      45
     ],
     "numinlets": 2,
     "numoutlets": 0,
     "outlettype": [],
     "id": "obj-14"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      470,
      190,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the board:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-15"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      470,
      215,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "root",
     "id": "obj-16"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      470,
      240,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "tuning",
     "id": "obj-17"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      470,
      265,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "decay",
     "id": "obj-18"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      470,
      290,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "damping",
     "id": "obj-19"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      470,
      315,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "detune",
     "id": "obj-20"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      470,
      345,
      240,
      48
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "decay and damping are not independent \u2014 a heavily damped string cannot ring for the time you ask, and past that point the damping decides",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-21"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      710,
      190,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the driver, and the balance:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-22"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      710,
      215,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "drive",
     "id": "obj-23"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      710,
      240,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "asymmetry",
     "id": "obj-24"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      710,
      265,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "saturation",
     "id": "obj-25"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      710,
      290,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "mix",
     "id": "obj-26"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      710,
      315,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "level",
     "id": "obj-27"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      710,
      340,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "smooth",
     "id": "obj-28"
    }
   },
   {
    "box": {
     "maxclass": "message",
     "patching_rect": [
      710,
      375,
      100,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "clear",
     "id": "obj-29"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      710,
      400,
      240,
      34
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "damp every string without disturbing the settings",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-30"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      20,
      555,
      920,
      78
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: play the note above, then retune the cycle~ a quarter-tone sharp (135) and play it again. On a string the board rings for seconds after the note stops; a quarter-tone off it barely answers at all, and the difference grows steeply toward the top of the board because a loop's Q scales with frequency at a fixed ring time. Then switch tuning to 1 (harmonic on the root) and hear it stop answering anything but A. The real point of the object is on other material: a guitar, a voice, a snare.",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-31"
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
      "obj-10",
      0
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
      "obj-13",
      0
     ],
     "destination": [
      "obj-14",
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
      "obj-12",
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
      "obj-12",
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
      "obj-12",
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
      "obj-12",
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
      "obj-12",
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
      "obj-12",
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
      "obj-12",
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
      "obj-12",
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
      "obj-12",
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
      "obj-12",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-28",
      0
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
      "obj-29",
      0
     ],
     "destination": [
      "obj-12",
      0
     ]
    }
   }
  ]
 }
}