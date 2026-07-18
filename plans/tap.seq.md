# Plan — `tap.808.seq~` / `tap.303.seq~` (the shared step-sequencer engine)

> Status: **shipped 2026-07-18** — drafted, §6-approved by the author, and implemented
> the same day (slices 0–3; see the REVIVAL.md §7 entry). The engine lives in the kernel
> repo as `step_seq.h` (`taptools::seq`: the phase→step engine + `trigger_row` +
> `note_row`, 19 test scenarios including the tb303-voice pairing); the wrappers,
> unittests, maxrefs, help patchers, and maxtest starters are here, and
> `help/tap.808.maxhelp` is rebuilt on real rows. Slice-0 pins: 808 trigger levels
> plain 0.01 / accented 0.5 (the 4–14 V bus mapping), 303 gate duty 0.5 (Open303
> `AcidPattern::stepLength`), slide flag on the *target* step (the package `note`
> convention; the hardware's source-note storage documented as a divergence in the
> kernel header). The non-blocking recommendations (§6 Q4–Q7) all shipped as
> recommended. The kernel repo also carries **`notebooks/step_seq.ipynb`** — the
> executed, asserted verification notebook (C ABI extended with the rows + a minimal
> kick), ending in four bars of acid + kick rendered from the shipping voices off one
> phasor ramp. Remaining: **runtime validation in Max** (the maxtest starters want
> extending on a licensed install), per the package-wide gate.
>
> Original preconditions, for the record: both voice families shipped (808 slices 0–5 +
> §7.2 calibration; 303 phases 1–2 + the approved WDF no-go), the contracts proven in
> their family patchers, and the `help/tap.303~-pattern.maxpat` dry run.

## 0. The headline answer, up front

**Can the 303 and 808 sequencers be shared? Yes — at the kernel level, not the object
level.** Everything hard is common: deriving the current step from a signal clock with
sample accuracy, swing, pattern data storage and slots, run/wrap logic, determinism. What
differs is only the per-step *payload* and how a step is *emitted* as signal:

| | `tap.808.seq~` (one drum row) | `tap.303.seq~` (one bass line) |
|---|---|---|
| Per-step data | on/off + accent (velocity) | pitch, gate/rest, accent flag, slide flag |
| Output | **1 signal outlet**: trigger impulses, edge amplitude 0..1 = accent — the `tap.808.*` trigger contract verbatim | **2 signal outlets**: pitch (MIDI note number) + gate with amplitude 1 (plain) / 2 (accented); slide = gate held across the boundary while pitch steps — the `tap.303~` inlet contract verbatim |
| Consumes | any `tap.808.*` voice, `tap.adsr~`, envelopes, anything edge-triggered | `tap.303~` (and any future melodic voice honoring the package contract) |

So: **one kernel header, `step_seq.h` (`taptools::seq`) — a clock/step engine plus two
thin emitters — and two thin Min wrappers**, one per family. Sharing between kernels is
exactly what the kernel repo is for; separate Max objects keep each family's surface,
help patchers, and `RELATED` graph coherent, and the two objects' outlet counts and
amplitude conventions genuinely differ. This is option **(b)** from `plans/tap.808.md`
§6 (row decomposition, recommended there), unified with the `tap.303.seq~` emitter the
303 plan §6 specified.

Explicitly rejected:

- **One generic `tap.seq~` object serving both.** The two contracts differ in arity and
  in accent encoding (808: absolute 0..1; 303: 1 + depth); a merged object needs mode
  switches that change its outlet count — un-Max-like. (The engine *is* generic; a
  generic third wrapper can always be added later as another thin emitter. Deferred,
  §8 Q5.)
- **A monolithic multi-row hardware-mirror** — option (c) in the 808 plan, rejected
  there for good reasons (12+ signal outlets, duplicates Max patching).
- **Recreating the 1981 entry workflows** — the 808's basic/fill chaining panel logic
  and the 303's pitch-mode/time-mode data entry. We keep the *data model* (steps with
  flags), not the data-entry UX. Both plans already pinned this.
