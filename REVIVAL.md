# TapTools Revival — Inventory & Plan

> Working document for bringing TapTools back to life in 2026.
> Status as of 2026-06-18. The revival work has been consolidated into `main`
> (Tiers 1–3 + infrastructure + all 5 Jitter objects; legacy preserved on the `legacy`
> branch). See §8 for the `taptools-min` reconciliation.

## 1. Where things actually stand

TapTools is a collection of **~48 shipping Max/MSP externals** (roots back to
1999; last substantive work 2014–2015; a lone README touch in 2020). The repo
is intact — nothing is lost. The "overwhelming mess" is really **three
half-finished migrations stacked on top of each other**:

| Era  | Attempt                                                | State                                   |
|------|-------------------------------------------------------|-----------------------------------------|
| 2013 | Monolithic → modular externals on **old Jamoma** (v4) | Shipped — this is the bulk              |
| 2014 | Pruned obsolete objects; vendored deps as git subtrees | Done                                    |
| 2015 | **Travis CI** + rewrite objects on **jamoma2**         | Abandoned after 1–2 objects             |

### The three real blockers

1. **Dead dependency.** 50 of 52 source objects are thin wrappers over the
   **old Jamoma** C++ library (`TTClassWrapperMax`, `TTDSPInit`, `TT(...)`).
   Jamoma is archived/dead. The 2015 escape hatch (jamoma2) is also abandoned
   *and* its submodule was never even cloned (`source/jamoma2` is empty). Only
   `tap.fourpole~` was ported to it.
2. **Dead build system.** Ruby (`build.rb`) driving Xcode projects + **Travis
   on Xcode 6.1**. Both long gone. Modern Max dev uses the **CMake-based
   `max-sdk-base`**.
3. **Dead binaries.** The checked-in `.mxo` files are `i386 + x86_64` — **no
   arm64**, so they don't run on any Apple Silicon Mac. Everything needs
   rebuilding regardless.

### Decisions locked in

- **Platforms:** macOS (Apple Silicon, universal arm64+x86_64) **and Windows**.
- **Dependency strategy:** **Cut Jamoma** — reimplement DSP standalone on the
  modern Max SDK. No dead dependency dragged along.
- **Scope:** review the full historical object set (current + retired +
  Jamoma-migrated) before committing to a per-object port list.

Only **`ttblue`** (a support lib, likely the SQLite glue for `tap.filecontainer`)
is currently Jamoma-free. Every shipping audio/utility object needs decoupling.

---

## 2. Currently in source (need Jamoma decoupling)

