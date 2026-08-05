# TapTools

[![build](https://github.com/tap/TapTools-Max/actions/workflows/build.yml/badge.svg)](https://github.com/tap/TapTools-Max/actions/workflows/build.yml)
[![Tap House Style](https://github.com/tap/TapTools-Max/actions/workflows/style.yml/badge.svg)](https://github.com/tap/TapTools-Max/actions/workflows/style.yml)

A potpourri of objects for [Max](https://cycling74.com/products/max) — roots back to 1999,
rebuilt in 2026 on a modern toolchain: the [Min SDK](https://github.com/Cycling74/min-api),
CMake, GitHub Actions, macOS universal (arm64 + x86_64) and Windows x64 binaries. All DSP
lives in a portable, Max-free kernel library — [TapTools](https://github.com/tap/TapTools),
pinned here as a submodule — where it is unit-tested, benchmarked, and measured in executed
verification notebooks; the externals in this repository are thin wrappers over those
kernels.

## The objects

Around eighty objects across MSP, control, and Jitter. The headliners, by family:

- **Sources** — `tap.vco~` (polyBLEP virtual-analog oscillator with a seeded,
  measured "analog section"), `tap.noise~` (five colors).
- **Filters** — `tap.svf~` (the morphing Simper SVF), `tap.ladder~` (nonlinear ZDF
  transistor ladder), `tap.fourpole~`, `tap.diode~` (the 303's diode ladder),
  `tap.autowah~` (Snow White envelope filter model), `tap.filter~`.
- **The rhythm machines** — `tap.303~` / `tap.303.seq~` (circuit-informed TB-303 voice and
  sequencer), and the `tap.808.*` family: eight TR-808 voice channels recreated circuit
  block by circuit block, calibrated against a real unit, plus `tap.808.seq~`.
- **Pitch and voice** — `tap.tune~` (real-time pitch correction), `tap.harmony~`
  (formant-preserving multi-voice harmonizer), `tap.shift~`, `tap.pitchaccum~` (the GRM
  shimmer loop), `tap.vocoder~` (24-band channel vocoder).
- **Rooms and strings** — `tap.convolve~` (exact true-stereo partitioned convolution),
  `tap.verb~` (algorithmic Moorer reverb), `tap.5comb~` (the GRM tuned comb bank),
  `tap.comb~`, `tap.delay~`, `tap.multitap~`.
- **The pedalboard and the spectral set** — `tap.overdrive~` (voiced feedback overdrive),
  `tap.nr~` (per-bin spectral gate), `tap.spectra~` (bin remapper), `tap.fft.*`.
- **Dynamics, utility, Jitter** — `tap.limi~`, `tap.vca~`, `tap.adsr~`, `tap.crossfade~`,
  `tap.pan~`, and a set of control and `tap.jit.*` objects.

Every DSP object ships its vertical slice: a reference page in `docs/`, a help patcher in
`help/`, and — for the kernel-backed objects — tests and measurements in the kernel repo.

## Documentation

- **[*Tools on Tap*](https://tap.github.io/TapTools/)** — the field guide: one chapter per
  object family, machine appendices deriving each kernel, and a Recipes part that patches
  the objects at famous records. Its standing promise: every performance claim is
  measured, not remembered.
- **Reference pages and help patchers** — in Max, as usual.
- **[`REVIVAL.md`](REVIVAL.md)** — the authoritative roadmap and progress log of the 2026
  revival, object by object.

## Installation

Clone (or download) into your Max Packages folder and build the externals (below), or grab
a built release when releases are cut. Requires Max 9+.

## Building from source

```sh
git clone https://github.com/tap/TapTools-Max.git
cd TapTools-Max
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Built externals land in `externals/` (`.mxo` on macOS — universal by default — and
`.mxe64` on Windows; on Windows configure with `-A x64`). CI builds both platforms on
every push and fails if a macOS binary is not universal.

The DSP kernels build and test standalone in their own repo:
`cmake -S submodules/taptools -B build-kernel && ctest --test-dir build-kernel`.

## Testing

Three layers: the kernel repo's Catch2 batteries (DSP correctness, oracle-measured),
min-api wrapper tests here (attributes and message plumbing, run via `ctest`), and a
`runtime-tests/` harness that loads the real externals in a licensed Max for in-Max
validation. See `REVIVAL.md` for per-object status.

## License

[BSD 3-Clause](LICENSE). © 2002–2026 Timothy Place and the TapTools contributors. The
kernel library and its DspTap primitives are MIT-licensed in their own repositories.
