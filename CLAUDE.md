# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

TapTools is a collection of Max/MSP externals (roots back to 1999), revived in 2026 off a dead
toolchain (old Jamoma C++ library, Ruby/Xcode build, Travis CI, Intel-only binaries) onto a modern
one (Min SDK + CMake + GitHub Actions). The bulk of the port is done and consolidated on `main`:
Tiers 1–3, infrastructure, and all 5 Jitter objects. **`REVIVAL.md` is the authoritative roadmap** —
read it for the per-object status, the tier plan (Tier 1 utility → Tier 2 simple DSP → Tier 3
complex DSP → Jitter), the remaining frontiers (the spectral resurrection candidates —
`tap.vocoder~`/`tap.nr~`/`tap.spectra~` — and runtime validation in Max), and the rationale behind
the decisions below. Keep its
progress log current as objects land.

## Repo layout & branches

This is the **TapTools-Max** repository — the Max/MSP wrapper package. The portable DSP kernel
now lives in its **own repository** (`tap/taptools`, the TapTools library), pinned here as the
`submodules/taptools` submodule — the AmbiTap / AmbiTap-Max split. `main` is the consolidated
modern package; the legacy Jamoma-era tree has been pruned. What matters here:

- **`submodules/taptools/`** — the portable DSP library (header-only, plain C++, no Max/Min
  dependency), pinned as a submodule: one self-contained header per object under
  `submodules/taptools/include/taptools/`, in the `tap::tools` namespace, plus the kernel's own
  Catch2 tests, render tools, C ABI, notebooks, and benchmarks. It is a standalone CMake project
  with its own CI; develop the DSP there (or against a sibling checkout via `TAPTOOLS_KERNEL_DIR`).
  See `submodules/taptools/README.md`.
- **`source/projects/<name>/`** — the Min-based externals (one folder per object: a `.cpp` + a
  `CMakeLists.txt`), thin wrappers over the kernel headers where the DSP has been extracted.
- **`source/min-api/`** — the Min SDK submodule.
- **`scripts/extract-kernel-repo.sh`** — the git-filter-repo recipe used to carve the kernel out
  of this repo's history into `tap/taptools`; kept for provenance (runs against pre-split history).

The historical material lives on branches, not in the working tree:

- **`legacy`** (and `windows`) — the original Jamoma-era source (`Core/`, `build.rb`, the old
  `tap.*` wrappers, etc.). Use it as a *behavioral reference* to recover the exact DSP a Jamoma
  object wrapped — `git show legacy:source/tap.NAME~/tap.NAME~.cpp`.
- **`taptools-min`** — an archived 2016–2019 Cycling '74 Min port (deleted upstream; preserved
  here). Independent history. Source of `tap.sustain~` and several prototypes — see `REVIVAL.md` §8.

## Porting philosophy (non-obvious, load-bearing)

- **Jamoma is cut.** Reimplement each object's DSP/logic directly. Do not reintroduce a dependency
  on Jamoma or any other dead library.
- **Min is a thin wrapper only.** Use `min-api` (`#include "c74_min.h"`) for the Max plumbing —
  inlets/outlets, attributes, messages, the perform loop. Write **all DSP as plain portable C++**
  with **no dependency on `min-lib`** (min-lib is the under-maintained piece; keeping DSP portable
  makes the wrapper a small swappable shim). Substantial DSP belongs in a **kernel header** in the
  kernel repo (`submodules/taptools/include/taptools/<name>.h`, namespace `tap::tools`, C++ stdlib
  only, C++17-clean) with the wrapper `.cpp` doing only Min glue — six objects follow this today.
  New/changed DSP is committed in the kernel repo, then the submodule pin is bumped here. Within
  `source/projects/`, no shared global lookup tables — each object is self-contained; sharing
  between kernels is allowed (and encouraged) inside the kernel repo.
- **Port faithfully.** Match the original Jamoma object's behavior, including magic constants (e.g.
  `tap.dcblock~` keeps `R = 0.9997` from Jamoma's `TTDCBlock`). Document the provenance in a comment.
- **C++20** everywhere.
- Targets: **macOS universal (arm64 + x86_64)** and **Windows x64**. The old binaries were
  Intel-only and ran on nothing modern — never ship a non-universal macOS build.
- **Minimum macOS is 11.0** (tracking Max 9; also required by `std::filesystem`). It's set globally
  via a `-mmacosx-version-min=11.0` compile/link flag in the root `CMakeLists.txt`, because Min's
  `min-pretarget.cmake` force-pins `CMAKE_OSX_DEPLOYMENT_TARGET` to 10.11 and CMake has no per-target
  deployment property. `max_version_min` is `9.0`.

## Build

Requires the `min-api` submodule. After cloning or pulling:

```sh
git submodule update --init --recursive
```

macOS (universal by default — the root `CMakeLists.txt` forces `arm64;x86_64`):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Windows:

```sh
cmake -S . -B build -A x64
cmake --build build --config Release
```

