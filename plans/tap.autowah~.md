# Plan — `tap.autowah~` (Snow White–style envelope filter / auto-wah)

> Status: **plan approved-pending-review, not yet implemented.** Drafted 2026-07-15.
> This is the design + work breakdown for the next net-new DSP object: an auto-wah
> ("auto-eah") in the spirit of the **Mad Professor Snow White AutoWah** (BJF design).
> The DSP will land in the kernel repo (`tap/taptools`) first, then the Min wrapper +
> vertical slice here, per the AmbiTap/AmbiTap-Max split.

## 1. Why this object, and why now

- REVIVAL.md §8 already reserves the idea: `tap.autowah~` was a **help-patcher-only
  prototype** in the Cycling '74 `taptools-min` effort (no C++ source ever existed).
  Note: the prototype patcher itself does **not** survive at the `taptools-min` branch
  tip (only 4 help files are there), so this design starts fresh from the hardware
  reference rather than from that patcher. The other prototype from that pair,
  `tap.selfconvolve~`, already became `tap.convolve~` — this completes the pair.
- TapTools now has every ingredient on the shelf: the **Simper SVF kernel** (`svf.h`)
  for the swept filter, the **house ramp + 16-slot preset-morph pattern**
  (`grm_comb.h` / `grm_pitchaccum.h`), and envelope-follower precedent (`vocoder.h`
  per-band followers, `grm_pitchaccum.h` pitch follower). The new work is genuinely
  small: an envelope detector and a control-mapping law.
- It fills a real gap in the package: TapTools has static and LFO-less filters
  (`tap.svf~`, `tap.ladder~`, `tap.filter~`, `tap.fourpole~`) but **nothing
  input-dynamics-driven** on the filter side.

## 2. Reference hardware — what the Snow White actually is

Researched 2026-07-15 from the official manuals, the freestompboxes.org trace, the
La Révolution Deux block analysis, and the PedalPCB "Poison Apple" clone docs
(links in §7). Confirmed unless flagged as inference.

**Controls (all versions):**

| Knob | Hardware function |
|------|-------------------|
| **Sensitivity** | Input gain into the envelope detector ("filter trigger level"). **At zero, the pedal becomes a fixed, manually swept filter** — Bias turns into a cocked-wah control. |
| **Decay** | Discharge rate of the envelope capacitor: how fast the filter falls back to the resting point. Fast = wah on every note; slow = classic auto-wah swells. |
| **Bias** | The filter's **resting center frequency** — the point the sweep starts from and returns to. |
| **Resonance** | Filter Q. |

