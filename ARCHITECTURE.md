# Technical Architecture

## Runtime ownership

- `ACricketGameMode` — orchestrates the vertical slice world and delivery lifecycle.
- `ACricketMatchState` — owns match phase plus `UScoreManager` and `UMatchRulesManager`.
- `UScoreManager` — score/wicket/legal-ball data and over notation.
- `UMatchRulesManager` — legal delivery lifecycle, completed runs, boundary/wicket finalization.
- `ACricketBall` — physical ball actor and delivery event source.
- `UBallPhysicsComponent` — aerodynamic forces, spin/swing parameters and pitch bounce response.
- `UBowlingComponent` — maps player line/length/pace input to a ballistic release.
- `UBattingComponent` — queues a timing-window shot and computes deterministic contact response.
- `ABasicFieldingAIController` — chases and fields a live ball without requiring a navmesh for the prototype.
- `ACricketGameplayCamera` — switchable prototype camera modes.
- `AScoreHUD` — lightweight score/phase overlay.

## Design rules

1. Physics is not tied to frame rate; UE physics substepping is enabled.
2. Bat outcomes are computed from distance-to-contact timing and ball position rather than random hit tables.
3. Rules/scoring are separated from rendering and characters.
4. Tunable gameplay parameters live as `UPROPERTY` values rather than scattered magic constants where practical.
5. The first vertical slice uses Engine basic meshes only, so it has no proprietary external art dependency.

## Planned extension points

- Replace proxy character visuals with licensed/original skeletal meshes and Animation Blueprints.
- Add delivery data assets for pace/spin variations.
- Add pitch surface data assets for green/dry/dusty/hard/soft/cracked behavior.
- Add shot data assets for full shot library.
- Add replicated authoritative match/ball state before multiplayer.
