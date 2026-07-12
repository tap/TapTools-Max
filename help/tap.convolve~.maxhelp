{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 9,
   "minor": 0,
   "revision": 0,
   "architecture": "x64",
   "modernui": 1
  },
  "rect": [
   326.0,
   80.0,
   512.0,
   564.0
  ],
  "bglocked": 0,
  "defrect": [
   326.0,
   80.0,
   512.0,
   564.0
  ],
  "openrect": [
   0.0,
   0.0,
   0.0,
   0.0
  ],
  "openinpresentation": 0,
  "default_fontsize": 11.0,
  "default_fontface": 0,
  "default_fontname": "Verdana",
  "gridonopen": 0,
  "gridsize": [
   5.0,
   5.0
  ],
  "gridsnaponopen": 0,
  "toolbarvisible": 1,
  "boxanimatetime": 200,
  "imprint": 0,
  "enablehscroll": 1,
  "enablevscroll": 1,
  "devicewidth": 0.0,
  "boxes": [
   {
    "box": {
     "id": "obj-27",
     "maxclass": "panel",
     "background": 1,
     "mode": 1,
     "grad1": [
      0.88,
      0.98,
      0.78,
      1.0
     ],
     "grad2": [
      0.9,
      0.9,
      0.9,
      1.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      5.0,
      5.0,
      495.0,
      52.0
     ],
     "varname": "autohelp_top_panel"
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "comment",
     "fontface": 3,
     "fontname": "Arial",
     "fontsize": 20.871338,
     "frgb": [
      0.2,
      0.2,
      0.2,
      1.0
     ],
     "textcolor": [
      0.2,
      0.2,
      0.2,
      1.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      10.0,
      8.0,
      485.0,
      30.0
     ],
     "text": "tap.convolve~",
     "varname": "autohelp_top_title"
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "comment",
     "fontname": "Arial",
     "fontsize": 12.754705,
     "frgb": [
      0.2,
      0.2,
      0.2,
      1.0
     ],
     "textcolor": [
      0.2,
      0.2,
      0.2,
      1.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      10.0,
      36.0,
      485.0,
      21.0
     ],
     "text": "True-stereo convolution reverb",
     "varname": "autohelp_top_digest"
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "frgb": [
      0.658824,
      0.658824,
      0.658824,
      1.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      10.0,
      57.0,
      470.0,
      19.0
     ],
     "text": "Convolve a stereo signal with an impulse response held in a buffer~.",
     "varname": "autohelp_top_description"
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "textbutton",
     "fontface": 3,
     "fontlink": 1,
     "fontname": "Arial",
     "fontsize": 12.754705,
     "bgcolor": [
      0,
      0,
      0,
      0
     ],
     "bgoncolor": [
      0,
      0,
      0,
      0
     ],
     "bgovercolor": [
      0,
      0,
      0,
      0
     ],
     "bgoveroncolor": [
      0,
      0,
      0,
      0
     ],
     "bordercolor": [
      0,
      0,
      0,
      0
     ],
     "numinlets": 1,
     "numoutlets": 3,
     "outlettype": [
      "",
      "",
      "int"
     ],
     "patching_rect": [
      318.690674,
      22.0,
      181.309311,
      14.666666
     ],
     "presentation_rect": [
      0.0,
      0.0,
      181.309311,
      14.666666
     ],
     "spacing_x": 0.0,
     "spacing_y": 0.0,
     "text": "open tap.convolve~ reference",
     "textcolor": [
      0.283,
      0.365,
      0.488,
      1.0
     ],
     "textoncolor": [
      0.27,
      0.35,
      0.47,
      1.0
     ],
     "textovercolor": [
      0.4,
      0.5,
      0.65,
      1.0
     ],
     "underline": 1,
     "varname": "autohelp_top_ref"
    }
   },
   {
    "box": {
     "id": "obj-30",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      84.0,
      470.0,
      18.0
     ],
     "text": "1.  Load a 1-, 2-, or 4-channel impulse response (mono / stereo / true-stereo):"
    }
   },
   {
    "box": {
     "id": "obj-31",
     "maxclass": "message",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      20.0,
      106.0,
      70.0,
      17.0
     ],
     "text": "replace"
    }
   },
   {
    "box": {
     "id": "obj-32",
     "maxclass": "newobj",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "float",
      "float",
      "float",
      "bang"
     ],
     "patching_rect": [
      100.0,
      106.0,
      150.0,
      19.0
     ],
     "text": "buffer~ tap.convolve.ir"
    }
   },
   {
    "box": {
     "id": "obj-33",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 9.0,
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      258.0,
      106.0,
      232.0,
      18.0
     ],
     "text": "a reverb IR, cabinet, or any sound to convolve with",
     "frgb": [
      0.658824,
      0.658824,
      0.658824,
      1.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-34",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      150.0,
      470.0,
      18.0
     ],
     "text": "2.  Fire an impulse \u2014 the wet output is the impulse response itself (the reverb tail):"
    }
   },
   {
    "box": {
     "id": "obj-35",
     "maxclass": "button",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ],
     "patching_rect": [
      20.0,
      172.0,
      24.0,
      24.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-36",
     "maxclass": "newobj",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      60.0,
      174.0,
      45.0,
      19.0
     ],
     "text": "click~"
    }
   },
   {
    "box": {
     "id": "obj-37",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 9.0,
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      112.0,
      176.0,
      200.0,
      18.0
     ],
     "text": "(feeds both L and R inputs)",
     "frgb": [
      0.658824,
      0.658824,
      0.658824,
      1.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-40",
     "maxclass": "newobj",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 2,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      "signal"
     ],
     "patching_rect": [
      20.0,
      262.0,
      180.0,
      19.0
     ],
     "text": "tap.convolve~ tap.convolve.ir",
     "color": [
      1.0,
      0.890196,
      0.090196,
      1.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-41",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 9.0,
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      300.0,
      96.0,
      120.0,
      18.0
     ],
     "text": "mix (0\u2013100 %)"
    }
   },
   {
    "box": {
     "id": "obj-50",
     "maxclass": "flonum",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "htextcolor": [
      0.870588,
      0.870588,
      0.870588,
      1.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "float",
      "bang"
     ],
     "patching_rect": [
      300.0,
      112.0,
      40.0,
      19.0
     ],
     "triscale": 0.9
    }
   },
   {
    "box": {
     "id": "obj-51",
     "maxclass": "message",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      345.0,
      112.0,
      55.0,
      17.0
     ],
     "text": "mix $1"
    }
   },
   {
    "box": {
     "id": "obj-42",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 9.0,
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      300.0,
      140.0,
      120.0,
      18.0
     ],
     "text": "gain (dB)"
    }
   },
   {
    "box": {
     "id": "obj-52b",
     "maxclass": "flonum",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "htextcolor": [
      0.870588,
      0.870588,
      0.870588,
      1.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "float",
      "bang"
     ],
     "patching_rect": [
      300.0,
      156.0,
      40.0,
      19.0
     ],
     "triscale": 0.9
    }
   },
   {
    "box": {
     "id": "obj-53b",
     "maxclass": "message",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      345.0,
      156.0,
      60.0,
      17.0
     ],
     "text": "gain $1"
    }
   },
   {
    "box": {
     "id": "obj-43",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 9.0,
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      300.0,
      184.0,
      120.0,
      18.0
     ],
     "text": "predelay (ms)"
    }
   },
   {
    "box": {
     "id": "obj-54b",
     "maxclass": "flonum",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "htextcolor": [
      0.870588,
      0.870588,
      0.870588,
      1.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "float",
      "bang"
     ],
     "patching_rect": [
      300.0,
      200.0,
      40.0,
      19.0
     ],
     "triscale": 0.9
    }
   },
   {
    "box": {
     "id": "obj-55b",
     "maxclass": "message",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      345.0,
      200.0,
      80.0,
      17.0
     ],
     "text": "predelay $1"
    }
   },
   {
    "box": {
     "id": "obj-44",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 9.0,
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      300.0,
      228.0,
      120.0,
      18.0
     ],
     "text": "normalize"
    }
   },
   {
    "box": {
     "id": "obj-56b",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ],
     "patching_rect": [
      300.0,
      244.0,
      18.0,
      18.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-57b",
     "maxclass": "message",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      324.0,
      244.0,
      90.0,
      17.0
     ],
     "text": "normalize $1"
    }
   },
   {
    "box": {
     "id": "obj-45",
     "maxclass": "message",
     "fontname": "Verdana",
     "fontsize": 10.0,
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      324.0,
      270.0,
      55.0,
      17.0
     ],
     "text": "clear"
    }
   },
   {
    "box": {
     "id": "obj-46",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 9.0,
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      382.0,
      272.0,
      110.0,
      18.0
     ],
     "text": "flush the tail"
    }
   },
   {
    "box": {
     "id": "obj-70",
     "maxclass": "scope~",
     "bgcolor": [
      0.529412,
      0.529412,
      0.529412,
      1.0
     ],
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      210.0,
      300.0,
      190.0,
      90.0
     ],
     "rounded": 0
    }
   },
   {
    "box": {
     "id": "obj-71",
     "maxclass": "comment",
     "fontname": "Verdana",
     "fontsize": 9.0,
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      300.0,
      180.0,
      30.0
     ],
     "text": "Route the wet+dry stereo output to your speakers:",
     "linecount": 2
    }
   },
   {
    "box": {
     "args": [],
     "id": "obj-22",
     "maxclass": "bpatcher",
     "name": "tap.help.dac~.maxpat",
     "numinlets": 2,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      "signal"
     ],
     "patching_rect": [
      20.0,
      340.0,
      150.0,
      86.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-14",
     "local": 1,
     "maxclass": "ezdac~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      436.0,
      45.0,
      45.0
     ]
    }
   },
   {
    "box": {
     "args": [],
     "id": "obj-69",
     "maxclass": "bpatcher",
     "name": "tap.badge.maxpat",
     "numinlets": 0,
     "numoutlets": 0,
     "patching_rect": [
      229.0,
      482.0,
      225.0,
      67.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-68",
     "maxclass": "panel",
     "background": 1,
     "bgcolor": [
      0.85,
      0.85,
      0.85,
      0.75
     ],
     "border": 2,
     "bordercolor": [
      0.5,
      0.5,
      0.5,
      0.75
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      21.0,
      490.0,
      175.0,
      50.0
     ],
     "varname": "autohelp_see_panel"
    }
   },
   {
    "box": {
     "id": "obj-66",
     "maxclass": "comment",
     "fontface": 1,
     "fontname": "Arial",
     "fontsize": 11.595187,
     "frgb": [
      0,
      0,
      0,
      1.0
     ],
     "textcolor": [
      0,
      0,
      0,
      1.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      26.0,
      494.0,
      120.0,
      20.0
     ],
     "text": "See Also:",
     "varname": "autohelp_see_title"
    }
   },
   {
    "box": {
     "id": "obj-67",
     "maxclass": "umenu",
     "fontname": "Arial",
     "fontsize": 11.595187,
     "items": [
      "(Objects:)",
      ",",
      "tap.verb~",
      ",",
      "buffer~",
      ",",
      "pfft~",
      ",",
      "<separator>"
     ],
     "numinlets": 1,
     "numoutlets": 3,
     "outlettype": [
      "int",
      "",
      ""
     ],
     "patching_rect": [
      26.0,
      514.0,
      160.0,
      20.0
     ],
     "types": [],
     "varname": "autohelp_see_menu"
    }
   },
   {
    "box": {
     "id": "obj-80",
     "maxclass": "newobj",
     "hidden": 1,
     "fontname": "Verdana",
     "fontsize": 10.0,
     "frozen_object_attributes": {
      "helpfontsize": 10.0,
      "helpfontname": "Verdana"
     },
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      21.0,
      470.0,
      54.0,
      19.0
     ],
     "text": "autohelp"
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "destination": [
      "obj-32",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-31",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-36",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-35",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-40",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-36",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-40",
      1
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-36",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-51",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-50",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-40",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-51",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-53b",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-52b",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-40",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-53b",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-55b",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-54b",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-40",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-55b",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-57b",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-56b",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-40",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-57b",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-40",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-45",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-22",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-22",
      1
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-40",
      1
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-70",
      0
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-70",
      1
     ],
     "hidden": 0,
     "midpoints": [],
     "source": [
      "obj-40",
      1
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-14",
      0
     ],
     "hidden": 1,
     "midpoints": [],
     "source": [
      "obj-22",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-14",
      1
     ],
     "hidden": 1,
     "midpoints": [],
     "source": [
      "obj-22",
      1
     ]
    }
   }
  ]
 }
}