- **An event-domain (scheduler-clocked) sequencer** — option (a): trigger timing would
  quantize to the scheduler, a mismatch with the voices' sample-accurate signal inlets.
  The `note`-message dry-run patcher already demonstrates the event-domain idiom for
  anyone who wants it.

## 1. Why this shape

- **Both §6 sections demanded coordination**: "if both land, they should share the
  clocking idiom, and a 303 row driving a `tap.808.*` voice (or vice versa) should Just
  Work." One engine makes the shared idiom structural rather than aspirational.
- **The voice contracts were designed for this.** The 808 voices take a signal edge
  whose amplitude is the accent (the shared 4–14 V trigger bus); `tap.303~` takes a
  pitch signal + a gate signal with amplitude-as-accent and legato-as-slide. The
  sequencer rows emit exactly those signals — no adapter objects, no new conventions.
- **Rows are useful beyond the families**: a trigger row drives `tap.adsr~`, `click~`
  chains, anything with an edge input; a pitch/gate pair drives any future melodic
  voice honoring the package contract (`plans/tap.303.md` §8 decision 2 made it
  package-wide).
- **Cross-family patching, honestly stated**: a 303 row's gate edge (amplitude ≥ 1)
  triggers any 808 voice — the voice clamps accent to 0..1, so it fires at full accent;
  musically useful (a bass line doubling a kick). The reverse (an 808 row's one-sample
  impulse into the 303 gate) triggers but gives a one-sample note — the 303 needs held
  gates for note length, so that direction is documented as not the idiom.

## 2. Reference hardware — the sequencer halves

Facts from the owner's manuals / service notes already fetched for the voice phases;
remaining timing constants (gate duty, trigger levels) pinned at slice 0 with provenance
comments, per house rule.

- **TR-808**: 16 steps × A/B pattern halves, 12 instrument rows plus the **accent row**
  (a shared bus raising every sounding voice's trigger voltage on accented steps, scaled
  by the AC level knob), basic/fill patterns with chaining, and "pre-scale" step
  resolutions including triplet grids. No swing on the hardware.
- **TB-303**: 16-step patterns; per step: note, gate/rest, **accent flag**, **slide
  flag**, octave up/down transposition. Slide = the sequencer holds the gate across the
  step boundary while the pitch CV glides (~60 ms RC — already in the voice); accent =
  a hotter CV into the coupled accent circuitry (already in the voice). The famously
  opaque entry workflow is not part of the data model.

Mapping to the design: the accent row and AC knob become per-row accent data plus the
voices' own accent handling (the bus is a patch cord now); A/B halves, fills, chaining,
and pre-scale become **pattern slots + boundary-quantized recall + the clock rate**
(§3.3); the 303's octave up/down flags are subsumed by pitch-as-MIDI-float (plus a live
`transpose` message, which is what the hardware's transpose actually did).

## 3. Design

### 3.1 The clocking idiom (the load-bearing shared decision)

**Phase-driven: one 0..1 ramp per pattern cycle, from `phasor~`.** The row derives the
current step as `floor(phase × length)` (swing warps the boundaries, §3.3), detecting
boundary crossings per sample.

Why phase, not a pulse clock:

- **Sample-accurate and tempo-agnostic** — `phasor~ @lock 1` (or `phasor~` off the
  transport) gives bar-locked ramps for free; the row never owns tempo.
- **Every row locked to one phasor stays phase-coherent forever** — no drift, no
  reset choreography; mute/unmute a row and it re-enters in place. This is the
  multi-row story: twelve rows = twelve objects fed by one patch cord.
- **Different `length` values off the same ramp are polymeter for free**; a
  `length 12` row against `length 16` rows is the 808's triplet pre-scale
  (and more than the hardware allowed).
- **Position is explicit** — jumping the phasor jumps the pattern; reversing or
  scrubbing the phase does the obvious thing. A pulse clock would need a separate
  reset/position protocol.

