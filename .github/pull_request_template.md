## What this changes

<!-- One or two sentences. Link the issue if there is one. -->

## Why

<!-- The problem this solves, not a restatement of the diff. -->

## Verification

<!-- What you actually built and ran, and what you did not — state the gap
     plainly rather than leaving it to be inferred. If CI is the first real
     gate for part of this change, say so.

     Every performance claim cites the executed notebook cell or the pinned
     test that carries the number: measured, not remembered. -->

## Notes for the reviewer

<!-- Delete the lines that do not apply; add anything that surprised you.

  - **Contract change.** A documented contract point moved — packing,
    conventions, normalization, latency, a Q format, a default. This is
    breaking for every consumer; say which ones.
  - **Submodule pin moved.** Which pin, and why. After this merges by
    rebase/squash, repoint any open consumer pins at the identical tree on
    `main` so they stay reachable once the branch is deleted.
  - **Notebooks re-executed**, because behavior changed. They are committed
    executed.
  - **Style configs** came from a `taphouse` sync, not a hand-edit. A
    hand-edited copy fails the drift job by design.
  - **Canonical TapHouse file changed.** Tag it, then re-sync consumers —
    merge here first, since a guarded unconditional file must exist at the
    pinned ref before a consumer references it.
  - **Max/Pd package.** Reference page and help patcher ship for each new
    object; the macOS binary is still universal (arm64 + x86_64).
  - **Documented exception.** This repo diverges from a house rule on
    purpose (see its `.clang-tidy` / `STYLE.md` note) and this change keeps
    that divergence intact.
-->