- Built externals land in **`externals/`** (`*.mxo` on macOS, `*.mxe64` on Windows) — gitignored.
- The root `CMakeLists.txt` auto-discovers every folder under `source/projects/` that has a
  `CMakeLists.txt` (via `SUBDIRLIST`) and builds it. Adding an object is just adding a folder; no
  central list to edit.
- `package-info.json` is generated from `package-info.json.in` by the Min package machinery and is
  gitignored — edit the `.in`, not the output.
- A local `_build/` directory may exist from prior runs; CI and `.gitignore` standardize on `build/`.
- The externals find the kernel headers via `TAPTOOLS_KERNEL_DIR` (defaults to the pinned
  `submodules/taptools` submodule; override with `-DTAPTOOLS_KERNEL_DIR=/path/to/kernel` to build
  against a sibling dev checkout). Run `git submodule update --init --recursive` after cloning. The
  kernel also builds/tests standalone in its own repo (`cmake -S submodules/taptools -B build-kernel
  && ctest --test-dir build-kernel`), and has its own CI there.

CI (`.github/workflows/build.yml`) checks out submodules recursively, builds both platforms on
every push, and **fails the macOS job if any `.mxo` is not universal** (checked with `lipo`). DSP
correctness is gated by the kernel repo's own CI, not here.

## Adding / porting an object

1. Create `source/projects/tap.NAME/` containing `tap.NAME.cpp` and a `CMakeLists.txt`. Copy the
   `CMakeLists.txt` from an existing port (e.g. `source/projects/tap.dcblock_tilde/`) — it is
   boilerplate except that it **overrides `CXX_STANDARD` to 20** because Min's posttarget pins it to
   17. Keep that override.
2. **Tilde (`~`) objects:** name the folder *and* the `.cpp` with the `_tilde` suffix — e.g.
   `source/projects/tap.dcblock_tilde/tap.dcblock_tilde.cpp`. The SDK's `max-pretarget.cmake` maps
   `_tilde` → `~` in the output binary name, so it still loads in Max as `tap.dcblock~`. The Max-side
   class name passed to `MIN_EXTERNAL`, and the `docs/`/`help/` filenames, keep the literal `~`.
3. Ship the full vertical slice, not just the binary: add a reference page
   `docs/tap.NAME.maxref.xml` and a help patcher `help/tap.NAME.maxhelp`. Port these from the legacy
   package where they exist. The repo root is the Max package (`externals/`, `docs/`, `help/`,
   generated `package-info.json`).

### Min object idioms

See `source/projects/tap.dcblock_tilde/tap.dcblock_tilde.cpp` for the canonical small DSP object.
The shape: a class deriving `object<T>` (and `sample_operator<ins, outs>` for per-sample DSP, or
`sample_operator<0, 1>` for generators); `MIN_DESCRIPTION/TAGS/AUTHOR/RELATED` metadata; `inlet<>` /
`outlet<>` members; `attribute<>` for state (e.g. `bypass`, `mute`); `message<>` for commands (e.g.
`clear`); the DSP in `sample operator()(...)`; and a final `MIN_EXTERNAL(classname);`. Multichannel
is handled by users wrapping the object in `mc.` — objects stay single-channel.

## Testing

Two test layers:

**Kernel tests** (in the kernel repo, `submodules/taptools/tests/`) — plain Catch2 (FetchContent),
no Max/min-api/mock kernel. They drive the kernel headers directly and run via `ctest` in the
kernel repo's own CI. DSP correctness tests belong there, at full C++ (no mock-harness constraints).

**Wrapper tests** (min-api harness) — for Min-level behavior (attribute defaults, clamping,
message plumbing). To add tests to an object, drop a
`tap.NAME_test.cpp` next to its source and add this line to the object's `CMakeLists.txt` (after the
`min-posttarget` include):

```cmake
include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)
```

The test file does `#include "c74_min_unittest.h"` then `#include "tap.NAME.cpp"`, and uses
`test_wrapper<classname>` to instantiate the object (call `ext_main(nullptr)` once per scenario). The
canonical example is `source/projects/tap.sift_tilde/tap.sift_tilde_test.cpp`. Tests build against a
mock kernel and run via `ctest --test-dir build`; **CI runs `ctest` on macOS**. Note the harness
forces the test target to C++17, so tests can't use C++20-only features.

Beyond unit tests, "tested" still also means: builds on both platforms, the macOS binary is
universal (enforced in CI via `lipo`), and it loads/behaves correctly in Max against its help patcher
— the DSP objects in particular still need runtime validation in Max.

For that in-Max validation there is now a **runtime-test harness** under `runtime-tests/`:
Cycling '74's [`max-test`](https://github.com/Cycling74/max-test) (vendored as a submodule) plus our
own `*.maxtest.maxpat` patchers in `runtime-tests/patchers/` (generate starters with
`runtime-tests/make_maxtest.py`). Unlike the Catch tests (mock kernel), these load the real objects
in Max and assert on actual behavior via `test.assert`/`test.sample~`/`test.terminate`, automatable
over OSC with the bundled Ruby runner. It requires a **licensed Max install**, so it's a local on-Mac
gate, not CI. See `runtime-tests/README.md`.
