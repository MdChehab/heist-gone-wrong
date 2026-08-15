# SESSION LOG - Heist Gone Wrong

A dated entry per working session: what changed, what's broken, what's next. Newest
entries go at the bottom. See CLAUDE.md for the required entry format and for the
start-of-session / end-of-session protocol. Read the last 2-3 entries at the start of
each session.

---

## [2026-07-11] Session 1 - Version control setup (Git, LFS, GitHub)
**Cycle/Week:** W1 (Setup)
**Linear issues touched:** (none recorded)

### Done
- Confirmed prerequisites: git 2.47.1, git-lfs 3.6.1. GitHub CLI (`gh`) is NOT installed.
- Initialized a git repo in the project root (the folder with `Heist_Gone_Wrong.uproject`);
  renamed the default branch `master` -> `main`.
- Reviewed the pre-existing `.gitignore` and `.gitattributes` (did not rewrite them, per
  CLAUDE.md). Both cover the required cases correctly.
- Ran `git lfs install`.
- Created this file and `Docs/DECISIONS.md`, seeding DECISIONS.md from the scope/architecture
  decisions in CLAUDE.md plus this session's version-control decisions.
- First commit made as MdChehab (no AI co-author): 854 files, of which 753 are LFS pointers.
  Verified `git lfs ls-files` shows all 753 assets as LFS, none in git history as raw blobs.
- Developer created the GitHub repo `heist-gone-wrong` by hand (gh not installed) and chose to
  keep it PUBLIC rather than private. No secrets/credentials are in the history (source, config
  .ini, assets only), so public is acceptable. Added HTTPS remote `origin` and pushed `main`;
  all 753 LFS objects uploaded (141 MB). `git lfs fsck` OK. Remote: https://github.com/MdChehab/heist-gone-wrong

### Decisions made
- Recorded three durable version-control decisions in DECISIONS.md: LFS without file locking
  (solo project), trial assets sandboxed out of the repo (`Content/Developers/`, `RawContent/`),
  and AI tooling excluded from the repo (`CLAUDE.md`, `.claude/`, etc.). See DECISIONS.md for
  reasoning and rejected alternatives.
- Renamed the default branch to `main` to match the intended GitHub default and avoid a
  default-branch mismatch on push.

### Current state
- Local repo exists on branch `main` with LFS installed. Nothing committed yet.
- `.gitignore` / `.gitattributes` verified and unchanged.

### Known issues / gotchas
- The user's entire home folder `C:\Users\cheha` is itself a git repo (a `.git` lives there).
  The new project repo is nested inside it. Git treats the nested repo independently, so the
  project repo works fine, but the home-level repo is almost certainly unintended and worth
  cleaning up separately. Flagged to the developer.
- `gh` is not installed, so the GitHub repo must be created manually via the web UI, then the
  remote added and pushed by hand. Commands provided to the developer.
- `.gitignore` does not ignore `*.slnx` (only `*.sln`); the two `.slnx` solution files in the
  root would be committed. Flagged, not changed (config files are developer-owned).

### Next steps
- Version control setup is COMPLETE. Repo is on GitHub (public) with LFS working.
- Begin W1 gameplay work: the C++ Character class (walk/run in all directions, crouch) and the
  walkable graybox museum level.
- Optional cleanup the developer may want later: the template ships three unused variants
  (Variant_Combat, Variant_Platforming, Variant_SideScrolling) under Source/ and Content/ - trim
  the ones not needed for the stealth game to reduce noise. Not urgent.

### Editor-side steps still needed from me
- None for this session (version-control setup only).

---

## [2026-07-11] Session 2 - Player movement (walk/run/crouch) + graybox plan
**Cycle/Week:** W1 (Setup)
**Linear issues touched:** (none recorded)
_Continued directly from Session 1 the same day._

