# Rollback Plan — Cricket AAA 1.0.0

## Trigger Conditions (shipping-and-launch thresholds)
- Error rate >2x baseline
- P95 latency >50% above baseline
- New JS error types >0.1% sessions
- Health check fails (health.json != ok)
- User reports of broken input/physics

## Rollback Steps

### Static web (2D/3D) — <1 min
1. `git revert <commit>` or `git checkout <prev-tag>`
2. `docker build -t cricket-aaa:rollback -f deploy/Dockerfile . && docker run -d -p 80:80 cricket-aaa:rollback`
3. Verify: `curl -f http://localhost/health.json` → status ok
4. Purge CDN: `cdn purge /playable/*`
5. Announce in #game-ops

### Native C# / C++ — <5 min
1. Re-publish previous exe from `releases/` tag
2. Verify: launch `Native/CsGame/CricketCs.exe` smoke test (scoreboard renders)
3. For C++: `Native/CppGame/CricketCpp.exe` smoke test

### Database (none for static; if added)
- No migrations in 1.0.0 — no DB rollback needed. Future migrations must ship with `migrate rollback` script.

## Time to Rollback
- Feature flag off: <1 min (flip `config/feature-flags.json` and redeploy)
- Redeploy previous image: <5 min
- Full CDN purge: <2 min

## Verification After Rollback
- [ ] health.json returns 200
- [ ] Playable/Cricket3D.html loads and ball physics ticks
- [ ] No new errors in monitoring
