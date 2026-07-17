# Plan — `tap.808.*` (TR-808-style analog drum voices, and later a sequencer)

> Status: **slice 1 shipped 2026-07-17** (`tap.808.kick~`: `bridged_t.h` +
> `tr808_kick.h` in the kernel + the full vertical slice here — see the REVIVAL.md §7
> progress-log entry; the §8 blocking decisions are answered below). Remaining: runtime
> validation in Max, then slice 2 onward. Drafted 2026-07-17.
> Design + phased work breakdown for the first net-new object *family*:
> circuit-informed recreations of the Roland TR-808's analog drum voices, with a
> flagged upgrade path to full circuit simulation, and a potential later phase adding
> the sequencer as its own object(s). As with `tap.autowah~` (see
> `plans/tap.autowah~.md`), all DSP lands in the kernel repo (`tap/taptools`) first,
> then the Min wrappers + vertical slices here.

## 0. The three fidelity phases, up front

| Phase | What | Ships as |
|-------|------|----------|
| **1** | **Circuit-informed structured synthesis** — reproduce each voice's signal-flow topology from the schematic (pulse shapers, bridged-T resonators, oscillator banks, swing-VCAs), with component-derived constants, implemented as clean portable DSP | The `tap.808.*` voice externals — the plan of record |
| **2** | **Circuit simulation** (wave digital filters) for the voices where it audibly matters, starting with the bass drum | A `@circuit` attribute on the *same* objects (`informed`/`simulated`), following the `svf.h` `circuit_clean`/`circuit_driven` precedent — drop-in, no new externals |
| **3** | **Sequencer** — the 808's other half, as separate composable signal-rate objects | `tap.808.seq~` (+ possibly a clock object) — deliberately deferred until the voice trigger convention is proven in real patches |

