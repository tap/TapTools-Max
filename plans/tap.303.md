# Plan — `tap.303.*` (TB-303-style acid bass voice, a diode-ladder filter, and later a sequencer)

> Status: **slices 0–3 shipped 2026-07-17** (source sweep; `diode_ladder.h` + `tap.diode~`;
> `tb303_voice.h` + `tap.303~`; the C13 accent-sweep circuit — see the REVIVAL.md §7
> progress-log entries). §8 blocking decisions approved by the author same day. Scope notes:
> slide/legato came forward into slice 2 (the note contract requires it); the slice-3 sweep's
> RC taus are component-derived (47 ms charge / ~150 ms drain) while `k_accent_sweep_oct` and
> the direct weight are informed approximations; the square-from-saw shaper is a
> polyBLEP-pulse baseline. Remaining: slice 4 polish — the Open303 A/B render calibration
> (sweep constants + square shaper), seed/tolerance, factory presets — then runtime
> validation in Max for both objects; phase 3 (the sequencer) stays deferred.
> Slice-0 findings now pinned: Stinchcombe's normalized TF is reproduced *exactly* by the
> equal-RC chain with the top cap halved (k_osc = 17, oscillation at √2× the stage rate —
> derived in the kernel header); feedback HPF 150 Hz, slide 60 ms, MEG 3 ms attack /
> 200 ms–2 s decay (accent shorts it to ~200 ms), VCA env ~0 ms attack / ~1.23 s decay /
> 1 ms release (50 ms accented), env-up fraction 2/3, measured cutoff-knob range
> 302–2394 Hz (all Open303 calibrations); accent-sweep circuit: diode + 47k → the resonance
> pot's second section (100k, 1 µF to ground at CW) → 100k mix resistor, ≈100/147 MEG
> scaling (Devil Fish). Emergent result worth noting: with the 150 Hz feedback HPF the
> filter — like a stock TB-303, famously — never quite self-oscillates; documented as a
> pinned behavior, not a defect (kernel header + tests).
> The Roland-recreation companion to `plans/tap.808.md` (the two machines are the
> acid-house rhythm section; this plan deliberately reuses that plan's decisions —
> naming convention, amplitude-as-accent triggering, slice discipline — so the two
> families read as one program). As with `tap.autowah~` and `tap.808.*`, all DSP lands
> in the kernel repo (`tap/taptools`) first, then the Min wrappers + vertical slices
> here.

## 0. The three phases, up front

| Phase | What | Ships as |
|-------|------|----------|
| **1** | **Circuit-informed structured synthesis** — the 303 voice reproduced from its schematic topology (saw VCO + square shaper → 4-stage diode-ladder VCF → VCA, with the accent and slide circuits that *couple* those blocks), component-derived constants, clean portable DSP | Two externals: **`tap.diode~`** (the diode-ladder filter, standalone — it earns its own object) and **`tap.303~`** (the complete voice) — the plan of record |
| **2** | **Fidelity upgrades** where they audibly matter — exact nonlinear solve of the diode loop is already in scope for phase 1 (the `ladder.h` `solver` precedent), so phase 2 narrows to the transistor VCA nonlinearity, square-shaper exactness, and (if A/B renders justify it) a WDF pass on the filter | Attributes on the *same* objects, `svf.h`-style — drop-in, no new externals |
| **3** | **Sequencer** — the 303's other half (and half its sound: accent/slide are *sequencer flags*) | `tap.303.seq~`, deliberately deferred until the phase-1 note convention has survived real patches, and coordinated with the `tap.808.seq~` decision (`plans/tap.808.md` §6) |

Explicitly rejected: sample playback, and a generic "acid preset" subtractive patch
(saw → any lowpass → ADSR). The entire reason the 303 is unmistakable is the
*coupling* — accent drives the filter and the VCA through shared circuitry with
memory across notes, slide is a gate-hold behavior, the envelopes are RC discharge
curves with fixed interrelations. A pile of separate generic objects cannot express
that; a circuit-informed voice can. That coupling **is** the deliverable.

