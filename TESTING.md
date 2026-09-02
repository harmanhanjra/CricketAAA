# Testing

## 1. Headless validation available now

From the project root:

```bash
python Tools/validate_project.py
```

This checks required files, the `.uproject` module declaration, and a deterministic default bowling trajectory sanity calculation.

## 2. Unreal compile validation required on Windows

Build `CricketGameEditor` in Visual Studio. Treat any compile error as blocking; do not mark the vertical slice complete until the editor target compiles.

## 3. Unreal automation tests

In Unreal Editor, open **Tools → Test Automation** and run:

- `CricketGame.Rules.ScoreOvers`
- `CricketGame.Rules.DeliveryLifecycle`

## 4. Manual vertical-slice acceptance

Verify in PIE:

1. Score HUD appears.
2. Space launches a delivery.
3. Mouse changes line/length.
4. LMB/RMB can contact the ball near the batter.
5. Bat contact sends the ball according to timing/direction.
6. Ball can hit stumps for a wicket.
7. Fielders chase and stop the live ball.
8. R starts a run and awards it only if the ball remains live through the run timer.
9. A completed delivery increments the ball count exactly once.
10. Six legal balls display as `1.0` overs.
11. Tab cycles three camera positions.
12. Match completes after configured overs or ten wickets.

## 5. Physics checks

Enable UE collision visualization and verify the ball does not tunnel through the pitch/stumps at high pace. Keep physics substepping enabled.