Explicitly rejected: sample playback (that's `playlist~` territory, adds nothing) and
purely phenomenological synthesis (decaying sine + noise burst — sounds *like* an 808,
not *an* 808, and sits below the bar `vco.h`/`svf.h` set for documented,
physically-informed modeling).

## 1. Why this family, and why now

- **It fits the house style unusually well.** The kernel repo's flagship headers
  (`vco.h`, `svf.h`) are physically-informed analog models with documented provenance,
  deterministic per-`seed` component tolerances, and analytical header comments. The
  TR-808 is the drum-voice equivalent of exactly that genre — and it is one of the most
  thoroughly *academically analyzed* instruments in existence, so "port faithfully,
  document the provenance" (CLAUDE.md) has peer-reviewed sources to port from.
- **The whole machine is analog synthesis** — no samples anywhere in the hardware — so a
  faithful recreation is pure DSP, one self-contained header per voice, exactly the
  kernel repo's shape.
- **The family shares a small set of circuit blocks** (bridged-T resonators, a
  square-oscillator bank, noise paths, swing-VCAs). Sharing between kernels is
  explicitly encouraged inside the kernel repo, so ~11 voices reduce to ~4 shared blocks
  plus thin per-voice compositions.
- **It fills a real package gap**: TapTools has generators (`tap.noise~`, `tap.vco~`,
  `tap.random~`, `tap.pulsesub~`) and an envelope (`tap.adsr~`) but no percussion
  voices at all.
- **Prior art blesses the approach**: the definitive bass-drum analysis (Werner, Abel &
  Smith, DAFx-14) was itself implemented in Cycling '74's **gen~** — a Max-native
  reference model we can cross-check against.

## 2. Reference hardware — what the TR-808 actually is

Roland TR-808 Rhythm Composer (1980–1983): fully analog voice circuits plus a 16-step
sequencer with a shared **accent bus**. Facts below verified 2026-07-17 against the
owner's manual / Roland's published specs and the Werner–Abel–Smith papers (links §9);
schematic-level constants come from the service manual at implementation time.

### 2.1 Voice channels and panel controls

The hardware has 16 sounds on 11 voice channels (5 channels have a hardware switch
selecting between two related sounds sharing one circuit):

| Channel | Sounds | Panel controls |
|---------|--------|----------------|
| BD | Bass drum | level, **tone**, **decay** |
| SD | Snare drum | level, **tone**, **snappy** |
| LT / LC | Low tom ⇄ low conga | level, **tuning** |
| MT / MC | Mid tom ⇄ mid conga | level, **tuning** |
| HT / HC | Hi tom ⇄ hi conga | level, **tuning** |
| RS / CL | Rim shot ⇄ claves | level |
| CP / MA | Handclap ⇄ maracas | level |
| CB | Cowbell | level |
| CY | Cymbal | level, **tone**, **decay** |
| OH | Open hi-hat | level, **decay** |
| CH | Closed hi-hat | level |
| AC | Accent (not a sound — the bus) | level |

### 2.2 The five circuit families

1. **Bridged-T resonators** — *the* 808 building block: a damped RC resonator excited
   by a shaped trigger pulse, used by BD, SD (twice), all toms/congas, RS, and CL. The
   BD adds a feedback network that raises the effective Q and a *retriggering pulse*
   that re-excites the resonator; the DAFx-14 paper untangles the resulting behaviors —
   notably that the famous **pitch "sigh" and the attack-transient frequency jump are
   two different mechanisms**, and that device nonlinearity matters less than folklore
   claims. Toms mix in filtered noise on top of the resonator; congas are the same
   circuit without the noise, tuned differently.
2. **The metal bank** — six fixed square-wave oscillators shared (on the schematic) by
   CY, OH, CH, and CB. Measured fundamentals span roughly **254–627 Hz**, and —
   critically for us — **resistor variance puts any given unit up to ~20% off** those
   values (Werner et al.), which is why no two 808s' cymbals sound alike. The bank
   feeds two bandpass stages (~3.4 kHz and ~7.1 kHz regions) and then per-voice
   envelope/VCA paths: CH a short fixed decay, OH a variable decay, CY two separately
   enveloped bands (the long "sizzle"). CB taps just the bank's two trimmer-tunable
   oscillators into a bandpass and a fast decay. On hardware **CH chokes OH** — the two
   hats can never sound together.
3. **Noise paths** — SD's "snappy" (enveloped noise mixed with the two resonators), CP
   (bandpassed noise through a VCA driven by a sawtooth-like retrigger producing ~3
   rapid pulses — the "multiple hands" transient — plus a parallel slower noise decay),
   and MA (high-passed noise burst with a very fast envelope).
4. **Swing-type VCAs and RC decay envelopes** — the percussive gain shapes are simple
   discharge curves and one-transistor VCAs, not ADSRs.
5. **The accent bus** — a shared voltage, scaled by the AC level knob, that raises the
   trigger pulse amplitude of every voice sounding on an accented step (and thereby,
   on some voices, subtly shifts timbre — the excitation is hotter, not just louder).

## 3. Design

### 3.1 Naming and trademark

- **Family prefix `tap.808.*`**, matching the existing `tap.fft.*` / `tap.jit.*`
  dot-family convention: `tap.808.kick~`, `tap.808.snare~`, `tap.808.tom~`,
  `tap.808.rim~`, `tap.808.clap~`, `tap.808.hat~`, `tap.808.cymbal~`,
  `tap.808.cowbell~`, and later `tap.808.seq~`.
- "Roland" and "TR-808" are trademarks. Keep both **out of shipped object names**;
  the bare number "808" is the established generic (808 State, "808s & Heartbreak",
  countless clones). Docs/maxrefs may say "inspired by the Roland TR-808" —
  descriptive/nominative use. Source-level identifiers (namespace, header names) can
  use `tr808` for clarity; they ship to developers, not into Max's object palette.
- One external per **hardware voice channel**; the hardware's switchable pairs become a
  **mode attribute** on one object (`tap.808.tom~ @model tom|conga`,
  `tap.808.rim~ @model rimshot|claves`, `tap.808.clap~ @model clap|maracas`). This
  mirrors the machine, halves the object count, and the pairs genuinely share a circuit.
- The two hats are **one object**, `tap.808.hat~`, with *two trigger inlets* (closed,
  open) — on hardware they are one circuit with two envelope paths and a choke, and the
  choke is unimplementable across separate externals (no shared global state in
  `source/projects/`, per CLAUDE.md).

### 3.2 Interface conventions (uniform across the family)

- **Signal-rate trigger inlet.** A rising edge above a small threshold fires the voice;
  **the sample value at the edge is the trigger amplitude** — velocity and accent in
  one number, which is literally how the hardware's accent bus works (a voltage summed
  onto the trigger pulse). Sample-accurate, `mc.`-friendly, and a `click~`/`train~`/
  future-`tap.808.seq~` output drives it directly.
- **`bang` convenience message** = trigger at amplitude 1.0 at the next vector boundary;
  `trigger <float>` message for an accented bang. Help patchers use bangs; serious
  patches use signals.
- **Attributes mirror the panel knobs**, normalized 0..1 (`level`, `tone`, `decay`,
  `snappy`, `tuning` — per §2.1, only where the hardware has the knob). Defaults = knobs
  at noon.
