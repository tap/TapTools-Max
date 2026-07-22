#!/bin/bash
# Canonical Tap House SessionStart hook for Claude Code on the web.
#
# Fresh web-session containers clone the repo bare: no submodules, no
# pre-commit hook installed — so `git commit` runs unformatted and the
# clang-format CI gate fails on code a local clone would have fixed at commit
# time. This hook closes that gap at session start:
#
#   1. init submodules recursively (kernels, SDKs, vendored test harnesses)
#   2. install pre-commit and register the repo's canonical hook
#      (.pre-commit-config.yaml — the Tap-wide pinned clang-format)
#   3. warm the pinned clang-format binary so the first commit doesn't
#      pay the download (the container snapshot caches it)
#
# Canonical copy: taphouse (distributed by scripts/sync.sh alongside
# .clang-format / .clang-tidy / .pre-commit-config.yaml; drift-guarded where
# present). Web-only; local clones are untouched.
set -euo pipefail

if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
    exit 0
fi

cd "$CLAUDE_PROJECT_DIR"

echo "session-start: initializing submodules ..."
git submodule update --init --recursive

echo "session-start: installing pre-commit ..."
if ! python3 -m pre_commit --version >/dev/null 2>&1; then
    python3 -m pip install --quiet pre-commit
fi
python3 -m pre_commit install
python3 -m pre_commit install-hooks

echo "session-start: done."
