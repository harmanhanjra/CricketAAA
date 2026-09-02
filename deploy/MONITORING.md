# Monitoring & Observability — Cricket AAA

## Health
- Endpoint: `/health.json` (static file, also served via nginx). Checks: cricket3d, cricket2d, csharp_native, cpp_native.
- Synthetic check: curl every 30s (Docker HEALTHCHECK).

## Client Instrumentation (web)
Add to both HTMLs before `</head>` (already CSP-safe):
```js
window.addEventListener('error', e=> navigator.sendBeacon('/api/log', JSON.stringify({type:'js_error', msg:e.message, stack:e.error?.stack, url:location.href})));
window.addEventListener('unhandledrejection', e=> navigator.sendBeacon('/api/log', JSON.stringify({type:'promise_rejection', reason:String(e.reason)})));
// Perf
new PerformanceObserver(l=> l.getEntries().forEach(e=> navigator.sendBeacon('/api/perf', JSON.stringify({lcp:e.startTime})))).observe({type:'largest-contentful-paint', buffered:true});
```

## Dashboards (to create in Grafana/Datadog)
- Error rate (total + by page: Cricket3D vs CricketPlayable)
- P50/P95 latency for health.json and playable assets
- JS error rate by type
- Active sessions
- Business: games started, innings completed, avg score

## Alert Thresholds (per shipping-and-launch)
| Metric | Advance | Hold | Rollback |
|--------|---------|------|----------|
| Error rate | within 10% | +10-100% | >2x |
| P95 latency | within 20% | +20-50% | >50% |
| New JS errors | none | <0.1% sessions | >0.1% |

## Post-Launch First Hour
1. `curl -f https://prod/health.json`
2. Check error dashboard — no new types
3. Manual smoke: bowl, bat, run, wicket, innings switch
4. Verify logs flowing
