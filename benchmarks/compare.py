#!/usr/bin/env python3
"""The tap.svf~ benchmark ratchet: compare a fresh `svf_bench --json` run against a
committed baseline. Exits nonzero if any case regressed past the threshold, so it can
gate a commit:

    cmake --build build --target svf_bench
    tests/svf_bench --json > /tmp/current.json
    python3 benchmarks/compare.py /tmp/current.json benchmarks/baselines/<machine>.json

Baselines are machine-specific (absolute ns/sample is meaningless across machines);
compare only against the baseline recorded on the same machine, and re-record the
baseline (`svf_bench --json > benchmarks/baselines/<machine>.json`) when an
optimization lands.
"""
import argparse
import json
import sys

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("current", help="fresh svf_bench --json output")
p.add_argument("baseline", help="committed baseline json for this machine")
p.add_argument("--max-regress", type=float, default=5.0,
               help="fail if any case is more than this %% slower (default 5)")
a = p.parse_args()

cur = json.load(open(a.current))["cases"]
base = json.load(open(a.baseline))["cases"]

fail = False
for name, b in base.items():
    c = cur.get(name)
    if c is None:
        print(f"MISSING {name} (in baseline but not in current run)")
        fail = True
        continue
    delta = (c["ns_per_sample"] / b["ns_per_sample"] - 1.0) * 100.0
    if delta > a.max_regress:
        flag, fail = "REGRESS", True
    elif delta < -2.0:
        flag = "faster "
    else:
        flag = "ok     "
    print(f"{flag} {name:24s} {b['ns_per_sample']:9.2f} -> {c['ns_per_sample']:9.2f} "
          f"ns/sample ({delta:+.1f}%)")
for name in cur:
    if name not in base:
        print(f"NEW     {name:24s} {cur[name]['ns_per_sample']:9.2f} ns/sample "
              f"(not in baseline)")

sys.exit(1 if fail else 0)