No attack knob (attack is **fast and fixed** — the pedal's signature), no dry/wet
mix (wet-only path), no up/down switch (**sweep is upward only**). The factory "GB"
version adds a single **GTR/BASS voicing switch** (almost certainly shifting the
sweep capacitors — i.e. the frequency range — downward for bass; the GB schematic
is not publicly traced).

**Topology (traced):** JFET input buffer → **2-pole OTA state-variable filter
(LM13700)**, control current = Bias standing current + envelope current. Sidechain:
signal tap → **diode rectifier + RC envelope** (single silicon diode charging a cap;
Decay pot sets the discharge). Published sweep range **250–2500 Hz** (≈3.3 octaves).
Stock output is voiced "like a real wah" — resonant and mid-focused, *not* Mu-Tron
quack; the traced circuit also has a **bandpass tap** that modders switch to
(noticeably quieter). *Inference to verify against the schematic PDFs: the stock tap
is the resonant lowpass node; slope ≈ 12 dB/oct.*

**Character targets (player consensus):** very fast tracking; throaty/vocal midrange
sweep rather than quack; works well on bass; the Sensitivity-0 fixed-filter mode is
a valued secondary use.

## 3. Design

### 3.1 Naming

**`tap.autowah~`** (folder/`.cpp`: `source/projects/tap.autowah_tilde/`), matching
the name REVIVAL.md §8 already reserves. (The request called it an "auto-eah" — if
that spelling is wanted as the actual object name, `tap.eah~` is a one-line rename
at implementation time; everything below is name-independent. Defaulting to
`autowah` for discoverability and continuity with the archived prototype.)

### 3.2 Architecture — model the behavior, reuse the SVF

A behavioral model, not a component-level circuit sim, per house practice
(`tap.5comb~`/`tap.pitchaccum~` model GRM behavior; `tap.ladder~`/`tap.svf~` model
circuit *families*):

```
in ──┬────────────────────────────────► svf_filter (2-pole, LP default, Q=resonance) ──► gain/mix ──► out
     │                                        ▲ cutoff, per sample via tick(cutoff_hz)
     └─► sensitivity gain ─► rectifier ─► AR follower ─► bias + env·range mapping (exp, clamped)
```

- **Filter core: compose `taptools::svf::svf_filter`** (sharing between kernels is
  encouraged in the kernel repo). One 2nd-order section (`order 2`), driven per
  sample through the existing `tick(cutoff_hz)` signal-rate-cutoff path — the exact
  mechanism the `tap.svf~`/`tap.ladder~` right inlets use, unconditionally stable
  under per-sample modulation (the reason the Simper TPT core was chosen). Default
  mode **lowpass** (the inferred stock tap); expose **`mode`** with at least
  `lowpass`/`bandpass` — the hardware genuinely has both taps, and the BP option is
  the well-known mod. The `circuit_driven` tanh path is available for free as an
  optional **`drive`** color control (OTA-ish saturation; default 0 = clean).
- **Envelope detector:** `sensitivity` gain (dB) → rectifier → one-pole
  attack/release follower: fixed-fast **attack** (default ~2 ms, exposed as an
  attribute since it costs nothing) and the **`decay`** control (ms, wide log
  range ~20 ms–2 s, matching the community's 1 MΩ-pot revision that widened the
  stock range). Rectifier: start **full-wave (|x|)** for level-symmetric tracking;
  the trace suggests half-wave (single diode) — make it a kernel-internal constant
  and A/B it during validation, since half-wave adds ripple at the sweep rate that
  may be part of the feel.
- **Control law:** `cutoff = clamp(bias · 2^(env_scaled · range_oct), floor, ceil)`
  — exponential (octave-linear) sweep upward from `bias`, which matches both the
  OTA current-to-frequency behavior and how wah sweeps are heard. Defaults sized to
  the hardware: `bias` default ≈ 250 Hz, `range` default ≈ 3.3 octaves (→ ~2500 Hz
  ceiling), both freely settable beyond the hardware span (we are not limited by
  two sweep caps). A soft saturation on `env_scaled` (e.g. tanh) rather than a hard
  clamp keeps hard playing from slamming the rail — verify feel in Max.
- **Faithful behaviors kept:** `sensitivity` at minimum ⇒ envelope contributes
  nothing ⇒ **fixed cocked-wah mode** where `bias` is the manual sweep (falls out
  of the model for free, but document + test it, since it's a headline feature);
  wet-only default; upward sweep default.
- **Deliberate extensions (all neutral at default, flagged in the maxref like
  `tap.5comb~`'s deviations):** `mix` (default 100 % wet), `direction up|down`
  (down-sweep, default up), `drive` (default 0), `attack` (default the fixed-fast
  hardware value), `mode` (default lowpass). The GB **GTR/BASS switch is not
  modeled as a switch** — it's just a range/bias preset in software; instead ship
  factory **preset slots** (guitar voicing, bass voicing, slow-swell, cocked-wah)
  via the house preset engine.
- **House kit, same as the GRM kernels:** every audible parameter rides a
  per-sample linear ramp (`smooth`, default ~20 ms); **16-slot preset-morph
  engine** (`store`/`recall` with timed interpolation — morphing between a bass and
  a guitar voicing *is* the GB switch, but better); allocation-free after
  `prepare(sr)`; deterministic (no wall-clock, no unseeded RNG — there is no RNG
  here at all); C++17-clean header, `taptools` namespace, house naming/layout per
  STYLE.md.
- **Multichannel:** kernel processes N channels sharing one detector? **No** — keep
  it strictly single-channel like the other effect kernels (per-channel envelope is
  the correct behavior under `mc.` wrapping; a shared-sidechain use case is served
  by the sidechain inlet below).

### 3.3 Kernel surface (in `tap/taptools`)

New header **`include/taptools/autowah.h`**, `namespace taptools::autowah`, class
`wah_filter` (working name), following the `grm_*` parameter-index pattern:

- `p_sensitivity` (dB, −60..+24, default 0 — 0 dB ≈ line-level instrument; −60 ⇒
  fixed-filter mode), `p_attack` (ms, 0.1..100, default 2), `p_decay` (ms,
  10..5000, default 250, log taper), `p_bias` (Hz, 20..20000, default 250),
  `p_range` (octaves, −5..+5 via `direction`… see below, default 3.3),
  `p_resonance` (0..1, default ~0.55 — a wah-ish Q; map through
  `svf::q_from_resonance`), `p_drive` (dB, 0..24, default 0), `p_gain` (dB),
  `p_mix` (%, default 100).
- Structural (not ramped/morphed): `mode` (lp/bp — forwarded to the SVF),
  `direction` (fold into signed `range` in the kernel; keep `direction` a wrapper
  nicety), rectifier constant.
- API: `prepare(sr)`, `set_param/clamp_param`, per-param setters, `process(x)` (and
  a sidechain overload `process(x, key)`), `envelope()` accessor for the wrapper's
  envelope outlet, `clear()`, `store_preset`/`recall_preset`/`set_preset` — same
  shapes as `comb_bank`/`accum_bank` so the wrapper macro pattern transfers.

### 3.4 Wrapper surface (here, `source/projects/tap.autowah_tilde/`)

- **Inlets:** left — signal in; right — **(signal) optional sidechain key input**:
  when a signal is connected (`has_signal_connection()`), the envelope tracks the
  sidechain instead of the main input. This is the one wrapper-level extension with
  real patching value (wah your pad from your kick), and it mirrors the
  modulator/carrier precedent of `tap.vocoder~`.
- **Outlets:** left — signal out; right — **(signal) the envelope** (0..1), so the
  detector can drive other things in the patch — cheap and very TapTools.
- **Attributes:** the §3.3 set via the `TAP_*_ATTR` macro pattern from
  `tap.ladder_tilde.cpp`, plus `mode` / `direction` as `attribute<int>` with named
  index constants (the GCC `enum class` lesson, REVIVAL.md §9.5), `smooth`,
  `interp` (preset-morph default ms), `bypass`, `mute`.
- **Messages:** `clear` (reset filter state + envelope), `store <slot>`,
  `recall <slot> [ms]`.
- Single-channel; `mc.` for multichannel. `MIN_RELATED`:
  `tap.svf~, tap.ladder~, tap.filter~, tap.adsr~`.

## 4. Testing plan

**Kernel Catch2 suite** (`tests/autowah_test.cpp`, kernel repo — DSP correctness
lives there):

1. **Attack/decay timing:** step a burst in, measure envelope rise to 63 % within
   the attack constant and fall per `decay`; decay change re-times the release.
2. **Sweep range + law:** silence ⇒ cutoff sits at `bias` (verify via a probe tone
   at the resting resonant peak, Goertzel — the house measurement idiom); full-scale
   drive ⇒ cutoff reaches but never exceeds the `range` ceiling (clamp/soft-sat
   bounded).
3. **Upward motion:** on a burst, the spectral centroid of filtered noise rises
   then falls back — and inverts with `direction down`.
4. **Fixed-filter mode:** `sensitivity` at floor ⇒ output is bit-close to a bare
   `svf_filter` at `bias`/`resonance` (the cocked-wah equivalence, tested directly
   against the composed SVF as reference).
5. **Resonance mapping:** peak gain at the resting frequency grows monotonically
   with `resonance`; matches `tap.svf~`'s Q behavior at the corners.
6. **Sidechain:** key input sweeps the filter while the main input is steady noise.
7. **Boundedness/abuse:** max sensitivity + max resonance + max drive on square
   bursts stays finite; per-sample parameter thrash (morph mid-note) is
   click-free (continuity bound between adjacent samples).
8. **Determinism:** two identical runs are bit-identical.
9. **Preset morph:** store two voicings, recall over N ms, parameters land exactly
   and move monotonically.

**Render tool** (`tools/render/autowah_render.cpp`): offline WAV demos — funk-chop
noise bursts through guitar/bass voicings, slow-swell, cocked-wah sweep, sidechain
demo — the listening-check material, same as `grm_comb_render`.

**Wrapper test** (`tap.autowah_tilde_test.cpp`, min-api harness): attribute
defaults, clamping, `mode`/`direction` index mapping, message plumbing.

**Runtime maxtest** (`runtime-tests/patchers/`, via `make_maxtest.py` with the
2-inlet arg): envelope outlet responds to a `test.sample~` burst; sensitivity-0
passes a steady tone at the bias-peak gain.

**Optional (nice-to-have): notebook** driving the kernel through the C ABI
(`tools/capi` + `notebooks/`), plotting the measured sweep law (envelope vs. cutoff)
against the 2^x design curve — the `convolution_reverb.ipynb` template.

## 5. Vertical slice checklist (definition of done)

Kernel repo (`tap/taptools`) — **land first**:
- [ ] `include/taptools/autowah.h` (+ umbrella `taptools.h` entry, README table row)
- [ ] `tests/autowah_test.cpp` green in kernel CI (3-OS)
- [ ] `tools/render/autowah_render.cpp`
- [ ] clang-format/-tidy clean (STYLE.md)

Wrapper repo (here) — after bumping the `submodules/taptools` pin:
- [ ] `source/projects/tap.autowah_tilde/` (`.cpp` + `CMakeLists.txt` with the
      `CXX_STANDARD 20` override; `_tilde` naming convention)
- [ ] `tap.autowah_tilde_test.cpp` + `min-object-unittest` include
- [ ] `docs/tap.autowah~.maxref.xml` (document the hardware provenance + the
      deliberate extensions and their neutral defaults)
- [ ] `help/tap.autowah~.maxhelp` (sound source → wah → dac~; sensitivity-0
      cocked-wah demo; preset store/recall demo)
- [ ] `runtime-tests/patchers/tap.autowah~.maxtest.maxpat`
- [ ] REVIVAL.md progress-log entry; CI green both platforms, `.mxo` universal
- [ ] The standing caveat, honestly recorded: **feel needs runtime validation in
      Max** — this object more than most, since "how it tracks your picking" *is*
      the product. Compare against demo videos of the pedal; A/B the half-wave vs
      full-wave rectifier; tune the default `resonance` and the soft-sat knee.

Estimated shape: the kernel is small (~300–400 lines — detector + mapping +
composed SVF + the boilerplate preset engine), the wrapper is boilerplate-sized,
and the tests/docs/help are the usual slice. One focused session for kernel+tests,
one for wrapper+slice.

## 6. Open questions (for the author, none blocking a start)

1. **Name:** `tap.autowah~` (as planned) or lean into `tap.eah~`?
2. **Envelope outlet:** signal-rate (proposed) or also a float/control tap à la
   `tap.sift~`? Proposed: signal only; patch `tap.sift~` after it if control-rate
   is wanted — that's what it's for.
3. **Rectifier default:** full-wave (cleaner tracking) vs half-wave (trace-faithful
   ripple). Proposed: decide by ear during runtime validation; kernel constant
   either way.
4. **Factory presets:** ship guitar/bass/slow-swell/cocked-wah in slots 0–3
   (proposed), or leave all 16 slots blank?

## 7. Sources

- Official manuals / product pages: mpamp.com Snow White AutoWah HW manual;
  SWBAW manual (manualslib); Snow White AutoWah GB product page (250–2500 Hz spec).
- freestompboxes.org trace thread t=29137 (also t=33562, t=24135, t=21330).
- La Révolution Deux block-by-block analysis (revolutiondeux.blogspot.com, 2012) —
  five-block breakdown, D6/C8 envelope mods.
- 320volt.com traced schematic page (LM324 + LM13700).
- PedalPCB "Poison Apple" build doc (direct clone schematic; 1 M decay pot
  revision); PedalPCB forum bass mod thread (sweep caps → ~47 n for bass range).
- tagboardeffects.blogspot.com layout + mod comments (LP/BP tap switching);
  sabrotone.com vero layout; GuitarPCB "SWAH" derivative.
- Player consensus threads (TalkBass etc.): fast tracking, mid-focused "real wah"
  voice vs. Mu-Tron quack, bass-friendliness.
