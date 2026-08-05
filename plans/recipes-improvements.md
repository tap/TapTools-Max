# Plan — improvements surfaced by the Recipes chapters

> **Status: planned, not started.** This is the design-of-record for the object improvements
> discovered while drafting *Tools on Tap* Part IX (Recipes) — see `book/PLAN-recipes.md` in
> the kernel repo for the chapters themselves. The recipe-drafting method (check every knob
> against the wrapper source before it goes in print) doubles as an audit; this file collects
> what the audit found. **Append to the discovery log (§10) as further chapters land.**

The recurring tell: every place a recipe had to print a *formula or a workaround* instead of
an attribute is a candidate hole. Items are ordered by how loudly the writing complained.

---

## 1. `tap.adsr~` — full rewrite as a virtual-analog envelope (kernel-first)

> **Status: shipped 2026-08-05** — kernel `taptools/adsr.h` + eight-scenario battery +
> capi/bridge + executed `adsr.ipynb`; wrapper shrunk to Min glue with `analog` as the new
> default and `threshold`/`velocity` exposed; maxref rewritten; REVIVAL §14. The design
> below was followed as written, with one refinement: the decay/release knob contract is
> "95 % of the gap closed at the knob time" (τ = t/3), documented and pinned.

**What it is today.** A faithful port of Jamoma's `TTAdsr` (2003): piecewise segments that
are straight lines in amplitude (`linear`) or straight lines in dB (`exponential`/`hybrid`),
with hard state switches at the targets. The DSP lives in the wrapper `.cpp` — it predates
the kernel-header rule. Trigger is a level gate (`x > 0.5`) with no velocity or accent
awareness.

**Why rewrite rather than patch.** Three independent reasons:

- **It is not virtual-analog, and the difference is audible.** An analog EG is an RC
  circuit: the attack charges toward an *overshoot* target (well above full scale) and is
  cut at full scale — that truncated-exponential shape is where the perceived punch lives;
  decay and release are true exponentials that *taper into* sustain and zero
  asymptotically. Straight lines in dB hit their targets at constant rate and stop dead.
  The curve family and its constants come from published sources (the CEM3310 and SSM2056
  datasheets and the standard Electronotes treatment — the same provenance discipline as
  every other kernel).
