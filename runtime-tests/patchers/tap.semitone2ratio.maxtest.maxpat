{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 7,
   "minor": 0,
   "revision": 4,
   "architecture": "x64",
   "modernui": 1
  },
  "rect": [
   66.0,
   79.0,
   1070.0,
   480.0
  ],
  "bglocked": 0,
  "openinpresentation": 0,
  "default_fontsize": 12.0,
  "default_fontface": 0,
  "default_fontname": "Helvetica Neue Light",
  "gridonopen": 1,
  "gridsize": [
   5.0,
   5.0
  ],
  "gridsnaponopen": 1,
  "statusbarvisible": 2,
  "toolbarvisible": 1,
  "description": "tap.semitone2ratio: 12 semitones converts to a ratio of 2.",
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      40,
      40,
      70,
      22.0
     ],
     "text": "loadbang"
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ],
     "patching_rect": [
      40,
      90,
      70,
      22.0
     ],
     "text": "delay 200"
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "message",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      40,
      140,
      60,
      22.0
     ],
     "text": "12."
    }
   },
   {
    "box": {
     "id": "obj-4",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      40,
      200,
      160,
      22.0
     ],
     "text": "tap.semitone2ratio"
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      40,
      260,
      150,
      22.0
     ],
     "text": "test.equals 2.0"
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      40,
      320,
      180,
      22.0
     ],
     "text": "test.assert tapsemitone2ratio_octave"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      300,
      320,
      110,
      22.0
     ],
     "text": "test.terminate"
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "obj-1",
      0
     ],
     "destination": [
      "obj-2",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-2",
      0
     ],
     "destination": [
      "obj-3",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-3",
      0
     ],
     "destination": [
      "obj-4",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
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
      "obj-7",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   }
  ]
 }
}