- **`seed` + `tolerance`** (0..1): deterministic per-unit component spread, exactly the
  `vco.h` `imperfect` idea — justified by the documented ~20% resistor variance on the
  metal bank. `tolerance 0` = the nominal schematic; every seed is a different unit off
  the line, and an `mc.` stack of cymbals decorrelates like real hardware.
- **Circuit-bent extensions** (the papers are explicitly "circuit-bendable": bend points
  like the BD's decay-feedback resistor or the bank's oscillator tuning): exposed as
  clearly-marked beyond-hardware attributes, defaulting to stock values. In scope for
  phase 1 where the papers document them (BD, CY, CB); otherwise deferred.
- Single-channel objects (`sample_operator<1, 1>`; the hat is `<2, 1>`); multichannel
  via `mc.` per package convention. C++20 wrappers, DSP in C++17-clean kernel headers.

### 3.3 Kernel surface (in `tap/taptools`)

Shared blocks, each its own header (shared between kernels is encouraged there):

| Header | Contents |
|--------|----------|
| `bridged_t.h` | The bridged-T resonator: trigger-pulse shaping, the damped resonator, the BD-style feedback/Q-enhancement and retriggering-pulse options |
| `metal_bank.h` | The six-square-oscillator bank (naive squares are *faithful* here — fixed low frequencies, then bandpassed; polyBLEP is unnecessary), the two bandpass voicings, per-unit tolerance spread |
| `swing_vca.h` | Swing-type VCA + the RC decay envelope shapes |
| `drum_noise.h` | The noise source + the clap's pulse-train VCA driver (or fold these into the voices if they stay small) |

Per-voice headers composing them: `tr808_kick.h`, `tr808_snare.h`, `tr808_tom.h`,
`tr808_rim.h`, `tr808_clap.h`, `tr808_hat.h`, `tr808_cymbal.h`, `tr808_cowbell.h` —
namespace `taptools::tr808`, each with the house `prepare(sr)` / `trigger(amp)` /
per-sample `process()` shape, header-only, stdlib-only, deterministic per seed.
Bandpass/lowpass stages reuse `svf.h` internally where the response matches the
schematic's filters; otherwise small dedicated biquads with the component derivation in
a comment.

All magic constants carry provenance comments (schematic reference designators and/or
paper section), per the `tap.dcblock~` precedent.

### 3.4 Wrapper surface (here)

One folder per object under `source/projects/` (`tap.808.kick_tilde/` →
`tap.808.kick_tilde.cpp`), boilerplate `CMakeLists.txt` with the `CXX_STANDARD 20`
override, `MIN_DESCRIPTION/TAGS/AUTHOR/RELATED` metadata, and the full vertical slice:
`docs/tap.808.NAME.maxref.xml`, `help/tap.808.NAME.maxhelp`, and a
`runtime-tests/patchers/tap.808.NAME.maxtest.maxpat`. The auto-discovering root
`CMakeLists.txt` needs no edits.

## 4. Phase 1 — the voices, in slices

Each slice is independently shippable and ends with the full definition of done (§7.4).

- ✅ **Slice 0 — decisions + scaffolding** (2026-07-17). §8 sign-off + `bridged_t.h`
  with tests. (`swing_vca.h` deferred to slice 2: the kick has no swing VCA, and the
  snare/clap analysis should drive that design.)
- ✅ **Slice 1 — the kick, end to end** (2026-07-17). `tr808_kick.h` (pulse shaper →
  retrigger → bridged-T with feedback → tone LPF → level), ported from the DAFx-14
  analysis with component values read off the Service Notes schematic in the paper's
  Fig. 1; then `tap.808.kick~` + wrapper tests + maxref + help + maxtest patcher.
  Validated the shared-block design, the trigger/accent convention, and the test
  workflow. Still open: runtime validation in Max (§7.3) and golden renders /
  sample-pack comparison (§7.2).
- ✅ **Slice 2 — snare + clap channel** (2026-07-17). `swing_vca.h` (decay_env, linear
  swing-VCA, seeded white noise), `tr808_snare.h` (two bridged-Ts at the late-revision
  ~173/336 Hz with the Roland design change documented, trigger divider, ~4 kHz snappy
  path), `tr808_clap.h` (`@model clap|maracas`; schematic-exact ~2 kHz dual band-pass,
  Figure-13 three-teeth envelope, Q70 reverberation tail + tail bend). Primary source:
  the Service Notes themselves (p.6 descriptions + Fig. 13, p.9 schematic, p.14 voice
  chart) — fetched and read directly, giving every component value. Vertical slices
  `tap.808.snare~` / `tap.808.clap~` shipped. Still open: runtime validation in Max,
  golden renders (§7.2).
