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
  "description": "A heterodyne envelope detector into two triode stages into the intensity key. A source: patch tap.palme~ or tap.metallique~ after it for the rest of the instrument. The ribbon is linear in semitones and nothing quantizes.",
  "digest": "The Ondes Martenot voice, minus its loudspeaker",
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
     "text": "tap.ondes~",
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
     "text": "The Ondes Martenot voice, minus its loudspeaker",
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
      72
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Not a VCO with waveform switches. The instrument is HETERODYNE: two oscillators near 80 kHz, one moved by the ribbon, summed \u2014 and the envelope of that sum is the note. Those oscillators measure as essentially pure, so all the character is downstream: the envelope of two equal oscillators is 2|cos|, which already puts the 2nd harmonic 14 dB down and the 3rd 21 dB down before any valve touches it. The carrier is never simulated \u2014 the envelope has a closed form that matches a full 80 kHz simulation to a tenth of a dB.",
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
      205,
      300,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the two hands:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-6"
    }
   },
   {
    "box": {
     "maxclass": "slider",
     "patching_rect": [
      20,
      230,
      22,
      200
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "size": 1001.0,
     "orientation": 1,
     "id": "obj-7"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      60,
      230,
      170,
      22
     ],
     "numinlets": 6,
     "numoutlets": 1,
     "text": "scale 0 1000 0. 48.",
     "outlettype": [
      ""
     ],
     "id": "obj-8"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      60,
      260,
      170,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "sig~ 24.",
     "outlettype": [
      "signal"
     ],
     "id": "obj-9"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      60,
      290,
      200,
      48
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the RIBBON, in semitones above A1 \u2014 linear in semitones, so this slider is a glissando",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-10"
    }
   },
   {
    "box": {
     "maxclass": "slider",
     "patching_rect": [
      280,
      230,
      22,
      200
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "size": 1001.0,
     "orientation": 1,
     "id": "obj-11"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      320,
      230,
      170,
      22
     ],
     "numinlets": 6,
     "numoutlets": 1,
     "text": "scale 0 1000 0. 1.",
     "outlettype": [
      ""
     ],
     "id": "obj-12"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      320,
      260,
      170,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "sig~ 1.",
     "outlettype": [
      "signal"
     ],
     "id": "obj-13"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      320,
      290,
      200,
      62
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the intensity KEY. Its bottom 45% is silent \u2014 that is the key bending before it reaches the powder bag, not a broken object",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-14"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      460,
      470,
      22
     ],
     "numinlets": 2,
     "numoutlets": 1,
     "text": "tap.ondes~ @drive 2.5 @level 0.5 @smooth 4",
     "outlettype": [
      "signal"
     ],
     "id": "obj-15"
    }
   },
   {
    "box": {
     "maxclass": "newobj",
     "patching_rect": [
      20,
      500,
      470,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "text": "tap.palme~ @root 110 @mix 55 @level 0.5",
     "outlettype": [
      "signal"
     ],
     "id": "obj-16"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      20,
      530,
      470,
      34
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the diffuseur is a separate object, exactly as it is a separate cabinet on the real instrument \u2014 swap in tap.metallique~, or delete it for the principal",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-17"
    }
   },
   {
    "box": {
     "maxclass": "gain~",
     "patching_rect": [
      20,
      580,
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
     "id": "obj-18"
    }
   },
   {
    "box": {
     "maxclass": "ezdac~",
     "patching_rect": [
      20,
      660,
      45,
      45
     ],
     "numinlets": 2,
     "numoutlets": 0,
     "outlettype": [],
     "id": "obj-19"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      540,
      205,
      240,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the circuit:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-20"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      540,
      230,
      230,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "depth",
     "id": "obj-21"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      540,
      255,
      230,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "detect",
     "id": "obj-22"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      540,
      280,
      230,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "drive",
     "id": "obj-23"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      540,
      305,
      230,
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
      540,
      330,
      230,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "oversample",
     "id": "obj-25"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      540,
      360,
      250,
      62
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "depth is the oscillator balance: at 1 the envelope closes and the tone is full, below that it thins. detect is the published 200 us RC \u2014 lengthen it and the harmonics go with it.",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-26"
    }
   },
   {
    "box": {
     "maxclass": "comment",
     "patching_rect": [
      800,
      205,
      220,
      20
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "the choices, and the switch:",
     "fontsize": 12.0,
     "fontname": "Arial",
     "id": "obj-27"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      800,
      230,
      200,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "keyplacement",
     "id": "obj-28"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      800,
      255,
      200,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "polarity",
     "id": "obj-29"
    }
   },
   {
    "box": {
     "maxclass": "attrui",
     "patching_rect": [
      800,
      280,
      200,
      22
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "attr": "power",
     "id": "obj-30"
    }
   },
   {
    "box": {
     "maxclass": "message",
     "patching_rect": [
      800,
      315,
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
      800,
      345,
      220,
      76
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "keyplacement and polarity are things the sources do not settle \u2014 whether pressure means level or dirt, and which way the transformer is wound. Both are audible, so both are switches rather than silent guesses.",
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
      690,
      980,
      92
     ],
     "numinlets": 1,
     "numoutlets": 0,
     "text": "Try: hold the key up and drag the ribbon slowly. It glides, because it is a ribbon \u2014 there are no notes to snap to. Then set keyplacement to 1 and play the key again: pressure now drives the valves, so soft is clean and hard is dirty, which is a very different instrument under the hand. Turn drive all the way down and notice the tone is still not a sine \u2014 the demodulator made those harmonics, not the valves. And what is missing on purpose: the waveform registers. The real instrument has switchable timbres whose filter shapes are in none of the sources, and inventing them is the one thing this object will not do.",
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
      "obj-15",
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
      "obj-13",
      0
     ],
     "destination": [
      "obj-15",
      1
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
      "obj-18",
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
      "obj-18",
      0
     ],
     "destination": [
      "obj-19",
      1
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
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
      "obj-15",
      0
     ]
    }
   }
  ]
 }
}