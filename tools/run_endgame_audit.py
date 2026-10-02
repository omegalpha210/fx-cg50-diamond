#!/usr/bin/env python3
"""Compare a compiled historical baseline with current V1/V2 endgame policies.

Baseline source is supplied by the caller; no historical algorithm is copied
into the production engine. All reported seconds are host CPU timing only.
"""
import argparse
import csv
import io
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
CASES = [(2, 325974845, 0, 1, 0, 2), (2, 325974845, 1, 2, 0, 1),
         (3, 610839777, 2, 0, 1, 2), (3, 610944506, 0, 2, 0, 1),
         (3, 611153964, 1, 1, 2, 0)]


def run(exe, args, target):
    result = subprocess.run([str(exe.resolve()), *map(str, args)], capture_output=True, text=True, check=True)
    target.write_text(result.stdout)
    return list(csv.DictReader(io.StringIO(result.stdout)))


def summarize(rows):
    result = {}
    for level in ("EASY", "NORMAL", "HARD"):
        group = [r for r in rows if r["level"] == level]
        if not group:
            continue
        result[level] = dict(selections=len(group),
            immediate_win_misses=sum(int(r["missed_win"]) for r in group),
            goal_exits=sum(int(r["goal_exit"]) for r in group),
            assignment_regressions=sum(int(r["assignment_after"]) > int(r["assignment_before"]) for r in group),
            reversals=sum(int(r["reversal"]) for r in group),
            mean_goal_after=sum(int(r["goal_after"]) for r in group)/len(group),
            mean_assignment_after=sum(int(r["assignment_after"]) for r in group)/len(group),
            mean_nodes=sum(int(r["nodes"]) for r in group)/len(group),
            mean_depth=sum(int(r["depth"]) for r in group)/len(group),
            mean_host_seconds=sum(float(r["host_seconds"]) for r in group)/len(group))
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=Path, default=ROOT / "build/host/endgame_audit")
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--output", type=Path, default=ROOT / "docs/ai/beta4")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    versions = [("beta4-v1", args.executable, ["--legacy"]), ("beta4-v2", args.executable, [])]
    if args.baseline:
        versions.insert(0, ("beta3-v1", args.baseline, []))
    summary = {"provenance": "60 deterministic synthetic positions, same board construction for all versions; 180 choices each",
               "assignment_metric": "common unrestricted lattice exact one-to-one step-distance proxy; production V2 also uses allowed-graph distances",
               "timing": "host CPU seconds, never calculator latency", "fixtures": {}, "historical_stalls": {}}
    for name, exe, prefix in versions:
        rows = run(exe, prefix, args.output / f"endgame-{name}.csv")
        assert len(rows) == 180 and sum(int(r["missed_win"]) for r in rows) == 0
        summary["fixtures"][name] = summarize(rows)
        cases = []
        for i, case in enumerate(CASES):
            rows = run(exe, [*prefix, *case, 1200], args.output / f"stall-{name}-{i}.csv")
            won = any(int(r["goal_after"]) == 10 for r in rows)
            endgame = [r for r in rows if int(r["goal_before"]) >= 7]
            cases.append(dict(case=i, players=case[0], seed=case[1], finished=won,
                              turns=len(rows), endgame=summarize(endgame)))
        summary["historical_stalls"][name] = cases
    # Thresholds chosen after baseline measurement, before final validation:
    # zero HARD exits and at most one assignment regression across 60 V2 cases.
    hard = summary["fixtures"]["beta4-v2"]["HARD"]
    assert hard["goal_exits"] == 0 and hard["assignment_regressions"] <= 1
    (args.output / "endgame-summary.json").write_text(json.dumps(summary, indent=2)+"\n")
    print(json.dumps({"fixtures": summary["fixtures"], "stalls_finished": {
        name: sum(c["finished"] for c in cases) for name, cases in summary["historical_stalls"].items()}}, indent=2))


if __name__ == "__main__":
    main()
