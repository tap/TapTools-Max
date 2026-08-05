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
  "classnamespace": "box",
  "rect": [ 85.0, 104.0, 640.0, 560.0 ],
  "bglocked": 0,
  "openinpresentation": 0,
  "default_fontsize": 12.0,
  "default_fontface": 0,
  "default_fontname": "Arial",
  "gridonopen": 1,
  "gridsize": [ 15.0, 15.0 ],
  "gridsnaponopen": 1,
  "objectsnaponopen": 1,
  "statusbarvisible": 2,
  "toolbarvisible": 1,
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 30.0, 20.0, 440.0, 27.0 ],
     "fontsize": 18.0,
     "text": "tap.harmony~ — formant-preserving multi-voice harmonizer"
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 30.0, 50.0, 560.0, 47.0 ],
     "text": "Up to four pitch-shifted copies of a monophonic input hold a chord (intervals in semitones, fractional allowed) over a latency-aligned dry path. Formant preservation keeps the singer's envelope — turn it off for the chipmunk bend. Latency is one FFT frame (fftsize samples)."
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "ezadc~",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [ "signal", "signal" ],
     "patching_rect": [ 30.0, 120.0, 45.0, 45.0 ]
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "" ],
     "patching_rect": [ 120.0, 120.0, 90.0, 22.0 ],
     "text": "chord 3 7 12"
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "" ],
     "patching_rect": [ 220.0, 120.0, 100.0, 22.0 ],
     "text": "chord -12 4 7"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "" ],
     "patching_rect": [ 330.0, 120.0, 70.0, 22.0 ],
     "text": "chord 12"
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "" ],
     "patching_rect": [ 120.0, 155.0, 70.0, 22.0 ],
     "text": "formant 0"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "" ],
     "patching_rect": [ 200.0, 155.0, 70.0, 22.0 ],
     "text": "formant 1"
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "" ],
     "patching_rect": [ 280.0, 155.0, 70.0, 22.0 ],
     "text": "glide 400"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "" ],
     "patching_rect": [ 360.0, 155.0, 62.0, 22.0 ],
     "text": "glide 10"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "" ],
     "patching_rect": [ 432.0, 155.0, 44.0, 22.0 ],
     "text": "clear"
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "" ],
     "patching_rect": [ 120.0, 190.0, 60.0, 22.0 ],
     "text": "dry 0."
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "" ],
     "patching_rect": [ 190.0, 190.0, 60.0, 22.0 ],
     "text": "dry 1."
    }
   },
   {
    "box": {
     "id": "obj-4",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [ "signal" ],
     "patching_rect": [ 30.0, 250.0, 130.0, 22.0 ],
     "text": "tap.harmony~ @dry 1"
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [ "signal" ],
     "patching_rect": [ 30.0, 300.0, 55.0, 22.0 ],
     "text": "*~ 0.7"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "ezdac~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [ 30.0, 350.0, 45.0, 45.0 ]
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 30.0, 420.0, 560.0, 60.0 ],
     "text": "Recipes: chord 3 7 12 over a sung line is the classic keyboard-harmonizer stack; chord -12 4 7 with dry 0 is a choir that replaces the singer; glide 400 turns chord changes into portamento. See the Tools on Tap book's harmonizer recipe (A choir of one) for the Bon Iver patches."
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [ "obj-3", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-5", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-6", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-7", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-8", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-9", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-10", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-11", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-12", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-13", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-14", 0 ],
     "destination": [ "obj-4", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-4", 0 ],
     "destination": [ "obj-15", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-15", 0 ],
     "destination": [ "obj-16", 0 ]
    }
   },
   {
    "patchline": {
     "source": [ "obj-15", 0 ],
     "destination": [ "obj-16", 1 ]
    }
   }
  ]
 }
}