- ✅ **Slice 3 — the metal voices** (2026-07-17). `metal_bank.h` (the six Schmitt
  squares at 205.3/369.6/304.4/522.7 + trimmed 800/540 Hz, duty 47.98%, per-seed ±20%
  `tolerance` spread, the ~3440/7100 Hz voicings, the Q19 attack smoother — constants
  from the Werner/Abel/Smith cymbal paper + the voicing-board schematic), then
  `tap.808.hat~` (one object, two trigger inlets, the hardware Q23/R173 choke pinned
  by test), `tap.808.cymbal~` (strike/ring/body over the two bands; chart's 350-1200 ms
  decay span), `tap.808.cowbell~` (the 540/800 pair, two-slope envelope, ~860 Hz IC2
  voicing). Still open: runtime validation in Max, golden renders (§7.2).
- ✅ **Slice 4 — the resonator variations** (2026-07-17). `tap.808.tom~` (`@size
  low|mid|high` × `@model tom|conga` = the hardware's six sounds; chart tunings with
  the panel knob sweeping each span; the D80/D81 attack pitch fall; the tom-only
  pink-noise layer — pinned by seed-sensitivity, since the diode bend's own harmonics
  defeat spectral separation) and `tap.808.rim~` (`@model rimshot|claves`; the
  ~1667+~455 Hz crack with the VCA's tanh harmonics vs the pure ~2500 Hz tick).
  **All eight voice channels of the family are now shipped.** Still open: runtime
  validation in Max, golden renders (§7.2).
- **Slice 5 — family polish.** Accent-bus conventions documented once and
  cross-referenced from every maxref; an overview help patcher wiring all voices to a
  drum-pattern demo; `RELATED` metadata linking the family; REVIVAL.md progress-log
  entries.

## 5. Phase 2 — circuit simulation (the fidelity upgrade)

**Goal:** capture what the informed model can't — component-level interaction under
unusual settings, arbitrary circuit bends, unit-to-unit behavior beyond the modeled
tolerances — using **wave digital filters** over the actual schematic.

- Kernel gets a small WDF toolkit (`wdf.h`): one-port elements (R, C, sources),
  series/parallel adaptors, a nonlinear root (diode/BJT), following Werner's Stanford
  dissertation lineage (he analyzed this exact machine).
- **Bass drum first and possibly only.** The DAFx-14 paper provides the full circuit
  analysis to validate against; the kick is where fidelity arguments live.
- Ships as **`@circuit informed|simulated` on the existing `tap.808.kick~`** — the
  `svf.h` two-circuit pattern. Same attributes, same trigger interface, drop-in.
- **Go/no-go gate:** proceed past the kick only if A/B renders (kernel render tools,
  §7.2) show an audible delta that justifies the CPU (measured with the kernel bench
  harness). If the informed model already nails it — a real possibility per the paper's
  own findings on nonlinearity folklore — phase 2 stops there, documented.

## 6. Phase 3 — the sequencer (potential, own objects)

The hardware's other half: 16 steps × A/B pattern halves, basic/fill patterns with
chaining, 12 instrument rows + the accent row, and "pre-scale" step resolutions
(including triplet grids). Deliberately **deferred until phase 1 has shipped** and the
trigger convention has survived real patches.

Design options, with a recommendation:

- **(a) Event-domain step sequencer** clocked by the Max transport. Easiest, but
  trigger timing quantizes to the scheduler — a mismatch with the voices'
  sample-accurate signal inlets.
- **(b) Small composable signal-rate objects — recommended.** `tap.808.seq~` is *one
  row*: a 16-step trigger sequencer with a signal clock/phase input, per-step on/off +
  accent, emitting **impulses whose amplitude encodes accent** — which plugs directly
  into any `tap.808.*` trigger inlet (or anything else: envelopes, `tap.adsr~`, etc.).
  A companion master (`tap.808.clock~`, or possibly just documented patterns around
  `phasor~`/transport) provides the step clock, pre-scale, and A/B–fill switching.
  Twelve rows = twelve small objects in a patch — decomposed, very Max-ish, and each
  piece is useful outside the 808 context.
- **(c) One monolithic multi-row object** mirroring the hardware panel. Faithful but
  un-Max-like (12+ signal outlets, a huge attribute surface) and duplicates what Max
  patching already does well.

Pattern storage via lists/messages plus `dictionary` save/recall; transport sync
optional, never required. Final (a)/(b)/(c) call is an author decision at phase-3
kickoff, informed by how phase-1 patches actually get built.

## 7. Testing & validation

### 7.1 Kernel tests (Catch2, kernel repo CI)

Per voice: trigger → nonzero energy with the expected decay slope; the kick's dominant
frequency trajectory (assert the pitch sigh *and* the distinct attack jump — the
DAFx-14 paper's own analytical results make these quantitative); the clap's pulse count
and spacing; the metal bank's spectral peaks and their tolerance spread bounds; CH
choking OH; bit-exact determinism for a fixed seed; denormal/NaN safety on long tails;
accent amplitude actually scaling excitation (not just output gain).

### 7.2 Calibration against reality

Golden renders via the kernel repo's render tools, compared in the notebook workflow
against reference recordings of real units (freely available 808 sample packs are
abundant; unit-to-unit variance is expected and is itself modeled). Per-voice deltas
documented in the header comment, autowah-plan style. The gen~ model from the DAFx-14
paper is a second, independent cross-check for the kick.

### 7.3 Wrapper + runtime tests

min-api unittests for attribute ranges/defaults and message plumbing (mock kernel,
C++17 constraint applies); a `*.maxtest.maxpat` per object asserting real behavior in
Max (trigger → audio, choke behavior, accent scaling) — local on-Mac gate per
`runtime-tests/README.md`.

### 7.4 Per-slice definition of done

Kernel header + tests green in kernel CI → submodule pin bumped here → wrapper builds
universal on both platforms (CI `lipo` gate) → maxref + maxhelp + maxtest patcher →
golden render committed → REVIVAL.md progress log updated.

## 8. Pre-implementation questions (author decisions)

### Author decisions — ✅ the three blocking ones answered (author, 2026-07-17)

1. **Naming:** ✅ **`tap.808.*` approved** (`taptools::tr808` as the internal namespace;
   marks kept out of shipped object names only).
2. **Switch pairs as mode attributes:** ✅ **approved** (`@model tom|conga` etc., one
   external per hardware voice channel).
3. **Trigger convention:** ✅ **approved** — signal-rate edge with amplitude-as-accent
   (mapped to the hardware's 4–14 V trigger bus), plus `bang`/`trigger <float>`
   messages. This is the family-wide (and future sequencer) contract, implemented
   first in `tap.808.kick~`.

### Still open (non-blocking)

4. **Circuit-bent attributes in v1** or deferred? *De facto* answered for the kick —
   the paper-documented bends shipped as stock-neutral attributes (`tuning`, `pulse`,
   `sigh`, `attack`) and it cost little; proposed default for later voices: ship the
   documented bends, skip speculative ones.
5. **Phase 3 shape** — (b) row-object decomposition acceptable in principle, or is a
   monolithic hardware-mirror sequencer the actual wish? (Deferred to phase-3
   kickoff.)

## 9. Sources

- Werner, Abel & Smith, *A Physically-Informed, Circuit-Bendable, Digital Model of the
  Roland TR-808 Bass Drum Circuit*, DAFx-14 —
  <https://www.dafx.de/paper-archive/2014/dafx14_kurt_james_werner_a_physically_informed,_ci.pdf>
- Werner, Abel & Smith, *The TR-808 Cymbal: a Physically-Informed, Circuit-Bendable,
  Digital Model*, 2014 —
  <https://pureadmin.qub.ac.uk/ws/portalfiles/portal/125044847/tr_808_cymbal_a_physically_informed_circuit_bendable_digital.pdf>
- Werner, Abel & Smith, *Three Models of Circuit-Bent TR-808 Voices: the Bass Drum,
  Cymbal, and Cowbell*, 2014 —
  <https://www.researchgate.net/publication/280534006_Three_Models_of_Circuit-Bent_TR-808_Voices_the_Bass_Drum_Cymbal_and_Cowbell>
- Roland TR-808 owner's manual & published technical specifications (panel controls,
  voice channels) — <https://support.roland.com/hc/en-us/articles/201963539-TR-808-Technical-Specifications>
- TR-808 service manual schematics (component values — pulled at implementation time,
  per-constant provenance comments in the kernel headers)
- Baratatronix, *Roland TR-808 Cymbal & Hi-Hat Synthesis* (oscillator bank + bandpass
  analysis) — <https://www.baratatronix.com/blog/cascadia-808-cymbal-hi-hat-synthesis>
