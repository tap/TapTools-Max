# Plan — improvements surfaced by the Recipes chapters

> **Status: planned, not started.** This is the design-of-record for the object improvements
> discovered while drafting *Tools on Tap* Part IX (Recipes) — see `book/PLAN-recipes.md` in
> the kernel repo for the chapters themselves. The recipe-drafting method (check every knob
> against the wrapper source before it goes in print) doubles as an audit; this file collects
> what the audit found. **Append to the discovery log (§7) as further chapters land.**

The recurring tell: every place a recipe had to print a *formula or a workaround* instead of
an attribute is a candidate hole. Items are ordered by how loudly the writing complained.

---

## 1. `tap.adsr~` — full rewrite as a virtual-analog envelope (kernel-first)

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

## 4. `tap.noise~` — determinism (`seed`)

The only stochastic object that violates the "a seed is a serial number" doctrine: it seeds
from `std::random_device`, so renders don't reproduce. Matters the first time a recipe wants
reproducible sample-and-hold wobble. Add `seed` (int ≥ 1, default 1, deterministic) with the
family semantics; document the behavior change in the release notes (an *unseeded* mode is
not worth keeping — no other object has one).

## 5. Documentation debts (no code)

- The vco chapter should say once that `smooth` is a per-object ramp time shared by every
  parameter — 280 ms of Lucky-Man portamento also slows `pw`/`gain` changes on that object.
- After §3 lands, sweep the book for `@mode 0`-style numeric workarounds and restore the
  symbolic spellings.

## 6. Sequencing and priorities

1. §3 (small, self-contained, un-breaks shipped documentation) and §4 (small).
2. §1 (kernel `adsr.h` + tests + notebook, then wrapper + pin bump) — the big one, and the
   gate for any recipe that leans on envelopes driven from the sequencer family.
3. §2 (kernel `vco.h` + tests, wrapper attribute plumbing, then simplify the
   famous-patches chapter's vibrato plumbing to the new attributes).
4. §5 doc sweep rides along with whichever PR touches each page.

## 7. Discovery log — append as further recipes land

| date | chapter being drafted | finding | disposition |
|---|---|---|---|
| 2026-08-05 | famous Moog patches | FM-inlet vibrato needs a printed formula | → §2 |
| 2026-08-05 | (drum scaffold audit) | `tap.adsr~` level-gate vs. family contract; not VA; DSP in wrapper | → §1 |
| 2026-08-05 | Moog voice | book's `@mode lp24` spelling doesn't match `attribute<int>` | → §3 |
| 2026-08-05 | (kit audit) | `tap.noise~` unseeded | → §4 |