## 1. Why this, and why now

- **It completes the pair.** `tap.808.*` is underway (slices 1–2 shipped); the TB-303
  is the same 1980–84 Roland lineage, the same analog-circuit genre, and the canonical
  musical partner. Same plan shape, same testing method, same provenance discipline.
- **The kernel repo already holds most of the parts.** `vco.h` is a polyBLEP
  saw/pulse oscillator with the deterministic per-`seed` analog-character machinery;
  `ladder.h` is a ZDF 4-stage *transistor*-ladder with per-stage tanh, fast and exact-
  Newton solvers, oversampling, ramps, and preset morphing — the diode ladder is its
  direct sibling (different stage coupling and feedback point, same toolbox);
  `swing_vca.h` has the RC `decay_env` discharge shape the 303's envelopes also use.
  Phase 1 is substantially *composition and calibration*, not greenfield DSP.
- **It fills the package's biggest gap**: TapTools has oscillators, filters,
  envelopes, and (soon) drum voices, but **no pitched, note-driven instrument**. The
  303 forces the note-interface convention (pitch/gate/accent/slide) that any future
  melodic voice or sequencer will reuse — designing it here, deliberately, beats
  backing into it later.
- **The literature is unusually good.** Tim Stinchcombe's filter analysis is the
  definitive circuit study; Robin Whittle's Devil Fish documentation describes the
  accent/slide/envelope circuits at component level; Open303 (Robin Schmidt) is a
  respected open-source reference implementation to cross-check renders against; and
  the x0xb0x clone published complete reverse-engineered schematics. "Port
  faithfully, document the provenance" has real sources to port from — same bar the
  808 papers set.
- **`tap.diode~` is independently valuable** regardless of the voice: the diode
  ladder is the one classic VA filter topology the package lacks
  (`tap.fourpole~`/`tap.ladder~` transistor ladder, `tap.svf~` state-variable,
  `tap.filter~` biquad — no diode ladder). Slice 1 ships a useful external even
  before any 303 voice exists.

## 2. Reference hardware — what the TB-303 actually is

Roland TB-303 Bass Line (1981–1984): a monophonic analog bass synthesizer with a
16-step pattern sequencer, designed as the bass companion to the TR-606/808. Facts
below are from the sources in §9; schematic-level constants (component values,
reference designators) come from the Service Notes and the x0xb0x schematics at
implementation time, each with a provenance comment.

### 2.1 Panel controls

One voice, seven knobs and a switch — the whole surface:

| Control | What it does |
|---------|--------------|
| **waveform** (switch) | sawtooth ⇄ square |
| **tuning** | master pitch trim (± roughly a semitone-scale range; exact span from the schematic) |
| **cut off freq** | VCF cutoff |
| **resonance** | VCF resonance — *also scales the accent sweep* (§2.2.5) |
| **env mod** | how much of the Main Envelope reaches the cutoff |
| **decay** | Main Envelope decay time (≈ 200 ms – 2 s; exact range from the schematic) |
| **accent** | how strongly accented steps hit (VCA *and* filter) |
| **volume** | output level |

Everything else about a note — pitch, gate, accented or not, slid or not — comes
from the **sequencer**, not the panel. That is why the note-interface design (§3.2)
matters so much: those four per-note facts are the voice's real-time inputs.

### 2.2 The six circuit blocks

1. **VCO** — a single sawtooth-core oscillator with an exponential (V/oct) converter;
   the pitch CV arrives *through the slide circuit* (§2.2.6). The **square is not a
   clean 50% pulse**: it is derived from the saw by a transistor wave-shaping circuit,
   giving a rounded, notched shape whose spectrum differs audibly from an ideal
   square. No PWM, no additional waveforms. There is **no keyboard tracking** into
   the filter — cutoff is set only by the knob, envelope, and accent.
