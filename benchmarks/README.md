# Benchmarks

CPU benchmarks for TapTools DSP kernels, starting with `tap.svf~` (`svf_bench`, built from
`source/projects/tap.svf_tilde/svf_bench.cpp` alongside the unit tests; the binary lands in the
gitignored `tests/` directory).

The harness times the **kernel only** (portable C++, no Max) over a matrix of configurations —
circuit (clean/driven) × order (2/4/8) × oversampling (1/2/4) × static vs signal-rate-modulated
cutoff — on deterministic noise, reporting the best-of-N ns/sample. A checksum keeps the
optimizer honest and doubles as a smoke check that a speedup didn't change the output wildly.

## The ratchet

1. Build and run: `tests/svf_bench` (human table) or `tests/svf_bench --json` (machine output).
2. Compare against the committed baseline **for the same machine**:

   ```sh
   tests/svf_bench --json > /tmp/current.json
   python3 benchmarks/compare.py /tmp/current.json benchmarks/baselines/<machine>.json
   ```

   Nonzero exit if any case regressed more than 5% (`--max-regress` to adjust).
3. When an optimization lands, re-record the baseline:
   `tests/svf_bench --json > benchmarks/baselines/<machine>.json` and commit it. The ratchet
   only tightens — a regression has to be justified explicitly.

Absolute numbers are machine-specific, so baselines live per-machine under `baselines/`.
`linux-x64-gcc-container.json` is a development reference (cloud container, Xeon 2.8 GHz, GCC
-O3); the authoritative baseline for release decisions should be recorded on the maintainer's
Mac (arm64), since macOS/arm64 is the primary shipping target. Correctness is gated separately
by the Catch suite (`ctest --test-dir build`) — every optimization must keep it green.
