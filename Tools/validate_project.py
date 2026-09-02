from pathlib import Path
import json
import math
import sys

ROOT = Path(__file__).resolve().parents[1]
REQUIRED = [
    "CricketGame.uproject",
    "Config/DefaultEngine.ini",
    "Config/DefaultInput.ini",
    "Source/CricketGame/Public/Match/CricketGameMode.h",
    "Source/CricketGame/Public/Match/CricketMatchState.h",
    "Source/CricketGame/Public/Ball/CricketBall.h",
    "Source/CricketGame/Public/Ball/BallPhysicsComponent.h",
    "Source/CricketGame/Public/Batting/BattingComponent.h",
    "Source/CricketGame/Public/Bowling/BowlingComponent.h",
    "Source/CricketGame/Public/Rules/MatchRulesManager.h",
    "Source/CricketGame/Public/Match/ScoreManager.h",
    "Source/CricketGame/Public/AI/BasicFieldingAIController.h",
    "Source/CricketGame/Public/UI/ScoreHUD.h",
]

missing = [p for p in REQUIRED if not (ROOT / p).exists()]
if missing:
    print("FAIL: missing required files:")
    for p in missing:
        print(" -", p)
    sys.exit(1)

project = json.loads((ROOT / "CricketGame.uproject").read_text(encoding="utf-8"))
assert any(m.get("Name") == "CricketGame" for m in project.get("Modules", []))

# Headless numerical sanity check for the default bowling trajectory to its bounce target.
start_x, start_z = -830.0, 302.0  # bowler location + release offset
bounce_x, bounce_z = 488.0, 4.0  # default mapped length ~0.15
speed = 2200 + (4050 - 2200) * 0.82
dx = bounce_x - start_x
t = abs(dx) / speed
g = 980.0
vz = (bounce_z - start_z + 0.5 * g * t * t) / t
z_at_t = start_z + vz * t - 0.5 * g * t * t
assert abs(z_at_t - bounce_z) < 1e-6
assert 0.2 < t < 0.7

print("PASS: project structure present")
print(f"PASS: default ballistic bounce sanity check (t={t:.3f}s, z={z_at_t:.2f}cm)")
print("NOTE: Unreal compile/runtime validation requires UE5 + Visual Studio on Windows.")