2. **The diode-ladder VCF** — the panel says "18 dB/oct"; the circuit is a **4-stage
   diode ladder** whose asymptotic slope is 24 dB/oct with a shallower (≈18 dB-ish)
   region near cutoff — Stinchcombe's analysis untangles this. Unlike the Moog
   transistor ladder (buffered stages), the diode ladder's stages load each other, so
   the poles interact: resonance is broader and less pure, and the passband behavior
   under resonance differs — this coupling is *the* 303 filter character and is why
   `ladder.h` cannot simply be reused with new constants.
3. **The resonance feedback high-pass** — the feedback path runs through a high-pass
   network, so **resonance thins out as cutoff drops**. This is load-bearing for the
   sound (low squelchy notes don't ring the way high ones do) and must be modeled,
   not idealized away.
4. **Main Envelope Generator (MEG)** — a decay-only RC discharge (near-instant
   attack), decay set by the panel knob, scaled by **env mod** into the cutoff sum.
   (The env-mod pot also feeds a small counter-acting DC offset into the cutoff sum —
   the "gimmick" — so turning env mod up shifts the sweep's resting point down;
   model it, it changes how the knobs feel.)
5. **VCA envelope + accent** — the VCA has its **own fixed envelope**: fast attack,
   long exponential decay, cut short at gate-off. On an **accented step**: the accent
   line (scaled by the accent knob) makes the MEG hotter and faster, routes envelope
   into the VCA (louder *and* punchier, not just louder), and drives the **accent
   sweep circuit** — the MEG through the resonance pot charges a capacitor (C13)
   whose voltage adds to the cutoff. Because C13 doesn't fully discharge between
   closely-spaced accents, **consecutive accented notes build up** — the famous
   "wow" that makes runs of accents bloom. The resonance knob scales this path, so
   accent quacks harder at high resonance. This block is the heart of the machine.
6. **Slide** — a per-step flag: the sequencer **holds the gate across the step
   boundary** (so neither envelope retriggers) while the pitch CV glides through a
   fixed-time-constant RC lag (≈ 60 ms; exact value from the schematic). Slide is
   legato + lag, nothing more — which makes it cheap to model exactly.
7. *(Phase 3)* **Sequencer** — 16-step patterns, per-step pitch, gate/rest, accent
   flag, slide flag, up/down octave transposition, and the notoriously opaque
   pitch-mode/time-mode entry system (which we will *not* be recreating — §6).

## 3. Design

### 3.1 Naming and trademark

- **`tap.303~`** — the voice. **`tap.diode~`** — the standalone diode-ladder filter,
  named for its topology exactly as `tap.ladder~` is (the filter shelf reads:
  `tap.filter~`, `tap.svf~`, `tap.fourpole~`, `tap.ladder~`, `tap.diode~`).
  **`tap.303.seq~`** reserved for phase 3, matching the `tap.808.*` dot-family style.
- Same trademark posture as the 808 plan (§3.1 there): "Roland" and "TB-303" stay
  **out of shipped object names**; the bare "303" is the established generic. Maxrefs
  may say "inspired by the Roland TB-303" — nominative use. Source-level identifiers
  use `tb303` for clarity (`taptools::tb303`); the filter header is topology-named
  (`diode_ladder.h`, `taptools::diode`) since it outlives the 303 context.
- One voice object, not a modular kit. The accent/slide coupling (§2.2.5–6) lives
  *between* the blocks; decomposing the voice into osc + filter + env externals would
  amputate exactly what we are here to capture. (Patchers who want the parts have
  them: `tap.vco~` + `tap.diode~` + `tap.adsr~`.)

### 3.2 Interface conventions

**The note interface is the big new decision** — TapTools' first pitched voice, and
the contract `tap.303.seq~` (and any future melodic object) must honor. Proposed:

- **Signal surface (primary).** Inlet 1: **pitch as a MIDI note number signal**
  (float messages accepted; fractional = cents; converted to Hz inside, `tuning`
  applied there). Inlet 2: **gate signal, amplitude-as-accent** — rising edge above
  threshold = note-on; a level above 1.0 = accented, with accent depth
  min(amplitude − 1, 1) — so 1.0 is a plain note, 2.0 a fully accented one
  (pinned at slice 0) — mirroring both the 808 family's
  amplitude-as-accent triggers and the hardware's own "accent is a hotter CV"
  reality. **Slide needs no input of its own**: a pitch change while the gate is
  held *is* a slide (legato → RC glide, no envelope retrigger) — exactly the
  hardware behavior, for free.
- **Message surface (convenience).** `note <pitch> [accent 0/1] [slide 0/1]` fires a
  step at the next vector boundary; `bang` retriggers the last note. Help patchers
  use messages; sequencers and serious patches use signals.
- **Attributes mirror the panel** (§2.1), normalized 0..1 where the hardware knob is
  unitless: `waveform saw|square`, `tuning` (cents), `cutoff` (Hz), `resonance`,
  `envmod`, `decay`, `accent`, `gain`. Defaults = knobs at noon.
- **`seed` + `tolerance`** per house convention — deterministic per-unit component
  spread (VCO converter trim, filter diode matching, envelope caps), so an `mc.`
  stack of 303s decorrelates like a wall of real units.
- **Devil-Fish-style circuit bends** as clearly-marked beyond-hardware attributes
  defaulting to stock (the 808 precedent, §8 there): candidates are `slide_time`,
  separate normal/accent decay, `soft_attack`, filter-input drive. Ship the
  documented ones, skip speculative ones.
- House kernel features throughout: per-sample linear ramps on every parameter,
  16-slot preset morphing, allocation-free after `prepare()`, setters safe while
  audio runs, signal-rate cutoff override on `tap.diode~`.
- Single-channel objects (`tap.303~` is `sample_operator<2, 1>`; `tap.diode~`
  `<2, 1>` with the right inlet = signal-rate cutoff, the `tap.ladder~` surface);
  multichannel via `mc.`. C++20 wrappers, C++17-clean kernel headers.

### 3.3 Kernel surface (in `tap/taptools`)

| Header | Contents |
|--------|----------|
| `diode_ladder.h` | The 4-stage diode-ladder ZDF filter (`taptools::diode`): stage-coupled one-poles solved as one system (the coupling is the topology — no buffered-cascade shortcut), embedded diode nonlinearity, the resonance-path high-pass, fast + exact-Newton solvers, 1/2/4× oversampling — structurally modeled on `ladder.h`, sharing its conventions rather than its math |
| `tb303_voice.h` | The voice (`taptools::tb303`): saw core + square shaper (reusing `vco.h`'s polyBLEP machinery where the shapes allow; the 303 square likely needs its own shaper model), MEG + VCA envelope (RC discharge — `swing_vca.h`'s `decay_env` as the starting point, with 303 constants), gate/legato logic, the accent path including the C13 "wow" capacitor and the resonance-scaled sweep, the slide lag — composing `diode_ladder.h` |

All magic constants carry provenance comments (Service Notes / x0xb0x reference
designators, paper sections), per the `tap.dcblock~` precedent.

### 3.4 Wrapper surface (here)

`source/projects/tap.diode_tilde/` and `source/projects/tap.303_tilde/`, boilerplate
`CMakeLists.txt` with the `CXX_STANDARD 20` override, full vertical slices:
`docs/tap.NAME.maxref.xml`, `help/tap.NAME.maxhelp`,
`runtime-tests/patchers/tap.NAME.maxtest.maxpat`. `RELATED` metadata cross-links the
filter shelf and the `tap.808.*` family. The auto-discovering root `CMakeLists.txt`
needs no edits.

## 4. Phase 1 — the voice, in slices

Each slice independently shippable, ending with the full definition of done (§7.4).

- **Slice 0 — decisions + source sweep.** §8 sign-off; fetch and read the Service
  Notes, Stinchcombe's filter papers, the Devil Fish circuit description, and the
  x0xb0x schematics; verify the §9 links and sweep for any DAFx/JAES 303 analyses
  (the 808 papers' authors worked adjacent territory). Pin the accent-threshold
  convention and the exact panel-knob ↔ parameter ranges.
- **Slice 1 — the filter, end to end.** `diode_ladder.h` + kernel tests + bench
  entry + render demo, then `tap.diode~` with the full vertical slice. Ships a
  standalone-valuable external and de-risks the hardest DSP first, exactly as
  `bridged_t.h`/`tap.808.kick~` did for the 808.
- **Slice 2 — the plain voice.** `tb303_voice.h` core: saw + square shaper, MEG,
  VCA envelope, gate handling — the un-accented, un-slid voice through the slice-1
  filter; `tap.303~` wrapper + vertical slice. A correct-but-polite 303.
- **Slice 3 — accent + slide.** The coupling circuits: accent → MEG/VCA routing,
  the C13 accent-sweep with its across-notes memory (assert the wow), resonance
  scaling of the sweep, legato slide. **This slice is where the acid lives**; it
  ends with golden renders A/B'd against Open303 and real-unit recordings (§7.2).
- **Slice 4 — family polish.** Documented Devil-Fish bends, factory preset-morph
  slots, an overview help patcher sequencing `tap.303~` from a Max-patched step
  sequencer (dry run for the phase-3 interface), REVIVAL.md progress-log entries.

## 5. Phase 2 — fidelity upgrades

The `ladder.h` precedent already puts a `solver fast|exact` (Newton) choice in
slice 1, so the classic "phase 2" item is in phase 1. What remains, each gated by
A/B renders + the kernel bench harness (the 808 plan's go/no-go discipline):

- **Transistor VCA nonlinearity** (the informed model's VCA is linear-first).
- **Square-shaper exactness** — measured-curve waveshaper vs. the slice-2 model.
- **WDF pass on the filter** only if the informed model demonstrably falls short of
  the references at extreme settings; Stinchcombe's analysis suggests the informed
  model can get very close, so the expected outcome is "documented no-go".

## 6. Phase 3 — the sequencer (own object, deferred)

`tap.303.seq~`: a signal-clocked step sequencer emitting **exactly the voice's inlet
contract** — a pitch signal and a gate signal with amplitude-encoded accent, with
slide realized as gate-hold + stepped pitch (the voice's legato detection does the
rest). Per-step data: pitch, gate/rest, accent, slide, octave up/down. Pattern
storage via lists/messages + `dictionary` save/recall; transport sync optional.

Deliberately deferred until phase 1 has shipped and the note convention has survived
real patches — and coordinated with the `tap.808.seq~` phase-3 decision
(`plans/tap.808.md` §6, option (b) row-decomposition recommended there): if both
land, they should share the clocking idiom, and a 303 row driving a `tap.808.*`
voice (or vice versa) should Just Work. The original's pitch-mode/time-mode entry
UX is explicitly **not** recreated — we keep the *data model* (the flags), not the
1981 data-entry workflow.

## 7. Testing & validation

### 7.1 Kernel tests (Catch2, kernel repo CI)

- **Filter:** asymptotic 24 dB/oct slope (and the documented ≈18 dB/oct
  near-cutoff region), self-oscillation frequency accuracy at low *and* high cutoff
  (the `ladder.h` bar), resonance thinning at low cutoff (the feedback high-pass —
  assert ring-down Q drops as cutoff falls), solver fast/exact agreement at gentle
  settings + exact-solver boundedness at max everything, oversampled alias
  reduction, morph/sweep continuity.
- **Oscillator:** saw alias suppression (the `vco.h` bar), square-shaper spectrum
  distinct from an ideal 50% pulse in the documented way (even-harmonic content /
  notch signature).
- **Envelopes:** MEG decay-knob range endpoints, VCA envelope attack/decay/gate-off
  timing vs. schematic constants.
- **Accent:** accented vs. plain note — output level *and* cutoff-peak both rise;
  env-mod interaction; the **wow**: over a run of closely-spaced accented notes,
  assert the cutoff peak grows note-over-note, and decays back when accents stop;
  resonance scaling of the sweep.
- **Slide:** pitch settles with the RC time constant; neither envelope retriggers on
  a slid note; non-slid notes retrigger both.
- **House invariants:** bit-exact determinism per `seed`, decorrelation across
  seeds, denormal/NaN safety on long tails and self-oscillation.

### 7.2 Calibration against reality

Golden renders via the kernel render tools, compared in the notebook workflow
against (a) **Open303** rendering the same note patterns — an independent,
well-regarded reference implementation, the role the DAFx gen~ model plays for the
808 kick — and (b) real-unit recordings (303 sample packs and pattern renders are
abundant; unit variance is expected and is itself modeled by `seed`/`tolerance`).
Per-block deltas documented in the header comments, autowah-plan style.

### 7.3 Wrapper + runtime tests

min-api unittests for attribute ranges/defaults, `note` message plumbing, and
waveform switching (mock kernel, C++17 constraint applies); `*.maxtest.maxpat`
patchers asserting real behavior in Max — note-on → audio, accent louder than
non-accent, slide glides without retrigger — local on-Mac gate per
`runtime-tests/README.md`.

### 7.4 Per-slice definition of done

Kernel header + tests green in kernel CI → submodule pin bumped here → wrapper
builds universal on both platforms (CI `lipo` gate) → maxref + maxhelp + maxtest
patcher → golden render committed → REVIVAL.md progress log updated.

## 8. Pre-implementation questions (author decisions)

### Author decisions — ✅ the three blocking ones answered (author, 2026-07-17)

1. **Naming:** ✅ **approved** — `tap.303~` + a standalone `tap.diode~`, with
   `tap.303.seq~` reserved (`taptools::tb303` / `taptools::diode` as the internal
   namespaces; marks kept out of shipped object names only).
2. **Note interface:** ✅ **approved** — pitch-as-MIDI-note signal + gate signal with
   amplitude-as-accent + legato-as-slide, plus the `note <pitch> [accent] [slide]`
   message. This is the **package-wide melodic-voice contract**; phase 3 (and any
   future pitched voice) inherits it.
3. **Pitch units:** ✅ **MIDI note number** on inlet 1 (fractional = cents;
   conversion to Hz and the `tuning` trim happen inside the kernel).

### Still open (non-blocking, deferrable to their slices)

4. **Devil-Fish bends in v1** or stock-only? Proposed default accepted in principle,
   per the 808 precedent: ship the documented bends (`slide_time`, accent/normal
   decay split, `soft_attack`, input drive), stock-neutral defaults, skip
   speculative ones. Final list confirmed at slice 4 against the slice-0 sources.
5. **Square shaper:** reuse `vco.h`'s pulse + a post-shaper, or a dedicated
   measured-curve model in `tb303_voice.h`? (Slice-2 call, informed by the
   schematic and Open303's tables.)

## 9. Sources

> Links verified as of drafting where noted; slice 0 re-verifies all of them and
> pulls the schematic-level documents.

- Tim Stinchcombe, *A Study of the Roland TB-303 Filter* (+ his companion diode-
  ladder analyses) — <https://www.timstinchcombe.co.uk/> (the definitive circuit
  analysis of the VCF; slope, pole coupling, resonance behavior)
- Vadim Zavalishin, *The Art of VA Filter Design* — diode-ladder chapter (the ZDF
  treatment `diode_ladder.h` will follow) —
  <https://www.native-instruments.com/fileadmin/ni_media/downloads/pdf/VAFilterDesign_2.1.2.pdf>
- Robin Whittle, *Devil Fish* — TB-303 modification documentation with
  component-level descriptions of the envelope, accent (C13 sweep), and slide
  circuits — <https://www.firstpr.com.au/rwi/dfish/>
- Robin Schmidt, **Open303** — open-source reference implementation (calibration
  cross-check, §7.2) — <https://github.com/RobinSchmidt/Open303>
- Adafruit / Limor Fried, **x0xb0x** — reverse-engineered TB-303 schematics and
  build documentation — <https://www.ladyada.net/make/x0xb0x/>
- Roland TB-303 Service Notes (schematics; component values pulled at
  implementation time, per-constant provenance comments in the kernel headers)
