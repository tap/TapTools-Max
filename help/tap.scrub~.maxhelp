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
  "description": "Record continuously and rake a granular playhead through it. Position and pitch are two independent signals \u2014 the two axes of a pad \u2014 and freeze stops the recorder so the position addresses fixed tape.",
  "digest": "A granular scrub pad over live capture",
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
     "text": "tap.scrub~",
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
     "text": "A granular scrub pad over live capture",
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
     "text": "The input is recorded continuously and a granular playhead reads it. POSITION (the middle inlet) is how far behind the live edge the head sits; PITCH (the right inlet) transposes without the position moving. Those two hands are the object \u2014 a tape head cannot do it, because on tape moving the playhead IS the pitch change. Held still at pitch 0 with no spray, this is exactly a delay.",
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
     "text": "something with transients to scrub:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-6"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      215,
      240,
      22
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "text": "adc~",
     "outlettype": [
      "signal",
      "signal"
     ],
     "id": "obj-7"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      270,
      218,
      280,
      34
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "\u2190 or swap in a play~ / a drum loop; a sustained pad has nothing to bite on",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-8"
    }
   },
   {
    "box": {
     "maxclass": "toggle",
     "patching_rect": [
      20,
      250,
      24,
      24
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ],
     "parameter_enable": 0,
     "id": "obj-9"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      50,
      253,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "audio on",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-10"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      390,
      480,
      22
     ],
     "numinlets": 3,
     "numoutlets": 1,
     "text": "tap.scrub~ 4. @size 70 @overlap 2 @mix 100 @smooth 8",
     "outlettype": [
      "signal"
     ],
     "id": "obj-11"
    }
   },
   {
    "box": {
     "maxclass": "gain~",
     "patching_rect": [
      20,
      440,
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
     "id": "obj-12"
    }
   },
   {
    "box": {
     "maxclass": "ezdac~",
     "patching_rect": [
      20,
      520,
      45,
      45
     ],
     "numinlets": 2,
     "numoutlets": 0,
     "outlettype": [],
     "id": "obj-13"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      300,
      190,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "position \u2014 ms behind the live edge:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-14"
    }
   },
   {
    "box": {
     "maxclass": "slider",
     "patching_rect": [
      300,
      215,
      22,
      150
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "size": 1001.0,
     "orientation": 1,
     "id": "obj-15"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      340,
      215,
      150,
      22
     ],
     "numinlets": 6,
     "numoutlets": 1,
     "text": "scale 0 1000 0. 2000.",
     "outlettype": [
      ""
     ],
     "id": "obj-16"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      340,
      245,
      150,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "sig~ 0.",
     "outlettype": [
      "signal"
     ],
     "id": "obj-17"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      340,
      275,
      200,
      48
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "drag it. the head rakes through the last two seconds; the grains hide the seams",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-18"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      560,
      190,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "pitch \u2014 semitones, independently:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-19"
    }
   },
   {
    "box": {
     "maxclass": "slider",
     "patching_rect": [
      560,
      215,
      22,
      150
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "size": 1001.0,
     "orientation": 1,
     "id": "obj-20"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      600,
      215,
      150,
      22
     ],
     "numinlets": 6,
     "numoutlets": 1,
     "text": "scale 0 1000 -12. 12.",
     "outlettype": [
      ""
     ],
     "id": "obj-21"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      600,
      245,
      150,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "sig~ 0.",
     "outlettype": [
      "signal"
     ],
     "id": "obj-22"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      600,
      275,
      220,
      48
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "a granular texture, not a hi-fi shift \u2014 use tap.pitchaccum~ when you want the transposition clean",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-23"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      560,
      330,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "and the rest:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-24"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      560,
      355,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "freeze",
     "id": "obj-25"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      560,
      380,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "drift",
     "id": "obj-26"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      560,
      405,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "size",
     "id": "obj-27"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      560,
      430,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "overlap",
     "id": "obj-28"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      560,
      455,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "spray",
     "id": "obj-29"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      560,
      480,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "seed",
     "id": "obj-30"
    }
   },
   {
    "box": {
     "maxclass": "message",
     "patching_rect": [
      560,
      510,
      100,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "clear",
     "id": "obj-31"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      670,
      513,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "erase the tape and rewind the seed",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-32"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      20,
      595,
      920,
      92
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: with audio running, drag the position slider slowly and then quickly. Then turn FREEZE on \u2014 the recorder stops, but the playhead does not, so everything you hear from that moment is made out of the tape you already have. Scrub it, transpose it, or set drift to a small value and let the head crawl through on its own. Bring spray up and the grain origins scatter, which turns the transposition's comb into a broadband smear. With spray at 0 the dice are never rolled at all, so the seed provably cannot matter and two instances stay identical.",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-33"
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "obj-9",
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
      "obj-11",
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
      "obj-11",
      1
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
      "obj-21",
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
      "obj-22",
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
      "obj-11",
      2
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
      "obj-12",
      0
     ],
     "destination": [
      "obj-13",
      1
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
      "obj-11",
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
      "obj-11",
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
      "obj-11",
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
      "obj-11",
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
      "obj-11",
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
      "obj-11",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-31",
      0
     ],
     "destination": [
      "obj-11",
      0
     ]
    }
   }
  ]
 }
}