### Done
- Extended the template character `AHeist_Gone_WrongCharacter` (Source/Heist_Gone_Wrong/
  Heist_Gone_WrongCharacter.h/.cpp) with three tunable ground-speed tiers and crouch:
  - `WalkSpeed` (400), `RunSpeed` (650), `CrouchSpeed` (200), all `EditAnywhere` under
    category "Movement|Stealth".
  - Run = hold input (DoStartRun/DoStopRun swap MaxWalkSpeed). Crouch = toggle
    (DoToggleCrouch uses engine Crouch()/UnCrouch(); MaxWalkSpeedCrouched caps crouch speed).
  - `bCanCrouch` enabled in the constructor; speeds re-applied in BeginPlay so Blueprint
    overrides take effect. `bIsRunning` bool exposed BlueprintReadOnly for the anim BP.
  - Added `RunAction` / `CrouchAction` UInputAction slots; bound in SetupPlayerInputComponent.
- Editor wiring (done by developer, committed): created IA_Run + IA_Crouch, mapped them in
  IMC_Default (Run=Left Shift, Crouch=Left Ctrl), assigned them on BP_ThirdPersonCharacter.
- Movement play-tested by developer: walk/run/crouch confirmed working as intended.
- Committed in two commits (9aff884 code + DECISIONS, b21a056 editor input assets) and pushed.
- Recorded movement decisions in DECISIONS.md (Player section): three speed tiers; run=hold,
  crouch=toggle - with reasoning and rejected alternatives.
- Designed the graybox museum layout (floorplan delivered in chat, not yet built): south
  Entrance Lobby (player start + exit) -> central Main Gallery Hall (guard patrol loop +
  cover plinths) -> west Security Office (switch) -> north Artifact Vault (behind a gated
  door). Loop forces two guard-space crossings out and one tense crossing back with the artifact.

### Decisions made
- Run = hold, Crouch = toggle (see DECISIONS.md for full reasoning / rejected options).
- Three speed tiers walk/run/crouch as the stealth risk dial.
- Kept the template's Jump binding for now; it will be removed once the W2 roll replaces it.
  Decision: leave it, it costs nothing and gives vertical testing until roll lands.
- Graybox exit placed near the entrance (loop-back escape) rather than a far-side one-way exit,
  to create the tense return trip with the artifact and reuse geometry. Not yet built, so
  revisit if it doesn't feel right in-level.

### Current state
- Player character has full third-person movement: walk/run in all directions + crouch, all
  tunable in the editor. Working and tested.
- Graybox museum level does NOT exist yet - only the layout plan. The level is the next task.

### Known issues / gotchas
- Crouch shrinks the capsule and slows movement but there is likely NO crouch animation on the
  mannequin ABP yet, so the character keeps the standing pose while crouched. Cosmetic only;
  crouch anim is a W3 animation-states task.
- Running while crouched has no visible effect (MaxWalkSpeedCrouched caps it) - intended.
- Source files are CRLF; the Edit tool's multi-line matches fail on them, so those two files
  were rewritten wholesale with Write. Harmless (git normalizes via `* text=auto`), just noting
  it so a fresh agent doesn't fight partial edits.

### Next steps
- Build the graybox museum level per the delivered floorplan: L_Museum in Content/Heist/Levels/,
  4 rooms from LevelPrototyping cubes (wall height ~400cm, doors ~150x230cm, Gallery ~1600x1000),
  PlayerStart in the Lobby, NavMeshBoundsVolume over the floor + Build Paths, GameMode Override =
  BP_ThirdPersonGameMode. Optionally drop placeholder cubes at gate/switch/artifact/exit spots.
  Then commit the .umap (LFS).
- After the graybox: begin W2 (roll, interaction system, throw mechanic).

### Editor-side steps still needed from me
- Build and save the graybox level (checklist above), then hand it back for the .umap commit.
- (Optional, offered but deferred) let Claude stub the placeholder C++ actor classes
  (AHeistSwitch/AHeistDoor/etc.) when W5 starts, not now.

---

