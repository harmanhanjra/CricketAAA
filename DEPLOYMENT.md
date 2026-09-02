# Deployment — Cricket AAA 1.0.0

## Staged Rollout (per shipping-and-launch)

1. **Staging** — `docker build -f deploy/Dockerfile -t cricket-aaa:staging .` + `python Tools/validate_project.py` + manual smoke (bowl/bat/run/innings)
2. **Prod deploy flag OFF** — push image, `health.json` ok, error rate baseline
3. **Team** (flag ON for internal) — 24h window, watch `deploy/MONITORING.md`
4. **Canary 5%** — CDN split or nginx `split_clients`; thresholds: error rate within 10%, P95 within 20%
5. **Gradual 25% → 50% → 100%** — same checks; rollback on >2x error or >50% P95
6. **Full + cleanup** — monitor 1 week, remove flag per `config/feature-flags.json` expiry 2026-12-01

## Rollback

See `deploy/ROLLBACK.md`: flag off <1 min, redeploy <5 min, CDN purge <2 min.

## Feature Flags

`config/feature-flags.json` — every flag has `owner` + `expires`, no nesting, both states tested.

## Native Releases

- Tag `v1.0.0` with `Native/CsGame/CricketCs.exe` and `Native/CppGame/CricketCpp.exe` (built on Windows runner).
- No auto-update; user downloads exe.

## Verification After Deploy

```
curl -f https://prod/health.json
# expect {"status":"ok","version":"1.0.0"}
# open /playable/Cricket3D.html — bowl, bat, run, innings switch, BOT board separate
# check logs: no new JS error types
```
