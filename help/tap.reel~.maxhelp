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
  "description": "Three reels at deliberately awkward lengths, summed. That is tap.airport~, patched \u2014 and because each loop is its own object you can reach into one: insert a filter, run one through tap.discreet~, or give one a length nothing else shares.",
  "digest": "One free-running tape loop \u2014 the lane tap.airport~ is made of",
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
     "text": "tap.reel~",
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
     "text": "One free-running tape loop \u2014 the lane tap.airport~ is made of",
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
     "text": "Three reels at deliberately awkward lengths, summed. That is tap.airport~, patched \u2014 and because each loop is its own object you can reach into one: insert a filter, run one through tap.discreet~, or give one a length nothing else shares."
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
     "text": "a phrase source to punch onto the reels:"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      195.0,
      110.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "cycle~ 330",
     "outlettype": [
      "signal"
     ]
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "gain~",
     "patching_rect": [
      20.0,
      225.0,
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
     "id": "obj-9",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      300.0,
      420.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "three lanes \u2014 the lengths are the score, keep them incommensurate:"
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      325.0,
      300.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 3,
     "text": "tap.reel~ 12. @length 6.3 @pan -0.7",
     "outlettype": [
      "signal",
      "signal",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      355.0,
      300.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 3,
     "text": "tap.reel~ 12. @length 7.7 @pan 0.",
     "outlettype": [
      "signal",
      "signal",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "newobj",
     "patching_rect": [
      20.0,
      385.0,
      300.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 3,
     "text": "tap.reel~ 12. @length 9.1 @pan 0.7",
     "outlettype": [
      "signal",
      "signal",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "comment",
     "patching_rect": [
      350.0,
      300.0,
      300.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the record gates \u2014 one per reel:"
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "toggle",
     "patching_rect": [
      350.0,
      325.0,
      22.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ]
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "message",
     "patching_rect": [
      378.0,
      325.0,
      70.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "record $1",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "toggle",
     "patching_rect": [
      350.0,
      355.0,
      22.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ]
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "message",
     "patching_rect": [
      378.0,
      355.0,
      70.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "record $1",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "toggle",
     "patching_rect": [
      350.0,
      385.0,
      22.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "int"
     ]
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "message",
     "patching_rect": [
      378.0,
      385.0,
      70.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "record $1",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "comment",
     "patching_rect": [
      460.0,
      325.0,
      300.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "recording starts wherever the head is \u2014"
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "comment",
     "patching_rect": [
      460.0,
      345.0,
      300.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "there is no downbeat, and there never was"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "comment",
     "patching_rect": [
      560.0,
      385.0,
      340.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "ask a reel where its head is, or how long it really is:"
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "message",
     "patching_rect": [
      560.0,
      410.0,
      60.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "phase",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "message",
     "patching_rect": [
      625.0,
      410.0,
      100.0,
      22.0
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "loopsamples",
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "newobj",
     "patching_rect": [
      560.0,
      440.0,
      150.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "print tap.reel~",
     "outlettype": []
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      430.0,
      300.0,
      20.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the sum \u2014 this is the whole of tap.airport~:"
    }
   },
   {
    "box": {
     "id": "obj-27",
     "maxclass": "gain~",
     "patching_rect": [
      20.0,
      455.0,
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
     "id": "obj-28",
     "maxclass": "gain~",
     "patching_rect": [
      60.0,
      455.0,
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
     "id": "obj-29",
     "maxclass": "ezdac~",
     "patching_rect": [
      20.0,
      530.0,
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
     "id": "obj-30",
     "maxclass": "comment",
     "patching_rect": [
      20.0,
      600.0,
      900.0,
      60.0
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: open the first record gate, let a phrase go by, close it; repeat on the other two. The three lengths never line up, so the texture keeps rearranging itself. Then do what the monolith cannot let you do \u2014 put a filter between one reel and the sum, or feed one reel a different source entirely."
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
      "obj-27",
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
      "obj-28",
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
      "obj-27",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-11",
      1
     ],
     "destination": [
      "obj-28",
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
      "obj-27",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-12",
      1
     ],
     "destination": [
      "obj-28",
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
      "obj-15",
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
      "obj-10",
      2
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
      "obj-27",
      0
     ],
     "destination": [
      "obj-29",
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
      "obj-29",
      1
     ]
    }
   }
  ]
 }
}