## [2026-07-21] Session 3 - W2 player mechanics: interaction, throw, roll
**Cycle/Week:** W2 (Player mechanics, due Jul 19)
**Linear issues touched:** (none recorded)

### Done
- Built the graybox museum level `Content/Heist/Levels/L_Museum.umap` (developer, in editor)
  using modeling-tool geometry; committed with its `_GENERATED` meshes.
- Interaction system, `Source/Heist_Gone_Wrong/Interaction/`:
  - `IHeistInteractable` - BlueprintNativeEvent contract: GetInteractionPrompt / CanInteract /
    Interact. Implemented by throwables now, switch/door/artifact later.
  - `UHeistInteractionComponent` - finds the nearest interactable on a 0.15s timer (never Tick),
    broadcasts `OnFocusChanged` for the future HUD prompt, `TryInteract()` re-validates before firing.
- Throwing:
  - `AHeistThrowable` - physics pickup that reports a `UAISense_Hearing` noise event on impact,
    so W4 guard investigation needs no changes to this class.
  - `UHeistThrowComponent` - carries one object, charged throw (hold to wind up, release to launch),
    speed lerped MinThrowSpeed 250 -> MaxThrowSpeed 1700 over MaxChargeTime 1.1s.
    `GetChargeRatio()` is ready for a W6 HUD power meter.
- Roll on the character: launches along current velocity (facing when idle), with a lockout.
- Editor wiring (developer): IA_Interact / IA_Throw / IA_Roll mapped in IMC_Default (throw is on
  left mouse), `BP_Throwable` created and placed in the Gallery, carry socket added to the
  mannequin skeleton, actions assigned on BP_ThirdPersonCharacter.
- All three mechanics play-tested and confirmed working by the developer.
- DECISIONS.md updated with the Interaction and Throwing sections.

### Decisions made
- Interaction uses a proximity sphere, NOT a camera-forward trace. See DECISIONS.md; the trace
  approach was implemented first and had to be replaced.
- Charged throw instead of fixed-speed, which covers the "different noise ranges" stretch goal
  with one throwable type.
- Character turns to face the throw on release.
- Jump binding is KEPT. The earlier plan to remove it once the roll proved out was reversed by
  the developer: keep the option open and revisit at the end of the project. See DECISIONS.md.

### Current state
- Player has: walk/run/crouch, roll, pick up and charged-throw objects. All working.
- Throwables report AI hearing noise on impact. Nothing listens yet - that is W4, by design.
- Graybox museum level exists and is walkable.

### Known issues / gotchas
- No animations for crouch, roll or throw. The character slides through the roll and keeps a
  standing pose while crouched. Cosmetic; W3 animation states.
- `bDebugThrow` on the throw component draws the aim line and logs throw vectors. Off by default,
  kept because it made both throw bugs findable in one pass.
- IMPORTANT for future sessions: Live Coding (Ctrl+Alt+F11) CANNOT apply new UPROPERTY members or
  changed function signatures - it silently keeps the old code, which cost real debugging time
  this session. For those changes the editor must be CLOSED and a full build run.
- Enhanced Input gotcha: a `Pressed` trigger on IA_Throw would fire Started and Completed on the
  same frame and break charging. Leave its trigger list empty.

### Bugs found in testing, and their causes (report material)
- Throw flew straight up. `ControlRotation` is unnormalized: looking slightly down reads as pitch
  350, so `Clamp(350 + 10, -89, 89)` pinned it to 89 = vertical. Fixed by normalizing first.
  Found by logging intermediate values; `pitch=89.0` sitting exactly on the clamp boundary gave it away.
- Throw scattered randomly. Physics was re-enabled while the object still sat inside the
  character's capsule, so the solver ejected it; and `AddImpulse` was adding to the velocity the
  animating hand bone had imparted. Fixed with a muzzle launch point and setting velocity outright.
- Pickup failed when standing on an object. A sphere sweep forward from eye height passes over
  objects on the floor. Replaced with proximity search.
