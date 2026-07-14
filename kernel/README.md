# TapTools kernel

The portable DSP library behind the TapTools Max package: header-only, plain C++ (C++17
minimum), **no Max SDK, no min-api, no Jamoma**. One self-contained header per object under
`include/taptools/`, everything in the `taptools` namespace. The Max externals at the repo root
are thin [Min](https://github.com/Cycling74/min-api) shims over these kernels — the same split
AmbiTap uses (AmbiTap = kernel library, AmbiTap-Max = wrappers), and this directory is staged as
a complete standalone project so it can be extracted into its own repository the same way.

## What's here

| Kernel | Max object | Contents |
|---|---|---|
| `include/taptools/ladder.h` | `tap.ladder~` | ZDF Moog-style ladder (`taptools::ladder`) |
| `include/taptools/svf.h` | `tap.svf~` | Simper/Cytomic morphing SVF (`taptools::svf`) |
| `include/taptools/vco.h` | `tap.vco~` | Virtual-analog oscillator (`taptools::vco`) |
| `include/taptools/grm_comb.h` | `tap.5comb~` | GRM comb-bank recreation (`taptools::fivecomb`) |
| `include/taptools/grm_pitchaccum.h` | `tap.pitchaccum~` | GRM PitchAccum recreation (`taptools::pitchaccum`) |
| `include/taptools/conv_engine.h` | `tap.convolve~` | Partitioned (UPOLS) true-stereo convolution (`taptools::conv_engine`) |

Plus, all Max-free:

- **`tests/`** — Catch2 unit tests for the kernels (fetched via FetchContent; run with `ctest`).
  Wrapper-level tests (attributes, Min plumbing) stay with the externals in
  `source/projects/*/tap.*_test.cpp` on the min-api harness.
- **`tools/render/`** — offline WAV renderers (`ladder_render`, `vco_render`, `grm_comb_render`,
  `grm_pitchaccum_render`) for listening checks outside Max.
- **`tools/capi/`** — a small C ABI (`taptools_capi`) over the kernels for non-C++ consumers.
- **`notebooks/`** — Jupyter verification notebooks driving the *actual shipping DSP* through the
  C ABI via ctypes (`taptools_py.py`).
- **`bench/`** — CPU benchmarks and the per-machine regression ratchet (see `bench/README.md`).

## Build & test (standalone)

```sh
cmake -S kernel -B build-kernel -DCMAKE_BUILD_TYPE=Release
cmake --build build-kernel
ctest --test-dir build-kernel --output-on-failure
```

CMake exports the header-only target `TapTools::taptools`; `TAPTOOLS_BUILD_{TESTS,TOOLS,BENCH}`
and `TAPTOOLS_INSTALL` default to on only when the kernel is the top-level project. Installing
provides a `find_package(TapToolsKernel)` config package.

## How the Max package consumes it

The root `CMakeLists.txt` points the externals at `${TAPTOOLS_KERNEL_DIR}/include`
(`TAPTOOLS_KERNEL_DIR` defaults to this directory; override it with
`-DTAPTOOLS_KERNEL_DIR=/path/to/kernel` to build against an external checkout — after the repo
split this becomes a pinned submodule path, per the AmbiTap-Max pattern).

## Rules

- Kernels depend on the C++ standard library only. Anything Max-specific belongs in the wrapper.
- Keep headers **C++17-clean**: the wrapper `.cpp`s include them and the min-api unit-test
  harness compiles those at C++17.
- Sharing code *between* kernels is allowed here (that's much of the point — e.g. the radix-2
  FFT duplicated between `conv_engine.h` and `tap.nr~` should consolidate here); the Max-side
  "each object is self-contained" rule applies to `source/projects/`, not to the kernel.
- Provenance and algorithm notes live in each header's top comment. `REVIVAL.md` (repo root) is
  the roadmap, including which objects' DSP is still inline Max-side and next to extract.
