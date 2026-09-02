# Gameplay Systems — Phase 1

## Bowling

The bowler releases from a fixed prototype hand offset. Mouse input controls line and length. The bowling component solves a gravity-aware initial vertical velocity so the default ball reaches a chosen bounce point on the pitch.

## Ball physics

The ball uses Unreal rigid-body physics with CCD. While live, a separate component adds:

- aerodynamic drag proportional to speed squared,
- Magnus-style force from angular velocity,
- deterministic lateral swing from the configured swing amount,
- custom pitch bounce response using pre-impact velocity, restitution, tangential retention and spin deviation.

## Batting

A shot creates a short timing window. Contact quality is based on distance from the ball to the ideal bat contact point. Better timing increases exit speed. Shot direction is continuous. Low-quality contact produces deterministic edge deviation from spatial miss distance rather than arbitrary RNG.

## Fielding

Three proxy fielders chase a live ball. When a fielder reaches the ball, the ball is stopped and the delivery completes. This is deliberately basic and should be replaced with anticipation/interception/catch/throw states in Phase 2–3.

## Scoring

The demo tracks runs, wickets, legal balls and overs. Boundaries finalize a delivery as four/six. A stump collision finalizes a wicket. Pressing R starts a short prototype run timer; the run is awarded only if the ball remains live until completion, preventing instant run spamming.

## Known simplifications

- One innings only.
- No strike rotation yet.
- No crease/run-out race yet.
- No wides/no-balls from spatial delivery classification yet.
- No catches or keeper yet.
- No LBW yet.
- Proxy geometry instead of final art/animation.