**No `tap.808.clock~` object in v1.** `phasor~` (+ the transport) already is the master
clock; the 808 plan left the master as "possibly just documented patterns around
`phasor~`/transport" — that's the recommendation. Every help patcher documents the
idiom (`phasor~ 1.8125` ≈ 116 BPM bars, or transport-locked); if real patches later
want A/B-fill chaining logic in an object, it can be added without touching the rows.

### 3.2 The two objects

**`tap.808.seq~`** — one trigger row (`sample_operator<1, 1>` + a step-index event
outlet, §3.4):

- Inlet: phase signal. Outlet 1: trigger impulses at (swung) step starts — a
  single-sample edge whose amplitude is the step's accent value 0..1 (the family
  contract; every `tap.808.*` voice fires on it). An optional `pulse` attribute (ms,
  default 0 = single-sample) widens the impulse into a held gate for driving
  `tap.adsr~`-style consumers.
- Per-step data is a **velocity 0..1** (0 = rest) — a superset of the hardware's
  on/accent pair. Convenience messages set the hardware view: `hits 1 0 0 1 ...`
  (on/off list) and `accents 0 0 1 0 ...` map onto velocities `plain`/`accented`,
  two attributes defaulting to the hardware's un-accented/accented trigger-bus levels
  (exact values pinned at slice 0 from the 4–14 V bus mapping already in the voices).
- The hardware's accent row falls out: rows own their accents; a patch that wants the
  shared-bus feel sets the same `accents` list on every row.

**`tap.303.seq~`** — one bass line (phase in; pitch + gate signal outlets + step-index
event outlet):

