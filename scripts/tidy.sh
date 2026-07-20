#!/usr/bin/env bash
# Local mirror of the CI clang-tidy gate (.github/workflows/style.yml): the
# TapHouse .clang-tidy naming + mandatory-braces checks over this project's
# own translation units. Run it before pushing.
#
# WHY THIS EXISTS (and is not a pre-commit hook): the pre-commit hook only
# runs clang-FORMAT, which is cheap and context-free. clang-TIDY needs a
# compile database (a configured CMake build) and compiles every TU to reason
# about types — too slow and stateful for a per-commit hook, so CI keeps it as
# its own gate. This script is the fast local equivalent of that gate.
#
# Usage:
#   scripts/tidy.sh                       # sweep every project TU (full CI mirror)
#   scripts/tidy.sh tests/test_foo.cpp …  # only the given TU(s) — fast, for a change
#
# Env:
#   CLANG_TIDY=clang-tidy-18   # binary to use (default: clang-tidy-18, then clang-tidy)
#   TIDY_BUILD=build-tidy      # compile-database build dir (kept separate from ./build)
#   TIDY_RECONFIGURE=1         # force a cmake re-configure of the compile database
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

# Prefer clang-tidy-18 — the version the CI gate installs and the .clang-tidy
# checks are validated against; a different major version may disagree.
tidy="${CLANG_TIDY:-}"
if [ -z "$tidy" ]; then
    if command -v clang-tidy-18 >/dev/null 2>&1; then
        tidy=clang-tidy-18
    elif command -v clang-tidy >/dev/null 2>&1; then
        tidy=clang-tidy
        echo "warning: clang-tidy-18 not found; using '$tidy'. CI pins v18 — results may differ." >&2
    else
        echo "error: no clang-tidy found. Install clang-tidy-18 (apt) or set CLANG_TIDY=..." >&2
        exit 127
    fi
fi

build="${TIDY_BUILD:-build-tidy}"

# A compile database is what clang-tidy needs; reuse it across runs unless it is
# missing or a reconfigure is forced. This does not touch your ./build dir.
if [ "${TIDY_RECONFIGURE:-0}" = "1" ] || [ ! -f "$build/compile_commands.json" ]; then
    echo "== configuring compile database in $build/ (one-time; reuses cached deps) =="
    cmake -B "$build" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null
fi

# File list: the given TUs, or — matching CI exactly — every project TU in the
# database with third_party/ and fetched deps (_deps) excluded.
if [ "$#" -gt 0 ]; then
    files=("$@")
else
    mapfile -t files < <(python3 -c "
import json
for e in json.load(open('$build/compile_commands.json')):
    f = e['file']
    if 'third_party' not in f and '_deps' not in f:
        print(f)")
fi

echo "== clang-tidy ($tidy) over ${#files[@]} file(s) =="
fail=0
for f in "${files[@]}"; do
    out="$("$tidy" -p "$build" "$f" 2>/dev/null || true)"
    if printf '%s\n' "$out" | grep -qE "warning:|error:"; then
        printf '%s\n' "$out"
        fail=1
    fi
done

if [ "$fail" -eq 0 ]; then
    echo "clang-tidy clean."
else
    echo "clang-tidy found violations (above); fix before pushing." >&2
    exit 1
fi
