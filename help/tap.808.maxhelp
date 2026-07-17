{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 8,
   "minor": 5,
   "revision": 0,
   "architecture": "x64",
   "modernui": 1
  },
  "rect": [
   50.0,
   50.0,
   1120.0,
   590.0
  ],
  "bglocked": 0,
  "openinpresentation": 0,
  "default_fontsize": 11.0,
  "default_fontface": 0,
  "default_fontname": "Verdana",
  "gridonopen": 1,
  "gridsize": [
   5.0,
   5.0
  ],
  "gridsnaponopen": 1,
  "statusbarvisible": 2,
  "toolbarvisible": 1,
  "description": "the tap.808 family: eight TR-808 voice channels on one accent bus.",
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      14.0,
      300.0,
      27.0
     ],
     "text": "the tap.808 family",
     "fontname": "Verdana",
     "fontsize": 18.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      42.0,
      560.0,
      18.0
     ],
     "text": "Eight circuit-informed TR-808 voice channels sharing one trigger contract.",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      64.0,
      1060.0,
      72.0
     ],
     "text": "Every voice speaks the same accent bus: a signal rising edge fires it sample-accurately, and the edge's amplitude (0..1) is the accent \u2014 modeled on the hardware's common 4-14 V trigger bus, so an accent is a harder hit into the circuit, not a volume knob. From the scheduler: bang = full accent, trigger 0.6 = accented hit, clear = reset. Voice state persists across triggers (no machine-gunning), and every noise source is seeded and deterministic \u2014 give mc. instances different @seed values. Each voice's own help file covers its panel controls.",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-4",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      20.0,
      152.0,
      20.0,
      20.0
     ],
     "outlettype": [
      "int"
     ]
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      48.0,
      152.0,
      70.0,
      20.0
     ],
     "outlettype": [
      "bang"
     ],
     "text": "metro 129",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 4,
     "patching_rect": [
      126.0,
      152.0,
      84.0,
      20.0
     ],
     "outlettype": [
      "int",
      "",
      "",
      "int"
     ],
     "text": "counter 1 16",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      220.0,
      154.0,
      280.0,
      18.0
     ],
     "text": "<- a two-bar pattern: sixteenths at 116 BPM",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "gain~",
     "numinlets": 2,
     "numoutlets": 2,
     "patching_rect": [
      20.0,
      428.0,
      30.0,
      60.0
     ],
     "outlettype": [
      "signal",
      "int"
     ]
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "ezdac~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      502.0,
      40.0,
      40.0
     ],
     "local": 1
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      70.0,
      512.0,
      520.0,
      18.0
     ],
     "text": "<- all eight voices sum here; the messages are also clickable to audition a voice",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      196.0,
      120.0,
      18.0
     ],
     "text": "kick",
     "fontname": "Verdana",
     "fontsize": 11.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      20.0,
      372.0,
      101.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.808.kick~",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 5,
     "patching_rect": [
      20.0,
      218.0,
      120.0,
      20.0
     ],
     "outlettype": [
      "",
      "",
      "",
      "",
      ""
     ],
     "text": "sel 1 5 9 13",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      20.0,
      246.0,
      94.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 1.",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      20.0,
      274.0,
      94.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 0.75",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      154.0,
      196.0,
      120.0,
      18.0
     ],
     "text": "snare",
     "fontname": "Verdana",
     "fontsize": 11.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      154.0,
      372.0,
      108.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.808.snare~",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 3,
     "patching_rect": [
      154.0,
      218.0,
      120.0,
      20.0
     ],
     "outlettype": [
      "",
      "",
      ""
     ],
     "text": "sel 5 13",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      154.0,
      246.0,
      94.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 0.9",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      288.0,
      196.0,
      120.0,
      18.0
     ],
     "text": "clap",
     "fontname": "Verdana",
     "fontsize": 11.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      288.0,
      372.0,
      101.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.808.clap~",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      288.0,
      218.0,
      120.0,
      20.0
     ],
     "outlettype": [
      "",
      ""
     ],
     "text": "sel 13",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      288.0,
      246.0,
      94.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 1.",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      422.0,
      196.0,
      120.0,
      18.0
     ],
     "text": "hats",
     "fontname": "Verdana",
     "fontsize": 11.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      422.0,
      372.0,
      96.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.808.hat~",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 7,
     "patching_rect": [
      422.0,
      218.0,
      120.0,
      20.0
     ],
     "outlettype": [
      "",
      "",
      "",
      "",
      "",
      "",
      ""
     ],
     "text": "sel 1 3 5 9 11 13",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-27",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      422.0,
      246.0,
      94.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 0.6",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-28",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 3,
     "patching_rect": [
      422.0,
      274.0,
      120.0,
      20.0
     ],
     "outlettype": [
      "",
      "",
      ""
     ],
     "text": "sel 7 15",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-29",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      422.0,
      302.0,
      94.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "open 0.8",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-30",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      556.0,
      196.0,
      120.0,
      18.0
     ],
     "text": "cymbal",
     "fontname": "Verdana",
     "fontsize": 11.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-31",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      556.0,
      372.0,
      192.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.808.cymbal~ @decay 0.5",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-32",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      556.0,
      218.0,
      120.0,
      20.0
     ],
     "outlettype": [
      "",
      ""
     ],
     "text": "sel 1",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-33",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      556.0,
      246.0,
      94.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 0.5",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-34",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      690.0,
      196.0,
      120.0,
      18.0
     ],
     "text": "cowbell",
     "fontname": "Verdana",
     "fontsize": 11.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-35",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      690.0,
      372.0,
      122.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.808.cowbell~",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-36",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      690.0,
      218.0,
      120.0,
      20.0
     ],
     "outlettype": [
      "",
      ""
     ],
     "text": "sel 11",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-37",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      690.0,
      246.0,
      94.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 0.7",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-38",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      824.0,
      196.0,
      120.0,
      18.0
     ],
     "text": "tom",
     "fontname": "Verdana",
     "fontsize": 11.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-39",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      824.0,
      372.0,
      150.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.808.tom~ @size 1",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-40",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      824.0,
      218.0,
      120.0,
      20.0
     ],
     "outlettype": [
      "",
      ""
     ],
     "text": "sel 15",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-41",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      824.0,
      246.0,
      94.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 0.9",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-42",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      958.0,
      196.0,
      120.0,
      18.0
     ],
     "text": "rim",
     "fontname": "Verdana",
     "fontsize": 11.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-43",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      958.0,
      372.0,
      96.0,
      21.0
     ],
     "outlettype": [
      "signal"
     ],
     "text": "tap.808.rim~",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-44",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [
      958.0,
      218.0,
      120.0,
      20.0
     ],
     "outlettype": [
      "",
      ""
     ],
     "text": "sel 8",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   },
   {
    "box": {
     "id": "obj-45",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      958.0,
      246.0,
      94.0,
      20.0
     ],
     "outlettype": [
      ""
     ],
     "text": "trigger 0.6",
     "fontname": "Verdana",
     "fontsize": 11.0
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "obj-4",
      0
     ],
     "destination": [
      "obj-5",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-5",
      0
     ],
     "destination": [
      "obj-6",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-13",
      0
     ],
     "hidden": 0,
     "disabled": 0
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
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-14",
      0
     ],
     "destination": [
      "obj-12",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-13",
      1
     ],
     "destination": [
      "obj-15",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-13",
      2
     ],
     "destination": [
      "obj-15",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-13",
      3
     ],
     "destination": [
      "obj-15",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-15",
      0
     ],
     "destination": [
      "obj-12",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-12",
      0
     ],
     "destination": [
      "obj-8",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-18",
      0
     ],
     "hidden": 0,
     "disabled": 0
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
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-18",
      1
     ],
     "destination": [
      "obj-19",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-19",
      0
     ],
     "destination": [
      "obj-17",
      0
     ],
     "hidden": 0,
     "disabled": 0
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
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-22",
      0
     ],
     "hidden": 0,
     "disabled": 0
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
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-23",
      0
     ],
     "destination": [
      "obj-21",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-21",
      0
     ],
     "destination": [
      "obj-8",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-26",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-26",
      0
     ],
     "destination": [
      "obj-27",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-26",
      1
     ],
     "destination": [
      "obj-27",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-26",
      2
     ],
     "destination": [
      "obj-27",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-26",
      3
     ],
     "destination": [
      "obj-27",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-26",
      4
     ],
     "destination": [
      "obj-27",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-26",
      5
     ],
     "destination": [
      "obj-27",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-27",
      0
     ],
     "destination": [
      "obj-25",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-28",
      0
     ],
     "hidden": 0,
     "disabled": 0
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
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-28",
      1
     ],
     "destination": [
      "obj-29",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-29",
      0
     ],
     "destination": [
      "obj-25",
      0
     ],
     "hidden": 0,
     "disabled": 0
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
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-32",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-32",
      0
     ],
     "destination": [
      "obj-33",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-33",
      0
     ],
     "destination": [
      "obj-31",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-31",
      0
     ],
     "destination": [
      "obj-8",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-36",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-36",
      0
     ],
     "destination": [
      "obj-37",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-37",
      0
     ],
     "destination": [
      "obj-35",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-35",
      0
     ],
     "destination": [
      "obj-8",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-40",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-40",
      0
     ],
     "destination": [
      "obj-41",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-41",
      0
     ],
     "destination": [
      "obj-39",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-39",
      0
     ],
     "destination": [
      "obj-8",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-44",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-44",
      0
     ],
     "destination": [
      "obj-45",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-45",
      0
     ],
     "destination": [
      "obj-43",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-43",
      0
     ],
     "destination": [
      "obj-8",
      0
     ],
     "hidden": 0,
     "disabled": 0
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
     ],
     "hidden": 0,
     "disabled": 0
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
      1
     ],
     "hidden": 0,
     "disabled": 0
    }
   }
  ]
 }
}