- **It ignores the package's trigger contract.** The whole family speaks
  amplitude-as-accent (808 voices, both sequencer rows, the 303's gate at 1.0/2.0), but
  `tap.adsr~` reads a bare level gate at exactly 0.5 — which is *precisely* the sequencer's
  default `accented` level, so the obvious cross-family patch fails silently at defaults
  (`0.5 > 0.5` is false).
- **The DSP is in the wrong repo.** House rule: substantial DSP is a kernel header with its
  own Catch2 battery, capi exposure, and notebook verification. An envelope kernel is also
  reusable — the 303/808 voices carry envelope circuits of their own today, and future
  kernels shouldn't re-derive RC segments.

**Target design.**

- `taptools/adsr.h`, `tap::tools::adsr`, house shape: `prepare(sr)`, per-sample
  `process(gate)`, allocation-free setters.
- **Analog core:** one-pole RC toward per-stage targets. Attack → overshoot target
  (datasheet-derived constant, on the order of 1.3–1.6× full scale), switch at 1.0. Decay →
  sustain, release → 0, both true exponentials with time constants defined so the knob's
  milliseconds mean the measured 10–90 % / T60-style figure the header documents (contract
  numbers, pinned by test — decide and document the exact convention, don't inherit
  ambiguity).
- **Retrigger from current level** (analog behavior; no reset click), and a documented
  legato rule.
- **Trigger contract:** gate opens above a `threshold` attribute (default well below 0.5 —
  align with the seq's `plain`/`accented` levels), and **gate amplitude is velocity**:
  peak and sustain scale by a `velocity` sensitivity attribute (0 = today's behavior,
  1 = full scaling). The 303 convention (1.0 plain / 2.0 accented) and the 808 rows'
  amplitudes both land meaningfully without adapters.
- **Compatibility:** keep `mode linear|exponential|hybrid` as the faithful Jamoma curves
  (they are a screenful); add `analog` and make it the default. The wrapper's own header
  note shows the project already chose "what users actually heard" over struct-reported
  defaults once — this is the same call in the other direction, and the release notes say
  so plainly.
- **Verification:** Catch2 scenarios pinning the curve contract numbers and the
  velocity/threshold behavior; capi + `taptools_py` exposure; an executed notebook cell
  plotting analog vs. hybrid attacks (the book's future machine chapter cites it).
- **Rollout:** kernel first (TapTools repo), then the wrapper here shrinks to Min glue and
  the submodule pin bumps — the standard release flow.

## 2. `tap.vco~` — a performance-modulation section (vibrato, bend)

**The tell in print:** the famous-patches chapter has to teach `cycle~ 5.5` scaled by
`0.006 × f` because the FM inlet is calibrated in Hz and constant-*cents* vibrato needs
per-note depth scaling — then hand-rolls the delayed onset with `line~`.

**Target:** extend the oscillator's analog section (already cents-calibrated, already
seeded) with a periodic sibling: `vibrato` (cents, default 0), `vibrato_rate` (Hz),
`vibrato_delay` (ms onset fade — the "singing" control). Optionally `bend` (semitones,
riding the existing ramp machinery) for wheel work. Kernel-first in `vco.h`; the hardware
precedent (the Minimoog's oscillator-bank modulation section) is the design argument.
Depth 0 must keep the bit-identical-ideal-by-default promise the vco test suite pins.

## 3. `tap.ladder~` — symbolic `mode`/`solver` (wrapper bug-fix, plus an audit)

**Confirmed bug-adjacent:** the wrapper's `mode` and `solver` are `attribute<int>`, but the
shipped book chapters (ladder chapter's recipes; vco chapter's Moog recipe) print
`@mode lp24` — which the object does not accept (and lp24 happens to be index 0, so the
mistake can *appear* to work). Fix: symbolic enum attributes (`lp24|lp12|bp12|bp24|hp12|hp24`,
`fast|exact`), keeping numeric acceptance for patch compatibility. Then: audit every object
for the same pattern (`tap.diode~` is the likely sibling), and update the Part IX recipes'
`@mode 0` workaround back to the symbolic spelling. Wrapper-only; no kernel change.

## 4. Determinism sweep — `tap.noise~` and `tap.verb~`

Two objects violate the "a seed is a serial number" doctrine:

- **`tap.noise~`** seeds from `std::random_device` — renders don't reproduce. Add `seed`
  (int ≥ 1, default 1, deterministic) with the family semantics; an *unseeded* mode is not
  worth keeping (no other object has one).
- **`tap.verb~`** calls `std::rand()` in `deviate()`, so its comb delays/decays
  re-randomize on every `prepare`/`configure` — the reverb is not bit-reproducible across
  instantiations. Same fix, same doctrine: a `seed` attribute, deterministic default,
  behavior change in the release notes.

## 5. `tap.delay~` / `tap.multitap~` — rebuild behind the same names (decision)

The second-wave sweep found the delay pair is the weakest DSP in the package: **integer-
sample delays with no interpolation** (modulating the time zipper-steps — while `tap.5comb~`
and `tap.pitchaccum~` Hermite-interpolate as a point of pride), no feedback, no `mix`, no
`bypass`/`mute`, and a trap on `tap.delay~`'s time inlet (a signal value of exactly 0.0
means "use the attribute," not "zero delay").

**Trash-or-rebuild was considered; the decision is rebuild, keeping the names.** The
reasoning, recorded so it doesn't get re-litigated:

- *Deleting* breaks every legacy patcher that instantiates them — the names date to the
  1999 package, and the revival's promise is continuity. A missing-object box is the worst
  outcome the package can hand an old user.
- *Keeping them as-is* is also indefensible: stock Max (`delay~`, `tapin~`/`tapout~`)
  already covers bare uninterpolated delay, so the un-upgraded objects duplicate the host
  with less. The only reason for a `tap.` delay to exist is the house treatment.
- The substrate is already written: `grm_comb.h` carries the Hermite fractional tap and
  the DC-blocked feedback loop. A kernel `delay.h` extracting that is small.

Target: kernel-first rebuild — Hermite taps, `feedback` (delay~), `mix`, `bypass`/`mute`,
per-tap gain *and pan* on multitap, house `smooth` ramps on times. Compatibility: keep an
`interp 0` integer mode for bit-faithful legacy behavior (non-default), and **kill the
0.0-signal trap** as a documented breaking change — a signal on the time inlet always wins,
including at zero. Wrapper defaults otherwise preserved.

## 6. `tap.vocoder~` — the missing conveniences

The robot-voice recipe works around three absences: no unvoiced/**sibilance path** (the
recipe's fix — ride 10 % noise in the carrier full-time — is the classic patch, but a
detector-switched noise path is the classic *hardware* answer and a good kernel exercise
from published vocoder literature), no `mix`, no `bypass`/`mute` (every comparable effect
object has them). Low urgency, documented workarounds; batch with any vocoder revisit.

## 7. Small QoL and bug notes

- **`tap.5comb~`:** tuning is Hz-only; a `notes <midi×5>` message would make the voicing
  tables in the comb-drones recipe one message instead of five attributes. (Fractional
  MIDI for JI intervals, as the 303 pair already does.)
- **`tap.vco~` driven by a pitch signal loses glide** — the frequency signal inlet
  bypasses `smooth` by design, so sequencer-driven patches need an external slew
  (`slide~`) to get portamento. Fold into §2: either a documented `glide` that applies to
  the signal inlet, or a MIDI-note signal input mode with the 303-style RC.
- **`tap.sustain~` `length`** has no setter — runtime changes silently wait for a DSP
  restart. Either wire the setter (re-size at next `dspsetup`, documented) or document the
  restriction in the maxref (which currently documents *nothing* — see §8).
- **`tap.crossfade~` `mode`** is a legacy no-op (both values compute identically) —
  deprecate in docs so nobody A/Bs a placebo.
- **`tap.harmony~` v1.1 — the house machinery.** The shipped v1 lacks `mute`/`bypass` and
  the 16-slot preset-morph engine its effect siblings carry (autowah/pitchaccum/5comb/303).
  A timed morph between two stored *chords* is very much in the recipes' spirit; the kernel
  already slews intervals and gains, so this is wrapper plumbing plus the store/recall
  pattern. Also fixed post-ship: the `chord` message now writes through the attributes, so
  queries stay truthful and a DSP restart no longer reverts the last chord.

## 8. Documentation debts (no DSP)

- The vco chapter should say once that `smooth` is a per-object ramp time shared by every
  parameter — 280 ms of Lucky-Man portamento also slows `pw`/`gain` changes on that object.
- After §3 lands, sweep the book for `@mode 0`-style numeric workarounds and restore the
  symbolic spellings.
- **Maxref drift found by the sweep:** `tap.verb~` XML documents `use_early_reflections`
  (wrapper: `er`) and omits most of the real surface; `tap.vocoder~` XML types `q` /
  `response_interval` as symbols (wrapper: number); `tap.sustain~` XML documents no
  attributes or methods at all; `tap.adsr~` XML lists `int`/`float` methods the wrapper
  doesn't define. The wrapper is the source of truth — regenerate or hand-fix the four.
- **Fixed already (book repo, this branch):** the pitchaccum chapter's `pitch1`/`feedback1`
  spellings → `trans1`/`fb1` with the 0–99 feedback scale.

## 9. Sequencing and priorities

1. §3 (small, self-contained, un-breaks shipped documentation), §4 (small), and the §7
   bug notes (`tap.sustain~` setter, `tap.crossfade~` deprecation).
2. §1 (kernel `adsr.h` + tests + notebook, then wrapper + pin bump) — the big one, and the
   gate for any recipe that leans on envelopes driven from the sequencer family.
3. §2 (kernel `vco.h` + tests, wrapper attribute plumbing, then simplify the
   famous-patches chapter's vibrato plumbing to the new attributes), absorbing the §7
   pitch-signal glide note.
4. §5 (the delay pair rebuild) — real work, worth its own design pass and plan file when
   picked up.
5. §6 and the §8 doc sweep ride along with whichever PR touches each object or page.

## 10. Discovery log — append as further recipes land

| date | chapter being drafted | finding | disposition |
|---|---|---|---|
| 2026-08-05 | famous Moog patches | FM-inlet vibrato needs a printed formula | → §2 |
| 2026-08-05 | (drum scaffold audit) | `tap.adsr~` level-gate vs. family contract; not VA; DSP in wrapper | → §1 |
| 2026-08-05 | Moog voice | book's `@mode lp24` spelling doesn't match `attribute<int>` | → §3 |
| 2026-08-05 | (kit audit) | `tap.noise~` unseeded | → §4 |
| 2026-08-05 | shimmer | pitchaccum chapter's `pitch1`/`feedback1` don't match wrapper `trans1`/`fb1` (0–99) | fixed in book; → §8 |
| 2026-08-05 | sequenced modular | vco frequency-signal inlet bypasses `smooth` — sequenced pitch can't glide without external `slide~` | → §2 / §7 |
| 2026-08-05 | sequenced modular | `tap.delay~`/`tap.multitap~`: integer-sample, no feedback/mix; 0.0-signal trap | → §5 |
| 2026-08-05 | shimmer | `tap.verb~` uses `std::rand()` — not reproducible | → §4 |
| 2026-08-05 | robot voice | vocoder has no sibilance path, no mix/bypass | → §6 |
| 2026-08-05 | comb drones | `tap.5comb~` tunes in Hz only — voicing tables want a MIDI list message | → §7 |
| 2026-08-05 | (sweep) | maxref drift: verb `er`, vocoder attr types, sustain XML empty, adsr phantom methods | → §8 |
| 2026-08-05 | (sweep) | `tap.sustain~` `length` setter missing; `tap.crossfade~` `mode` is a no-op | → §7 |
| 2026-08-05 | robot voice (songbook) | the "Hide and Seek" mechanism is a *formant-corrected multi-voice harmonizer* — no package object does formant-true shifting (`tap.shift~` moves formants with pitch; the recipe caps its stack at ±7 st for that reason). Candidate new object on the DspTap LPC substrate (`pvoc.h` already does envelope-preserving shifts) | **shipped 2026-08-05** as `tap.harmony~` (kernel `harmonizer.h` + wrapper; REVIVAL.md §13) |
| 2026-08-05 | delay-pair decision | trash-or-rebuild resolved: rebuild behind the same names, `interp 0` legacy mode, kill the 0.0-signal trap | → §5 |
