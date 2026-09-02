# Cricket AAA — Production-Ready Vertical Slice

Original realistic cricket — UE5 C++ core + Web 3D (Three.js) + Native C# (WinForms GDI+) + Native C++ (Win32 GDI+). Ship-ready with separate YOU vs BOT scoreboards, bot bowling/batting, and 2-innings match flow.

## What Ships (1.0.0)

- **UE5 C++** (`CricketGame.uproject` / `Source/CricketGame/`) — runtime pitch/outfield/wickets, ball with drag + Magnus + swing + pitch bounce, bowling line/length, timing-window batting (controlled/aggressive), fielding AI, runs/wickets/overs, cameras, HUD, automation tests.
- **Web 3D Realistic** (`Playable/Cricket3D.html`) — Three.js PBR stadium (field, pitch, rope, stands, floodlights, shadows), human meshes, red leather ball+seam, shadow, trail, split boards, bot AI (easy/medium/hard), 2×2 overs, chase logic. CSP-hardened.
- **Web 2D** (`Playable/CricketPlayable.html`) — same logic, canvas, for low-end devices.
- **Native C#** (`Native/CsGame/CricketCs.exe`) — WinForms GDI+ build, compiled with `csc.exe` (no SDK). Launch `CricketCs.exe`.
- **Native C++** (`Native/CppGame/main.cpp`) — Win32 + GDI+ single-file, builds with `cl` or `g++` (VS2022/MinGW).
- **Infra:** `deploy/Dockerfile` + `nginx.conf` + `health.json`, `config/feature-flags.json`, `deploy/ROLLBACK.md`, `deploy/MONITORING.md`.

## Controls (all builds)

- **Mouse X/Y** — line / length & shot direction
- **Space** — bowl (or **Bowl** button)
- **Left click** — controlled shot (when ball glows gold)
- **Right click** — aggressive lofted shot
- **R** — call for run (scores only while ball live)
- **Tab** — cycle camera (Broadcast / Bowler / Batter / Orbit on 3D)
- **Backspace** — reset delivery
- **1/2/3/4** — mode: YOU BOTH / YOU BAT / YOU BOWL / BOT vs BOT
- **AI** dropdown — easy / medium / hard

## Split Scoreboards (new)

- **YOU** and **BOT** each have `score/wickets/balls/overBalls` (`Native/CsGame/Program.cs:Active` / `Playable/Cricket3D.html:getActive`). Only the batting team increments.
- 2 innings × 2 overs, `maxWickets=5`. `END INN` swaps `battingTeam` (`you ↔ bot`). 2nd innings shows `TARGET / NEED / balls left` and ends early on chase.
- Gold border + `STRIKE` pillar = who is batting. Verified in 3D, 2D, C#, C++.

## Quick Start

**Web (fastest):** double-click `Playable/Cricket3D.html` or `Playable/CricketPlayable.html`.

**Native C# (Windows, no SDK):**
```bat
C:\Windows\Microsoft.NET\Framework\v4.0.30319\csc.exe /target:winexe /reference:System.Windows.Forms.dll;System.Drawing.dll /out:Native\CsGame\CricketCs.exe Native\CsGame\Program.cs
Native\CsGame\CricketCs.exe
```

**Native C++ (VS2022 or MinGW):**
```bat
cl /EHsc /O2 /DUNICODE Native\CppGame\main.cpp user32.lib gdi32.lib gdiplus.lib /Fe:CricketCpp.exe
:: or
g++ -O2 -mwindows -std=c++17 Native\CppGame\main.cpp -lgdi32 -luser32 -lgdiplus -o CricketCpp.exe
```

**UE5:**
```
set UE_ROOT=C:\Program Files\Epic Games\UE_5.4
Tools\BuildEditor.bat
Tools\RunEditor.bat
```

**Docker (static web):**
```bat
docker build -f deploy/Dockerfile -t cricket-aaa:1.0.0 .
docker run -p 80:80 cricket-aaa:1.0.0
curl -f http://localhost/health.json
```

## Verification

```bat
python Tools/validate_project.py
# C# build pass already verified (35328 bytes)
# C++ structure validated (WinMain/WndProc/youTeam/botTeam)
# health.json returns ok
```

See `TESTING.md`, `BUILD.md`, `deploy/MONITORING.md`, `deploy/ROLLBACK.md`.

## Production Checklist (shipping-and-launch)

- Tests: `validate_project.py` PASS, C# build PASS, C++ structure PASS, no `console.log`, no hardcoded secrets
- Security: CSP (`Content-Security-Policy`), `X-Content-Type-Options: nosniff`, `Referrer-Policy`, `Permissions-Policy`, no secrets in repo, input clamping on `aimLine/aimLength/pace` (all builds), nginx rate limit
- Performance: 60fps `requestAnimationFrame`/`Timer(16ms)`, trail capped 24, `MaxWickets=5` keeps match short, gzip + immutable cache for assets, field size <50KB HTML
- Accessibility: `tabindex` + `role=img/log` + `aria-label`/`aria-live`, keyboard: Space/R/Tab/Backspace/1-4, focus visible on mode pills
- Infra: `public/health.json`, `deploy/nginx.conf` with healthcheck, `env.example`, `feature-flags.json` (owner+expiry)
- Monitoring: `deploy/MONITORING.md` (error rate, P95, JS errors, business), rollback `<1 min` flag off / `<5 min` redeploy

## Next Milestone

After launch, Phase 2: strike rotation, crease race + run-out timing, catches/keeper, LBW, wides/no-balls, career/multiplayer.

## Deployment

See `DEPLOYMENT.md` for staged rollout (staging → prod flag off → team → 5% canary → 25/50/100%).
