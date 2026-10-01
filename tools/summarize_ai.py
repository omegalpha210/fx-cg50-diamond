#!/usr/bin/env python3
"""Curate public-safe AI evidence; raw per-game logs remain in build/."""
import argparse
import csv
import json
import statistics
from collections import defaultdict
from pathlib import Path


def read(path):
    with Path(path).open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def mean(values):
    return statistics.fmean(values) if values else None


def write_csv(path, rows):
    if not rows:
        return
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def summarize(args):
    output = Path(args.out)
    output.mkdir(parents=True, exist_ok=True)
    choices, baseline, quality = read(args.choices), read(args.baseline), read(args.quality)
    quality_by = {(r["mode"], r["position"], r["level"]): r for r in quality}
    nonterminal = {}
    for mode in ("2P", "3P"):
        positions = {r["position"] for r in quality if r["mode"] == mode}
        nonterminal[mode] = {
            position for position in positions
            if all(abs(int(quality_by[mode, position, level][field])) < 900000
                   for level in ("EASY", "NORMAL", "HARD")
                   for field in ("reference_best", "reference_choice"))
        }
    quality_summary, comparisons, metrics, baseline_summary = [], [], [], []
    for mode in ("2P", "3P"):
        for level in ("EASY", "NORMAL", "HARD"):
            rows = [r for r in quality if r["mode"] == mode and r["level"] == level]
            regrets = [int(r["regret"]) for r in rows]
            regular = [int(r["regret"]) for r in rows if r["position"] in nonterminal[mode]]
            quality_summary.append(dict(
                mode=mode, level=level, positions=len(rows), reference_best_matches=regrets.count(0),
                mean_regret=mean(regrets), median_regret=statistics.median(regrets),
                common_nonterminal_positions=len(regular), common_nonterminal_mean_regret=mean(regular),
                terminal_choice_regrets=sum(int(r["regret"]) > 10000 for r in rows),
            ))
            observed = [r for r in choices if r["mode"] == mode and r["level"] == level]
            nodes = [int(r["nodes"]) for r in observed]
            times = [float(r["host_seconds"]) for r in observed]
            metrics.append(dict(
                mode=mode, level=level, positions=len(observed), legal_moves_mean=mean([int(r["legal_moves"]) for r in observed]),
                nodes_mean=mean(nodes), nodes_max=max(nodes), completed_depth_min=min(int(r["depth"]) for r in observed),
                completed_depth_max=max(int(r["depth"]) for r in observed),
                host_seconds_mean=mean(times), host_seconds_max=max(times),
            ))
            if level in ("EASY", "HARD"):
                before = [r for r in baseline if r["mode"] == mode and r["level"] == level]
                indexed = {(r["position"], r["board_hash"]): r for r in observed}
                identical = 0
                for row in before:
                    after = indexed[row["position"], row["board_hash"]]
                    if all(row[field] == after[field] for field in ("current", "from", "to", "type", "hops", "next_rng")):
                        identical += 1
                baseline_summary.append(dict(
                    mode=mode, level=level, positions=len(before), identical_choice_and_rng=identical,
                    baseline_nodes_mean=mean([int(r["nodes"]) for r in before]), current_nodes_mean=mean(nodes),
                    baseline_host_seconds_mean=mean([float(r["host_seconds"]) for r in before]),
                    current_host_seconds_mean=mean(times),
                ))
        for stronger, weaker in (("NORMAL", "EASY"), ("HARD", "NORMAL"), ("HARD", "EASY")):
            rows = [r for r in quality if r["mode"] == mode and r["level"] == stronger]
            differences = [int(quality_by[mode, r["position"], weaker]["regret"]) - int(r["regret"]) for r in rows]
            regular = [delta for r, delta in zip(rows, differences) if r["position"] in nonterminal[mode]]
            comparisons.append(dict(
                mode=mode, comparison=f"{stronger}_vs_{weaker}", positions=len(rows),
                improved=sum(delta > 0 for delta in differences), tied=differences.count(0), worse=sum(delta < 0 for delta in differences),
                mean_regret_reduction=mean(differences), common_nonterminal_mean_regret_reduction=mean(regular),
            ))
    tournament_rows = read(args.tournament)
    grouped = defaultdict(list)
    for row in tournament_rows:
        grouped[row["pair"]].append(row)
    tournament_summary = []
    for pair, rows in grouped.items():
        stronger, weaker = pair.split("_vs_")
        tournament_summary.append(dict(
            mode="2P", pair=pair, games=len(rows), stronger_wins=sum(r["winner"] == stronger for r in rows),
            weaker_wins=sum(r["winner"] == weaker for r in rows),
            diagnostic_repetition_halts=sum(int(r["cycle"]) for r in rows),
            diagnostic_cap_timeouts=sum(int(r["timeout"]) for r in rows), illegal_moves=0,
            mean_turns=mean([int(r["turns"]) for r in rows]), maximum_turns=max(int(r["turns"]) for r in rows),
            mean_visited_nodes=mean([int(r["nodes"]) for r in rows]),
            mean_host_seconds=mean([float(r["host_seconds"]) for r in rows]),
        ))
    three_summary = []
    for path in [args.three] + ([args.control] if args.control else []):
        rows = read(path)
        for level in ("EASY", "NORMAL", "HARD"):
            goals, ranks = [], []
            for row in rows:
                counts = {color: int(row[f"{color}_goal"]) for color in ("red", "yellow", "green")}
                for color, count in counts.items():
                    if row[color] != level:
                        continue
                    goals.append(count)
                    ranks.append(1.0 + sum(1.0 if other > count else 0.5 if other == count else 0.0
                                           for key, other in counts.items() if key != color))
            if goals:
                three_summary.append(dict(
                    mode=rows[0]["mode"], level=level, games=len(rows), player_exposures=len(goals),
                    wins=sum(r["winner"] == level for r in rows), win_share=sum(r["winner"] == level for r in rows) / len(rows),
                    mean_goal_count=mean(goals), mean_goal_rank_proxy=mean(ranks),
                    diagnostic_repetition_halts=sum(int(r["cycle"]) for r in rows),
                    diagnostic_cap_timeouts=sum(int(r["timeout"]) for r in rows), illegal_moves=0,
                    mean_turns=mean([int(r["turns"]) for r in rows]),
                    mean_visited_nodes=mean([int(r["nodes"]) for r in rows]),
                    mean_host_seconds=mean([float(r["host_seconds"]) for r in rows]),
                ))
    write_csv(output / "quality-summary.csv", quality_summary)
    write_csv(output / "quality-comparisons.csv", comparisons)
    write_csv(output / "choice-metrics.csv", metrics)
    write_csv(output / "baseline-comparison.csv", baseline_summary)
    write_csv(output / "tournament-summary.csv", tournament_summary)
    write_csv(output / "three-player-summary.csv", three_summary)
    write_csv(output / "quality-positions.csv", quality)
    write_csv(output / "current-choices.csv", choices)
    recheck_summary = []
    for path in args.rechecks or []:
        rows = read(path)
        if rows:
            recheck_summary.append(dict(
                mode=rows[0]["mode"], cases=len(rows),
                original_stop_min=min(int(r["original_turns"]) for r in rows),
                original_stop_max=max(int(r["original_turns"]) for r in rows),
                extended_cap=int(rows[0]["recheck_cap"]),
                completed=sum(r["winner"] != "NONE" for r in rows),
                still_unfinished=sum(r["winner"] == "NONE" for r in rows),
                mean_goal_red=mean([int(r["red_goal"]) for r in rows]),
                mean_goal_green=mean([int(r["green_goal"]) for r in rows]),
            ))
    write_csv(output / "recheck-summary.csv", recheck_summary)
    summary = dict(
        schema=1, timing="Host CPU seconds only; calculator timing remains HARDWARE TEST REQUIRED",
        reference="Every legal root move scored separately at a common bounded horizon; same evaluator, wider selective beams; not an oracle",
        nonterminal_definition="Common positions where every level's selected reference utility and best utility have magnitude below 900000",
        repetition_policy="Diagnostic stop on equal occupancy, turn and RNG; gameplay rules have no automatic repetition draw or turn cap",
        opening_policy="Matched fixed-seed EASY openings: eight plies in 2P, nine in 3P; then compared profiles",
        quality=quality_summary, comparisons=comparisons, choice_metrics=metrics,
        baseline=baseline_summary, tournament=tournament_summary, three_player=three_summary, rechecks=recheck_summary,
    )
    (output / "summary.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(summary, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("choices", "baseline", "quality", "tournament", "three"):
        parser.add_argument(f"--{name}", required=True)
    parser.add_argument("--control")
    parser.add_argument("--rechecks", nargs="*")
    parser.add_argument("--out", default="docs/ai")
    summarize(parser.parse_args())
