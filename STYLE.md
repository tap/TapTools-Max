# Tap House Rules

> *The Tap house style — always on tap.*

The shared house style for the Tap libraries (AmbiTap, SampleRateTap, OscTap,
and future `*Tap` libraries). Anchored to the C++ standard library's own
conventions (per the ISO C++ Core Guidelines "NL" section), with a small set
of deliberate, documented exceptions.

Two config files enforce this and must be copied verbatim into every repo:

- **`.clang-format`** — layout (whitespace, braces, alignment, includes).
- **`.clang-tidy`** — identifier naming (`readability-identifier-naming`).
  clang-format *cannot* check names; this is what does.

CI runs `clang-format --dry-run --Werror` and `clang-tidy` so drift can't
return.

---

## 1. Naming

| Kind | Convention | Example |
|------|-----------|---------|
| Types (class/struct/enum/alias) | `snake_case` | `encoder`, `spsc_ring` |
| Functions / methods | `snake_case` | `push`, `write_available` |
| Variables / parameters / locals | `snake_case` | `frame_count`, `min_capacity` |
| Concepts | `snake_case` (like types) | `sample_type` |
| Template parameters | `PascalCase` — the ONLY leading-capital names | `T`, `S`, `Sample`, `Allocator` |
| Private/protected data members | `m_` + `snake_case` | `m_channels`, `m_order` |
| Public data members (struct fields) | `snake_case`, no prefix | `sample_rate_hz` |
| Constants (namespace/class/static) | `k_` + `snake_case` | `k_smoothing_samples`, `k_cache_line` |
| Enumerators | `snake_case` | `state::filling` |
| Macros | `ALL_CAPS` | `SRT_VERSION_MAJOR`, `TAP_EXPECTS` |

A leading capital letter means **template parameter** and nothing else. This
is the standard library's own allocation (`CharT`, `Rep`, `Period`,
`Allocator`) and is why concepts are lower-case: they read in type position,
so they look like the types they constrain.

**Deliberate deviations from strict std:**
- `k_` prefix on constants (std uses bare `snake_case`) — kept for use-site
  clarity. Applies to namespace-, class-, and static-scope constants;
  `constexpr` *locals* stay bare.
- `m_` prefix on encapsulated data members (std reserves `_`; user code has no
  standard convention here) — kept for self-documentation and greppability.

**Parameters take no prefix** (no `a_`/`an_`). `m_` already prevents any
member/parameter collision, and prefixes would clutter the public signatures
that *are* the library's contract. Lean on `const` and small functions for
input/local clarity.

**Repo exception — OscTap (drop-in legacy continuation).** OscTap continues
[oscpack](http://www.rossbencina.com/code/oscpack) as a *drop-in
source-compatible* successor: its public API keeps oscpack's original
identifiers — PascalCase types and methods (`ReceivedMessage`,
`OutboundPacketStream`, `BeginBundle()`, `AsFloat()`) and trailing-underscore
data members (`size_`, `value_`). Renaming these to the house `snake_case`/`m_`
scheme would break the source compatibility that is the library's reason to
exist. OscTap therefore adopts the **layout** rules (`.clang-format`) in full
but is **exempt from the naming rules** (`readability-identifier-naming`): it
ships a local `.clang-tidy` that disables that check while keeping mandatory
braces, and its CI runs a format-only style gate instead of the shared
`drift-check.yml`. The exemption is specific to legacy-continuation repos;
greenfield `*Tap` code follows the naming rules above.

## 2. Layout

- **Indent:** 4 spaces, including inside namespaces.
- **Braces:** attached everywhere (functions included); only `else` and
  `catch` break onto their own line. Every control-flow body is braced *and
  expanded* onto its own lines — no single-line `if`/`for`/`while`, even for
  guard clauses (braces via clang-tidy `readability-braces-around-statements`;
  expansion via `AllowShortBlocksOnASingleLine: Never`). Short accessor
  functions and lambdas may still be inline.
- **Brace-init spacing:** no space before a braced-init list — `float x{0.0f}`,
  not `x {0.0f}`.
- **Column alignment:** consecutive declarations, assignments, and trailing
  comments are aligned.
- **Constructor initializers:** comma-first, one per line, never packed.
- **Pointers/references:** bound to the type — `const float* p`, `T& r`.
- **Member declaration order (per NL.16):** `public` -> `protected` ->
  `private`; within a class: types/aliases -> constructors/assignment/
  destructor -> functions -> data members last.
- **Column limit:** 120.
- **`const` placement:** west-const (`const T`, not `T const`) — enforced by
  review, not tooling.

## 3. Files

- **Extension:** `.h` for headers (family-wide).
- **Header guard:** `#pragma once` (first line after the banner). Universally
  supported by GCC/Clang/MSVC; replaces the `#ifndef`/`#define`/`#endif` triple.
- **Per-file banner:** three lines —
  ```cpp
  /// @file spsc_ring.h
  /// @brief Lock-free single-producer single-consumer ring buffer.
  // SPDX-License-Identifier: MIT
  // Copyright 2025-2026 Timothy Place.
  ```
- **Doc comments:** `///` triple-slash with `@`-style commands
  (`@param`, `@return`, `@throws`, `@pre`). Not the `\`-command dialect.
- **Include ordering:** (1) the file's own corresponding header (in a `.cpp`),
  (2) C++ standard headers, (3) third-party, (4) this project — enforced by
  clang-format `IncludeBlocks: Regroup`, blank line between groups.

## 4. Safety idioms

The Tap libraries are header-only, zero-dependency, and target real-time /
embedded use (some build `-fno-exceptions`). We adopt the *vocabulary* of the
GSL but take **no dependency on it**; the helpers below are freestanding.

- **Contracts:** `TAP_EXPECTS(cond)` / `TAP_ENSURES(cond)` — assert in debug,
  clamp or no-op in release, **never throw**. (Generalizes AmbiTap's
  `validate.h`.) Not `gsl::Expects` (terminates + adds a dependency).
- **Narrowing:** `narrow_cast<T>(x)` — a documented `static_cast` synonym for
  intentional lossy conversions (e.g. Q15/Q31 fixed-point). Not `gsl::narrow`
  (throws; unusable under `-fno-exceptions`).
- **Views:** `std::span` (C++20, freestanding-friendly). Never `gsl::span`.
- **Non-null / ownership / bounds:** expressed via `@pre` documentation and
  debug asserts, **not** wrapper types. Raw pointers and raw indexing stay in
  hot paths for performance; `not_null`/`owner`/`at()` are not used as types.
