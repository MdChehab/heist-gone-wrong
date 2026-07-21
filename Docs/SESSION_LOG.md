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
- Jump binding kept for now; agreed to remove it once the roll had proven out, which it now has,
  so this is outstanding.

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
- Remove the jump binding now that the roll is proven.

### Editor-side steps still needed from me
- None outstanding. NavMesh on L_Museum was confirmed working by the developer (built during the
  W1 level pass), so W3 guard pathing is unblocked.
