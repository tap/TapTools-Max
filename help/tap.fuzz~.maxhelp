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
  "description": "A two-stage tone-stacked fuzz: cascaded clipping stages with a bass / contrast / treble voicing section. gain drives the first stage, edge sharpens the second stage's knee, and asymmetry brings in the even harmonics a symmetric curve cannot make.",
  "digest": "A two-stage tone-stacked fuzz",
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
     "text": "tap.fuzz~",
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
     "text": "A two-stage tone-stacked fuzz",
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
     "text": "Two cascaded clipping stages and a voicing section \u2014 the harder, more scooped school beside tap.overdrive~. gain drives the first stage, edge sharpens the second stage's knee toward a hard corner, and asymmetry brings in the even harmonics a symmetric curve cannot make. Built on the Yeh/Abel/Smith DAFx-07 cascade; a recreation of a circuit class, not a model of any one pedal.",
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
      340,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "a source (a plucky tone shows the knee best):",
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
      150,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "text": "saw~ 110"
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
     "text": "how hard you hit the front end",
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
      330,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "text": "tap.fuzz~ @gain 0.6 @contrast 0.4"
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
     "id": "obj-13",
     "maxclass": "comment",
     "patching_rect": [
      400,
      175,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the dirt:",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "attrui",
     "patching_rect": [
      400,
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
     "attr": "gain"
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "attrui",
     "patching_rect": [
      400,
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
     "attr": "edge"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "attrui",
     "patching_rect": [
      400,
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
     "attr": "asymmetry"
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "attrui",
     "patching_rect": [
      400,
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
     "attr": "level"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "comment",
     "patching_rect": [
      400,
      320,
      250,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "aliasing (2 is the measured best):",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "attrui",
     "patching_rect": [
      400,
      345,
      220,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "oversample"
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "comment",
     "patching_rect": [
      400,
      372,
      270,
      48
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "every factor beats 1 by orders of magnitude, but the sequence is not monotone here \u2014 bigger is not automatically better.",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "comment",
     "patching_rect": [
      670,
      175,
      250,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the voicing:",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "attrui",
     "patching_rect": [
      670,
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
     "attr": "bass"
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "attrui",
     "patching_rect": [
      670,
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
     "attr": "treble"
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "attrui",
     "patching_rect": [
      670,
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
     "attr": "contrast"
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "comment",
     "patching_rect": [
      670,
      295,
      260,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "a few places to start:",
     "fontsize": 12.0,
     "fontname": "Arial"
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "message",
     "patching_rect": [
      670,
      320,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "gain 0.3, edge 0.2, contrast 0., bass 0."
    }
   },
   {
    "box": {
     "id": "obj-27",
     "maxclass": "message",
     "patching_rect": [
      670,
      345,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "gain 0.8, edge 0.6, contrast 1., bass 0.4"
    }
   },
   {
    "box": {
     "id": "obj-28",
     "maxclass": "message",
     "patching_rect": [
      670,
      370,
      250,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "text": "gain 0.9, edge 1., asymmetry 0.7, oversample 8"
    }
   },
   {
    "box": {
     "id": "obj-29",
     "maxclass": "message",
     "patching_rect": [
      670,
      395,
      250,
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
     "id": "obj-30",
     "maxclass": "comment",
     "patching_rect": [
      20,
      530,
      920,
      50
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: start clean and walk gain up \u2014 the second stage is what saturates, so most of the change happens in the first half of the knob. Then bring edge up for the corner, and asymmetry for the even harmonics that make it sound lopsided rather than merely loud. contrast is the scoop this class of pedal is known for; it is a real mid cut, not a fader.",
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
      "obj-28",
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
   }
  ]
 }
}