- Roll was a stutter. `GroundFriction`, `BrakingDecelerationWalking` and the `MaxWalkSpeed` clamp
  cancel a ground `LaunchCharacter` within a few frames. Suspended for the roll, restored after.
- Pattern worth noting in the report: in three of four cases the gameplay code was correct and the
  bug was an interaction with an engine default or convention.

### Next steps
- W3 (Guard AI part 1): NavMesh, guard pawn + AIController, waypoint patrol, animation states.
- (Jump removal is no longer a next step - see DECISIONS.md, jump is kept.)

### Editor-side steps still needed from me
- None outstanding. NavMesh on L_Museum was confirmed working by the developer (built during the
  W1 level pass), so W3 guard pathing is unblocked.

---

## [2026-07-21] Session 4 - W3 guard AI part 1: waypoint patrol
**Cycle/Week:** W3 (Guard AI part 1, due Jul 26)
**Linear issues touched:** (none recorded)

### Done
- Guard AI in `Source/Heist_Gone_Wrong/AI/`:
  - `AHeistGuardCharacter` - guard body. Holds `PatrolPoints` (EditInstanceOnly), `WaitTimeAtPoint`,
    `bLoopPatrol`, `PatrolAcceptanceRadius`, `PatrolSpeed`. Auto-possesses its controller,
    orients to movement. Re-applies PatrolSpeed in BeginPlay for BP tuning.
  - `AHeistGuardController` - C++ state machine (`EGuardState` Patrol/Investigate/Alerted; only
    Patrol implemented). OnPossess -> StartPatrol -> MoveTo waypoint -> OnMoveCompleted -> timer
    wait -> AdvancePatrol. No Tick. Null waypoints skipped, failed moves deferred via timer so
    they cannot infinite-loop.
- Editor (developer): BP_GuardCharacter (mesh SKM_*_Simple, mesh Z -90 / yaw -90, Anim Class
  ABP_Unarmed), TargetPoints placed around the Gallery, guard placed and PatrolPoints assigned.
- Fixed guard locomotion animation (see below). Guard now patrols AND animates. Confirmed by developer.
- Committed guard code + BP_GuardCharacter + L_Museum + the edited ABP_Unarmed (commit 0fd866b).
- Recorded three Guard AI decisions in DECISIONS.md.

### Decisions made
- C++ state machine in the AIController, not Behavior Tree / StateTree (diffable, C++-first).
- Patrol via placed TargetPoint actors in an EditInstanceOnly array on the guard.
- Guard walk animation gated on ground speed alone (see gotcha). Full reasoning in DECISIONS.md.

### Current state
- Guard patrols its waypoint loop, pauses ~2s per point, animates walk/idle, turns through corners.
- Vision / detection / investigate / alerted are NOT built - that is W4.

### Known issues / gotchas
- ANIMATION ROOT CAUSE (worth the design report): the shared ABP_Unarmed gated Idle->Walk on
  ground speed AND non-zero acceleration ("input applied"). A player holds a key so acceleration
  is continuous; an AI moved by nav requested-velocity has zero acceleration at cruising speed,
  so the guard never left Idle while sliding. Fix: ShouldMove now gates on speed only. Editing
  the shared ABP is intentional - one asset, both characters, player unaffected. The developer's
  own hypothesis (input vs speed) led to the fix.
- ABP_Unarmed is now a modified shared asset. If it is ever re-imported/reset from the template,
  the ShouldMove edit must be re-applied or the guard reverts to sliding in idle.
- Guard pauses at waypoints without a look-around beat yet; that only becomes meaningful with the
  W4 vision cone, so it was left minimal.

### Next steps
- W4 (Guard AI part 2, due Aug 2): AI Perception (sight) vision cone + line-of-sight, detection
  meter, and Investigate state reacting to the thrown-object noise events the throwable already
  reports. The Investigate/Alerted enum states and OnMoveCompleted branch are already stubbed.

### Editor-side steps still needed from me
- None outstanding for W3.

---

