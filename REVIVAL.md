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
| `tap.5comb~` | 5× comb filter | maybe |
| `tap.adapt~` | (audio processor) | review |
| `tap.buffer.record2~` | Smooth buffer recording (v2) | merge into `tap.buffer.record~`? |
| `tap.smooth` | Data-stream smoother | maybe (native alts exist) |
| `tap.deviate` | Randomize & "prime" input | maybe |
| `tap.semitone2ratio` | Semitones → frequency ratio | trivial; maybe fold in |
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
  (`svf.h`, `taptools::svf`, no Min dependency) under a thin wrapper. The kernel is
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

**2. Help patchers.** ✅ `tap.sustain~` and `tap.filter~` now have help patchers (authored
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
`tap.5comb~`, `tap.adapt~`, `tap.buffer.record2~` (merge into `tap.buffer.record~`?),
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
