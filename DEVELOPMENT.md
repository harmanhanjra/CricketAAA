# Development Loop

For each feature:

1. State the gameplay requirement.
2. Identify the owning module/class.
3. Implement the smallest testable version.
4. Compile the editor target.
5. Run automation tests.
6. Test in PIE.
7. Fix root causes rather than retrying blindly.
8. Refactor only after behavior is correct.
9. Profile before optimizing.
10. Update docs and commit.

Suggested first commits:

- `feat: add first playable cricket runtime world`
- `feat: implement cricket ball aerodynamics and pitch bounce`
- `feat: add batting timing and deterministic contact`
- `feat: implement scoring rules and delivery lifecycle`
- `feat: add basic fielding ai and score hud`