- Per-step data: `pitch` (MIDI note number, float — fractional = cents, subsuming the
  hardware's octave flags), `gate` on/rest, `accent` flag, `slide` flag.
- Plain step: gate rises to 1.0 at the (swung) step start, falls at `gatetime` (a
  fraction of the step; stock value pinned at slice 0 from the service notes).
  Accented step: gate rises to 2.0 (the voice reads accent depth = amplitude − 1; the
  *amount* stays on the voice's accent knob, exactly like the hardware). Rest: gate
  stays low.
- **Slide step: the gate does not fall before it** — it holds through the boundary
  while the pitch outlet steps to the new note; the voice's legato detection does the
  glide and skips the retrigger. Chained slides chain; a slide from the last step into
  the first works across the phase wrap. This is the hardware behavior verbatim
  (§2.2.6 of the 303 plan) and costs the engine one flag.
- `note`-shaped setters mirror the voice's message: `step 3 45 1 0` (index, pitch,
  accent, slide), plus whole-row lists (`pitches`, `gates`, `accents`, `slides`) and a
  live `transpose <semitones>` message.

### 3.3 Shared engine features (both objects, one implementation)

- **`length` 1..64** (default 16), changeable while running (takes effect immediately;
  the step derivation is stateless in phase, so no glitch states exist).
- **`swing` 0..1** (default 0): off-numbered steps delayed by up to a half step
  (0 = straight, ~0.67 ≈ classic triplet shuffle), implemented as boundary warping in
  the step derivation. Beyond-hardware (neither machine had it), so it defaults off —
  the "documented bends, stock defaults" posture.
- **Pattern slots**: 16 slots with `store <n>` / `recall <n>`, mirroring the voices'
  preset idiom. **`recall` is quantized** by a `quantize cycle|step|now` attribute
  (default `cycle`): the recalled pattern arms and swaps at the next cycle (or step)
  boundary — which *is* the 808's A/B-half and basic/fill switching, patchable from
  one message box. Pattern storage/restore via Max `dictionary` (`dict` in/out
  messages) for saving with the patcher, per both plans' §6.
- **Run semantics**: the row is stateless against phase — no run/stop of its own
  (stop the phasor, or `mute` the row). Forward-only in the data model; play the
  phasor backwards and the row follows (documented, tested, free).
- House invariants: allocation-free after `prepare()`, setters safe while audio runs,
  bit-exact determinism (no randomness in this object — determinism is trivial but
  still pinned by test), denormal-free (trivially — the engine emits constants).

### 3.4 Kernel surface (in `tap/taptools`)

| Header | Contents |
|--------|----------|
| `step_seq.h` | `taptools::seq`: the phase→step engine (boundary detection, swing warp, length, wrap) over a fixed-capacity `pattern` store (64 steps × the union payload: velocity, pitch, gate, accent, slide) with slots and quantized recall arming; two emitters composing it — `trigger_row` (impulse/pulse output, the 808 contract) and `note_row` (pitch + gate output with gate-hold slide, the 303 contract) |

C++17-clean, stdlib-only, header-only, no allocation after `prepare()` — the house
shape. The emitters are what the wrappers instantiate; the engine itself is reusable
for any future row flavor (a CV row, a probability row) without new clock code.

### 3.5 Wrapper surface (here)

`source/projects/tap.808.seq_tilde/` and `source/projects/tap.303.seq_tilde/`,
boilerplate `CMakeLists.txt` with the `CXX_STANDARD 20` override, full vertical slices
(`docs/*.maxref.xml`, `help/*.maxhelp`, `runtime-tests/patchers/*.maxtest.maxpat`).
Each wrapper adds a **rightmost non-signal outlet emitting the current step index**
(deferred to the scheduler) for UI feedback (`live.grid`, step displays) — Min glue
only, not engine state. `RELATED` cross-links each family and the other row. The
family overview patchers get upgraded: `help/tap.808.maxhelp`'s metro/counter/sel
pattern section is rebuilt on `phasor~` + eight `tap.808.seq~` rows (the accent-bus
comment becomes a live demonstration), and `help/tap.303~-pattern.maxpat` is
superseded by a `phasor~` + `tap.303.seq~` patcher (kept: the coll-based patch stays
as the event-domain alternative it demonstrates).

## 4. Work breakdown, in slices

Each slice independently shippable, ending with the full definition of done (§5.4).

- ✅ **Slice 0 — decisions + constants** (2026-07-18). §8 sign-off. Pin from the service notes (both
  already in hand from the voice phases): the 808 trigger-bus plain/accented levels as
  emitted 0..1 amplitudes (consistent with the voices' 4–14 V mapping), and the 303's
  gate duty fraction (`gatetime` stock value). Pin the swing convention and the
  quantized-recall semantics in the header comment.
- ✅ **Slice 1 — the engine + the drum row, end to end** (2026-07-18). `step_seq.h` (engine +
  `trigger_row`) + kernel tests (no bench entry — the engine is a handful of arithmetic
  ops per sample, below the bench harness's noise floor), then `tap.808.seq~` with the full
  vertical slice, and the `tap.808.maxhelp` overview rebuilt on real rows. De-risks
  the clock/step derivation (the only genuinely new machinery) on the simpler emitter.
- ✅ **Slice 2 — the bass line** (2026-07-18). `note_row` (gate-hold slide, accent amplitudes,
  gatetime) + kernel tests, then `tap.303.seq~` with the full vertical slice, the
  successor pattern patcher, and a cross-family demo patch (one phasor, a 303 line +
  808 rows; the 303 row's gate doubling the kick). The slide-across-the-wrap and
  chained-slide cases get explicit tests.
- ✅ **Slice 3 — patterns as data** (2026-07-18). Slots + quantized recall + `dictionary` save/recall
  on both objects (one implementation, two surfaces), the A/B-fill idiom documented in
  both help patchers, REVIVAL.md progress-log entries, and the two family plans' §6
  sections marked resolved (pointing here).

## 5. Testing & validation

### 5.1 Kernel tests (Catch2, kernel repo CI)

- **Step derivation**: boundary samples land exactly where analytic
  `floor(phase·length)` says, across lengths 1..64, at multiple sample rates and
  phasor frequencies; wrap-around correctness; length changes mid-cycle; reversed
  phase.
- **Swing**: off-step boundaries delayed by the exact warp; swing 0 is bit-identical
  to no swing path.
- **Trigger row**: impulse amplitude = per-step velocity; rests emit nothing; `pulse`
  widths; a full-velocity step following a rest produces a clean below-threshold gap
  (the voices' edge detectors re-arm).
- **Note row**: gate rises/falls at the pinned duty; accent steps emit 2.0; **slide
  holds the gate across the boundary with no falling edge** (assert against a
  `tb303_voice.h` instance: the slid note does not retrigger and glides — engine and
  voice tested as the pair they are); chained slides; slide across the wrap.
- **Slots/recall**: quantized swap happens on the boundary sample, `now` is
  immediate, armed recalls survive length/swing changes.
- **House invariants**: determinism, allocation-free processing, setters during audio.

### 5.2 Wrapper + runtime tests

min-api unittests for attribute defaults/clamping and list/`step` message plumbing
(mock kernel, C++17 constraint). `*.maxtest.maxpat` patchers asserting real behavior in
Max: phasor in → triggers out at the right count per cycle, accent amplitudes, the 303
row driving `tap.303~` (slid step audibly glides — no retrigger), step-index outlet
firing. Local on-Mac gate per `runtime-tests/README.md`.

### 5.3 Calibration

Nothing to calibrate against a unit — the sequencer is logic, not circuitry; the two
pinned constants (trigger levels, gate duty) carry service-notes provenance comments
and are asserted by test. The musical validation is the upgraded family patchers
playing the same patterns they played before, now sample-accurately.

### 5.4 Per-slice definition of done

Kernel header + tests green in kernel CI → submodule pin bumped here → wrapper builds
universal on both platforms (CI `lipo` gate) → maxref + maxhelp + maxtest patcher →
REVIVAL.md progress log updated.

## 6. Pre-implementation questions (author decisions)

### Author decisions — ✅ the three blocking ones answered (author, 2026-07-18)

1. **The shape:** ✅ **approved** — one kernel engine (`step_seq.h`) + two family
   wrappers (`tap.808.seq~` = one trigger row, `tap.303.seq~` = one pitch/gate line),
   per §0 — i.e. 808-plan option (b) unified with the 303 emitter. (This also
   resolves `plans/tap.808.md` §8 Q5.)
2. **The clocking idiom:** ✅ **approved** — phase-driven (one 0..1 ramp per pattern
   cycle from `phasor~`), **no clock object in v1**; the master clock is a documented
   `phasor~`/transport idiom.
3. **Pattern slots + quantized recall:** ✅ **approved** — 16 slots, `quantize`
   defaulting to `cycle`, as the A/B-half / fill / chaining mechanism, with
   `dictionary` save/recall.

### Still open (non-blocking, accepted in principle — confirmed at their slices)

4. **Per-step velocity vs. hardware flags** on `tap.808.seq~`: stored velocity 0..1
   with `hits`/`accents` list conveniences mapping onto `plain`/`accented` levels
   (slice-1 confirmation).
5. **A generic third wrapper (`tap.seq~`)** exposing the engine for non-family uses —
   deferred until someone wants it; the kernel design keeps it a thin add.
6. **`pulse` gate-width attribute** on the trigger row for envelope consumers — in
   (cheap, contained); confirm at slice 1.
7. **Step-index UI outlet** — in (scheduler-domain, Min-glue only); confirm at
   slice 1.

## 7. Sources

- `plans/tap.808.md` §6 and `plans/tap.303.md` §6 — the deferred phase-3 designs this
  plan resolves; the voice contracts in those plans' §3.2 (both author-approved
  2026-07-17) are the emitted interfaces.
- Roland TR-808 owner's manual / service notes — sequencer section: steps, A/B halves,
  fills, pre-scale, the accent row and AC bus (trigger levels pinned at slice 0).
- Roland TB-303 owner's manual / service notes — pattern memory data model (note,
  gate, accent, slide, octave), gate timing (duty pinned at slice 0); Robin Whittle's
  Devil Fish notes on gate/slide behavior (already in hand from the voice phases).
- `help/tap.303~-pattern.maxpat` — the phase-3 interface dry run (event-domain);
  `help/tap.808.maxhelp` — the family pattern section both slices upgrade.
