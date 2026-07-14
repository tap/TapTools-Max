#!/usr/bin/env bash
#
# extract-kernel-repo.sh — carve the portable DSP kernel out of the TapTools monorepo into a
# standalone, history-preserving git repository (the AmbiTap / AmbiTap-Max split).
#
# It rewrites history with git-filter-repo so that every file now under kernel/ keeps its full
# history — including the commits from before it was moved into kernel/ (headers that used to
# live under source/projects/<obj>/, the C ABI under tools/capi/, the benchmarks under
# benchmarks/, the notebooks under notebooks/). The result is promoted so kernel/ becomes the
# repository root (include/ at top level, matching AmbiTap's layout).
#
# Prerequisites:
#   - git-filter-repo on PATH   (pip install git-filter-repo)
#
# Usage:
#   scripts/extract-kernel-repo.sh <source-repo> <ref> <output-dir>
#     <source-repo>  path or URL of the TapTools (monorepo) repo to read from
#     <ref>          branch/tag/commit that contains kernel/ (e.g. main)
#     <output-dir>   directory to create the extracted kernel repo in (must not exist)
#
# Example:
#   scripts/extract-kernel-repo.sh https://github.com/tap/taptools-max main /tmp/taptools-kernel
#   cd /tmp/taptools-kernel
#   cmake -S . -B build && cmake --build build && ctest --test-dir build   # verify
#   git remote add origin https://github.com/tap/taptools.git              # the NEW kernel repo
#   git push -u origin main
#
# Copyright 2002-2026 Timothy Place. Distributed under the New BSD License.

set -euo pipefail

if [ "$#" -ne 3 ]; then
    sed -n '2,32p' "$0"
    exit 2
fi

SRC="$1"
REF="$2"
OUT="$3"

if ! git filter-repo --version >/dev/null 2>&1; then
    echo "error: git-filter-repo not found on PATH (pip install git-filter-repo)" >&2
    exit 1
fi
if [ -e "$OUT" ]; then
    echo "error: output dir '$OUT' already exists" >&2
    exit 1
fi

echo ">> cloning $SRC @ $REF"
git clone --no-local --branch "$REF" --single-branch "$SRC" "$OUT"
cd "$OUT"

echo ">> rewriting history (keeping kernel/ files with their pre-move history)"
# Each --path keeps a file/dir across all history; each --path-rename maps an *old* location to
# its kernel/ home so the pre-move and post-move history of the same file stitch together. The
# final `--path-rename kernel/:` promotes kernel/ to the repository root.
git filter-repo --force \
    --path kernel/ \
    --path source/projects/tap.ladder_tilde/ladder.h \
    --path source/projects/tap.svf_tilde/svf.h \
    --path source/projects/tap.vco_tilde/vco.h \
    --path source/projects/tap.5comb_tilde/grm_comb.h \
    --path source/projects/tap.pitchaccum_tilde/grm_pitchaccum.h \
    --path source/projects/tap.convolve_tilde/conv_engine.h \
    --path source/projects/tap.convolve_tilde/tap.convolve_tilde_test.cpp \
    --path source/projects/tap.ladder_tilde/ladder_render.cpp \
    --path source/projects/tap.vco_tilde/vco_render.cpp \
    --path source/projects/tap.5comb_tilde/grm_comb_render.cpp \
    --path source/projects/tap.pitchaccum_tilde/grm_pitchaccum_render.cpp \
    --path source/projects/tap.svf_tilde/svf_bench.cpp \
    --path benchmarks/ \
    --path notebooks/ \
    --path tools/capi/ \
    --path-rename source/projects/tap.ladder_tilde/ladder.h:kernel/include/taptools/ladder.h \
    --path-rename source/projects/tap.svf_tilde/svf.h:kernel/include/taptools/svf.h \
    --path-rename source/projects/tap.vco_tilde/vco.h:kernel/include/taptools/vco.h \
    --path-rename source/projects/tap.5comb_tilde/grm_comb.h:kernel/include/taptools/grm_comb.h \
    --path-rename source/projects/tap.pitchaccum_tilde/grm_pitchaccum.h:kernel/include/taptools/grm_pitchaccum.h \
    --path-rename source/projects/tap.convolve_tilde/conv_engine.h:kernel/include/taptools/conv_engine.h \
    --path-rename source/projects/tap.convolve_tilde/tap.convolve_tilde_test.cpp:kernel/tests/conv_engine_test.cpp \
    --path-rename source/projects/tap.ladder_tilde/ladder_render.cpp:kernel/tools/render/ladder_render.cpp \
    --path-rename source/projects/tap.vco_tilde/vco_render.cpp:kernel/tools/render/vco_render.cpp \
    --path-rename source/projects/tap.5comb_tilde/grm_comb_render.cpp:kernel/tools/render/grm_comb_render.cpp \
    --path-rename source/projects/tap.pitchaccum_tilde/grm_pitchaccum_render.cpp:kernel/tools/render/grm_pitchaccum_render.cpp \
    --path-rename source/projects/tap.svf_tilde/svf_bench.cpp:kernel/bench/svf_bench.cpp \
    --path-rename benchmarks/:kernel/bench/ \
    --path-rename notebooks/:kernel/notebooks/ \
    --path-rename tools/capi/:kernel/tools/capi/ \
    --path-rename kernel/:

echo
echo ">> done. Extracted kernel repo at: $OUT"
echo ">> tree:"
git -C "$OUT" ls-files | sed 's/^/     /'
echo
echo "Next:"
echo "  cd $OUT"
echo "  cmake -S . -B build && cmake --build build && ctest --test-dir build   # verify"
echo "  git remote add origin <NEW-kernel-repo-URL>"
echo "  git push -u origin $REF"
