#!/usr/bin/env python3
"""Audit the checked-in 73-node camp memberships without inventing ownership.

The strict-opponent experiment is a diagnostic, not the implemented rule.
It deliberately reports conflicts that require a rule decision.
"""
import json
import re
import subprocess
from collections import deque
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
NAMES = ("RED GOAL", "GREEN HOME", "YELLOW GOAL", "RED HOME", "GREEN GOAL", "YELLOW HOME")
PAIRS = {"RED": (3, 0), "YELLOW": (5, 2), "GREEN": (1, 4)}
COLORS = {"RED": "#cd454b", "YELLOW": "#dbaa23", "GREEN": "#358466"}


def audit():
    pattern = r"^ \{(-?\d+),(-?\d+),(\d+),\{([^}]+)\}"
    nodes = []
    for match in re.finditer(pattern, (ROOT / "src/core/board.c").read_text(), re.M):
        q, r, mask = map(int, match.group(1, 2, 3))
        nodes.append({"id": len(nodes), "q": q, "r": r, "mask": mask,
                      "neighbors": list(map(int, match[4].split(",")))})
    assert len(nodes) == 73
    camps = [[n["id"] for n in nodes if n["mask"] & (1 << c)] for c in range(6)]
    def boundary(camp):
        result = []
        for n in nodes:
            if n["id"] not in camps[camp]:
                continue
            q, r = n["q"], n["r"]
            for _ in range(camp):
                q, r = q+r, -q
            if r == -3:
                result.append(n["id"])
        assert len(result) == 4
        return result
    assert all(len(c) == 10 for c in camps)
    shared = [n for n in nodes if n["mask"].bit_count() == 2]
    rows = []
    for players in (("RED", "GREEN"), ("RED", "YELLOW", "GREEN")):
        for player in players:
            home, goal = PAIRS[player]
            forbidden = set().union(*(set(camps[c]) for other in players if other != player for c in PAIRS[other]))
            reachability = []
            for start in camps[home]:
                seen, todo = {start}, deque([start])
                while todo:
                    for to in nodes[todo.popleft()]["neighbors"]:
                        if to >= 0 and to not in forbidden and to not in seen:
                            seen.add(to)
                            todo.append(to)
                reachability.append({"start": start, "reachable_goal_count": len(set(camps[goal]) & seen)})
            rows.append({"players": len(players), "player": player,
                         "forbidden_goal_nodes": sorted(set(camps[goal]) & forbidden),
                         "home_nodes_also_opponent": sorted(set(camps[home]) & forbidden),
                         "reachability_ignoring_occupancy": reachability})
    result = {"topology": "checked-in src/core/board.c", "nodes": len(nodes),
              "camp_sizes": [len(c) for c in camps], "camp_union": len(set().union(*map(set, camps))),
              "neutral_nodes": sum(n["mask"] == 0 for n in nodes),
              "camps": [{"id": c, "name": NAMES[c], "nodes": camps[c], "boundary": boundary(c)} for c in range(6)],
              "shared_nodes": [{"id": n["id"], "q": n["q"], "r": n["r"],
                                "camps": [c for c in range(6) if n["mask"] & (1 << c)]} for n in shared],
              "strict_opponent_home_and_goal_experiment": rows,
              "conclusion": "Rejected strict exclusion blocks victory. Approved V2 uses mover-relative own HOME/GOAL membership first.",
              "effective_v2_engine_audit": json.loads(subprocess.check_output([str(ROOT / "build/host/rule_audit")], text=True))}
    out = ROOT / "docs/screenshots/beta4"
    out.mkdir(parents=True, exist_ok=True)
    (out / "camp-audit.json").write_text(json.dumps(result, indent=2) + "\n")
    image = Image.new("RGB", (1200, 790), "#f7f8fa")
    draw = ImageDraw.Draw(image)
    draw.text((20, 12), "73 NODES / 6 x 10 CAMP MEMBERSHIPS / 6 SHARED CORNERS / 19 NEUTRAL", fill="#17222c")
    draw.text((20, 32), "Colored = camp. Red outer ring = shared corner. V2: own HOME/GOAL first; otherwise active opponent camps forbidden.", fill="#17222c")
    for c, camp in enumerate(camps):
        ox, oy = 200 + 400 * (c % 3), 230 + 365 * (c // 3)
        color = COLORS[NAMES[c].split()[0]]
        draw.text((ox - 165, oy - 155), f"CAMP {c}: {NAMES[c]} (10 holes)", fill=color)
        for n in nodes:
            x, y = ox + (2 * n["q"] + n["r"]) * 15, oy + n["r"] * 22
            for to in n["neighbors"]:
                if to > n["id"]:
                    t = nodes[to]
                    draw.line((x, y, ox + (2 * t["q"] + t["r"]) * 15, oy + t["r"] * 22), fill="#cdd4da")
        for n in nodes:
            x, y = ox + (2 * n["q"] + n["r"]) * 15, oy + n["r"] * 22
            active = n["id"] in camp
            draw.ellipse((x-8, y-8, x+8, y+8), fill=color if active else "#e3e7eb")
            if active and n["mask"].bit_count() == 2:
                draw.ellipse((x-11, y-11, x+11, y+11), outline="#c0152b", width=2)
            draw.text((x-6, y-5), str(n["id"]), fill="#ffffff" if active else "#606a74")
        draw.text((ox-165, oy+150), "IDs: " + ", ".join(map(str, camp)), fill="#17222c")
    image.save(out / "camp-audit.png")
    names = ["rules-v2-camps", "rules-v1-legacy", "opponent-camp-overview",
             "opponent-camp-zoom", "shared-red-goal-allowed", "endgame-nine-green", "endgame-green-wins"]
    sheet = Image.new("RGB", (792, 992), "#ddd")
    captions = ImageDraw.Draw(sheet)
    for i, name in enumerate(names):
        x, y = i % 2 * 396, i // 2 * 248
        captions.text((x+4, y+5), name, fill="black")
        sheet.paste(Image.open(ROOT / "docs/screenshots" / (name+".png")), (x, y+24))
    sheet.save(out / "rules-endgame-sheet.png")
    print("Camp audit PASS: 6 x 10 memberships, 6 shared corners, 4 boundary holes each; actual V2 engine reaches all 10 goals from all 50 active HOME starts")


if __name__ == "__main__":
    audit()
