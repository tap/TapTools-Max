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
   740.0
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
  "description": "The Ondes Martenot intensity key as a gain law: the published measurement, 50 dB across 4.5 mm of key travel, interpolated through all seven measured points. The bottom of the throw is silent because the key is still bending.",
  "digest": "The Ondes Martenot intensity key as a gain law",
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
     "text": "tap.touche~",
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
     "text": "The Ondes Martenot intensity key as a gain law",
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
      58
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Not a model \u2014 the published measurement (Quartier et al., Acta Acustica 2015, instrument No. 320): 50 dB of range across 4.5 mm of key travel, interpolated through all seven measured points. Roughly the first 45% of the throw is silent, which is the key bending before it reaches the powder bag; the useful 50 dB is packed into the 4.5 mm right after. That is what lets the instrument be played with very sharp attacks from a slow-looking gesture.",
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
      190,
      340,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "a steady tone, so only the key is moving:",
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
      215,
      140,
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
     "maxclass": "newobj",
     "patching_rect": [
      20,
      320,
      290,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "text": "tap.touche~ @smooth 5"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "gain~",
     "patching_rect": [
      20,
      375,
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
     "id": "obj-10",
     "maxclass": "ezdac~",
     "patching_rect": [
      20,
      455,
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
     "id": "obj-11",
     "maxclass": "comment",
     "patching_rect": [
      400,
      190,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "ride the key here:",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "slider",
     "patching_rect": [
      400,
      215,
      22,
      220
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "size": 1001.0,
     "orientation": 1
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "newobj",
     "patching_rect": [
      440,
      215,
      150,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "text": "sig~ 0."
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "newobj",
     "patching_rect": [
      440,
      250,
      180,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "text": "scale 0 1000 0. 1."
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "comment",
     "patching_rect": [
      440,
      285,
      240,
      34
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the slider drives the right inlet as a signal \u2014 that is the performance surface",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "comment",
     "patching_rect": [
      690,
      190,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "or set it as an attribute:",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "attrui",
     "patching_rect": [
      690,
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
     "attr": "position"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "attrui",
     "patching_rect": [
      690,
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
     "attr": "mm"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "attrui",
     "patching_rect": [
      690,
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
     "attr": "mode"
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "attrui",
     "patching_rect": [
      690,
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
     "attr": "smooth"
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "comment",
     "patching_rect": [
      690,
      335,
      250,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the published points:",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "message",
     "patching_rect": [
      690,
      360,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "mm 8.8"
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "message",
     "patching_rect": [
      690,
      385,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "mm 7.3"
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "message",
     "patching_rect": [
      690,
      410,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "mm 6.4"
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "message",
     "patching_rect": [
      690,
      435,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "mm 4.3"
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "comment",
     "patching_rect": [
      690,
      462,
      250,
      34
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "full press, then f, mf, and the noise floor \u2014 the nuance boundaries the paper measured",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-27",
     "maxclass": "comment",
     "patching_rect": [
      20,
      530,
      920,
      64
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: sweep the slider slowly from the bottom. Nothing happens for the first stretch \u2014 do not adjust your set, that dead travel is the instrument. Then the whole 50 dB arrives in a short distance, steepening through the middle and flattening at the top. Compare against a plain gain~ fader on the same tone: the difference is where in the gesture the loudness actually changes, which is the whole reason the key is famous. It works on anything, not just an ondes \u2014 try it on a guitar as a swell pedal.",
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
      "obj-12",
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
      "obj-8",
      1
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
      "obj-8",
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
      "obj-8",
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
      "obj-8",
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
      "obj-8",
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
      "obj-8",
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
      "obj-8",
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
      "obj-9",
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
   }
  ]
 }
}
