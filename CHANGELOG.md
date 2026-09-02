# Changelog

## 1.0.0 — Production-ready (2026-09-02)

- **Split scoreboards:** YOU vs BOT each have score/wickets/balls/overBalls; only batting team increments; 2 innings x 2 overs, maxWickets=5, chase TARGET/NEED, early win.
- **Bot bowling/batting:** modes YOU BOTH / YOU BAT / YOU BOWL / BOT vs BOT + easy/medium/hard AI (Magnus/swing aware, gap finding).
- **Web 3D Realistic:** Three.js PBR stadium, humans, ball+seam, trail, shadows — `Playable/Cricket3D.html`.
- **Web 2D:** `Playable/CricketPlayable.html` updated with same split-board + bot + innings logic.
- **Native C#:** `Native/CsGame/Program.cs` + `CricketCs.exe` (WinForms GDI+, csc.exe, no SDK).
- **Native C++:** `Native/CppGame/main.cpp` (Win32+GDI+, cl/g++).
- **Production:** CSP + security headers, health.json, nginx + Dockerfile, feature-flags.json, ROLLBACK.md, MONITORING.md, DEPLOYMENT.md, README/BUILD updated; accessibility (aria, keyboard, focus) and 60fps perf hardening.

## 0.1.0 — First generated vertical slice

- Added UE5 C++ project/module.
- Added runtime cricket practice world.
- Added ball physics, bowling, batting, fielding AI, scoring/rules, cameras and HUD.
- Added Unreal automation tests and headless validation.
- Added build, architecture, gameplay and testing documentation.
