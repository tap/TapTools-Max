{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 8,
   "minor": 6,
   "revision": 2,
   "architecture": "x64",
   "modernui": 1
  },
  "classnamespace": "box",
  "rect": [ 155.0, 44.0, 661.0, 760.0 ],
  "bglocked": 0,
  "openinpresentation": 0,
  "default_fontsize": 11.0,
  "default_fontface": 0,
  "default_fontname": "Arial",
  "gridonopen": 1,
  "gridsize": [ 15.0, 15.0 ],
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
  "description": "",
  "digest": "",
  "tags": "",
  "style": "",
  "subpatcher_template": "",
  "assistshowspatchername": 0,
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 10.0, 8.0, 485.0, 30.0 ],
     "text": "tap.autowah~"
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 10.0, 36.0, 585.0, 21.0 ],
     "text": "envelope filter (auto-wah) in the Snow White style"
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 10.0, 57.0, 585.0, 60.0 ],
     "text": "A resonant 2-pole filter swept up from the bias resting frequency by your playing dynamics, after the Mad Professor Snow White AutoWah: fast attack, decay-controlled fall-back, a throaty wah voice from 250 Hz to 2.5 kHz. Sensitivity at -60 turns it into a fixed, manually swept cocked wah. The right inlet sidechains the detector; the right outlet emits the envelope (0..1)."
    }
   },
   {
    "box": {
     "id": "obj-4",
     "maxclass": "panel",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 5.0, 5.0, 605.0, 52.0 ]
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 30.0, 132.0, 260.0, 19.0 ],
     "text": "The four knobs of the pedal"
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [ 30.0, 154.0, 60.0, 19.0 ],
     "outlettype": [ "", "bang" ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 30.0, 177.0, 96.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "sensitivity $1"
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 134.0, 154.0, 190.0, 19.0 ],
     "text": "trigger level dB (-60 = manual)"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [ 30.0, 201.0, 60.0, 19.0 ],
     "outlettype": [ "", "bang" ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 30.0, 224.0, 68.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "decay $1"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 134.0, 201.0, 300.0, 19.0 ],
     "text": "fall-back ms - fast = wah every note, slow = swells"
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [ 30.0, 248.0, 60.0, 19.0 ],
     "outlettype": [ "", "bang" ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 30.0, 271.0, 58.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "bias $1"
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 134.0, 248.0, 280.0, 19.0 ],
     "text": "resting frequency Hz (the sweep floor)"
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [ 30.0, 295.0, 60.0, 19.0 ],
     "outlettype": [ "", "bang" ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 30.0, 318.0, 92.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "resonance $1"
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 134.0, 295.0, 220.0, 19.0 ],
     "text": "Q, 0..1 (1 = edge of self-oscillation)"
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 360.0, 132.0, 240.0, 19.0 ],
     "text": "Extensions (neutral at their defaults)"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [ 360.0, 154.0, 54.0, 19.0 ],
     "outlettype": [ "", "bang" ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 360.0, 177.0, 68.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "range $1"
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 436.0, 154.0, 170.0, 19.0 ],
     "text": "sweep span, octaves (0..5)"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "umenu",
     "numinlets": 1,
     "numoutlets": 3,
     "patching_rect": [ 360.0, 201.0, 80.0, 19.0 ],
     "outlettype": [ "int", "", "" ],
     "items": [ "up", ",", "down" ]
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 360.0, 224.0, 84.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "direction $1"
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 448.0, 201.0, 150.0, 19.0 ],
     "text": "the pedal only sweeps up"
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "umenu",
     "numinlets": 1,
     "numoutlets": 3,
     "patching_rect": [ 360.0, 248.0, 90.0, 19.0 ],
     "outlettype": [ "int", "", "" ],
     "items": [ "lowpass", ",", "bandpass" ]
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 360.0, 271.0, 64.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "mode $1"
    }
   },
   {
    "box": {
     "id": "obj-27",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 458.0, 248.0, 145.0, 19.0 ],
     "text": "the circuit's two taps"
    }
   },
   {
    "box": {
     "id": "obj-28",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [ 360.0, 295.0, 54.0, 19.0 ],
     "outlettype": [ "", "bang" ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-29",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 360.0, 318.0, 62.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "drive $1"
    }
   },
   {
    "box": {
     "id": "obj-30",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 430.0, 295.0, 180.0, 19.0 ],
     "text": "dB into the saturating circuit"
    }
   },
   {
    "box": {
     "id": "obj-31",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [ 360.0, 342.0, 54.0, 19.0 ],
     "outlettype": [ "", "bang" ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-32",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 360.0, 365.0, 54.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "mix $1"
    }
   },
   {
    "box": {
     "id": "obj-33",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 430.0, 342.0, 180.0, 19.0 ],
     "text": "dry/wet % (pedal = 100, wet only)"
    }
   },
   {
    "box": {
     "id": "obj-34",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 30.0, 400.0, 560.0, 19.0 ],
     "text": "Factory voicings ship in slots 1-4 (guitar, bass, slow swell, cocked wah) - recall morphs over the interp time"
    }
   },
   {
    "box": {
     "id": "obj-35",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 30.0, 422.0, 54.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "recall 1"
    }
   },
   {
    "box": {
     "id": "obj-36",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 90.0, 422.0, 54.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "recall 2"
    }
   },
   {
    "box": {
     "id": "obj-37",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 150.0, 422.0, 54.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "recall 3"
    }
   },
   {
    "box": {
     "id": "obj-38",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 210.0, 422.0, 54.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "recall 4"
    }
   },
   {
    "box": {
     "id": "obj-39",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 270.0, 422.0, 88.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "recall 2 4000"
    }
   },
   {
    "box": {
     "id": "obj-40",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 366.0, 422.0, 50.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "store 5"
    }
   },
   {
    "box": {
     "id": "obj-41",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 422.0, 422.0, 54.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "recall 5"
    }
   },
   {
    "box": {
     "id": "obj-42",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 482.0, 422.0, 40.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "clear"
    }
   },
   {
    "box": {
     "id": "obj-43",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 2,
     "patching_rect": [ 30.0, 447.0, 54.0, 19.0 ],
     "outlettype": [ "", "bang" ],
     "parameter_enable": 0
    }
   },
   {
    "box": {
     "id": "obj-44",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 30.0, 470.0, 70.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "interp $1"
    }
   },
   {
    "box": {
     "id": "obj-45",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 30.0, 505.0, 400.0, 19.0 ],
     "text": "Pluck something into it - the dynamics are the whole point"
    }
   },
   {
    "box": {
     "id": "obj-46",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [ 30.0, 527.0, 20.0, 20.0 ],
     "outlettype": [ "int" ]
    }
   },
   {
    "box": {
     "id": "obj-47",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 30.0, 551.0, 66.0, 19.0 ],
     "outlettype": [ "bang" ],
     "text": "metro 400"
    }
   },
   {
    "box": {
     "id": "obj-48",
     "maxclass": "message",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 30.0, 575.0, 62.0, 17.0 ],
     "outlettype": [ "" ],
     "text": "1, 0 250"
    }
   },
   {
    "box": {
     "id": "obj-49",
     "maxclass": "button",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [ 104.0, 551.0, 20.0, 20.0 ],
     "outlettype": [ "bang" ]
    }
   },
   {
    "box": {
     "id": "obj-50",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 2,
     "patching_rect": [ 30.0, 599.0, 40.0, 19.0 ],
     "outlettype": [ "signal", "bang" ],
     "text": "line~"
    }
   },
   {
    "box": {
     "id": "obj-51",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 104.0, 599.0, 62.0, 19.0 ],
     "outlettype": [ "signal" ],
     "text": "saw~ 110"
    }
   },
   {
    "box": {
     "id": "obj-52",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 1,
     "patching_rect": [ 30.0, 623.0, 34.0, 19.0 ],
     "outlettype": [ "signal" ],
     "text": "*~"
    }
   },
   {
    "box": {
     "id": "obj-53",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 2,
     "patching_rect": [ 30.0, 651.0, 90.0, 21.0 ],
     "outlettype": [ "signal", "signal" ],
     "text": "tap.autowah~"
    }
   },
   {
    "box": {
     "id": "obj-54",
     "maxclass": "scope~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [ 300.0, 651.0, 130.0, 60.0 ]
    }
   },
   {
    "box": {
     "id": "obj-55",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 300.0, 630.0, 300.0, 19.0 ],
     "text": "right outlet: the envelope (0..1) - patch it anywhere"
    }
   },
   {
    "box": {
     "id": "obj-56",
     "maxclass": "ezdac~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [ 30.0, 690.0, 45.0, 45.0 ]
    }
   },
   {
    "box": {
     "id": "obj-57",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 85.0, 702.0, 160.0, 19.0 ],
     "text": "start audio here"
    }
   },
   {
    "box": {
     "id": "obj-58",
     "maxclass": "bpatcher",
     "name": "tap.badge.maxpat",
     "numinlets": 0,
     "numoutlets": 0,
     "patching_rect": [ 430.0, 505.0, 180.0, 67.0 ]
    }
   },
   {
    "box": {
     "id": "obj-59",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [ 130.0, 527.0, 470.0, 19.0 ],
     "text": "A signal in the right inlet sidechains the detector, so one sound can wah another."
    }
   }
  ],
  "lines": [
   { "patchline": { "source": [ "obj-6", 0 ], "destination": [ "obj-7", 0 ] } },
   { "patchline": { "source": [ "obj-9", 0 ], "destination": [ "obj-10", 0 ] } },
   { "patchline": { "source": [ "obj-12", 0 ], "destination": [ "obj-13", 0 ] } },
   { "patchline": { "source": [ "obj-15", 0 ], "destination": [ "obj-16", 0 ] } },
   { "patchline": { "source": [ "obj-19", 0 ], "destination": [ "obj-20", 0 ] } },
   { "patchline": { "source": [ "obj-22", 0 ], "destination": [ "obj-23", 0 ] } },
   { "patchline": { "source": [ "obj-25", 0 ], "destination": [ "obj-26", 0 ] } },
   { "patchline": { "source": [ "obj-28", 0 ], "destination": [ "obj-29", 0 ] } },
   { "patchline": { "source": [ "obj-31", 0 ], "destination": [ "obj-32", 0 ] } },
   { "patchline": { "source": [ "obj-43", 0 ], "destination": [ "obj-44", 0 ] } },
   { "patchline": { "source": [ "obj-7", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-10", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-13", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-16", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-20", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-23", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-26", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-29", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-32", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-35", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-36", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-37", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-38", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-39", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-40", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-41", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-42", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-44", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-46", 0 ], "destination": [ "obj-47", 0 ] } },
   { "patchline": { "source": [ "obj-47", 0 ], "destination": [ "obj-48", 0 ] } },
   { "patchline": { "source": [ "obj-49", 0 ], "destination": [ "obj-48", 0 ] } },
   { "patchline": { "source": [ "obj-48", 0 ], "destination": [ "obj-50", 0 ] } },
   { "patchline": { "source": [ "obj-50", 0 ], "destination": [ "obj-52", 1 ] } },
   { "patchline": { "source": [ "obj-51", 0 ], "destination": [ "obj-52", 0 ] } },
   { "patchline": { "source": [ "obj-52", 0 ], "destination": [ "obj-53", 0 ] } },
   { "patchline": { "source": [ "obj-53", 0 ], "destination": [ "obj-56", 0 ] } },
   { "patchline": { "source": [ "obj-53", 0 ], "destination": [ "obj-56", 1 ] } },
   { "patchline": { "source": [ "obj-53", 1 ], "destination": [ "obj-54", 0 ] } }
  ],
  "dependency_cache": [],
  "autosave": 0
 }
}