## [2026-07-21] Session 5 - W4 guard AI part 2: vision, detection, investigate
**Cycle/Week:** W4 (Guard AI part 2, due Aug 2)
**Linear issues touched:** (none recorded)

### Done
- AI Perception on `AHeistGuardController`: Sight (cone + line-of-sight) and Hearing configs,
  event-driven via OnTargetPerceptionUpdated. Tuning exposed (SightRadius, LoseSightRadius,
  vision half-angle, HearingRange).
- `UHeistDetectionSubsystem` (world subsystem): global detection meter, fills while any guard
  sees the player, drains otherwise, fires OnPlayerDetected at 100% (W5 fail hook). On-screen
  debug % readout.
- State machine completed: Patrol -> Alerted (sees player: stop, face, meter fills) -> Investigate
  (heard noise OR lost sight: walk to point, widen cone to survey, resume patrol).
- Player footstep noise (`Heist_Gone_WrongCharacter`): running emits an AI hearing noise on a
  0.35s timer (RunNoiseRange); walking and crouch-moving are silent. Same hearing sense as throwables.
- Many playtest-driven fixes (all committed 4535bdc guard AI, 9f3cd43 footstep):
  - Sight cone was world-fixed -> re-enabled the controller's default tick (drives control rotation).
  - Guard now faces the player when alerted (swaps orient-to-movement for controller-yaw).
  - Investigate look-around: switched from body-rotating scan to WIDENING the cone (no slide,
    no gaps, forward-compatible with a future investigation animation).
  - Guard overshoots a couple steps past a player's last-known point so it enters a room instead
    of stopping in the doorway. Overshoot is player-hunt only, not noise.
  - Priority: Alerted > player-hunt > noise > patrol. A thrown object can't pull a guard off a hunt.
  - Noise investigations linger longer (NoiseInvestigateTime ~7s) than player hunts (~3s) so the
    distraction buys real time.
- Confirmed by playtest: vision, LoS, detection meter, hearing, investigate/survey, distraction
  with correct priority + linger, footstep run/walk/crouch, and crouch-breaks-LoS all work.

### Decisions made
- AI Perception over a manual cone; detection meter as a world subsystem (supersedes the
  "detection component" wording in CLAUDE.md); detection instant not ramped (deferred). See DECISIONS.md.
- Investigate uses widen-cone, not rotate-to-scan (developer's idea, better than the first pass).
- Footstep noise: running loud, walk/crouch silent.

### Current state
- The full guard stealth loop works end to end except the fail/restart on full detection (W5).
- Crouch-breaks-line-of-sight confirmed: AI sight traces to the crouch-lowered body point.

### Known issues / gotchas
- DEBUG IS ON: guard investigate spheres (cyan=noise, orange=lost-sight), the detection %
  readout (bDebugMeter/bDebugGuard default true), and a green sphere on every throwable impact
  (unconditional in non-shipping). Turn these off before the W6 packaged build.
- No animations yet for crouch, roll, throw, guard turn, or guard investigate. The guard turning
  to face the player (alerted) and any body turn slides. All deferred to one animation pass.
- Crouch depth is the engine default CrouchedHalfHeight (40 => ~0.8m, quite low/crawl-like). When
  the crouch animation is added, tune CrouchedHalfHeight + the anim pose + level cover heights
  together so hiding reads naturally.
- Throwable noise fires at first physics impact, which can differ slightly from where the object
  visually comes to rest. Cosmetic; refine if it matters.

### Next steps
- W5 (Stealth loop + win condition, due Aug 9): fail/restart from checkpoint on full detection
  (hook UHeistDetectionSubsystem::OnPlayerDetected), guard reset, switch-and-door puzzle, artifact
  pickup, exit trigger. Full loop end to end.
- The W5 stealth-actor placeholders (switch/door/artifact/exit) can be stubbed as C++ now.

### Editor-side steps still needed from me
- None outstanding for W4. (W5 will need editor placement of switch/door/artifact/exit actors.)