> **Status: ✅ all of §2 is done.** Every object in the tables below — all of Tier 1–3,
> all 5 Jitter objects, and the infrastructure set (`tap.midimapper`, `tap.folder`,
> `tap.filecontainer`) — has been ported off Jamoma onto Min. `tap.loader` was
> intentionally dropped (obsolete). The tables are kept as the original inventory; see
> §7 for the per-object completion log. (Note: `tap.filecontainer` does **not** use
> `ttblue` — it drives Max's native `sqlite` object — and `ttblue`/`Core/` were pruned.)

Effort tiers below are a **first-pass estimate** pending per-object code review.
"Cut" = reimplement the DSP/logic directly on the Max SDK.

### Tier 1 — Trivial utility / data (fast)
| Object | What it does |
|--------|--------------|
| `tap.change` | Filter out repetitions (lists/symbols/numbers) |
| `tap.route` | A more flexible `route` |
| `tap.list.index` | Construct/decompose lists by element |
| `tap.sieve` | Only allow filtered values through |
| `tap.prime` | Generate prime numbers |
| `tap.bits` | Bit operations |
| `tap.gang` | Link objects together |
| `tap.random` | Floating-point RNG |
| `tap.radians~` | Convert to/from radians |
| `tap.inquisitor` | Interrogate another object's attributes |
| `tap.biquadcalc` | Calculate biquad coefficients |

### Tier 2 — Simple MSP DSP (moderate)
| Object | What it does |
|--------|--------------|
| `tap.dcblock~` | DC offset filter |
| `tap.noise~` | Colored noise (white/pink/brown/blue/gauss) |
| `tap.pan~` | Stereo panner |
| `tap.crossfade~` | Crossfade two signals |
| `tap.split~` | Signal-rate `split` |
| `tap.autothru~` | Auto pass-through |
| `tap.count~` / `tap.counter~` | Count samples / signal transitions |
| `tap.zerox~` | Count zero crossings |
| `tap.width~` | Measure pulse width |
| `tap.sift~` | Sift samples to control rate |
| `tap.random~` | Signal-rate RNG |
| `tap.pulsesub~` | Pulse-based envelope substitution |

### Tier 3 — Complex DSP (heavier)
| Object | What it does |
|--------|--------------|
| `tap.svf~` | State-variable filter w/ LFO modulation *(✅ 2026-07 redesigned: Simper/Cytomic morphing SVF; the Chamberlin+LFO port it replaces is in git history)* |
| `tap.comb~` | Comb filter with filtered feedback |
| `tap.fourpole~` | 4-pole ladder filter *(✅ re-cut standalone — the 2015 jamoma2 source was gone)* |
| `tap.rotate` | (revived 2015) |
| `tap.verb~` | Reverb |
| `tap.limi~` | Look-ahead limiter |
| `tap.multitap~` | Self-contained multitap delay |
| `tap.procrastinate~` | Cascading delay effect |
| `tap.elixir~` | Gain-structure management |
| `tap.adsr~` | Envelope generator |
| `tap.shift~` | Pitch shifter |
| `tap.fft.binmodulator~` | Modulate FFT bins |
| `tap.fft.list~` | FFT analysis → list |
| `tap.fft.normalize~` | Normalize an FFT |
| `tap.buffer.peak~` | Hottest sample in a `buffer~` |
| `tap.buffer.record~` | Smooth recording to a `buffer~` |
| `tap.buffer.snap~` | Snap to zero-crossings in a `buffer~` |

### Jitter (need the Jitter SDK — separate sub-effort)
`tap.jit.ali` · `tap.jit.colortrack` · `tap.jit.kernel` · `tap.jit.proximity` · `tap.jit.sum`

### Special / infrastructure (investigate individually)
| Object | Note |
|--------|------|
| `tap.loader` | Package loader shim — **may be obsolete** under the modern Max package system; verify before porting |
| `tap.filecontainer` | Bundles files into a SQLite container (uses `ttblue`) |
| `tap.midimapper` | Map MIDI input to user-defined output |
| `tap.folder` | Filesystem create/delete/copy |
| `ttblue` | SQLite support library (already Jamoma-free) |

---

## 3. Formerly existed — retired but **documented** (real shipping objects, source removed)

These have surviving `.maxref.xml` docs and/or help patchers but no current
source. Strong candidates to **resurrect from docs + git history** if still useful:

| Object | What it did | Revive? (first take) |
|--------|-------------|----------------------|
| `tap.delay~` | Sample-accurate delay | ✅ **done** (reconstructed from docs) |
| `tap.delay` | Delay lists/symbols/numbers | ✅ **done** (reconstructed from docs) |
| `tap.sustain~` | Sample-and-loop sounds | ✅ **done** (from `taptools-min`; multi-voice — §8) |
| `tap.vocoder~` | 24-band vocoder | ✅ **done** (reinvented standalone) |
| `tap.spectra~` | Spectral remapping | ✅ **done** (reinvented standalone) |
| `tap.nr~` | Spectral noise reduction | ✅ **done** (reinvented standalone) |
| `tap.5comb~` | 5× comb filter | ✅ **done** (recreated as a native external + portable kernel — see §7 2026-07-11) |
| `tap.adapt~` | (audio processor) | review |
| `tap.buffer.record2~` | Smooth buffer recording (v2) | merge into `tap.buffer.record~`? |
| `tap.smooth` | Data-stream smoother | maybe (native alts exist) |
| `tap.deviate` | Randomize & "prime" input | maybe |
| `tap.semitone2ratio` | Semitones → frequency ratio | ✅ **done** (native object; was an abstraction — see §7 2026-07-11) |
| `tap.string.sub` | String substitution | maybe |
| `tap.thru` / `tap.thru~` | Feedback utilities | maybe |
| `tap.decay_calc` | Feedback coefficient calc | maybe (pair w/ `tap.biquadcalc`) |
| `tap.jit.delay` | Matrix-stream frame delay | Jitter tier |
| `tap.jit.motion` / `motion+` / `motion2` | Video motion detection | Jitter tier |
| `tap.jit.grayscale` | Grayscale conversion | Jitter tier (native alts exist) |
| `tap.jit.pan` | Video panner | Jitter tier |
| `tap.jit.getattributes` | Jitter abstraction helper | Jitter tier |

## 4. Formerly existed — retired (in git history, no current docs)

Recovered from `git log` (source was deleted). Most were intentionally retired:

`tap.applescript` · `tap.buffer.norm~` · `tap.buildassist` · `tap.colorspace` ·
`tap.decibels~` · `tap.diff~` · `tap.lfo~` · `tap.onepole~` · `tap.path` ·
`tap.pi` · `tap.pulserouter~` · `tap.quantize~` · `tap.twopole~`

## 5. Superseded / migrated (per historical changelog)

Documented as retired in favor of native Max or Jamoma — review for **repatriation**
now that Jamoma is also dormant:

| Old TapTools object | Was replaced by | Repatriate? |
|---------------------|-----------------|-------------|
| `tap.colorspace` | `j.unit` (Jamoma) | ⚠️ Jamoma dormant — candidate to bring back |
| `tap.decibels~` | `atodb~`/`dbtoa~` (native) or `j.unit~` | native covers it |
| `tap.onepole~` / `twopole~` / `fourpole~` | `tap.filter~` | ✅ `tap.filter~` now built (unified multimode biquad — see §7); `tap.fourpole~` also stands alone |
| `tap.average~` | `average~` (native) | native covers it |
| `tap.degrade~` | `degrade~` (native) | native covers it |
| `tap.diff` | `gen~` | native covers it |
| `tap.path` | native path resolution | native covers it |
| `tap.buildassist` | Max Projects | obsolete |
| `tap.xml.sax` | `mxj` XmlParse | obsolete |
| `tap.svn` | (dropped) | obsolete |

> **Resolved — `tap.filter~`:** ✅ built (batch 3). A unified multimode RBJ biquad
> (lowpass/highpass/bandpass×2/notch/allpass/peaking/low-+high-shelf) with a
> `mode`/`frequency`/`q`/`gain` surface — the standalone replacement for the old
> Jamoma flagship. It can absorb the individual filter objects over time. New maxref
> + unit test; still wants a Max audition.

> **Jamoma repatriation:** a deeper pass over the vendored `Core/` Jamoma
> modules (Foundation/DSP/AudioGraph/Modular) can identify `j.*` objects that
> originated from or pair with TapTools and are worth pulling back. Flagged as a
> follow-up survey, not part of the first build.

---

## 6. Recommended path

1. **Proof of life.** Stand up a modern **CMake + `max-sdk-base`** build and get
   **one** Tier-1 object compiling as a **universal macOS external** that loads
   in Max 9. Establishes the template every other object follows.
2. **Windows from day one.** Wire the same CMake build for Windows so we never
   bolt it on later.
3. **CI.** Replace Travis with **GitHub Actions** building both platforms.
4. **March the tiers.** Port Tier 1 → 2 → 3, then Jitter, then evaluate
   resurrecting the documented-but-retired set. Ship incrementally via GitHub
   releases / the Max Package Manager.
5. **Prune the corpse.** Once the new build proves out, remove the dead `Core/`
   Jamoma subtree, old Xcode projects, `build.rb`, Travis config, and stale
   `.mxo` binaries.

### Suggested first proof-of-life object
`tap.change` or `tap.prime` — Tier 1, no audio, minimal surface, exercises the
whole toolchain (build → package → load in Max) without DSP distractions.

---

## 7. Progress log

**Foundation decision (locked):** Build on **Min as a thin wrapper only** — Min
handles the Max plumbing (inlets/outlets, attributes, messages, the DSP perform
loop), while **all DSP is written as plain portable C++** with **no dependency on
`min-lib`**. Rationale: `min-api` + `max-sdk-base` are actively maintained (last
commit 2026-03-24) and compile clean against the current toolchain, but `min-lib`
is the under-maintained piece. Keeping DSP portable means shallow lock-in — the
wrapper is a small, swappable shim if Min ever stalls. Standard: **C++20**.
Targets: **macOS universal (arm64+x86_64) + Windows**, via CMake + GitHub Actions.

- ✅ **Modern build foundation** — root `CMakeLists.txt`, `min-api` submodule,
  GitHub Actions CI (mac+win), universal-binary verification. Replaces the
  retired Ruby/Xcode + Travis build. Dead `jamoma2` submodule removed.
- ✅ **`tap.change`** (Tier 1) — first object; proof of life. CI green; macOS
  binary verified universal.
- ✅ **`tap.dcblock~`** (Tier 2, DSP) — first full **vertical slice**: object +
  reference page (`docs/`) + help patcher (`help/`) + package layout. DSP is
  portable C++ (faithful to Jamoma's R=0.9997); `bypass`/`mute`/`clear`
  preserved.
- ✅ **Tier-1 batch** — `tap.prime`, `tap.sieve`, `tap.list.index`, `tap.bits`,
  each with object + reference page + help patcher. `tap.prime` ports Jamoma's
  exact `TTPrime` stepping; `tap.list.index` improves on the original (correctly
  handles symbol-leading lists). Ported reference pages + help patchers from the
  legacy package.
- ✅ **DSP batch** — `tap.random` (control RNG, faithful LCG), `tap.crossfade~`
  and `tap.pan~` (equal-power / linear / square-root curves computed directly
  from Jamoma's "calculate" formulas, position drivable by a signal inlet or the
  `position` attribute). Self-contained — no `min-lib`, no shared lookup-table
  global. New Min patterns exercised: `samples<N>` multi-output, attribute
  `setter` caching, `has_signal_connection()`.
- ✅ **`tap.noise~`** — white / pink / brown / blue / gaussian noise generator,
  faithful port of Jamoma's TTNoise colouring filters. First generator
  (`sample_operator<0,1>`); gaussian via std normal distribution.
- ✅ **Tier-1 completion** — `tap.route` (flexible router; searchstring is now a
  single `symbol` attribute since Min only specializes `vector<int>`/`vector<number>`
  attributes, not `vector<symbol>`), `tap.gang` (4-in/4-out deferred fan-out via
  per-outlet `queue`, with change-detection breaking feedback loops),
  `tap.radians~` (hz/radians/degrees converter; signal + float outlet, sample rate
  from `samplerate()`), `tap.inquisitor` (queries another object's attributes via
  the Min `patcher`/`box` patcher API — no Jamoma), `tap.biquadcalc` (RBJ Audio EQ
  Cookbook coefficient calculator, control-rate). Each with reference page + help
  patcher ported from the legacy package.
- ✅ **Tier-2 DSP batch** — `tap.split~` (range router, `sample_operator<3,3>`,
  limits drivable by signal or float/attribute), `tap.autothru~` (auto
  pass-through via `has_signal_connection()`), `tap.width~` (pulse-width meter in
  ms), `tap.count~` (gated sample counter), `tap.counter~` (signal-transition
  counter), `tap.zerox~` (zero-crossing counter — faithful port of Jamoma's
  TTZerocross, `sample_operator<1,2>`), `tap.random~` (signal-triggered S&H RNG;
  the original's per-vector edge test is fixed to per-sample so every zero→non-zero
  transition fires). All DSP is portable C++ — no `min-lib`, no Jamoma. New Min
  patterns exercised: `queue` deferral, the `dspsetup` message, the `patcher` API,
  and mixed signal + non-signal outlets. **Every object in this batch was verified
  to compile cleanly with the CMake/Min toolchain.**
- ✅ **`tap.adsr~`** (Tier 3) — attack/decay/sustain/release envelope generator,
  faithful port of Jamoma's TTAdsr (linear / exponential / hybrid curves),
  triggered by the `trigger` attribute or a signal crossing 0.5. Defaults to
  hybrid (the original's true audible default). Unblocks `tap.pulsesub~`.
- ✅ **`tap.sift~`** — both original modes via a `vector_operator` whose single
  outlet is created (signal vs control) to match the mode argument at
  instantiation; the float-dump mode uses an SPSC ring buffer drained on the main
  thread by a `queue`. (Resolved the deferral noted below.)
- ✅ **Tier-3 batch (filters)** — `tap.rotate` (3D coordinate rotation, pure data
  object), `tap.svf~` (stereo Chamberlin state-variable filter + vector-rate LFO +
  portamento ramp — the ttblue tt_svf/tt_lfo/tt_ramp trio folded into one
  `vector_operator`), `tap.comb~` (IIR comb with a lowpass in the feedback loop,
  faithful port of tt_comb). All DSP is portable C++; the LFO is computed directly
  from a phase accumulator (equivalent to the original wavetable). Compile-verified
  against the toolchain; **audio behavior still needs runtime validation in Max.**
- ✅ **`tap.pulsesub~`** (Tier 2, was deferred) — phasor → duty-cycle offset → ADSR
  → multiply, as one `sample_operator`. Resolved now that the ADSR exists.
- ✅ **Tier-3 batch (delays / dynamics / filters)** —
  `tap.multitap~` (circular-buffer multitap delay, per-tap delay/gain via vector
  attributes), `tap.limi~` (stereo look-ahead limiter w/ DC blocker + pre/post gain,
  faithful port of tt_limiter), `tap.fourpole~` (4-pole Moog ladder — **reimplemented**,
  since the 2015 original's jamoma2 `LowpassFourPole` source is not in the repo).
- ✅ **Tier-3 batch (buffers)** — `tap.buffer.peak~`, `tap.buffer.snap~`,
  `tap.buffer.record~`, all rebuilt on Min's `buffer_reference`/`buffer_lock`
  (binding, `set`, notifications, and `dirty()` handled by the framework).
  `snap~`/`record~` write/read in the audio thread via `buffer_lock<true>`.
**Tier-3 batch (mixers / FFT / pitch):** `tap.elixir~` (variable-inlet 2–10
equal-gain crossfade mixer with per-inlet slew; dynamic inlets),
`tap.fft.binmodulator~` (per-bin LFO modulation inside pfft~), and `tap.shift~`
(two-grain Welch-windowed delay-line pitch shifter — the exact 256-point padded
Welch table is embedded and mirrored to 512). Compile-verified; audio behavior
needs runtime validation in Max.

**Infrastructure objects:** ✅ `tap.midimapper` (MIDI-message remapper, pure
control logic) and ✅ `tap.folder` (filesystem make/delete/copy/move —
**modernized** onto portable `std::filesystem`, replacing the original's
AppleScript + Win32-shell code; `unzip` dropped, no portable std support). Their
reference pages/help were restored from git history after the prune.
- ⏸ `tap.loader` — a package-loader shim; **obsolete** under the modern Max package
  system (externals auto-load), so intentionally not ported.
- ✅ `tap.filecontainer` — bundles files into a SQLite-backed container. Ported on Min;
  no SQLite needed to be vendored — like the original it drives **Max's built-in
  `sqlite` object** via the C API (`object_new_typed(CLASS_NOBOX, "sqlite", …)` +
  `execstring`). Schemas, BLOB import/export, and all messages reproduced; the
  temp-folder + file-moddate handling is reimplemented on `std::filesystem` (the
  moddate restore is best-effort and wants a Max audition). Docs/help restored from
  legacy.

**Remaining frontiers (each its own sub-effort):**
- **Jitter family — ✅ 5 of 5 done.** The matrix→value (analysis) objects are plain
  Min objects that read a named `jit.matrix` through the Jitter API (`c74::max`) — ✅
  `tap.jit.sum` (sum all cells), ✅ `tap.jit.proximity` (nearest 2D point), ✅
  `tap.jit.ali` (Ali Momeni proximity-weighted interpolation), ✅ `tap.jit.colortrack`
  (4-tracker HSL colour tracker — full RGB→HSL, hue-wrap, bounds/centroid/size, all 36
  attributes preserved). The matrix→matrix object uses the MOP path — ✅ `tap.jit.kernel`
  (radial-kernel **generator** built on Min's `matrix_operator<>` as a no-input MOP; the
  original's off-by-one out-of-bounds column write is fixed by the proper MOP cell
  routing). All five compile against the toolchain (JitterAPI links on the mac/win CI;
  the object compile is verified on Linux/GCC too). **Runtime validation in Max still
  pending** for the two new ones.
- **Resurrection candidates** — the two classic delays are now done: ✅ `tap.delay~`
  (sample-accurate audio delay line, circular buffer, ms delay drivable by the right
  inlet or `@delay`, `clear`/`dspsetup`) and ✅ `tap.delay` (control-rate single-pending
  message delay via Min `timer`, faithful to the original `delay`-based abstraction) —
  both reconstructed from their surviving maxref docs (no source survived). Still open:
  the **spectral set** — but a closer look (the surviving `.maxpat` abstractions on
  `legacy`) shows these are **not** standalone DSP externals to "rebuild from docs."
  They are `pfft~`/patcher **abstractions over a chain of other un-ported objects**, so
  each is really a mini sub-project:
    - ✅ `tap.spectra~` — **reinvented** as a self-contained external (originally a
      `pfft~ tap.spectra.pfft` subpatcher that used `tap.scale~` to reorder bins). Same
      in-house STFT as `tap.nr~`; each output bin k takes its value from input bin
      round(k·`remap`) with Hermitian mirroring (identity at remap=1, stretch/compress
      otherwise). Unit-tested for identity reconstruction and for spectral displacement
      at remap≠1. maxref rewritten; help patcher ported (needs runtime rework).
    - ✅ `tap.nr~` — **reinvented** as a self-contained external (originally wrapped
      `pfft~ tap.xnr~`; no surviving source). Runs its **own STFT** — an in-house radix-2
      FFT with a Hann window at 4× overlap and COLA-normalised overlap-add — so no `pfft~`
      host is needed. Each frame's bins below `threshold` are attenuated with a `slope`
      soft-knee. **Unit-tested for perfect reconstruction** (gate open ⇒ output equals
      input delayed by one FFT frame, &lt;1e-6) and for below-threshold attenuation.
      Latency = one FFT frame. maxref rewritten; help patcher ported from the legacy
      abstraction (needs runtime rework). Audio quality still needs Max validation.
    - ✅ `tap.vocoder~` — **reinvented** as a self-contained standalone external (the
      original was a real external; the abstraction just added smoothing/gain around it).
      A bank of 24 log-spaced (50 Hz–12 kHz) RBJ constant-0 dB-peak bandpass biquads
      analyses the modulator (left inlet); a per-band one-pole envelope follower (period
      = `response_interval` ms) shapes the matching carrier band (right inlet); bands are
      summed to the output. Honours the documented `q` and `response_interval` attributes
      (made `number`, not the original's odd `symbol` registration) plus a practical
      `gain`. DSP smoke-tested (silence→silence; a band tone passes). Help patcher ported
      from the legacy abstraction — **needs runtime rework in Max** for the standalone
      object. Audio quality still needs runtime validation.
  > **Decision (author, 2026-06-17): reinvent the lost spectral DSP** rather than defer.
  > **Done — the entire spectral set (`tap.vocoder~`, `tap.nr~`, `tap.spectra~`) is now
  > reinvented as self-contained externals.** The original `pfft~` chains and their
  > support objects (`tap.scale~`, `tap.typecheck~`, `tap.avg~`, `tap.thru`, `tap.xnr~`)
  > were bypassed rather than revived. The three help patchers are the legacy abstractions
  > and **need runtime rework in Max** for the standalone objects; audio quality across
  > all three needs Max validation.
  (`tap.sustain~` was recovered from the `taptools-min` archive — see §8.)
  > **Doc cleanup — ✅ done (batch 2):** the legacy `tap.delay.maxref.xml` had copy-pasted
  > filter boilerplate attributes (`clip`/`coefficients`/`gain`) that don't belong to a
  > delay; they were removed and the real `delaytime` attribute + `stop` message documented.
- ✅ **`tap.filter~`** — built as a new unified multimode filter: a Transposed-Direct-Form-II
  biquad driven by the RBJ Audio EQ Cookbook coefficients (the same set `tap.biquadcalc`
  uses), with a `mode` `attribute<symbol>` selecting lowpass/highpass/bandpass(×2)/notch/
  allpass/peaking/low-/high-shelf, plus `frequency` (signal- or float-driven), `q`, and
  `gain`. Per-vector coefficient recompute with per-sample linear smoothing to avoid zipper
  noise; `clear`/`dspsetup` handled. Single-channel (`mc.` for multichannel). New maxref +
  unit test (16 assertions vs. analytic RBJ references).
- ✅ **`tap.verb~` oversampling** — the deferred internal oversampling stage is in. An
  `oversampling` attribute (factors 1/2/4/8, **default 1 = true bypass, bit-identical to
  before**) runs the reverb cores at `host_sr × factor` with an anti-imaging upsample and a
  4-stage one-pole anti-aliasing downsample. *Deviation:* the legacy `downsample` ran the
  core at a **lower** rate with no antialiasing; this is inverted to genuine oversampling
  (cleaner feedback) per the roadmap — documented in-file. Wants a Max audition for the
  >1× sound.

**Latent-bug fixes made along the way (all noted in-file):** `tap.random~`
per-vector→per-sample edge test; `tap.buffer.snap~` post-clamp loop that could
never terminate; `tap.fft.normalize~` 0.49-biased equality that disabled the
DC/Nyquist halving; `tap.comb~` undefined-when-unset feedback/decay coupling.

**Tier 3 is complete.** The final two — the heaviest objects — are done:
- ✅ `tap.procrastinate~` — four `tap.shift~`-style voices chained in cascade, each
  with a randomized pitch ratio / delay / gain / equal-power pan drawn from
  configurable ranges (regenerated on bang). Embeds the padded-Welch window.
- ✅ `tap.verb~` — stereo Moorer-style reverb reconstructed from tt_verb: an 18-tap
  early-reflection pattern → six LFO-modulated comb filters (damping lowpass in each
  feedback loop) → Schroeder allpass → output lowpass → equal-power dry/wet mix →
  gain, with DC-block and clip stages. Two prime-"deviated" cores give the stereo
  image. The optional look-ahead limiter and the internal oversampling (both deferred
  initially) have since been added — see the `tap.verb~ oversampling` note above.

All ~48 core objects across Tiers 1–3 are now ported and compile-verified against
the Min/Max SDK toolchain. **Runtime validation in Max remains the outstanding
step** for the DSP objects (the filters, delays, pitch shifters, and reverb in
particular).

**Established Min patterns now available for the rest of Tier 3:** dynamically
created outlets + `vector_operator` (for variable I/O like the buffer/FFT
objects), SPSC ring buffer + `queue` for audio→control hand-off, per-vector
coefficient updates inside `vector_operator`, and `buffer_reference`/`buffer_lock`
for buffer~ access.

**Convention (tilde objects):** MSP objects whose Max name ends in `~` must have
their **project folder and `.cpp` named with `_tilde`** (e.g.
`source/projects/tap.dcblock_tilde/tap.dcblock_tilde.cpp`). The SDK maps the
project name back to `~` for the output binary, so the object still loads in Max
as `tap.dcblock~`, and its `docs/` + `help/` files keep the `~` name. This
applies to every remaining tilde object (`tap.noise~`, `tap.svf~`, …).

**Package layout:** the repo root is now the Min-DevKit-style package
(`externals/`, `docs/`, `help/`, generated `package-info.json`).

**Jitter completion + delay resurrection + test coverage (2026-06-17, batch 2):**
- ✅ **Jitter family complete (5/5).** `tap.jit.colortrack` (full 4-tracker HSL port —
  integer RGB→HSL, hue-wheel wrap, bounding-box/centroid/size, all 36 attributes,
  dumpout outlet; `ttblue`'s `onewrap` ported inline) and `tap.jit.kernel` (radial-kernel
  generator on Min's `matrix_operator<>` as a no-matrix-input MOP; the original's
  off-by-one column write — which wrote to `x = -1` out of bounds — is eliminated by
  per-cell MOP routing). Docs + help ported from `legacy` for both.
- ✅ **`tap.delay~` / `tap.delay` resurrected** from their maxref docs (no source
  survived) — see the frontiers note above for the implemented surface and the
  flagged maxref-boilerplate cleanup.
- ✅ **Unit-test coverage expanded** from 1 object to 10. New Catch tests with real
  input→output assertions (not just instantiation) for `tap.dcblock~` (filter
  recurrence, bypass/mute/clear), `tap.radians~` (all four conversions), `tap.zerox~`
  (per-sample trigger + normalized count), `tap.prime` (sequence + `next_prime`),
  `tap.sieve`, `tap.bits` (pack/explode round-trips), `tap.list.index`, `tap.change`,
  and `tap.biquadcalc` (RBJ coefficients vs. independently-computed references). `ctest`
  is green: **10/10 passing.**
- **Toolchain note:** `tap.crossfade~` and `tap.pan~` (pre-existing) don't compile under
  Linux/**GCC** (a `-Wchanges-meaning` + enum-`operator==` quirk in the `shapes`/`modes`
  enum pattern); they build fine under the CI clang/MSVC toolchains. Worth a small
  GCC-clean pass if local Linux builds are ever wanted, but not a CI blocker.

**Polish batch + runtime-test harness (2026-06-18, batch 3):**
- ✅ **`tap.verb~` oversampling**, ✅ **`tap.sustain~` multi-voice + rise**, ✅
  **`tap.filecontainer`** ported (Max native `sqlite` object via the C API), and ✅ a new
  unified **`tap.filter~`** (RBJ multimode biquad). See the frontiers/§8 notes for each.
  `tap.sustain~` graduates from EXPERIMENTAL single-voice to a 5-voice round-robin bank
  with per-voice equal-power rise. Unit tests added for verb~/filter~/sustain~ (the suite
  is now **24/24 green**); filecontainer is runtime-only (DB needs a live Max).
- ✅ **Runtime test harness wired up.** Cycling '74's [`max-test`](https://github.com/Cycling74/max-test)
  is vendored as a submodule under `runtime-tests/` (with `max-test-config.json`, a
  `make_maxtest.py` patcher generator, two starter `*.maxtest.maxpat` examples, and a
  `README.md`). This is the path to closing the **runtime validation in Max** gap: the
  Catch tests run against a mock kernel, whereas max-test loads the real objects in Max and
  asserts on actual (audio) behavior via `test.assert`/`test.sample~`/`test.terminate`,
  automatable over OSC with the bundled Ruby runner. It needs a Max **install** but **not a
  paid license** — the standalone "Max Runtime" was dropped in Max 7, and since then
  unlicensed Max runs patchers indefinitely (only *saving* is disabled), and the runner only
  runs them. So it stays a **local on-Mac gate** for now, with CI feasible later on a
  self-hosted macOS runner (gated by Max's GUI/activation, not by licensing). The example
  patchers, generated headless, want a first open-in-Max verification.

**New object — `tap.convolve~` (2026-07-12): a true-stereo convolution reverb.** The first
*net-new* DSP object of the revival (not a Jamoma port or a doc resurrection) — the only prior
art was the source-less `tap.selfconvolve~` help-patcher prototype on the `taptools-min` archive.
It convolves a stereo input with an impulse response held in a `buffer~` using **uniformly-
partitioned overlap-save (UPOLS)** FFT convolution (the standard convolution-reverb engine): the
IR is split into `blocksize`-sample partitions, each transformed once with the same in-house
radix-2 FFT as `tap.nr~`, and the output is a frequency-domain multiply-accumulate over a
frequency-domain delay line of past input spectra. Latency = one `blocksize`; otherwise it's exact
linear convolution.
  - **True stereo** — four IR paths (LL/LR/RL/RR) form the full 2×2 response; the two input
    channels are FFT'd once per block and shared across the paths. Paths are read from the bound
    `buffer~` by channel count: 4+ → true stereo, 2 → stereo (no cross-feed), 1 → mono on both
    diagonals.
  - **Click-free IR swaps** — the IR is (re)analysed off the audio thread in a deferred `queue`
    into an inactive double-buffered slot, then published with a single atomic (slot + partition
    count kept consistent), so loading a new IR while audio runs never reallocates or tears the
    table the perform loop reads. Engine geometry (`blocksize`/`maxsize`) is only (re)allocated in
    `dspsetup`, where perform is guaranteed idle — runtime changes to those two apply on the next
    DSP restart (documented in-attribute).
  - **Surface:** `mix` / `gain` / `predelay` / `normalize` (energy) / `blocksize` / `maxsize` /
    `bypass` / `mute`, mirroring `tap.verb~` conventions so the two reverbs read as siblings.
  - **Architecture:** the DSP core is a Min-free header (`conv_engine.h`) so it can be unit-tested
    directly — buffer~-backed objects can't link the mock kernel (which is why no `tap.buffer.*`
    object ships a test), so the portable engine is factored out and the `.cpp` is a thin Min shim.
    New **Catch unit test** (4 cases / 7 assertions, **green**) checks the engine against a direct
    time-domain convolution: true-stereo path correctness, one-block latency, delta⇒pure-delay,
    silent-when-no-IR, and a lock-free hot IR swap settling to the new response. Compile-verified
    against the Min/Max SDK toolchain (compiles *and* links the external on Linux/GCC).
  - **Deferred optimisation (noted in-file):** the MAC and IR tables use the full complex spectrum;
    a real-input half-spectrum (N/2+1 bins) form would halve both CPU and memory. **Runtime
    validation in Max still pending** (like the rest of the DSP set) — feel, IR-swap smoothness,
    CPU with long IRs, and the buffer channel-mapping all want a live audition.
  - **Vertical slice:** ships a maxref (`docs/tap.convolve~.maxref.xml`) and a help patcher
    (`help/tap.convolve~.maxhelp`, authored headless from the autohelp template — **wants a first
    open-in-Max check**, like the other new help patchers).
  - **Verification notebook + C ABI (new tooling, modelled on AmbiTap's `tools/capi` + `notebooks/`).**
    Because the portable engine is a Min-free header, a tiny **C ABI** (`tools/capi/taptools_capi.*`,
    a standalone CMake that needs no min-api) wraps `conv_engine`, and a **ctypes bridge**
    (`notebooks/taptools_py.py`, `Convolver` class + `PALETTE`, auto-builds the lib on first import)
    lets a Jupyter notebook drive the **actual shipping DSP** — not a Python re-implementation. The
    first notebook, `notebooks/convolution_reverb.ipynb`, machine-checks and plots five claims
    against the real engine: exactness vs. direct convolution (<1e-9), one-block latency +
    blocksize-invariance, the true-stereo 2×2 cross-feed, impulse→IR reconstruction with a Schroeder
    RT60 read-back (0.60 s target, 0.599 s measured), and the atomic dropout-free IR swap converging
    bit-identically to a from-scratch new-IR engine one block later. This is the first Python
    verification tooling in TapTools and the template for future DSP notebooks.

**Corpse pruned (step 5 done):** the dead trees have been removed now that all
objects are migrated and the build is self-contained on `min-api` — gone are the
old Jamoma `Core/`, the legacy `TapTools/` package (its docs/help were already
copied into `docs/`+`help/`, and its `.mxo` binaries were i386/x86_64-only),
`source/ttblue/`, every legacy `source/tap.*` wrapper, the old `max-sdk/` and
`objectivemax/` SDK/framework trees, plus `build.rb` and `.travis.yml`. All of it
is preserved in git history if any algorithm needs to be referenced again. The
working tree now contains only the modern package: `CMakeLists.txt`, `source/min-api`
(submodule), `source/projects/`, `docs/`, `help/`, `package-info.json.in`, and the
GitHub Actions CI.

**`tap.svf~` redesigned as a Simper/Cytomic morphing SVF (2026-07-12):**
- ✅ The Jamoma-faithful port (stereo Chamberlin + built-in LFO + portamento, batch above) is
  **replaced**, not patched: the new object is Andy Simper's trapezoidal-integration
  (zero-delay-feedback) SVF from his Cytomic papers — the design behind Ableton Live's
  filters, including Auto Filter's Morph type. Deliberate behavior break: the LFO and
  portamento are gone (patch a signal into the new right inlet for per-sample cutoff
  modulation; `smooth` covers glide), and the object is now **single-channel** (mc.-friendly)
  per house convention.
- Architecture follows the `tap.ladder~` kernel pattern: all DSP in a portable header
  (`svf.h`, `tap::tools::svf`, no Min dependency) under a thin wrapper. The kernel is
  genuinely **multichannel** (shared coefficients per tick(), per-channel state via
  `process(channel, x)`) for reuse outside Max; the Max object runs it with one channel.
- Surface: discrete types (lowpass/highpass/bandpass/notch/peak/allpass), a **morph** type
  sweeping continuously LP → BP → HP → notch → LP with corners bit-identical to the discrete
  modes, and parametric-EQ types (**bell/lowshelf/highshelf**, ±24 dB gain, always
  2nd-order) from Simper's coefficient tables. **Orders 2/4/8** (12/24/48 dB/oct) as a
  Butterworth-spread cascade — resonance 0 is maximally flat at every order; resonance
  (0..1) sharpens only the highest-Q section (a `q` message + kernel helpers convert
  Q ↔ resonance). Two **circuits**: `clean` (pure linear, never oversampled) and `driven`
  (drive dB into a tanh limiter on each section's band node, oversampled 1x/2x/4x, true
  bounded self-oscillation at resonance 1 — tuned slightly past the threshold so it
  actually sings).
- Full vertical slice: rewritten maxref + help patcher, a `tap.svf~` runtime-test patcher
  (`make_maxtest.py` gained a `numinlets` arg), and a 14-scenario Catch suite (Butterworth
  −3 dB at fc for all orders, measured 12/24/48 dB/oct slopes, morph-corner identity,
  bell/shelf gain targets, allpass unity, notch depth, self-oscillation frequency/bounds,
  modulation-abuse stability, multichannel state independence). Suite green: **27/27**.
  Runtime validation in Max still pending, as everywhere.
- **Benchmarks + ratchet (`benchmarks/`).** `svf_bench` (kernel-only executable, config matrix,
  best-of-N ns/sample) + `compare.py` gate per-machine baselines against >5% regressions.
  First optimization pass landed: the coefficient update is split into a shape tier (damping /
  output mix / EQ gains — refreshed only when those parameters actually change) and a cutoff
  tier (tan + three solve constants per section, skipped when the cutoff is unchanged), and the
  core is templated on the circuit. Signal-rate-modulated cases dropped 28–64% (e.g. modulated
  2nd-order lowpass 36→19 ns/sample, modulated morph 77→28 on the Linux reference box) with
  bit-identical output (the morph-corner identity tests pin this). The authoritative baseline
  should be recorded on an arm64 Mac. Still open, by choice: fast-tanh for the driven circuit
  and a polyphase halfband resampler — both change output microscopically, awaiting sign-off.
- ✅ **`tap.vco~` added (2026-07-11)** — a **virtual-analog oscillator**, the source-side
  companion to `tap.ladder~` and the repo's first oscillator (`tap.noise~` was the only
  generator). Kernel (`source/projects/tap.vco_tilde/vco.h`, `tap::tools::vco::vco_osc`):
  **polyBLEP** alias-suppressed saw and pulse (PWM 1–99%), triangle via leaky integration
  of the BLEP square, pure sine — all driven by one master phase and **morphed by a
  continuous `shape` parameter (0 sine → 1 tri → 2 saw → 3 pulse)** that rides the
  per-sample ramps, so shape sweeps and preset morphs glide through hybrid waveforms.
  Full VCO tier: **hard sync** (rising-zero-cross reset with sub-sample accuracy and a
  one-sided first-order step correction; minBLEP tables flagged as the upgrade path),
  **through-zero linear FM** calibrated in Hz (negative effective frequency runs the
  phase backward, tested bounded at depths far past the carrier), and an
  **analog-character section** — slow random pitch drift (S&H ~2 Hz through a ~0.5 Hz
  one-pole, depth in cents) plus static detune, **deterministic per `seed`** so renders
  and tests reproduce and mc. instances decorrelate by seed. House ramps + 16-slot preset
  morphing. Wrapper: 3 inlets (frequency signal/float — true per-sample; FM; sync),
  `waveform sine|triangle|saw|pulse` snap message. 10 Catch scenarios (frequency
  accuracy, **alias suppression > 35 dB at a folded 13th harmonic** — the VA proof —,
  sine/triangle purity, PWM duty cycle, morph continuity, sync locking, TZFM sidebands +
  boundedness, drift determinism/decorrelation, preset morph, wrapper defaults) plus
  `vco_render` demos (PWM pad, sync sweep, TZFM growl, shape-morph tour, preset morph).
  Compile/ctest-verified on Linux/GCC; **audio still needs runtime validation in Max.**
- ✅ **`tap.ladder~` added (2026-07-11)** — a **virtual-analog transistor-ladder filter**,
  the nonlinear sibling of `tap.fourpole~` (which stays as the cheap linear Stilson/Smith
  ladder; the two maxrefs cross-reference, and fourpole~ finally *got* a maxref — it had
  none). New ground for the repo: **nothing else here had tanh/nonlinear feedback or
  filter oversampling**. Kernel (`source/projects/tap.ladder_tilde/ladder.h`,
  `tap::tools::ladder::ladder_filter`): zero-delay-feedback TPT 4-stage ladder, prewarped
  tuning, per-stage tanh via linear-ZDF prediction + one corrective nonlinear pass
  (full Newton solve flagged in-file as a possible future upgrade — unnecessary at these
  oversampling factors), `k = 4·resonance` up to 4.4 for clean bounded self-oscillation
  (unit-tested: oscillation frequency within 3% at 1 kHz *and* 8 kHz — the top-octave
  accuracy the Stilson/Smith model can't deliver), `drive` into the nonlinearity, `comp`
  passband-loss compensation, **1/2/4× oversampling** (4th-order Butterworth anti-image/
  anti-alias cascades, tap.verb~ pattern, default 2×), and **Xpander-style pole-mixing
  multimode** (lp24/lp12/bp12/bp24/hp12/hp24). Wrapper: mono + a right inlet for **true
  per-sample signal-rate cutoff** (one better than tap.filter~'s per-vector read), house
  ramps + 16-slot preset morphing. 10 Catch scenarios (self-osc tuning/boundedness,
  24 dB/oct slope, resonance monotonicity, drive THD, mode shapes, comp, oversampling
  passband-consistency + alias reduction at a driven 5 kHz tone, morph/sweep continuity)
  plus `ladder_render` demos. Note in docs: exact unity gain is impossible in a
  saturating filter — the runtime maxtest uses a small signal with a loosened tolerance.
  Compile/ctest-verified on Linux/GCC; **audio still needs runtime validation in Max.**
  **v1.1 (same day):** two upgrades from comparing the model against
  Simper/Zavalishin/Arturia methodology — **`asym`** (0..1, morphable: a biased operating
  point in every tanh stage models transistor mismatch and adds the even harmonics of real
  hardware; 0 = symmetric, verified: 2nd harmonic < 1e-8 relative at 0, orders of
  magnitude up when engaged) and **`solver`** (0 = fast one-pass default, 1 = **exact
  Newton iteration to convergence** on the true nonlinear loop, seeded by the linear
  prediction, clamped + fallback-guarded). Tests: solvers agree within 1e-2 at gentle
  settings, exact stays finite at max drive + max res + full asym, and still
  self-oscillates in tune.
- ✅ **`tap.shift~` engine modernized + `tap.semitone2ratio` resurrected (2026-07-11)** —
  the author approved retiring tt_shift's implementation artifacts rather than preserving
  them: the taps are now **Hermite-interpolated** (was linear with a constant one-sample
  offset), the grain envelopes are an exact **complementary Hann pair summing to 1** at
  every phase (replacing the 256-point padded-Welch table whose uneven sum imposed an
  amplitude ripple at the grain rate — verified by a new DC-through-moving-taps test that
  holds unity to 1e-9), and **ratio/window_size ride per-sample ramps** (new `smooth`
  attribute, default 20 ms) so window changes no longer click. The **two float inlets of
  the original (ratio, window ms) are restored** — the first Min port had dropped them
  (the legacy wrapper's 3-inlet surface recovered from the `legacy` branch). Control
  surface otherwise unchanged. Also fixed: the object had **no unit tests and no
  `min-object-unittest` include in its CMakeLists** — both added (Goertzel transposition
  checks at ±1 octave in normalized frequency, level-invariance, glide continuity,
  clamping), plus runtime maxtest patchers for both objects. **`tap.semitone2ratio`**
  (§3 candidate) is resurrected as a native object — the legacy version was a patcher
  abstraction computing `mtof(60+st)/mtof(60)`, implemented directly as `2^(st/12)`
  (numbers, lists, bang-to-repeat; unit-tested) — which un-breaks `tap.shift~`'s help
  patcher, whose signal chain depends on it. Both maxrefs rewritten (the old shift maxref
  documented a nonexistent `windowsize` attribute — it is `window_size`). Legacy help
  patcher for `tap.semitone2ratio` ported as-is (valid modern JSON). Compile/ctest-verified
  on Linux/GCC; **wants the usual open-in-Max audition.**
- ✅ **`tap.5comb~` resurrected (2026-07-11)** — recreated as a native external modeled on
  the **GRM Tools Classic "Comb Filters"** plugin, not ported from the legacy version: the
  legacy `tap.5comb~` was a patcher *abstraction* over five `tap.comb~` objects (recovered
  from git history at `b62bba8^`) and never matched the GRM sound — integer-sample delays
  detuned the combs and killed the beating between them, control-rate stepping zippered on
  sweeps, and the in-loop hard clipper distorted at high resonance. Only its 20-name
  parameter surface (`freq`/`res`/`lp` masters, `freq1..5`/`res1..5`/`lp1..5`, `gain`,
  `mix`) and 5 Hz frequency floor were kept. **All DSP lives in a portable, header-only,
  Max-free kernel** (`source/projects/tap.5comb_tilde/grm_comb.h`,
  `tap::tools::fivecomb::comb_bank`) — a first for the repo, per the "Min is a thin shim"
  philosophy — with: fractional delays (4-point Hermite), every parameter on a per-sample
  linear ramp, resonance mapped to ring time on a log curve (20 ms → 100 s; feedback derived
  from the current delay, capped at 0.99999 — no clipper, DC blocker in the loop instead),
  an exact one-pole feedback lowpass, and a **16-slot preset-morph engine** (GRM's hallmark:
  `store`/`recall` interpolates everything over a settable time; grabbing one slider
  mid-morph overrides just that parameter). New capabilities beyond the Classic: `warp`
  (negative-coefficient in-loop allpass → stiff-string inharmonic partial stretch,
  fundamental-compensated) and `phase` (half-loop pickup tap; 100 = odd harmonics only),
  both neutral at 0. Ships the full slice: maxref, help patcher, two runtime maxtest
  patchers, 11 Catch scenarios (echo spacing, RT60-vs-res, morph continuity, warp/phase
  spectra via Goertzel, master math, mix law), and `grm_comb_render` — a kernel-only offline
  WAV renderer (lands in `tests/`) proving the kernel runs outside Max and giving the
  listening-check material. **Deliberate deviations flagged for audition:** 1/5 wet-sum
  normalization (legacy summed raw and ran hot), res→ring-time map (legacy was linear
  feedback), equal-power mix. Compile/ctest-verified on Linux/GCC; **audio still needs
  runtime validation in Max.**

- ✅ **`tap.pitchaccum~` added (2026-07-11)** — second GRM recreation, **net-new** (no legacy
  TapTools ancestor): the GRM Tools Classic **"PitchAccum"** — two independent granular
  transposers ("shadows"), each ±24 semitones with its own delay (≤ 3 s), feedback, and
  gain, where the feedback **re-enters the transposer** so pitch accumulates pass after
  pass (the shimmer/spiral the plugin is named for). Same architecture as `tap.5comb~`:
  all DSP in a portable header-only kernel
  (`source/projects/tap.pitchaccum_tilde/grm_pitchaccum.h`,
  `tap::tools::pitchaccum::accum_bank`), 17 per-sample-ramped parameters, 16-slot
  preset-morph engine, thin Min shim. The transposer is the `tap.shift~`/tt_shift engine
  (phasor sweeping two taps half a cycle apart) modernized: Hermite fractional taps and a
  **complementary cos²-flank envelope pair that sums exactly to 1** (constant level at any
  `xfade`, vs tt_shift's fixed padded-Welch ripple), with the crossfade width exposed as
  GRM's Cross-fade control. Global LFO (voice 2 phase-offset via `modphase`) + per-voice
  deterministic random transposition modulation; DC blocker in each feedback loop, fb
  capped at 0.99. Optional **pitch follower** (`follow`, default off): decimated
  normalized autocorrelation, smallest-lag-near-max peak picking (global max picks
  subharmonics — caught by the unit test), confidence-gated, window → ~2 detected periods.
  GRM's stereo-width fader intentionally dropped (mono object; `mc.` for multichannel).
  Full slice: maxref, help patcher, runtime maxtest, 10 Catch scenarios (Goertzel
  transposition checks, the two-pass accumulation signature at +7→+14 st, delay timing,
  modulation spread, bit-exact determinism, morph continuity, follower convergence and
  noise fallback, 0.99-feedback boundedness), and `grm_pitchaccum_render` (offline WAV
  demos, lands in `tests/`). Compile/ctest-verified on Linux/GCC; **audio still needs
  runtime validation in Max.**

- ✅ **Kernel/wrapper separation staged (2026-07-14)** — the portable DSP layer now lives in
  **`kernel/`**, a complete standalone CMake project with **zero Max dependency**, staged for
  extraction into its own repository (the **AmbiTap / AmbiTap-Max** pattern: kernel repo =
  header-only C++ library consumed by the wrapper repo as a pinned submodule via a
  `*_ROOT`-style override). What moved: the six extracted kernel headers
  (`ladder.h`, `svf.h`, `vco.h`, `grm_comb.h`, `grm_pitchaccum.h`, `conv_engine.h`) →
  `kernel/include/taptools/` (with `conv_engine` brought into the `tap::tools` namespace — it was
  the only global-namespace kernel — plus a `taptools/taptools.h` umbrella); the offline render
  tools → `kernel/tools/render/`; the C ABI → `kernel/tools/capi/`; the notebooks →
  `kernel/notebooks/`; `benchmarks/` + `svf_bench` → `kernel/bench/`. The kernel exports a
  header-only `TapTools::taptools` target (C++17-clean — the min-api test harness compiles
  wrapper TUs at C++17) with install/`find_package(TapToolsKernel)` support, and grew its own
  **Max-free Catch2 test suite** (`kernel/tests/`, FetchContent) seeded with the conv_engine
  tests, which no longer need the min mock kernel. The externals consume only
  `${TAPTOOLS_KERNEL_DIR}/include` (root `CMakeLists.txt`, defaults to `kernel/`, overridable —
  this becomes the submodule pin after the split). CI gained a 3-OS `kernel` job
  (build + ctest, no submodules); the clang-format gate now covers `kernel/`. Verified: kernel
  standalone build + tests green, Max package configure + all wrapper tests green, zero
  behavioral change. Paths cited in older log entries above predate these moves. **Next
  extractions:** the spectral trio (`tap.nr~` / `tap.spectra~` / `tap.vocoder~`) whose STFT math
  is already portable but inline — consolidating the radix-2 FFT currently duplicated between
  `conv_engine.h` and `tap.nr~` into a shared kernel header — then simple inline-DSP objects
  opportunistically. Control/utility and Jitter objects are Max-bound by nature and stay
  wrapper-side permanently.

- ✅ **`tap.autowah~` added (2026-07-15)** — an **envelope filter / auto-wah modeled on the
  Mad Professor Snow White AutoWah** (BJF's LM13700 OTA-SVF design), the third net-new DSP
  object and the follow-through on the `tap.autowah~` prototype idea from the `taptools-min`
  archive (§8). Full design + hardware research + validation method: **`plans/tap.autowah~.md`**.
  First object developed **cross-repo from the start**: the kernel
  (`include/taptools/autowah.h`, `tap::tools::autowah::wah_filter`) landed in `tap/taptools`
  and the submodule pin was bumped here. Also the first **kernel composing another kernel** —
  it wraps `svf.h`'s Simper filter (one 2nd-order section, per-sample `tick(cutoff_hz)`)
  behind a sensitivity→rectifier→attack/release detector and an exponential `bias · 2^(sweep ·
  range)` law (isolated in `map_cutoff()` — the hardware calibration pass may swap it for
  linear-in-Hz, see the plan §7.6). Faithful: fast fixed attack, Decay fall-back, upward
  250→2500 Hz sweep, wet-only, and sensitivity-at-floor = the pedal's **cocked-wah manual
  mode**. Extensions (neutral defaults): mix, down-sweep `direction`, `drive` into the svf
  driven circuit (2×OS), bandpass tap (`mode`), adjustable `attack`, half/full-wave rectifier
  select (kernel-side A/B), **sidechain right inlet** and an **envelope (0..1) right outlet**.
  House ramps + 16-slot preset morph, with **factory voicings in slots 0–3** (guitar, bass,
  slow swell, cocked wah — the GB pedal's GTR/BASS switch is just a preset morph here). Kernel:
  9 Catch scenarios (attack/decay timing, sweep law + ceiling, burst sweep ± direction,
  cocked-wah **bit-equivalence** to a bare `svf_filter`, Q monotonicity, sidechain routing,
  max-everything boundedness, bit-exact determinism, preset morph) and `autowah_render`
  (factory-voicing funk-chop demos **plus a `--in` WAV mode** for the hardware-vs-model
  comparisons in the validation plan). Wrapper: 4-scenario Min test (defaults, clamping,
  range/direction sign forwarding, preset/clear plumbing), maxref, help patcher, and a runtime
  maxtest (cocked-mode DC-unity; `make_maxtest.py` gained a `numoutlets` arg). Compile/ctest
  green on Linux/GCC in both repos; **audio needs runtime validation in Max**, and the
  **hardware calibration pass** (a real Snow White is on order) will tune the envelope
  constants, sweep law, and default filter tap against reamped recordings.
  **Same-day follow-up — verification notebooks (kernel repo):** the C ABI now wraps
  all four parameter-indexed kernels (was conv_engine-only) and four new executed
  notebooks join `convolution_reverb.ipynb`: **`svf.ipynb`**, **`ladder.ipynb`**,
  **`vco.ipynb`**, and **`autowah_validation.ipynb`** — every documented claim
  measured against the shipping C++ through ctypes (Butterworth slopes and
  bit-identical morph corners; in-tune self-oscillation, asym even harmonics, and
  oversampling alias reduction; polyBLEP alias suppression, TZFM, sync, drift
  determinism; and the autowah's full §6 validation harness with the wet-audio-only
  peak-trajectory extractor and a waiting hardware-comparison cell).

- ✅ **`tap.vco~` analog-character v2 + the book (2026-07-15).** Four upgrades toward
  real hardware, all deterministic per seed and **exactly zero by default** (kernel-pinned:
  at `imperfect` 0 every seed renders bit-identically): **`imperfect`** (0..1 waveform
  imperfection with per-seed component tolerances — bowed shark-fin saw with a rounded
  reset corner measured 6.4 dB down at h40, asymmetric triangle whose 2nd harmonic rises
  from −185 dB to −34 dB at 0.8, mild sine THD, per-seed pulse-width offset — every seed a
  different unit off the line), **`jitter`** (0..20 cents fast pitch noise, the short-time
  companion to `drift`), and **`track`** (±10 cents/oct V/oct calibration error from A440,
  measured exact at the trim point and ±15 c at ±3 octaves with track 5). Kernel-side test
  suite added (`tests/vco_test.cpp`, 6 scenarios — an honest measurement note: the saw bend
  is a shape/phase effect, its parabola sits in quadrature with the saw's components, so
  the test pins the waveform bow and the corner rounding, not a phantom harmonic shift);
  `vco.ipynb` §7 measures everything; wrapper gains the three attributes + maxref entries.
  **And the book:** the kernel repo now has **`book/` — *Tools on Tap***, the mdBook field
  guide in the AmbiTap/SampleRateTap/MuTap (*Quieting the Loop*) pattern, first chapter
  **"The oscillator and its knobs"** (the aliasing contract, every attribute as a trade,
  the analog section with measured numbers, the honest Moog recipe), published to Pages by
  the kernel repo's new `docs.yml`.

- ✅ **`tap.808.*` §7.2 calibration against a real unit (2026-07-17).** The plan's
  sample-pack comparison, done quantitatively: the **Fischer/Technopolis 1994 TR-808
  sample set** (unit s/n 103852, recorded from the individual outs,
  machines.hyperreal.org) encodes **knob positions in its filenames** (0/2.5/5/7.5/10
  dial grid, 116 samples), so the comparison ran per knob cell, not by ear:
  `tr808_render` gained repeatable **`--set NAME VALUE`** knob args to re-render the
  exact grid, and identical measurements (spectral-peak fundamental, −40 dB decay,
  power centroid) were applied to both sides. The tunings were already right nearly
  everywhere — kick fundamental within 2.4% and decay endpoints within 6% across the
  whole tone×decay grid, snare within 1.2% including its tone-max mode flip, every
  tom/conga/cowbell/claves pitch within ~4% — while the decay classes and noise
  voicings were **re-fit to the measured unit**: tom/conga/cowbell/clap tails roughly
  doubled (the chart's figures read as a hotter reference level), the snare's snappy
  path band-limited (the schematic high-pass alone left it white to Nyquist) and
  re-enveloped with a fitted VR9 dial law, the rimshot re-voiced low-dominant (the
  real RS peaks at ~455 Hz), and the cymbal's decay span and brightness corrected to
  the measured 0.65–2.7 s / centroid range. Every kernel header now carries a §7.2
  calibration note with its numbers and honest residuals. The one structural residual —
  the closed hat measuring ~29% brighter than the shared-band model could produce — was
  then resolved by the **hats' sizzle blend** (the bank's raw upper harmonics, ~13 kHz
  high-passed, weighted per path inside the existing VCAs): CH centroid now −1.3%, OH
  +0.5%, pinned by a HF-fraction test. The whole workflow is reproducible from the
  kernel repo as **`notebooks/tr808_calibration.ipynb`** (house notebook pattern:
  downloads and caches the reference set, re-renders the 116-cell grid via
  `tr808_render --set`, prints per-voice delta tables, overlays spectra/envelopes;
  committed executed — median |Δ| fundamentals 0.4–4.8%, decays mostly 1–8%). Kernel
  suite green at 87 scenarios with the decay pins updated to the calibrated classes.

- ✅ **`tap.808.*` slice 5 — family polish (2026-07-17). PHASE 1 OF THE PLAN IS
  COMPLETE.** Kernel side: **family output balance** — the bridged-T's impulse gain
  grows with fc·Q, so the six tom/conga channels and the claves left their resonators
  at wildly different levels (high conga ~5.3 at full accent, ~6× the mid tom; claves
  ~6× the rimshot); per-channel summing gains (`k_tomc_mix[2][3]`, `k_cl_mix` — the
  hardware's per-channel summing resistors into the mix bus) now land every channel's
  full-accent peak in a consistent band at any knob position, pinned by new balance
  scenarios in the kernel tests. Plus **`tools/render/tr808_render`**: demo mode
  renders each voice's characteristic moves and an eight-voice kit pattern; `--hit`
  mode renders single hits — the stimulus for the plan's §7.2 golden-render
  comparisons. Wrapper side: **`help/tap.808.maxhelp`**, the family overview patcher —
  all eight voices wired to a 16-step, two-bar pattern with the accent-bus contract
  documented once (edge amplitude = accent on the 4–14 V bus, `bang`/`trigger`/
  `clear`, seeded determinism); every voice's maxref cross-references it, and the
  family's `RELATED` metadata is cross-linked. Still open: runtime validation in Max
  (§7.3) and the sample-pack calibration itself (§7.2).

- ✅ **`tap.808.tom~` + `tap.808.rim~` added (2026-07-17, slice 4) — ALL EIGHT
  `tap.808.*` VOICE CHANNELS NOW SHIPPED.** The resonator variations over the shared
  `bridged_t.h` core (kernel `tr808_tom.h` / `tr808_rim.h`): the three tom/conga
  channels (`@size` × `@model` = the hardware's six sounds; p.14 chart tunings with
  the panel knob sweeping each span; the D80/D81 attack pitch fall as an
  amplitude-dependent leg, the kick's mechanism class; toms' pink-noise "artificial
  reverberation" — congas ring pure) and the RS/CL channel (rimshot = ~1667 + ~455 Hz
  — the 455 computes from its own schematic parts — cut to ~10 ms by the
  harmonic-generating VCA, modeled as tanh drive; claves = pure ~2500 Hz, ~25 ms).
  12 kernel scenarios (84 total green), 4 wrapper scenarios (42 total green on
  Linux), maxrefs/helps/maxtests. Test craft worth noting: tom frequency probes use
  spectral peaks (the noise layer pollutes zero-crossing counts) and the noise-layer
  pin asks the seed. **Runtime validation in Max still open** across the family; next
  per the plan: slice 5 polish, then the phase-2 WDF gate and the phase-3 sequencer
  decision.
- ✅ **`tap.808.hat~` + `tap.808.cymbal~` + `tap.808.cowbell~` added (2026-07-17,
  slice 3)** — the metal voices, completing the family's shared blocks. Kernel:
  **`metal_bank.h`** — the six Schmitt-trigger squares every 808 metal voice draws on
  (205.3/369.6/304.4/522.7 Hz nominal + the trimpot pair at 800/540 Hz — Roland's own
  "1.25 ms / 1.85 ms" margin notes on the voicing-board schematic; duty 47.98%), with a
  deterministic per-seed **±20% `tolerance` spread** (the documented production variance
  — why no two 808s' cymbals match; `vco.h` `imperfect` convention), the ~3440/7100 Hz
  band-pass voicings and the Q19 attack smoother (τ 102.44 µs, VBE 0.7258 — the
  Werner/Abel/Smith cymbal paper's least-squares fits). **`tr808_hat.h`** — closed and
  open as ONE circuit with two triggers, per the Service Notes, with the **hardware
  choke** (Q23/R173: a CH trigger terminates a sounding OH) implemented and pinned by
  test; CH fixed ~50 ms class, OH decay pot 90-600 ms, per-path levels →
  `tap.808.hat~` is `sample_operator<2,1>` (first two-inlet voice; `open <accent>`
  message). **`tr808_cymbal.h`** — strike/ring/body components over the two bands,
  VR2 decay spanning the chart's 350-1200 ms, VR4 tone as the strike/body ratio.
  **`tr808_cowbell.h`** — the 540/800 pair through the two-slope "abrupt initial
  decay" envelope (C9 + R82/C34) and the ~860 Hz IC2 voicing derived from the
  schematic's MFB values. 18 kernel scenarios (74 total green) + 7 wrapper scenarios
  (40 total green on Linux); maxrefs, help patchers, maxtest patchers for all three.
  **Runtime validation in Max still open.**
- ✅ **`tap.808.snare~` + `tap.808.clap~` added (2026-07-17, slice 2)** — the snare and
  the CP/MA channel, **primary-sourced from the TR-808 Service Notes themselves** (the
  manual was fetched and its p.6 circuit descriptions, p.9 main-board schematic, and
  p.14 voice chart read directly — every component value carries its designator).
  Kernel: **`swing_vca.h`** (shared noise-voice blocks: RC `decay_env`, linear swing-VCA,
  seeded xorshift64* `white_noise` — deterministic renders, mc. decorrelation by seed),
  **`tr808_snare.h`** (two bridged-T resonators reusing `bridged_t.h` — stock at the
  **late-revision ~173.3/336 Hz** tuning with Roland's design change from the first
  edition's ~250/499 Hz documented; the R189/C57/R190 trigger divider — its omission
  made the first build 40x too hot, caught by the calibration test; ~4 kHz snappy path
  with its 14 ms RC envelope; VR8 tone crossfade / VR9 snappy / VR7 level; `tuning` +
  `seed` bends), **`tr808_clap.h`** (the CP/MA switch as `@model`: the clap's IC21
  **dual** band-pass — schematic-exact ~2.06 kHz, *not* the ~1 kHz folklore — the
  Figure-13 **three-teeth sawtooth envelope** ~10 ms apart, the Q70 reverberation wash
  with a `tail` bend that disconnects it; maracas as the short bright burst). 19 new
  kernel scenarios (59 total green); wrappers with 8 more scenarios (37 total green on
  Linux), maxrefs, help patchers, maxtest patchers. The GCC `attribute<symbol>` cast
  ambiguity resurfaced in the clap test — solved with the house `== symbol{"..."}`
  idiom (§9 item 5 precedent). **Runtime validation in Max still open.**
- ✅ **`tap.808.kick~` added (2026-07-17)** — the first of the **`tap.808.*` drum-voice
  family** (plan: **`plans/tap.808.md`**): a **circuit-informed TR-808 bass drum**, block for
  block after Werner/Abel/Smith (DAFx-14) with component values read off the Service Notes
  schematic in the paper's Fig. 1. Kernel-first, cross-repo: **`bridged_t.h`** (the op-amp
  bridged-T resonator every 808 voice shares — trapezoidal companion-model solve on the
  network states, so the time-varying leg resistance needs no per-sample coefficient redesign;
  topology verified by re-deriving the paper's printed transfer functions, which it reproduces
  exactly) + **`tr808_kick.h`** (pulse shaper + diode clamp, behavioral envelope generator,
  Q43 attack shift ~49→~129 Hz for ~6 ms, retriggering pulse, the fitted pitch-sigh
  nonlinearity — the paper's Eqn. 9 is garbled in print, so the leg formula is re-derived from
  KCL and matches their stated limits — feedback-buffer decay loop, tone/level/output stages).
  **Family contract established:** signal-rate trigger where the rising edge's amplitude
  (0..1) *is* the accent (mapped to the hardware's 4–14 V trigger bus — harder excitation, not
  post-gain), `bang`/`trigger <float>` convenience messages, panel knobs as 0..1 attributes,
  circuit bends stock-neutral at defaults (`tuning`, `pulse`, `sigh`, `attack`). Measured along
  the way: the attack shift is **where the punch comes from** — with it disconnected the same
  trigger barely couples into the ~49 Hz resonator (~15× quieter), matching the hardware-mod
  lore. Kernel: 14 Catch scenarios (~49 Hz ring, attack punch + disconnection, sigh +
  disconnection, decay monotonicity — 0.15/0.63/2.4 s at pot 0.1/0.5/1.0, consistent with the
  paper's "multi-second" top end — accent monotonicity, tone brightness, tuning bend, no
  machine-gun retrigger, bit-exact determinism, long-tail silence). Wrapper: 5-scenario Min
  test (defaults, clamping, silence-at-rest/bang, edge-amplitude accent, clear), maxref, help
  patcher, runtime maxtest. Compile/ctest green on Linux/GCC in both repos; **audio needs
  runtime validation in Max**. `swing_vca.h` deferred to slice 2 (the kick has none; the
  snare/clap analysis should drive its design).

- ✅ **`tap.diode~` added (2026-07-17, tap.303 slice 1)** — a **virtual-analog diode-ladder
  filter, the TB-303 topology** (plan: **`plans/tap.303.md`**; §8 blocking decisions approved
  same day) and the first deliverable of the 303 recreation. Kernel-first:
  **`diode_ladder.h`** (`tap::tools::diode`), a ZDF 4-stage *unbuffered* diode chain whose
  linearized transfer function reproduces **Stinchcombe's published TB-303 response exactly**
  — the header derives that the equal-component chain with the top cap halved yields his
  normalized denominator (4·2^(3/4), 10√2, 8·2^(1/4)) to four digits, that Routh–Hurwitz puts
  the oscillation threshold at **exactly k = 17** (Open303's 1/17), and that oscillation sits
  at √2× the stage rate — which the prewarped tuning maps onto `frequency` exactly. tanh on
  every diode-pair edge (the coupling elements saturate, not buffer amps), secant-gain ZDF
  solvers (`fast`/`exact`), 1/2/4× oversampling, house ramps + 16-slot preset morphing.
  **Emergent authenticity, found and pinned during implementation:** with the hardware's
  150 Hz feedback high-pass (Open303's calibrated value) in the loop, its phase lead raises
  the needed k to ~19 at 2 kHz / ~25 at 500 Hz — so at stock resonance the model, **like a
  real TB-303, famously never quite self-oscillates**. The resonance range extends to 1.5 as
  the documented Devil-Fish-style bend (it sings up high, riding sharp near the corner by the
  predicted phase shift), and `fbhp 0` bends to DC-coupled feedback with exact-tuned
  oscillation. Kernel: 10 Catch scenarios (the analytic Stinchcombe magnitudes at fc/2fc/8fc
  ±1 dB, the ~14 dB first octave + 24 dB/oct asymptote, fc-independent tuning to 0.5%,
  stock-no-osc + bent-osc-sharp, resonance thinning with unity DC, solver agreement/
  boundedness, alias reduction, morph continuity, determinism, tail safety), `diode_render`
  demo WAVs, `diode_bench` + container baseline. Wrapper: `tap.diode~` (mono + per-sample
  signal-rate cutoff right inlet, the `tap.ladder~` surface minus `comp`/`asym`/`mode` — the
  header documents why each is absent — plus `fbhp`), Min test (defaults, clamping,
  kernel-reach, stock-vs-bent smoke), maxref, help patcher, runtime maxtest. Compile/ctest
  green on Linux/GCC in both repos; **audio needs runtime validation in Max**. Next
  (slice 2): `tb303_voice.h` — oscillator + envelopes around this filter → `tap.303~`.

- ✅ **`tap.303~` added (2026-07-17, tap.303 slice 2)** — the **acid-bass voice**, TapTools'
  first pitched, note-driven instrument, implementing the approved **melodic-voice contract**:
  pitch as a MIDI-note signal (left inlet, per-sample; floats accepted), a gate signal whose
  **edge amplitude encodes accent** (1 = plain, 2 = full — the 808 trigger convention's
  melodic sibling), and **slide as legato** — a pitch change while the gate is held glides
  through the hardware's ~60 ms RC without retriggering, no separate slide input (pulled
  forward from plan slice 3: the contract requires legato semantics from day one). Kernel
  (`tb303_voice.h`, `tap::tools::tb303::voice`) composes the existing kernels — `vco.h`'s
  polyBLEP saw/square driven per sample, `diode_ladder.h` with per-sample envelope-modulated
  cutoff — around the voice circuits with **Open303-calibrated constants**: decay-only MEG
  (3 ms rise; 200–2000 ms knob; accent bypasses it to ~200 ms), the 2/3-up env-mod cutoff law
  with the "gimmick" resting-point offset, fixed VCA envelope (~1.23 s decay, no sustain,
  ~2 ms gate-off chop), 44.5/24 Hz coupling high-passes. **Deliberately deferred to slice 3:**
  the C13 accent-sweep capacitor (the across-notes "wow") and its resonance-pot scaling —
  slice-2 accent is the routing level only (louder + faster MEG). The 303's square-from-saw
  shaper is approximated by the polyBLEP pulse this slice (flagged; cf. Open303's tables).
  Kernel: 8 Catch scenarios (gate/legato/retrigger semantics, pitch + tuning accuracy to 1%,
  envmod brightness sweep + decay-knob scaling, VCA decay profile, slice-2 accent scope,
  saw/square spectra, determinism, tail hygiene — suite 103/103 green) and `tb303_render`
  (16-step accent/slide pattern demos — the §7.2 calibration material). Wrapper: full slice —
  `note <pitch> [accent] [slide]` / `bang` / `release` messages, panel attributes, maxref,
  help patcher, runtime maxtest. Compile/ctest green on Linux/GCC in both repos; **audio
  needs runtime validation in Max**. Next (slice 3): the accent-sweep circuit — the wow.

- ✅ **`tap.303~` accent-sweep circuit (2026-07-17, tap.303 slice 3)** — **the wow**: the C13
  capacitor circuit that makes runs of accented notes bloom, modeled from Devil Fish's
  component-level description (diode + 47k into the resonance pot's second section, 1 µF to
  ground, 100k mix into the cutoff sum). In the kernel: diode-gated charge (tau = 47k·1µF =
  47 ms, component-derived), ~150 ms drain through the output path (the across-notes memory),
  the direct-MEG-minus-cap curve shaping (Devil Fish's "~100/147 of the MEG minus the cap
  voltage" — what rounds the first accent), the ganged resonance-pot scaling (accent quacks
  harder at high resonance), and direct injection into the cutoff sum **bypassing envmod** —
  accents sweep even with envmod at zero, like the circuit. Measured (h8-brightness units,
  16th-note spacing): first accent 3.75 → second 9.70 riding the residual charge → saturating
  ~10.3; after a rest the charge drains and the wow starts over at 3.75. Two new Catch
  scenarios pin the buildup monotonicity + fade, envmod independence, resonance scaling, and
  plain-notes-never-charge (suite **105/105 green**); `tb303_wow.wav` render added (plain bar
  → two all-accent bars). Wrapper/maxref accent descriptions updated; no interface change —
  the sweep rides the existing note contract. Still flagged for slice 4: the
  `k_accent_sweep_oct`/direct-weight calibration against Open303 renders, the square shaper,
  seed/tolerance. **Runtime validation in Max still pending**, as everywhere.

- ✅ **`tap.303~` slice 4 — Open303 calibration + the bends (2026-07-17).** Phase 1 of the 303
  plan is complete. **Open303 was built and rendered side by side** (its 2008-era DSPCode
  compiles standalone with two forced includes) and the calibration is measured, not guessed:
  - The envmod law is now **Open303's hardware-measured mapping verbatim** (their
    `calculateEnvModScalerAndOffset` constants): ~4.5–5 octaves at full envmod, 2/3 above the
    knob, a residual ~0.8-octave sweep at envmod 0, cutoff-dependent depth.
  - The square is the real thing: **−tanh(10^(36.9/20)·saw_shifted + 4.37)** — Open303's
    measured transistor-shaper constants applied to our polyBLEP saw (keeping the edges
    bandlimited); `waveform` becomes a continuous saw↔square blend. Measured h2/h1 = 0.076
    vs Open303's 0.108 (both an order under their saws' ~0.4–0.55) — the same shaped duty.
  - Their accent path turned out to be a **memoryless 15 ms integrator** — no across-notes
    wow — so our Devil-Fish C13 model stays (deliberate, documented divergence).
  - **Devil-Fish bends** as morphable params, stock at defaults: `slide` (10–500 ms),
    `attack` (0.3–30 ms Soft Attack), `accdecay` (50–2000 ms accent clock), `drive`
    (±24 dB into the diode ladder — measured: 9.2× RMS growth for a 15.85× gain increase,
    the diodes compress).
  - **`seed`/`tolerance`** per-unit spread (tuning trim, VCF tracking, envelope/slide/C13
    RCs, osc imperfection; tolerance 0 = bit-exact nominal — pinned by test) and **factory
    presets** in slots 1–8 (squelch/sub/screamer/rubber/knock/bloom/overdriven/glass).
  - Tests: three scenarios recalibrated for the measured law (the wow's brightness probe
    moved above the swept range where it is monotone in cutoff), three new (bends,
    seed/tolerance, presets); kernel suite **108/108 green**. Wrapper: six new attributes,
    maxref updated, and **`help/tap.303~-pattern.maxpat`** — a Max-patched 16-step
    sequencer driving the `note` contract, the phase-3 `tap.303.seq~` interface dry run
    (headless-authored; wants an open-in-Max check). **Runtime validation in Max still
    pending**, as everywhere.

- ✅ **`tap.303~` phase 2 + the verification notebook (2026-07-18).** The 303 program's
  fidelity phase, closed on evidence:
  - **The transistor VCA** shipped as **`vca clean|warm`** (the `svf.h` two-circuit pattern):
    the one-transistor class-A stage as a slope-normalized biased saturator (d = 2.0, b = 0.3,
    probe-calibrated) in the hardware order — post-envelope-gain, pre-output-coupling, the
    24 Hz coupling HPF absorbing the shaper's DC. The distortion tracks the envelope
    (measured 5.4% difference signal quiet vs 11.5% on hot accents); `clean` stays the
    default, bit-identical to phase 1. Kernel scenario + `tb303_vca_ab.wav`; wrapper `vca`
    attribute + maxref.
  - **`notebooks/tb303.ipynb`** (kernel repo) — the tap.303/tap.diode verification notebook,
    house pattern: the shipping kernels driven through the C ABI (extended with
    `taptools_diode` + `taptools_tb303`, including the full note interface and a direct C13
    `accent_charge` readout). Seven executed, asserted sections: Stinchcombe TF to
    **0.028 dB**, the stock-never-self-oscillates trait, the solver fast/exact A/B matrix
    (≤ **−44.9 dBr** worst case) + CPU, the measured envmod law, the wow (×1.94 build,
    ×0.998 reset), the warm VCA.
  - **WDF: documented no-go, author-approved (2026-07-18)** on the notebook's §3 evidence —
    `solver exact` already converges the circuit's nonlinear equations; a WDF would re-solve
    the same network with only the diode-curve shape as the delta. Recorded in the plan §5.
  Phase 2 of `plans/tap.303.md` is **complete**; what remains for the 303 program is runtime
  validation in Max and the deferred phase-3 sequencer.

- ✅ **The sequencers — `tap.808.seq~` + `tap.303.seq~` (2026-07-18).** The coordinated
  phase 3 both family plans deferred, designed and author-approved same day as
  **`plans/tap.seq.md`** and shipped end to end: **one shared kernel engine**
  (`step_seq.h`, `tap::tools::seq`) — phase-clocked from a `phasor~` ramp
  (floor(phase × length), swing warping the odd steps, sample-accurate entries,
  stateless against phase), 16 slots with cycle/step/now-quantized recall — with two
  thin emitters speaking the shipped voice contracts verbatim. **`tap.808.seq~`**: one
  drum row, impulses whose amplitude 0..1 is the accent (plain 0.01 = the 4 V base bus,
  accented 0.5 = the accent knob at noon; provenance `tr808_kick.h`'s 4–14 V mapping),
  `hits`/`accents`/`velocities` surfaces, `pulse` widening for envelope consumers.
  **`tap.303.seq~`**: the acid line emitting the `tap.303~` pitch + gate pair (1.0/2.0
  accent amplitudes; gate duty 0.5 per Open303's `AcidPattern`), **slide as gate-hold**
  across the boundary — chained slides and the across-the-wrap slide pinned by test —
  with per-step pitch/gate/accent/slide, live `transpose`, and the slide flag on the
  *target* step per the package `note` convention (hardware stores it on the source;
  divergence documented in the kernel header). Both rows: `length` 1..64 (polymeter off
  one phasor — the triplet pre-scale generalized), swing (beyond-hardware, default 0),
  `store`/`recall`, `todict`/`fromdict` dictionary patterns, a 1-based step-index UI
  outlet (coalescing audio→scheduler handoff, the `tap.sift~` idiom). Kernel: 19 new
  scenarios (suite 129/129), including the tb303-voice pairing test asserting slid
  steps glide without retrigger. Wrappers: min-api unittests (compiled and run against
  the mock kernel), maxrefs, help patchers, maxtest starters — and
  **`help/tap.808.maxhelp` rebuilt on real rows** (one phasor, nine rows, the old
  metro/counter/sel pattern retired), with `help/tap.303.seq~.maxhelp` superseding the
  `tap.303~-pattern.maxpat` dry run (which stays as the event-domain alternative).
  **Runtime validation in Max still pending**, as everywhere.

- ✅ **The sequencer verification notebook (2026-07-18).** **`notebooks/step_seq.ipynb`**
  (kernel repo), house pattern: the shipping `step_seq.h` driven through the C ABI
  (extended with `taptools_seqtrig`/`taptools_seqnote` + a minimal `taptools_kick`).
  Six executed, asserted sections: the analytic grid (≤ 1 sample; polymeter), the swing
  warp (odd-step delay = swing/2), the trigger-bus levels (0.01/0.5/1.0) + `pulse_ms`,
  the 303 line signals (duty 0.5, accent 2.0, gate-hold slide — including the
  full-step hold on a slide *source* step, a case the Catch2 suite's isolated-step
  scenario didn't exhibit), **four bars of acid + kick rendered from the real
  `tb303_voice.h`/`tr808_kick.h` off one phasor ramp** (embedded audio), and
  cycle-quantized recall swapping exactly on the wrap sample.

- ✅ **"Tools on Tap" Part V — The rhythm section (2026-07-18).** Two field-guide
  chapters in the kernel repo's book: *The acid machine* (`tap.diode~`/`tap.303~`/
  `tap.303.seq~`) and *The drum machine* (the eight `tap.808.*` channels +
  `tap.808.seq~`), every number sourced from the executed notebooks or the kernel
  suite per the book's own rules; the machine deep-dives renumber to Part VI —
  which same-day gained its own four rhythm-section chapters: *Seventeen, not four*
  (`diode_ladder.h` — the k = 17 threshold derived, the closed-form coupled ZDF solve,
  the WDF no-go), *The couplings are the instrument* (`tb303_voice.h` — the C13 wow as
  three lines, the measured envmod law, the documented Open303 divergence), *One
  network, eight voices* (the `tr808_*` headers — the bridged-T solved on its states,
  the calibration lesson), and *Time as a function of phase* (`step_seq.h` — the O(1)
  derivation, gate-hold look-ahead, armed-recall re-derivation).
- ✅ **Net-new object — `tap.vca~` (2026-07-18).** The 303's transistor VCA stage, lifted out
  where it can amplify anything. Kernel: **`vca.h`** (`tap::tools::vca`) — the two-circuit `svf.h`
  idiom applied to a gain stage: `clean` is the pure linear multiply (bit-identical to `*~`),
  `warm` is the one-transistor class-A saturator `S(v) = (tanh(d·v+b) − tanh(b)) / (d·sech²(b))`
  (stock `d=2.0`, `b=0.3` — the probe-calibrated 303 constants, now exposed as `drive`/`bias`),
  applied to the post-gain signal so the even-harmonic warmth and compression **track the control
  voltage** the way a discrete VCA does and a multiply cannot, plus an optional output-coupling DC
  block (`dcblock`) for the shaper's signal-dependent offset. **`tb303_voice.h` was refactored to
  compose `vca::shape()`** — one implementation shared between `tap.303~` and the standalone; the
  voice keeps its own Open303 output high-pass and gain, so the extraction left it **bit-identical**
  (pinned by `tests/vca_test.cpp`, which matches `shape()` to the old inline formula and re-runs the
  303 warm-vs-clean scenarios). Wrapper `tap.vca_tilde` (audio in + signal/float gain CV in),
  `circuit clean|warm` + `drive`/`bias`/`dcblock`/`bypass`/`mute`, the full vertical slice (maxref +
  help patcher). Remaining: runtime validation in Max.
- ✅ **The 808 swing-VCA harmonics — `vca.h` `swing` mode + the noise voices (2026-07-18).** The
  first piece of the flagged 808 circuit-sim work, and the lowest-risk one: the swing-type VCA's
  "many high harmonics" (Service Notes, RS/CL VCA), which `swing_vca()` had modeled as a flat
  `x·env`. Kernel: **`vca.h` gains `mode_swing`** — a symmetric (odd-harmonic) `tanh(d·v)/d`
  saturator, unity-slope-at-0 so quiet tails stay clean and hot transients pick up grit and
  compression, no DC (symmetric → no coupling block needed). One implementation: a static
  `vca::swing_shape(v, drive)` that both `mode_swing` and **`swing_vca(x, env, drive)`** route
  through. **`drive` defaults to 0 → the exact linear passthru**, so every calibrated 808 voice
  stays **bit-identical** until it opts in (the full existing `tr808_*` suite passes unchanged; the
  snare test adds an explicit off-is-bit-identical / on-saturates-and-compresses scenario). Wired
  into the three noise-path voices — **snare** (snappy), **clap** (CP + maracas output), **tom**
  (noise "reverberation") — each exposing a `drive` circuit-bend attribute (0..12). `tap.vca~` gains
  the matching `circuit swing`. The heavier WDF `@circuit` pass (the kick bridged-T etc.) stays
  where the field guide left it — gated on an A/B showing an audible delta the informed model misses,
  which the DAFx-14 finding suggests may never open. Remaining: A/B calibration of the swing `drive`
  against reference, and runtime validation in Max.
- ✅ **Net-new object — `tap.overdrive~` (2026-07-22).** The spiritual successor to the Jamoma-era
  `tap.overdrive~`, deliberately *not* a port (per the overdrive handoff brief): TTOverdrive's two
  memoryless odd-function curves — the sine shaper's hard ±1 plateau and its aliasing were the
  "digital" tell — are dropped, replaced by a voiced feedback clipper chasing the *class* of
  TS-lineage pedals (Mad Professor Little Green Wonder as the listening reference). Kernel:
  **`overdrive.h`** (`tap::tools::od`) — the nonlinearity sits inside a lowpass **feedback loop**
  solved zero-delay (the `svf.h` driven-circuit / `ladder.h` solver_fast one-pass scheme), with the
  LF loop gain pinned so bass stays tight and nearly clean at any drive while mids take the full
  gain — the frequency-dependent clipping a static curve cannot do, and most of the perceptual
  distance to the reference. Even harmonics via an `asymmetry` bias (0..1), a unity clean-through so
  the transfer never flattens, an always-on DC blocker (`R = 0.9997`, the TTDCBlock constant — this
  time actually in the signal path, unlike TTOverdrive's vestigial one), `body` (−1..+1) sliding the
  pre-clipper HP corner + CW upper-mid push / CCW treble lift (all linear voicing EQ, placeholders
  pending the by-ear pass against LGW demos), 1/2/4/8× oversampling (default 4×), and normalized
  0..1 parameters throughout (mappable, and Q15/Q31-ready for a future fixed-point M33 target).
  Kernel Catch2 tests pin the design goals: gain-tilt-grows-with-drive, even-harmonic emergence,
  DC blocking, never-flat slope, alias-floor improvement with oversampling, body tilt, decay to
  silence, determinism. Wrapper `tap.overdrive_tilde` (thin `vector_operator`), attributes
  `drive`/`body`/`asymmetry`/`preamp`/`output`/`oversample`/`smooth`/`bypass`/`mute`, min-api
  attribute tests, the full vertical slice (maxref + help patcher + a bypass-passthrough
  `maxtest` starter). Same-day follow-up: the rest of the kernel-repo house treatment — a
  `taptools_od_*` C ABI + `Overdrive` ctypes class, the **executed verification notebook**
  (`overdrive.ipynb`: tilt +5/+16.3/+17.2 dB at drive 0/0.5/0.9; H2 −151 → −26 dB across
  asymmetry; folded H7 −22 → −36 dB at 4×; DC means ~1e-10), and two field-guide chapters —
  *Distortion with a memory* (new book Part VII, The pedalboard — after `tap.tune~`'s Part VI,
  Staying in tune, landed the same week) + *The clipper in the loop: overdrive.h* (machine part,
  now VIII). Remaining: the in-Max voicing pass (the
  `k_voice_*` constants are the sound of the object) and runtime validation.

**Docs/help vertical slice completed (2026-07-27):** the package now ships a reference page
*and* a help patcher for **all 78 objects**. `tap.change` had neither — its maxref is written
from the current source rather than ported, because the legacy page predates the behavior the
revived object actually has (the bang-on-repeat middle outlet, the silent-set right inlet, and
atom-*type* participating in the comparison, so int 5 then float 5. is two messages).
`tap.delay~` and `tap.rotate` had reference pages but no help patcher; both were authored fresh.
Note legacy is **not** a source for these two: it has no `tap.rotate` help at all, and its
`tap.delay.maxhelp` documents the *control-rate* `tap.delay`, not the signal object — porting it
would have been wrong. All three patchers follow the `tap.fourpole~` skeleton (house `tap` style,
gradient background, Lato Light, dependency cache) and **want an open-in-Max check**.

*Finding, recorded rather than fixed:* `tap.convolve~` includes the min-api unittest harness but
ships no test file, which looks like a dangling target and is not one — `min-object-unittest.cmake`
self-guards on the test file existing. A wrapper test was attempted and abandoned: the object
loads its IR through `buffer_reference`, the mock kernel implements none of the `buffer~` API, and
`test_wrapper<convolve>` calls `buffer_ref_new` from the constructor, so the executable does not
link. That is why all four `buffer_reference` objects (`tap.convolve~`,
`tap.buffer.peak~`/`record~`/`snap~`) have no wrapper tests, and it is now a comment in
`tap.convolve_tilde/CMakeLists.txt` so the next reader does not retrace it. Those objects need the
real kernel — i.e. `runtime-tests/`. Wrapper-test coverage stood at 49/78 at this point (raised to
66/78 the next day — see the entry below).

**Wrapper-test sweep completed (2026-07-28):** every object that *can* be tested against the mock
kernel now is — **66/78**, up from 49. Seventeen new suites (92 scenarios, ~835k assertions):
`tap.autothru~`, `tap.comb~`, `tap.delay~`, `tap.elixir~`, `tap.fft.binmodulator~`,
`tap.fft.list~`, `tap.fft.normalize~`, `tap.fourpole~`, `tap.gang`, `tap.limi~`, `tap.midimapper`,
`tap.multitap~`, `tap.noise~`, `tap.procrastinate~`, `tap.pulsesub~`, `tap.random~`, `tap.rotate`.
The remaining twelve are **blocked, not skipped**, and for one of three reasons the mock kernel
cannot fake: the four `buffer_reference` objects (above), the five Jitter objects
(`tap.jit.ali`/`colortrack`/`kernel`/`proximity`/`sum`), the two file-IO objects
(`tap.filecontainer`, `tap.folder`), and **`tap.inquisitor`**, which needs
`object_attr_getnames` and `jbox_get_object` — neither exists anywhere in min-api's mock, so its
test executable would not link. All twelve want `runtime-tests/`.

Objects whose randomness or sample rate would otherwise make them untestable were made
deterministic rather than tested loosely: `tap.procrastinate~`'s four random voices are pinned by
collapsing every range to a single value (low == high), which makes the cascade geometry
measurable — one impulse in yields four taps at 2204-sample intervals, one per voice, proving the
voices are both cascaded *and* summed; `tap.random~`'s bounds are collapsed so the drawn value is
exact; `tap.noise~`'s white LCG is reproduced sample-for-sample in the test, pinning the ported
TTNoise constants, and the four colors are ordered brown < pink < white < blue by a
level-normalized mean-absolute-first-difference measure.

*Real defect found and fixed — `tap.multitap~` was silent from instantiation.* Members are
initialized in declaration order, so private state declared *after* an attribute is
default-initialized **again**, after that attribute's setter has already written to it. The `gain`
attribute's 0 dB default converts to a linear 1.0, and a trailing `m_gain_lin{}` threw it away —
and unlike the other cached state here, nothing recomputes the gains, so a fresh object summed
every tap at zero gain until the user touched `@gain`. Fixed by moving the cached state above the
attributes, which is the pattern `tap.noise~` already documents in a comment; the comment there is
now duplicated at the fix site so the next reader meets it before repeating the mistake.
`tap.comb~` (`m_lp_coef` left at its 0.1 placeholder), `tap.fourpole~` (`m_f`/`m_fb` left at 0, so
it passes nothing), `tap.limi~` (`m_recover`) and `tap.pulsesub~` (the envelope steps) have the
same hazard but are **latent**: their remaining member initializers were written to mirror the
attribute defaults, and `dspsetup` — which Max always calls before the perform routine — recomputes
what was lost. Their test files send `dspsetup` first and say why in the file header, so the
dependency is recorded rather than accidental.

*Two min-api landmines, recorded at the four sites that hit them:* (1)
`attribute<std::vector<T>>::operator=(const T)` wraps the whole vector in a single atom via the
catch-all `atom(T)` template, which **recurses until the stack runs out** — assigning a list-valued
attribute must go through the `atoms` overload (`tap.multitap~`, `tap.midimapper`). (2) Anything
that reaches an outlet through a `queue` is invisible in the mock, whose `qelem_set` is a no-op and
whose `queue` service function is private. That makes `tap.gang` — whose entire fan-out is
deferred — a construction-and-dispatch test only, stated plainly in its file header; `tap.fft.list~`
escapes it because `bang` runs the same `emit()` synchronously, so its list output *is* asserted.

*Two honest limits pinned rather than papered over.* `tap.fourpole~`'s resonance does **not** peak
at the frequency it is nominally tuned to: measured at the 1 kHz default, `@q 0.9` boosts 500 Hz by
~1.6× and *thins* 100 Hz to ~0.27×, while the response at 1 kHz itself moves by under 1%. That is
the Stilson/Smith one-pole-cascade model's known detuning, and the test says so — retuning the model
should change those numbers with it. `tap.noise~`'s white source is documented as ranging over
(-1, 1), but its LCG (modulus 139968) does reach 0, at which point `1 - 2*accum/139968` is exactly
1.0 — so the true range is (-1, 1]; the test asserts the endpoint is hit. Third, smaller:
`tap.rotate`'s `cart_to_pol` divide-by-zero guard substitutes 0.000001 for a zero real part, which
gives axis-aligned points an absolute error floor around 1e-6; the axis tests carry a 1e-5 tolerance
and a comment explaining that the floor belongs to the algorithm, not the assertions.

---

## 8. The `taptools-min` reconciliation (2026-06-17)

A second, previously-unknown Min port surfaced: **`taptools-min`** — an official
Cycling '74 effort (`github.com/Cycling74/taptools.git`, 2016–2019), since
**deleted upstream**. The only surviving copy is now preserved as the
**`taptools-min` branch** of this repo (pushed for archival; an independent history
with no shared base with `main`). It contained 7 Min-ported objects plus prototypes.
This revival had already independently re-ported **6 of the 7**, so a per-object
diff decided which implementation wins:

| Object | Outcome |
|--------|---------|
| `tap.fft.list~` | **Keep revival** — taptools-min had an out-of-bounds write and dropped the dumpout outlet. |
| `tap.fft.normalize~` | **Keep revival** — both math-correct; revival adds a divide-by-zero guard. |
| `tap.elixir~` | **Keep revival** — taptools-min was broken (wrong gain model, OOB for >2 channels) *and* depended on `min-lib`. |
| `tap.buffer.snap~` | **Keep revival** — taptools-min had a `wrap`-vs-clamp bug and dropped multichannel / `mode` / `channel` / dumpout. |
| `tap.sift~` | **Keep revival + graft** — added the `high_priority` attribute and a scheduler-thread `timer` delivery path (faithful to the original's clock-based delivery), keeping the main-thread `queue` as the low-priority option. |
| `tap.buffer.record~` | **Keep revival; optimization deferred** — taptools-min's power-of-two ring-buffer fade is faster for large fades, but bit-equivalence to the current (correct, faithful) shift-based fade was not cleanly established and the win is marginal at typical fade sizes. Not worth risking a working recorder; revisit with a proper equivalence/perf test if large-fade performance becomes a need. |

**New from `taptools-min`:**
- ✅ **`tap.sustain~`** — the one genuinely unique object (capture recent audio →
  trim to zero-crossings → crossfaded forward/backward sustaining loop). Ported onto
  the current toolchain (C++20 `std::numbers` instead of MSVC-fragile `M_PI_2`;
  initialized/guarded voice state so it is safe to instantiate before the first
  capture). Ported onto the current toolchain; **multi-voice + `rise` now implemented**
  (batch 3): a 5-voice round-robin/oldest-first bank (`voices` attribute, 1–5) summed at
  the output, each voice with its own capture buffer, loop, fade, and a one-shot
  equal-power `rise` fade-in. (The `taptools-min` archive turned out NOT to contain the
  sustain source or the `sustain1~…4~`/`.js` redesign patchers §8 referenced, so the
  polyphony model is a clean documented reconstruction.) Reference page corrected (its
  digest was copy-pasted boilerplate); still **no help patcher**, and the DSP **feel still
  wants a Max audition** (rise/fade timing, zero-crossing trim quality, headroom as voices
  stack). Provenance noted in-file; relicensed to the package's New BSD with attribution
  (originally Cycling '74 / MIT — flag for the author if stricter notice retention is
  wanted).
- 📋 **`tap.sustain1~`…`tap.sustain4~` redesign** (+ `tap.sustain.voice.maxpat`,
  `tap.sustain.analyze/calc/find.js`) — a later Max-abstraction reimagining of
  sustain. Left in the `taptools-min` archive branch as design input for a future
  proper `tap.sustain~`.
- 📋 **`tap.autowah~`, `tap.selfconvolve~`** — help-patcher-only prototypes (no C++
  source ever existed); ideas for future objects, in the archive branch.

**Test harness adopted:** the Catch-based `min-object-unittest.cmake` is now wired
up (first real test: `tap.sift_tilde_test.cpp`, guarding the new `high_priority`
default), and CI runs `ctest` on macOS. taptools-min's own test files were empty
stubs; this is the first actual coverage. Pattern established for the rest.

**Platform floor raised:** minimum is now **macOS 11** (tracking Max 9; also
required by `std::filesystem` in `tap.folder`) and **`max_version_min` 9.0**. macOS
11 is set globally via a `-mmacosx-version-min=11.0` compile/link flag in the root
`CMakeLists.txt`, since Min's `min-pretarget.cmake` force-pins the deployment target
to 10.11 and CMake has no per-target deployment property.

---

## 9. Remaining open items

Everything in §2 (all Tier 1–3 + Jitter + infrastructure), the delays, and the
spectral set is **done and compile-verified**. What's left:

**1. Runtime validation in Max (the headline gap).** All DSP is compile- and
unit-tested against a mock kernel but not yet auditioned in a live Max. Priorities,
roughly by risk: the reinvented spectral trio (`tap.vocoder~`/`tap.nr~`/`tap.spectra~`),
`tap.sustain~` (multi-voice feel/headroom), `tap.verb~` (the new >1× oversampling),
`tap.filter~` (sweeps/stability), the delays, and `tap.filecontainer` (DB round-trip +
best-effort moddate restore). The `runtime-tests/` (max-test) harness is the vehicle —
**first verify the two generated example patchers in Max**, then extend patcher coverage
to these objects.

✅ **Wrapper-test coverage is now complete at 66/78** (2026-07-28, see §7) — every object the mock
kernel can host has a suite. That makes the twelve it cannot host the *first* claim on
`runtime-tests/`, since they have no automated coverage at all today: the four `buffer_reference`
objects (`tap.convolve~`, `tap.buffer.peak~`/`record~`/`snap~`), the five Jitter objects, the two
file-IO objects (`tap.filecontainer`, `tap.folder`), and `tap.inquisitor` (needs the real patcher
API). `tap.gang` belongs on the list too: its suite covers construction and message dispatch, but
its outlets fan out through `queue`s that only Max services, so the ring-of-gangs
change-detection behavior is still unasserted.

**2. Help patchers.** ✅ **Every object now has both a reference page and a help patcher
(78/78)** — `tap.change` was missing both, and `tap.delay~` / `tap.rotate` had a maxref but no
patcher; all three were filled in 2026-07-27 (see §7) and **want an open-in-Max check**.
✅ `tap.sustain~` and `tap.filter~` now have help patchers (authored
headless from the maxref + templates; **want a first open-in-Max check**). The spectral
trio's help patchers are still the *legacy `pfft~` abstractions* and **need rework** for the
new self-contained objects. *Pre-existing shared-asset gap found & fixed:* the help
patchers reference shared bpatchers that were dropped in the prune — ✅ restored
`tap.badge.maxpat` (referenced by 50+ patchers) and `tap.jit.ali.kernel-assist.maxpat` from
`legacy`, and ✅ **recreated `tap.help.dac~.maxpat`** (9 refs; the repo history only had an
older `gain~`/`meter~` form, so it was rebuilt to spec: 2 L/R inlets, a horizontal stereo
`live.meter~`, a local `dac~` toggle, text, 2 pass-through outlets — wants an open-in-Max
check). `demosound.maxpat` is a stock Max abstraction (fine).

**3. Resurrection candidates still open** (§3, all "maybe/review"):
`tap.adapt~`, `tap.buffer.record2~` (merge into `tap.buffer.record~`?),
`tap.smooth`, `tap.deviate`, `tap.string.sub`, `tap.thru`/`tap.thru~`,
`tap.smooth`, `tap.deviate`, `tap.semitone2ratio`, `tap.string.sub`, `tap.thru`/`tap.thru~`,
`tap.decay_calc`; and the retired **Jitter** ones (`tap.jit.delay`, `tap.jit.motion`/`+`/`2`,
`tap.jit.grayscale`, `tap.jit.pan`, `tap.jit.getattributes`). None are committed to yet.

**4. Repatriation (§5).** `tap.colorspace` (its replacement `j.unit` is Jamoma-dormant) is
a candidate to bring back; plus the broader `j.*` survey flagged in §5.

**5. Toolchain.** ✅ **Done — the whole tree now builds under Linux/GCC.** The `enum class`
attribute quirk in `tap.crossfade~`/`tap.pan~` is fixed by storing `shape`/`mode` as
`attribute<int>` (with named index constants) instead of `attribute<enum class>` — the
help-patcher umenu-index path is preserved exactly. (Root cause: min-api's `atom::operator==`
has no enum overload; GCC rejects it where clang accepts a templated conversion.) Also fixed
the `-Wchanges-meaning` shadow by renaming the `number` message member. Both objects gained
unit tests (now testable for the first time, guarding the curve selection).

**6. Deferred optimization.** `tap.buffer.record~` power-of-two ring-buffer fade (§8).

**7. Release engineering.** A versioned GitHub release + Max Package Manager submission;
and, later, automating the runtime tests on a self-hosted macOS runner (feasible — an
unlicensed Max runs patchers; the blocker is GUI/activation, not licensing).

**8. Net-new object — `tap.autowah~` (2026-07-15).** ✅ **Shipped** — see the §7
progress-log entry. An envelope filter / auto-wah modeled on the **Mad Professor
Snow White AutoWah**; design, hardware research, and the validation method live in
**`plans/tap.autowah~.md`**. Still open from its plan: runtime validation in Max,
and the **hardware calibration pass** when the ordered pedal arrives (envelope
constants, exp-vs-linear sweep law, default filter tap — all isolated kernel-side).

**9. Kernel/wrapper repo split.** ✅ **Done (2026-07-14)** — the physical split is complete, the
AmbiTap / AmbiTap-Max pattern:
- The old `tap/taptools` repo was **renamed to `tap/taptools-max`** (this repo — the Max wrapper
  package), keeping all history, issues, and the `legacy`/`taptools-min`/`windows` branches.
- A new **`tap/taptools`** repo holds the portable DSP **library** (kernel), extracted with
  `git filter-repo` so every file keeps its full pre-move history; `include/` sits at the repo
  root (AmbiTap layout). It builds/tests standalone with its own CI. Recipe:
  `scripts/extract-kernel-repo.sh` (kept here for provenance).
- This repo pins the kernel as the **`submodules/taptools`** submodule (canonical URL
  `github.com/tap/taptools.git`); `TAPTOOLS_KERNEL_DIR` defaults there and overrides to a sibling
  dev checkout, exactly like AmbiTap-Max's `submodules/AmbiTap` + `AmbiTap_ROOT`. The `kernel` CI
  job moved to the kernel repo.

✅ **Spectral trio extracted (2026-07-14).** `tap.nr~`, `tap.spectra~`, and `tap.vocoder~` now
consume kernel headers (`tap::tools::nr::reducer`, `spectra::remapper`, `vocoder::bank`); the
radix-2 FFT that was byte-identical across `conv_engine`/`nr`/`spectra` is consolidated into
`fft.h`, and the overlap-add machinery `nr`/`spectra` shared lives in `stft.h`. (`vocoder~` turned
out to be a time-domain biquad filterbank, so it follows the `svf`/`ladder` `prepare(sr)` idiom,
not the STFT scaffold.) DSP correctness is covered by the kernel's own Catch2 suite; the wrappers
kept their behavior tests. No behavior change — same code, relocated.

**10. Net-new object family — `tap.808.*` (2026-07-17).** ✅ **Phase 1 complete — all
five slices shipped** — see **`plans/tap.808.md`**. Circuit-informed recreations of the
Roland TR-808's analog drum voices: the eight voice channels
`tap.808.{kick,snare,clap,hat,cymbal,cowbell,tom,rim}~` (per hardware voice channel,
shared kernel blocks: bridged-T resonator, metal bank, swing-VCA), the family accent-bus
contract (signal-edge amplitude = accent on the 4–14 V trigger bus), the family overview
patcher, and the kernel's `tr808_render` tool. Provenance: the Werner–Abel–Smith
DAFx-14/ICMC papers + the TR-808 Service Notes schematics, read directly. **The phase-3
sequencer shipped 2026-07-18** as `tap.808.seq~` (see `plans/tap.seq.md` and the §7
entry — one row per voice, phase-clocked, sharing its engine with `tap.303.seq~`).
Flagged WDF circuit-simulation upgrade path (`@circuit`, the `svf.h` two-circuit
pattern) remains. See the §7 progress-log entries (slices 1–5, §7.2 calibration).
Next: runtime validation in Max, then the Phase 2 WDF go/no-go on the kick.

**11. Net-new object family — `tap.303.*` (2026-07-17).** 📋 **Plan approved; slices 1–2
shipped** — see **`plans/tap.303.md`**. The TB-303 recreation, companion to the `tap.808.*`
program: the standalone diode-ladder filter (**`tap.diode~`** — ✅) and the voice
(**`tap.303~`** — ✅, TapTools' first pitched instrument, carrying the package-wide
melodic-voice contract: MIDI-note pitch signal + amplitude-as-accent gate + legato-as-slide);
see the §7 progress-log entries. Provenance: Stinchcombe's filter analysis, the Devil Fish
circuit documentation, Open303, x0xb0x schematics, Service Notes. **Phases 1 and 2 are ✅ complete**
(phase 1: the filter, the voice, the C13 wow, the slice-4 Open303 calibration + bends +
seed/tolerance + factory presets; phase 2: the `vca clean|warm` transistor stage, the
square shaper resolved via Open303's measured constants, and the author-approved WDF no-go
— grounded in the kernel repo's **`notebooks/tb303.ipynb`** verification notebook; see the
§7 entries). **The phase-3 sequencer shipped 2026-07-18** as `tap.303.seq~` (see
`plans/tap.seq.md` and the §7 entry — the acid line on the shared step engine, slide as
gate-hold). Remaining: **runtime validation in Max** for all three objects (help
patchers, maxtests, the render WAVs, and the notebook as the evaluation material).

**12. Net-new object — `tap.tune~` (2026-07-22).** ✅ **v1 shipped** — real-time monophonic
pitch correction (the classic retune / hard-snap effect). Cross-repo vertical slice: the YIN
detector landed in **DspTap** as the shared `tap::dsp::yin` primitive (full-rate, cumulative-mean
normalized, parabolic sub-sample lag; double golden model + float32 profile, the `fft.h`
backend pattern — promoted from the decimated follower inside `grm_pitchaccum.h`); the corrector
kernel landed in **`tap/taptools`** as `taptools/tune.h` (`tune::corrector`: scale/key mapper
with per-note enables + MIDI-target mode, retune-speed glide, and the two-tap `tap.shift~`
transposer engine with its window locked to an **even multiple of the detected period** — the
lock measurably makes the average retune ratio exact); this repo adds the Min wrapper, wrapper
tests, reference page, and help patcher. IP framing (per the pitch-correction handoff doc): the
implemented pipeline is the public-domain expired-patent design; polyphonic per-note editing
(Celemony DNA) deliberately out of scope; "tune", not the trademarked product name.
**Backends shipped (2026-07-22, same day):** resynthesis is now selectable via `@backend` —
`grain` (the validated low-latency two-tap default), `psola` (true TD-PSOLA, new DspTap
primitive `tap::dsp::psola`; formant-preserving on voice, and honestly documented as a
spectral-envelope resampler — pure tones far from a new harmonic thin out, pinned by its
tests), and `pvoc` (peak-locked phase vocoder, new DspTap primitive `tap::dsp::pvoc` on the
shared real FFT; Laroche–Dolson-style rigid peak-region shifting, exact waveform identity at
ratio 1). Detector, mapper, and retune glide are shared; only the last stage swaps, and
switching live is click-safe (the incoming engine starts from silence).
**Formant preservation shipped (2026-07-22, same day):** `@formant` enables the LPC
source-filter method on the pvoc backend (DspTap: autocorrelation LPC + Levinson–Durbin,
order 48, `envelope(target)/envelope(source)` per relocated bin — from the published
literature only, per the IP policy); psola preserves formants inherently. **Verification
notebooks shipped** in both upstream repos, driving the shipping C++ through each repo's
C ABI: DspTap `notebooks/pitchshift.ipynb` (YIN accuracy; Finding 1 — PSOLA is a
spectral-envelope resampler; Finding 2 — naive phase-vocoder remapping measurably loses
half its level vs the peak-locked design; the formant demo) and TapTools
`notebooks/tune.ipynb` (retune-speed family, the three backends compared on the same
vibrato voice, MIDI-mode formant demo; `Tune`/`Yin` added to the kernel C ABI).
**Deferred list closed (2026-07-22, same day):** auto-key detection shipped (kernel:
Krumhansl–Kessler profile scoring over a slowly-forgetting pitch-class histogram —
learning-only, `@autokey` + `getkey`/`applykey` in the wrapper); the **detected-pitch
outlet** shipped (right outlet, timer-driven `pitch <midi> <hz>` reports every `@interval`
ms with an atomic audio→scheduler handoff, plus the `key ...` replies); and the **runtime
maxtest** shipped (`runtime-tests/patchers/tap.tune~.maxtest.maxpat`, generated via
`make_maxtest.py` — unpitched DC passes the grain engine at exactly unity). Remaining for
this object: the in-Max validation pass itself (help patcher + maxtest under a licensed
Max), like the rest of the DSP set.

**13. Net-new object — `tap.harmony~` (2026-08-05).** ✅ **v1 shipped** — formant-preserving
multi-voice harmonizer (the keyboard-harmonizer effect: up to four pvoc voices at fractional-
semitone intervals over a kernel-aligned dry path, formants preserved by default, `chord`
message, `glide` portamento). Kernel: `taptools/harmonizer.h` (`tap::tools::harmony`),
composing DspTap's `pvoc` (Laroche–Dolson peak locking + LPC formant correction) — seven
Catch2 scenarios under the yin oracle, capi + ctypes bridge. Origin story: the *Tools on Tap*
Recipes drafting flagged the gap ("Hide and Seek" is a harmonizer, not a vocoder — see
`plans/recipes-improvements.md` §10). Build verification landed the same day: CI built the
wrapper green on both platforms (macOS universal, lipo-checked, and Windows x64) alongside
the full external set, with clang-format/clang-tidy clean. Remaining for this object: the
in-Max validation pass and a runtime maxtest (licensed-Max, on-Mac), and the v1.1
house-machinery items in the improvements plan (mute/bypass, 16-slot preset morph).

**14. `tap.adsr~` rebuilt as virtual-analog (2026-08-05).** ✅ **Shipped** — the envelope's DSP
moved to the kernel repo (`taptools/adsr.h`, `tap::tools::adsr::generator`) and gained a
circuit-model `analog` default: truncated RC attack toward a 1.4× overshoot target (CEM 3310
architecture), asymptotic decay/release (95 % closure at the knob time, pinned), retrigger from
the current level. The Jamoma TTAdsr curves survive verbatim as `hybrid`/`linear`/`exponential`
modes. The family trigger contract lands: `threshold` (default 0.005 — a seq row's plain 0.01
now registers; the hard-coded 0.5 gate is retired) and `velocity` sensitivity (gate amplitude
scales the hit; 0 = legacy). **The default mode change is deliberate and documented** — the
recipes-improvements plan §1 carries the reasoning. Kernel: eight Catch2 scenarios + executed
`adsr.ipynb`; wrapper shrunk to Min glue, maxref rewritten (retiring its phantom `int`/`float`
methods), wrapper tests updated. Remaining: in-Max validation.

**15. The improvements wave (2026-08-05).** ✅ **Shipped in parallel** (four worktree agents,
serial integration): kernel side — the vco performance section (vibrato in cents with
per-note-re-armed onset delay, bend; formula-free at last), the vocoder sibilance path and
carrier-dry mix (silent-carrier contract intact at defaults), the harmonizer 16-slot preset
morph, and the delay-pair kernel (`taptools/delay.h`, Hermite taps, DC-blocked feedback,
`interp 0` bit-compat pinned against the Jamoma source). Max side — ladder/diode symbolic
`mode`/`solver` (the min-api enum finding is recorded in the plan §3), noise/verb `seed`
determinism, the sustain `length` setter, crossfade `mode` deprecated in docs, six maxrefs
rewritten, the vco/vocoder/harmony wrapper surfaces grown, and tap.delay~/tap.multitap~
rebuilt (multitap now stereo with per-tap pan; the 0.0-time-signal trap retired as a
documented breaking change). Still open in the plan: 5comb `notes`, the verb `clear`
DC-blocker one-liner, help-patcher refreshes, and the on-Mac validation passes.

**16. The Eno family (2026-08-12).** ✅ Three new objects on the freshly-landed kernel family
(`tap/taptools` PR #30: `tape_loop.h` + `discreet.h` / `airport.h` / `garden.h` — the
degradation-is-the-stabilizer inversion, fully tested and notebook-verified kernel-side):
`tap.discreet~` (the Discreet Music two-machine loop; regen legally 1.0, doppler loop moves,
deterministic wow/flutter, the input send fader), `tap.airport~` (eight free-running
incommensurate loops; record/freeze bit-exact, per-loop lengths/levels/pans/darken lists,
`period` reporting the lcm; a guarded `dspsetup` so a DSP restart never erases running tape),
and `tap.garden~` (a `<0,1>` source: `note` plants, scale as the symbolic-enum house pattern,
seeded gardener). Full vertical slice each: maxref + help patcher + maxtest starter + mock-
kernel unit tests (green), clang-tidy/format clean; submodule pin at the kernel family's tree
on `main` (9d63666; repointed from the topic branch after the kernel PR merged by rebase —
identical tree, so the wrappers are byte-for-byte unaffected). Still
open: the on-Mac validation pass (open the three help/maxtest patchers once in Max), and the
in-Max audition.

**17. The Eno family, opened up (2026-08-15).** ✅ `tap.airport~` and `tap.garden~` were monoliths
by accident rather than design — the kernel already held the parts, but nothing outside the
monolith could reach one. Both were split kernel-side into components that are now objects, so the
block diagram is patchable: `tap.reel~` (one free-running tape loop — sum several at awkward
lengths and you have the airport, with every loop reachable for an insert, a varispeed, or a
different source), and `tap.chime~` / `tap.bloom` / `tap.scale` / `tap.gardener` (the rack, the
event ring, the entry quantizer, and the idle wind — chained, they are the garden). The monoliths
stay: they are the put-it-on-and-walk-away objects, and the split is additive.

The extractions are behavior-preserving and shown to be: multi-second renders of both kernels
through splices, punch-ins, mode changes and the seeded gardener hash bit-for-bit identical before
and after, and the kernel suite gained null-test scenarios requiring the hand-wired components to
match the monolith bitwise (a 1e-12 nudge on one lane fails the airport one). This repo's own
notebooks reach the components through new C ABI entry points.

Three decisions worth keeping. (1) `tap.chime~` is the whole sixteen-voice rack rather than a mono
voice you wrap in `poly~`: `poly~` steals round-robin and is Max-only, so delegating voice stealing
would have cost both the glide-not-click promise and every non-Max target the kernel is meant to
reach. (2) `tap.bloom` and `tap.gardener` run on Max's scheduler, not the audio clock, so returns
land within an `@interval` tick rather than on the exact sample — inaudible at loop lengths measured
in seconds, but a real difference from the monolith, and the reason the null tests live in the
kernel rather than in a patch. (3) Two name collisions the compiler caught and the next object
should avoid: a message named `samples` shadows Min's `samples<N>` return type, and a class named
`scale` collides with Min's own `scale()` utility badly enough to make the class template invalid.

Full vertical slice each: maxref + help patcher + mock-kernel unit tests, clang-tidy/format clean.

Then two follow-ups closed the gaps that split left. **`tap.chime.voices~`** is the same rack with
every bell on its own outlet — sixteen mono signals, each carrying its tube dry, before the seat in
the stereo image — so a patch can filter, gate, or place one voice without touching the other
fifteen. It is a separate object rather than a mode because outlet count is fixed when a Min object
is constructed; and it is sixteen discrete outlets rather than one mc outlet because min-api's mc
support is inlet-side only (it sets `Z_MC_INLETS` and provides no `multichanneloutputs`, which Max
requires before an external may declare a variable-channel mc outlet). If a later Min gains that,
this object is where to adopt it. Kernel side, `bell::process_mono` was factored out and `process`
rewritten in terms of it, so there is still one oscillator path — bit-identical, and pinned by a new
scenario requiring the per-voice taps put back through their seats to equal the stereo pair exactly.

**`tap.period`** closes the other one: the composite period needs every length at once, so it had
nowhere to live in a patch of independent reels. `composite_period_seconds` came out of `loop_bank`
as a free function and the bank now calls it, and the seconds-to-samples quantization is shared
rather than copied (`loop_samples_for`), so `tap.period` and the reels it is asked about cannot
drift apart. That detail carries weight: the lcm is over sample counts, and lengths that look
commensurate as decimals are not as samples.

76 ctest cases green. Still open: the on-Mac validation pass (open the seven help patchers and the
null-test patcher once in Max), and maxtest starters for the objects that do not have them yet.

**18. The tape echo, and the start of the Radiohead family (2026-08-15).** ✅ `tap.tapecho~` — a
multi-head tape echo in the Copicat / Space Echo tradition, and the first object of a new family
planned in the kernel repo's `book/PLAN-radiohead-family.md` (drawn from Radiohead's *performed*
electronics: the tape echoes, the live Max stutter rigs, the Ondes Martenot, the Kaoss-pad
scrubbing). The family's spine, where the Eno family's was "degradation is the stability
mechanism": **the control is the instrument** — these are objects you ride, so the no-zipper rule
is the feature rather than the hygiene.

The object is one record head, a span of moving tape, and up to four playback heads along it,
summed to stereo. `@span` is the motor — the delay of a head at the far end of the path — and each
head sits at `@span` times its `@ratios` entry, so the motor moves the whole layout together and
bends pitch on the way, the `tap.discreet~` doppler contract. The default layout is four evenly
spaced heads; that spacing is nominal, deliberately *not* claimed as measured from any unit, and
`@ratios` builds a Copicat-style three in one message.

It carries the tape family's inversion one step further, which is the reason the object exists.
`tap.delay~` caps feedback at 0.99; `tap.discreet~` reaches exactly 1.0 because the wear path is
the stabilizer; here `@regen` goes **past** 1.0 into deliberate sound-on-sound self-oscillation,
bounded by the `@drive` saturator (≤ 1/drive) rather than by a gain cap. Because that bound only
exists while the saturator is engaged, the effective regeneration is capped back to 1.0 *per
sample* whenever `@drive` is 0 — the attribute keeps its value and it returns when drive does, so
dropping drive mid-howl lands the loop rather than letting it run away.

The claim that mattered most was structural, and it is measured rather than asserted: with the tape
path neutral (no transport error, no regeneration) a one-head echo is **bitwise** `delay.h`'s
Hermite multitap — pinned in the kernel suite and again across the C ABI in the notebook. That is
what makes "`tape_loop.h` is a library and this kernel is only composition" a measurement; the
shared machinery needed no changes at all to serve a second topology. Also measured at ship:
per-pass generation loss within 0.2% of the analytic wear transfer, wow at 10.91 cents against
10.88 predicted, and every past-unity drive setting plateauing under its analytic ceiling.

Kernel side (`tap/taptools`): `tapecho.h`, eleven Catch2 scenarios, the C ABI plus ctypes surface
(`TapEcho`), the executed `notebooks/tapecho.ipynb`, and `radiohead_render` with four *performed*
scenarios — the controls move while rendering, including a self-oscillation take that pushes past
unity, drops the input, rides the tone control and brings it home. Here: the wrapper, six min-api
scenarios, the reference page and the help patcher, and the submodule pin bumped to match. Still
open for this object: the on-Mac validation pass (open the help patcher once in Max), a maxtest
starter, and the book chapter.

**19. The stutter rig (2026-08-15).** ✅ `tap.stammer~` — the second Radiohead-family object, and
an **original design** rather than a port: the input is captured continuously and, on a `@step`
grid, the machine rolls dice and re-fires a slice of what just went past. `@density` is how often
it grabs, `@divisions` how finely it chops, `@repeats` how many passes it holds on for, `@reverse`
the per-repeat chance of running backwards, `@jump` how far further back it may reach. It sits in
the brassage/granular tradition (Roads, *Microsound*); the band's own Max rigs are known from
published interviews and broadcast films, and that record informs what the object is *for* and
nothing about what the code does. This is the object with the most direct lineage to this package —
a Max stutter patch, arriving as a Max object.

Two things are worth carrying forward from building it. The first is the **pinned-dice identity**
the kernel suite is built on: set density 1, whole-step slices, one forward pass and no flank, and
only one outcome is possible, so the machine must reduce to *exactly* a one-step delay, bitwise.
That single identity pins the grid countdown, the slice origin arithmetic and the playback head
together — much stronger than chasing three off-by-ones separately, and it is the technique to
reach for the next time an object has several interacting integer clocks. The second is that the
**material contract got measured at its premise** rather than asserted: slices of a sustained sine
are 1.000 alike by magnitude spectrum and slices of a plucked phrase 0.286, so "feed it transients"
stops being taste and becomes the observation that re-ordering interchangeable things cannot do
much.

The seeded-randomness convention carries over from the garden intact and is now a family habit:
every draw comes from the shared xorshift64* in a fixed order, so the same seed and the same moves
give the same render bit for bit (a different seed changes 89% of samples), and at `@density` 0 the
dice are never rolled at all — the seed provably cannot matter and the object is a bitwise bypass
at any mix. That last one is the contract a patcher can check without a scope, and it is pinned
here as well as in the kernel.

Kernel side (`tap/taptools`): `stammer.h` with the planned `capture` + `slicer` split under a thin
`machine`, nine Catch2 scenarios, the C ABI plus ctypes surface (`Stammer`), the executed
`notebooks/stammer.ipynb`, and three more `radiohead_render` scenarios including a forty-second
performed disintegration. Here: the wrapper, five min-api scenarios, the reference page, the help
patcher, and the pin bumped to match. Still open for this object: the on-Mac validation pass, a
maxtest starter, and the book chapter — the same three that `tap.tapecho~` is waiting on.

**20. The fuzz, and a house pattern that measured wrong (2026-08-15).** ✅ `tap.fuzz~` — two
cascaded clipping stages and a bass/contrast/treble voicing section: the OK Computer-era dirt,
and the harder, more scooped school beside `tap.overdrive~`'s feedback soft-clipper. It is a
recreation of a circuit *class*, not a component model of any one pedal — no value or corner is
claimed as measured from a unit, and the control names follow the class's conventional layout.

The method is Yeh, Abel & Smith's DAFx-07 **simplified cascade** (conditioning filter →
memoryless nonlinearity → equalization filter, twice), which supplies the architecture *and*
the justification for the central simplification: the diode limiter is really a lowpass whose
pole moves with voltage, its exact ODE is expensive, and approximating it as a static curve
between fixed filters is defended and measured there against real pedals. The same paper's note
that a real op-amp stage clips asymmetrically — producing the even harmonics an odd-only model
predicts away — is why `@asymmetry` exists.

Two findings worth carrying, both caught by measurement and neither by listening.

The first is about **gain staging in a cascade**. The tanh family's small-signal slope is
`knee/tanh(knee)`, which is greater than one and grows with the knee. The first cut put a fixed
×2.2 in front of a knee-3 curve, so the second stage saw an effective ×6.6 and was already fully
clipped with the gain knob at zero — the knob did nothing over most of its travel, and it
sounded like a distortion the whole way, which is exactly why ears did not catch it. Retuned,
the harmonic-to-fundamental ratio now sweeps 0.010 → 0.358 across the knob.

The second is a **correction to a house pattern**, and it is the one to remember. The
oversampling chain in `tap.ladder~` / `tap.svf~` / `overdrive.h` uses a 4th-order Butterworth
anti-image/anti-alias pair. In this kernel that is not steep enough: alias energy at 4× measured
*worse* than at 2×. Eighth order improves 4× about sixfold — but it does **not** make the
sequence monotone, and an earlier draft of this entry wrongly said it did. Measured against a
3733 Hz tone, fold energy runs 1.2e-1 / 2.7e-5 / 7.4e-4 / 1.8e-3 at 1× / 2× / 4× / 8×, so
**every factor is worth having over none and 2× is the best of them** — which is why
`@oversample` defaults to 2 rather than to the largest available factor. The cause is open: the
obvious suspect, biquads going ill-conditioned at the low normalized cutoffs a high factor
needs, was tested and ruled out by an impulse-response check showing clean decay to denormal at
every factor; the untested hypothesis is imaging from zero-stuff upsampling intermodulating in
the clipper, which would point at cascaded 2× resampling as the real fix. Whether `overdrive.h`
is owed the 8th-order change is a separate live question — different nonlinearity, different
gain structure, so it needs its own measurement rather than this one's conclusion.

Two test-design errors are recorded in the kernel suite because both are easy to repeat: an
alias test whose tone divided the sample rate (every fold lands on a harmonic and is invisible),
and probe frequencies close enough to the fundamental to measure window leakage instead of
aliasing.

Kernel side (`tap/taptools`): `fuzz.h`, nine Catch2 scenarios, the C ABI plus ctypes surface
(`Fuzz`), the executed `notebooks/fuzz.ipynb`, and three more `radiohead_render` scenarios. Here:
the wrapper, five min-api scenarios, the reference page, the help patcher, and the pin bumped to
match. Still open: the on-Mac validation pass and a maxtest starter — the same two every object
in this family is waiting on.

**21. The first piece of the Ondes Martenot (2026-08-15).** ✅ `tap.touche~` — the *touche
d'intensité*, the pressure key an ondes player's left hand rides, as a standalone gain object.
It is the first part of `tap.ondes~` to land and it ships alone deliberately: a measured
expressive gain law is useful on anything, so there is no reason to hold it back until the rest
of the instrument exists.

What makes this object different from everything else in the package is its contract. Every
other kernel here makes design choices and then measures them; **this one is obliged to
reproduce someone else's measurement.** Quartier, Meurisse, Colmars, Frelat & Vaiedelich (*Acta
Acustica united with Acustica* 101(2), 421–428, 2015) measured finger force, key displacement
and the resulting sound simultaneously on ondes No. 320 and published the boundaries of the six
musical nuances across the key's travel. Those seven points are the specification. The kernel
interpolates them with monotone cubic (Fritsch–Carlson) segments and reproduces them to within
6e-5 dB — monotone cubic rather than a spline because an overshoot here is a non-monotone gain,
which is audible as a dip while you press *harder*.

Interpolating rather than fitting is the load-bearing decision. The published dB steps are equal
by construction and the displacement steps are not — 1.0, 0.6, 0.5, 0.4, 0.5, 1.5 mm — so the
law steepens through the middle of the travel and flattens hard at the top, and a straight line
in dB-against-millimetres departs from it by 8.3 dB. That shape is the entire reason the key is
famous, and fitting would have thrown it away.

Two things the reproduce-the-table test caught, both worth carrying. **The normalized domain has
to be the physical travel, not the measured band.** An early cut mapped `@position` 0..1 onto
4.3–8.8 mm, which put position 0 exactly on the first published point while returning silence
there — contradicting the measurement it was supposed to honour. The paper puts playable
gestures at roughly 3–9.5 mm with the measured band inside, so position now spans 9.5 mm and the
bottom 45 % is genuinely silent. That dead travel is the key's own first phase, bending before
it reaches the powder bag, and it is why the instrument can be played with such sharp attacks:
the useful 50 dB is packed into the 4.5 mm immediately after. Expect to explain that to patchers
who think the object is broken — the help patcher says it in as many words. **And the dead zone
belongs in the lookup, not the table**: zeroing dense-table entries below the floor put a cliff
next to it, so a query landing exactly on the floor lerped toward zero and read 4.2 dB low.

Kernel side (`tap/taptools`): `touche.h`, eleven Catch2 scenarios, the C ABI plus ctypes surface
(`Touche`), the executed `notebooks/touche.ipynb`, and a `radiohead_render` scenario putting the
curve against the two laws anyone would otherwise reach for — a linear fade and a fade linear in
dB. It is audibly neither. Here: the wrapper (signal or attribute position), four min-api
scenarios, the reference page, the help patcher, and the pin bumped to match.

The rest of `tap.ondes~` is designed but unbuilt — see the kernel repo's `book/PLAN-ondes.md`,
which was written against the sources *after* they were read and which corrects the family
plan's original assumption that the voice was a `vco.h` descendant. It is not: the instrument is
heterodyne, its oscillators measure as essentially pure, and the timbre comes from the triode
stages and the diffuseur downstream.

**22. The diffuseurs, and the scrub pad (2026-08-17).** ✅ `tap.metallique~`, `tap.palme~` and
`tap.scrub~` — three objects in one pass, two of them the second and third pieces of the Ondes
Martenot and the third the last of the Radiohead family's original five.

**The diffuseurs.** The Ondes Martenot does not have a loudspeaker, it has a rack of them, and
choosing between them is part of playing it. `tap.metallique~` is the gong (1944–45, patented
1947), a plate driven by a motor transducer; `tap.palme~` is the electromagnet driving twelve
metal strings on a lyre-shaped soundboard (1949–50). Both ship as standalone effects because
that is where their value is: run a guitar through the palme.

Three things the build settled, all of them worth carrying.

**Driven, not struck** — the correction that shaped the file. `garden.h`'s modal machinery
carries over intact (mode ratios, doublet splitting, per-mode decay) but its strike envelopes do
not. There is no `decay_env` here and no trigger: the input excites the body continuously and
the body rings at its own rates, which is `grm_comb.h`'s situation rather than the chime's.

**The order is a claim, so it gets a null test.** The signal reaches the transducer first and
the transducer's motion excites the body, so the nonlinearity is *upstream* of the resonator. A
Catch2 scenario pins that the cabinet is exactly `transducer -> plate`, bitwise, while the
reversed wiring differs by 28 % of peak — the order is a real choice, not a notional one.

**Two numbers that are not what you would guess.** The mode filter is Steiglitz's
constant-peak-gain two-pole resonator, whose peak magnitude is 1 at *any* Q and whose zeros at
±1 null DC and Nyquist exactly — so with weights that sum to 1 the body is bounded by its input
and needs neither a limiter nor a DC blocker. And the transducer's output is bounded by
**2/saturation, not 1/saturation**: a hard-driven squared law is a nearly-constant positive
waveform, and removing its DC doubles the worst-case swing (measured 1.49 where the naive
argument predicts 1.25).

Honest about what these are. The instruments, their dates, their excitation and their
moving-iron transducer come from the peer-reviewed sources; the *mode data does not*, because no
ondes-specific measurement of either body exists in any of them. The ratios are Fletcher &
Rossing's free circular plate and the harmonic series, the string tuning is a design choice, and
both nonlinear coefficients are voiced by ear — the source establishes *that* the driver is
nonlinear without handing over a curve. All of that is stated in the kernel header, the
reference pages and the help patchers rather than left to be discovered. The palme gets twelve
strings per the peer-reviewed source, not the twenty-four the widely copied hobbyist build pages
claim.

**The scrub pad.** `tap.scrub~` records continuously and rakes a granular playhead through the
recording, with **position and pitch as two independent signals** — the two axes of a pad. Its
tape is `tap.stammer~`'s capture class *itself*, shared rather than copied, which is what that
object's header promised when it shipped; the only addition the stutter needed was the
fractional read its ±1-rate slices never used.

The object rests on a null: Hann overlap-adds to exactly 1 at hop = size/2, so held still at
pitch 0 with no spray, `tap.scrub~` is the input delayed and nothing else — 4.4e-16 in the
kernel suite, and re-pinned at the wrapper so a plumbing mistake cannot hide it.

**One real defect, found by measurement, and one measurement trap, worth both entries.** The
first cut anchored every grain at the position. Grain origins then advance at the *write head's*
speed while each grain plays at the pitch ratio, so the transposition applied only inside a
grain, the average read rate came back to 1, and a steady tone emerged at its **original** pitch
with a comb of grain-rate sidebands around it. The pitch knob did nothing but add texture, and
no other test on the page could see it. The fix is a phase-continuous read head, wrapped back
toward the position only after it has wandered ±1.5 grains — a bound chosen by sweep (band
energy retained 0.933 / 0.958 / 0.965 / 0.990 / 0.993 at ±0.5 / ±1 / ±2 / ±3 / ±4 grains, flat
past 3, and every extra grain of wander is a grain of position error).

Then the trap: **a single-bin probe reads the fixed kernel as badly broken.** The wraps spread
the transposed partial into a comb a few Hz wide, and a rectangular-window Goertzel on one line
sees whichever comb tooth happens to sit there — 0.02 where the band figure was 0.43. Measured
properly, 98.8 % of a clean shifter's energy lands within ±15 Hz of the transposed pitch (worst
91.7 %); what the wraps cost is *concentration*, 92.0 % as focused as a clean shift and 75.0 %
at worst, which is audibly a warble. Measure the band, not the bin.

That measurement also turned up something about a shipped object, recorded rather than acted on:
on the same sweep, **`tap.pitchaccum~` returns mean 0.908 and worst 0.004** — a near-total
cancellation at 311 Hz up nineteen semitones, where its ratio is exactly 3 and its two taps land
a half-window apart. Fixing that is its own job with its own tests and its own consumers, and it
should not ride along on an unrelated kernel; it is logged as an open question in the kernel
repo's `book/PLAN-radiohead-family.md`.

Kernel side (`tap/taptools`): `diffuseur.h` and `scrub.h`, 31 Catch2 scenarios between them, the
C ABI plus ctypes surfaces (`Metallique`, `Palme`, `Scrub`, and the bare `Plate` and
`Transducer` components — the family's rule is that parts get reachability from the start), the
executed `notebooks/diffuseur.ipynb` and `notebooks/scrub.ipynb`, and four more
`radiohead_render` scenarios. Here: three wrappers, fourteen min-api scenarios, three reference
pages, three help patchers, and the pin bumped to match.

Still open for this family: the on-Mac validation pass and maxtest starters for every object in
it, and book chapters for `tap.touche~`, the diffuseurs and `tap.scrub~` — the first two
probably belong inside an Ondes-family chapter once the `triode` and heterodyne `source` stages
exist, and the scrub's beside the stammer's, since they share a tape.

**23. The Ondes Martenot, finished (2026-08-17).** ✅ `tap.triode~` and `tap.ondes~` — the last
two pieces of the instrument, and the two the family plan had wrong.

**The plan assumed a `vco.h` descendant with waveform registers.** The Ondes Martenot is nothing
of the kind: it is **heterodyne**. Two oscillators near 80 kHz, one fixed and one moved by the
ribbon, sum into an amplitude-modulated signal whose envelope is the note — and Najnudel, Hélie,
Roze & Boutin (*IEEE/ACM TASLP* **28**, 2651–2660, 2020), who model instrument No. 169 as five
port-Hamiltonian stages, measure those oscillators at about **0.03 % second harmonic** even
coupled to the rest of the circuit. All the character is downstream.

**`tap.triode~` is almost entirely a citation.** The design pass had framed the valve as a choice
between "a published grid-conduction curve" and "the tanh family with an asymmetry bias, decided
by listening". It is neither, because the circuit paper does not merely mention a tube model — it
names one (the **enhanced Norman Koren** law: Koren, *Glass Audio* 8(5), 1996, with Cohen &
Hélie's grid-current extension, AES 129, 2010), writes out its equations, and publishes parameter
sets **fitted to the valves actually in No. 169** — 6F5, 6C5, 2A3 — alongside every stage's supply
voltage, cathode resistor and plate load. So there was nothing to voice by ear. A stage is the
static solution of its load line, which is a memoryless nonlinearity in the DAFx-07 sense, so
tabulating it is not an approximation of the model: it *is* the model. The fitted 6C5 lands at
8.85 mA against its datasheet's 8 mA typical, the published operating points bias sanely, and the
curve is asymmetric in a 2.17 : 1 ratio — which is where a triode's even harmonics live.

**The plan's instruction for the source was wrong, and catching it was the most valuable thing in
this entry.** "Synthesize the difference tone directly as a sinusoid" would have thrown away the
instrument's largest single source of harmonics. The paper's licence to substitute a sinewave
generator applies to the **oscillators**; the demodulator is not a mixer handing you a difference
tone, it is an envelope detector, and the envelope of two summed oscillators is `2|cos|` — whose
series puts H2 at −14.0 dB and H3 at −21.3 dB **before any valve touches the signal**.

**What replaces the carrier turned out to be an identity rather than a simplification.** For
amplitudes 1 and *d* the envelope of `cos(Φ) + d·cos(Φ−φ)` is exactly `sqrt(1 + d² + 2d cos φ)`,
so the 80 kHz carrier drops out of the arithmetic instead of being approximated away. Running the
published 200 µs detector (R4·C21) on that closed form reproduces the full
heterodyne-plus-diode-plus-RC simulation to **within 0.10 dB on every harmonic at every pitch
tried**, with one systematic difference — a uniform 3 % level offset, because a follower chasing
real carrier half-cycles never quite reaches the peak between them. The detector's characteristic
pitch dependence comes free (H2 runs −14.0 dB at A2 to −19.3 dB at A6, and the level falls 2.0 dB
across those five octaves), and because the closed form is parameterized by the two oscillator
amplitudes, **oscillator balance becomes a real physical timbre control** rather than an invented
knob. The ribbon law is the paper's Eq. 7 and it is simple: the ribbon is **linear in semitones**,
which is exactly why an ondes glissando sounds the way it does.

**Three defects found by measurement.** A stage that quietly un-inverted itself applied the
valve's asymmetry to the wrong side of the waveform, so the drive knob *reduced* harmonics as it
was turned up — a real common-cathode stage inverts, and the sign is load-bearing rather than
cosmetic. An aliasing probe measured its own spectral leakage instead of aliasing (the same trap
the fuzz and scrub suites already record). And the voice's anti-zipper setting never reached the
intensity key, which keeps its own slew — so a key set to rest kept sounding for 20 ms nobody had
asked for. That last one was caught by a **wrapper** test in this repo and fixed in the kernel,
which is the two-layer test split doing exactly what it is for.

**Two things the sources do not settle are switches, not silent guesses**, because both measure as
audible: where the intensity key sits in the chain (`@keyplacement` — the paper's five stages do
not include it, and placing it before the valves makes pressure mean dirt rather than level; worth
about 0.09 of total harmonic content at a half-press) and the winding sense of the transformer
between the two valves (`@polarity` — worth about 0.12).

**And a data point for `fuzz.h`'s open oversampler question.** That kernel measured 4× coming out
*worse* than 2× and left an untested hypothesis behind: imaging, since zero-stuffing by N leaves
N−1 images for one filter to suppress and their residuals intermodulate in the clipper.
`tap.ondes~` runs the same 8th-order chain around a comparably hard nonlinearity, but as a
**source** — nothing is zero-stuffed on the way up, the detector simply runs fast, so there are no
images at all. Its sequence never reverses: about 12 dB per doubling to 4×, and 7–12 dB more at 8×
in the top octave. Same filters, no upsampler, no reversal. Evidence rather than proof, since the
nonlinearity differs too, but it is the first evidence either way and it points at the upsampler.

Kernel side (`tap/taptools`): `ondes.h` (`triode`, `detector`, `voice`), 18 Catch2 scenarios, the
C ABI plus ctypes surfaces (`Triode`, `Detector`, `Ondes`, and the bare tube law so a notebook can
plot the published valve curves from the shipping code), the executed `notebooks/ondes.ipynb`, and
four more `radiohead_render` scenarios — including `ondes_diffuseurs`, which is the whole
instrument wired together for the first time: the voice through the principal, the palme and the
métallique in turn. Here: two wrappers, nine min-api scenarios, two reference pages, two help
patchers, and the pin bumped to match.

**What is still missing from the instrument, and deliberately: the waveform registers.** The real
ondes has switchable timbres whose filter shapes are in none of the four sources obtained. Leipp
(*Bulletin du GAM* n°60, 1972) and Laurendeau's monograph are where to look next. Until then the
header says they are absent rather than filling the gap with invention — which, for an object
whose whole claim is that its numbers are published, is the only defensible answer.

Remaining (ongoing, now cross-repo — DSP lands in `tap/taptools`, then bump the submodule pin
here): lift the remaining simple inline-DSP objects' math into kernel headers opportunistically as
they're touched. Control/utility and Jitter objects never move — they are Max message-logic, not
kernel material.
