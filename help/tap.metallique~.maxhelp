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
  "description": "The Ondes Martenot's motor-driven gong, as a bank of plate modes excited continuously by whatever you send in. The transducer sits upstream of the body, which is the order the instrument wires them.",
  "digest": "The Ondes Martenot's gong diffuseur, as a driven resonator",
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
     "text": "tap.metallique~",
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
     "text": "The Ondes Martenot's gong diffuseur, as a driven resonator",
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
     "text": "Driven, not struck: there is no trigger here. Whatever goes in excites the plate continuously and the plate rings at its own rates \u2014 eight modes at the free circular plate's transverse ratios, each split into a slowly beating doublet. The moving-iron transducer (drive / asymmetry / saturation) is UPSTREAM of the body, because that is where the instrument puts it: driving a distorted waveform into a gong is a different sound from distorting a gong.",
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
      400,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "an impulse, the quickest way to hear the modes:",
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
      60,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "bang",
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
     "numinlets": 1,
     "numoutlets": 1,
     "text": "click~",
     "outlettype": [
      "signal"
     ],
     "id": "obj-8"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      230,
      240,
      320,
      34
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "\u2190 click it. But the object is DRIVEN: put something sustained in and it keeps ringing while the input does",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-9"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      330,
      420,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "tap.metallique~ @pitch 146 @decay 7 @mix 70 @drive 1.6 @asymmetry 0.4",
     "outlettype": [
      "signal"
     ],
     "id": "obj-10"
    }
   },
   {
    "box": {
     "maxclass": "gain~",
     "patching_rect": [
      20,
      390,
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
     "id": "obj-11"
    }
   },
   {
    "box": {
     "maxclass": "ezdac~",
     "patching_rect": [
      20,
      470,
      45,
      45
     ],
     "numinlets": 2,
     "numoutlets": 0,
     "outlettype": [],
     "id": "obj-12"
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
     "text": "the body:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-13"
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
     "attr": "pitch",
     "id": "obj-14"
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
     "attr": "decay",
     "id": "obj-15"
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
     "attr": "tilt",
     "id": "obj-16"
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
     "attr": "brightness",
     "id": "obj-17"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      470,
      320,
      240,
      34
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "pitch retunes sixteen resonators, so it is set-and-hold rather than a knob to sweep",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-18"
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
     "id": "obj-19"
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
     "id": "obj-20"
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
     "id": "obj-21"
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
     "id": "obj-22"
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
     "id": "obj-23"
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
     "id": "obj-24"
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
     "id": "obj-25"
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
     "id": "obj-26"
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
     "text": "silence the body without disturbing any of the settings",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-27"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      20,
      545,
      920,
      78
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: set mix to 100 and asymmetry to 0 \u2014 that is a linear resonator, and it is missing a documented stage of the instrument. Bring asymmetry up and the driver starts adding a second harmonic in proportion to level, which is the moving-iron principle (force follows the square of the gap flux). Push drive hard and the saturator holds it bounded. Then put something that is not an ondes through it: a snare, a whole mix at a low mix setting. The mode ratios are Fletcher & Rossing's general plate physics rather than a measurement of Martenot's gong, so this is a recreation \u2014 stated here, in the reference page, and in the kernel header.",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-28"
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
      "obj-11",
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
      "obj-11",
      0
     ],
     "destination": [
      "obj-12",
      1
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
      "obj-15",
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
      "obj-25",
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
   }
  ]
 }
}