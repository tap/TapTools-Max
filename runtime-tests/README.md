# TapTools runtime tests (max-test)

The Catch unit tests under `source/projects/*/...test.cpp` run against a **mock**
Max kernel — they verify pure logic but never load a real object into Max. This
directory adds the missing half: **runtime tests that run inside Max itself**,
using Cycling '74's [`max-test`](https://github.com/Cycling74/max-test) harness
(MIT-licensed, vendored here as a submodule). This is how we finally close the
"needs runtime validation in Max" gap that `REVIVAL.md` keeps flagging — and the
only way to validate the DSP objects' actual audio behavior.

```
runtime-tests/
├── max-test/                 # the harness (git submodule → Cycling74/max-test)
├── patchers/                 # our *.maxtest.maxpat tests (one per object)
├── max-test-config.json      # OSC ports for the Ruby automation runner
└── make_maxtest.py           # generator that emits starter patchers
```

## What a test patcher is

A max-test patcher is an ordinary Max patcher whose filename ends in
`.maxtest.maxpat` and which:

1. **starts itself** (a `loadbang`),
2. contains one or more **`test.assert <name>`** objects (the checks),
3. **terminates itself** when done (a bang into **`test.terminate`**).

Helper objects the harness provides: `test.sample~` (sample a signal — use
instead of `snapshot~`), `test.equals` (tolerant float compare),
`test.string.equals` (multi-line, line-ending-tolerant string compare). See
`max-test/patchers/` for the upstream reference examples (the audio example
`2087-bitxor~.maxtest.maxpat` is the topology our generator mirrors).

## Null tests: proving a patch is an object

`tap.airport~` and `tap.garden~` were split into components you can patch
(`tap.reel~`; `tap.chime~` / `tap.bloom` / `tap.scale` / `tap.gardener`). That
split makes a claim — *these parts wired together ARE that object* — and a claim
like that should be measured rather than asserted.

`patchers/tap.reel~-is-airport.maxtest.maxpat` measures it: one `sig~` drives
both a `tap.airport~` and three `tap.reel~` at the same lengths, the component
lanes are summed, the monolith is subtracted from that sum, and the result must
be zero. The record gates open while the DSP chain is still **off**, which is
what keeps the two sides sample-aligned — no audio has been processed yet, so
every tape head is still at zero when the gates open.

The kernel repo already pins the same identity bitwise in C++
(`airport_test.cpp`, "standalone lanes summed are the bank, bitwise"). This
patcher covers what that test cannot see: the wrappers. A mis-forwarded
attribute, or a `dspsetup` that re-prepares one side and not the other, would
pass in the kernel and fail here.

**There is deliberately no garden equivalent.** `tap.bloom` and `tap.gardener`
run on Max's scheduler rather than the audio clock, so their returns land within
an `@interval` tick of the grid instead of exactly on it. The patched garden is
the same machine but not the same sample stream, and a null test would be
asserting something that is not true. That identity is pinned in the kernel
instead (`garden_test.cpp`, "the bed is exactly its components wired together,
bitwise"), where both sides can share one clock.

## First-time setup (one-time, on your Mac)

The harness needs the **Max application** installed (it can't run in a headless
container). A **paid license is not required to *run* patchers**: the standalone
"Max Runtime" was discontinued in Max 7, and since then unlicensed Max runs (and
even edits) patchers indefinitely — only *saving* is disabled after the 30-day
demo. The max-test runner only ever runs patchers, so an unlicensed Max install is
sufficient. After cloning TapTools:

```sh
git submodule update --init --recursive          # pulls in runtime-tests/max-test
cp runtime-tests/max-test-config.json runtime-tests/max-test/misc/max-test-config.json
```

So Max can find both the test patchers and the `test.*` objects, add these to the
Max file preferences search path (Options → File Preferences), or symlink them
into your Max packages folder:

- `runtime-tests/max-test`   (provides the `test.*` objects)
- `runtime-tests/patchers`   (our tests)
- the TapTools package root  (provides the `tap.*` objects under test)

## Running

**Manually:** just open any `runtime-tests/patchers/*.maxtest.maxpat` in Max and
look at the `test.assert` objects — green/​red shows pass/fail live.

**Automated (all patchers, logged):** the harness ships a Ruby runner that drives
Max over OSC/UDP and writes results to a SQLite DB.

```sh
cd runtime-tests/max-test/ruby
ruby test.rb "/Applications/Max.app"     # point at your Max 9 install
```

It launches Max, waits for the file database, runs every `*.maxtest.maxpat` it can
find in the search path, and summarizes pass/fail. Results land in a SQLite file in
`runtime-tests/max-test/` (open with `sqlite3`).

## Authoring more tests

`make_maxtest.py` generates structurally-valid starter patchers (modeled on the
upstream working example) for both control-rate and signal-rate objects:

```sh
python3 runtime-tests/make_maxtest.py     # regenerates the bundled examples
```

> ⚠️ The generator runs headless (no Max), so a freshly generated patcher is a
> faithful **starting point**, not a guaranteed-passing test. Open each new
> patcher in Max once to confirm the assert/terminate timing, tweak expected
> values against real output, then commit it. The two bundled examples
> (`tap.prime`, `tap.radians~`) cover the two topologies; copy/extend them (or
> call `audio_test()` / `control_test()` from the script) for the rest — the DSP
> objects (filters, delays, reverb, the spectral trio, sustain~) are the priority.

## CI

These can't run on GitHub-hosted runners (they don't ship Max), so for now runtime
tests are a **local, on-Mac** gate — the Catch unit tests + the universal-binary
`lipo` check remain the automated CI gates.

Automating them later is feasible, and **not** blocked by licensing: an unlicensed
Max can run patchers (see setup above), so no paid seat is needed on the runner.
The real frictions are practical — Max is a GUI app that wants a logged-in GUI
session (and a first-run activation prompt), and it must be installed on the
runner. The path is therefore a **self-hosted macOS runner** with Max pre-installed
and a GUI session available, driving `ruby test.rb`. Tracked as a follow-up.
