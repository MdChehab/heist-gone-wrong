# DECISIONS - Heist Gone Wrong

The durable "why" for this project. Architecture, design decisions, trade-offs, and
rejected options. Organized by system, not chronologically. Each entry records the
decision, the reasoning, the alternatives rejected, and the date. Change this only
when a decision actually changes. See CLAUDE.md for how this file is used.

---

## Scope

### One small, polished, complete gameplay loop over a large feature set - 2026-07-11
**Decision:** Build a single, complete stealth loop (infiltrate, avoid a guard, distract,
solve a switch-and-door puzzle, steal the artifact, escape) and polish it, rather than a
broad feature set.
**Reasoning:** This is an 8-week, 1-credit solo honors project (Jun 29 - Aug 14, 2026).
A finished, coherent loop demonstrates more and de-risks the graded deliverables (build,
report, presentation) better than a pile of half-working systems.
**Rejected:** Wider feature sets (multiple floors, several puzzle types, camera systems,
coordinated multi-guard AI). Explicitly out of scope; see the Out of Scope list in CLAUDE.md.

### No combat - 2026-07-11
**Decision:** The game has no combat of any kind. The player relies on stealth, movement,
and distraction only.
**Reasoning:** Combat would add animation, balancing, and AI cost that competes directly
with the stealth loop that is the actual deliverable. Distraction-based tension is cheaper
to build and more on-theme.
**Rejected:** Any takedown/attack/weapon mechanic.

### Single floor only - 2026-07-11
**Decision:** One compact, single-floor museum level (graybox plus existing asset packs).
**Reasoning:** Multiple floors multiply nav, streaming, and level-design work without
adding to the core loop. One well-tuned floor is enough to show the loop.
**Rejected:** Multi-floor level; separate stretch-goal floors.

### Throw-to-distract as the core stealth verb - 2026-07-11
**Decision:** The player's main tool against guards is picking up and throwing objects to
make noise and lure guards away. Guards investigate the noise, look around, then return to
patrol.
**Reasoning:** A single expressive verb gives the stealth loop its decisions without the
cost of a large gadget system. It pairs directly with the one puzzle and the detection meter.
**Rejected:** Multiple gadget types, keycards, camera-disable tools (some kept as stretch
goals only if clearly ahead of schedule).

---

## Player

### C++-first architecture, thin Blueprints on top - 2026-07-11
**Decision:** Implement gameplay systems as C++ classes that expose `UPROPERTY`
(EditAnywhere/BlueprintReadWrite) and `UFUNCTION(BlueprintCallable)` members. Blueprints
are used only for wiring visuals and tuning values, kept thin.
**Reasoning:** Keeps logic version-controllable, diffable, and reviewable in the repo
(Blueprints are opaque binary `.uasset`). Plays to existing C++/UE strength. Editor-side
tuning stays fast because values are exposed to Blueprint/editor.
**Rejected:** Blueprint-first implementation. Faster to prototype but produces binary,
un-diffable logic that is hard to review, hard to keep DRY, and painful to merge - a poor
fit for a project whose grade depends on a documented, analyzable design.

### Three movement speed tiers: crouch / walk / run - 2026-07-11
**Decision:** The player has three ground-speed tiers, all exposed as `EditAnywhere` on the
character for in-editor tuning: Walk (default, 400), Run (650), Crouch (200). Implemented by
extending the template's `AHeist_Gone_WrongCharacter` rather than adding a parallel class.
**Reasoning:** These three tiers are the stealth risk/reward dial (crouch = slow but quiet,
run = fast but loud). Extending the existing template character reuses its camera, movement,
look, and input wiring instead of duplicating them (DRY). Crouch uses UE's built-in
`Crouch()`/`MaxWalkSpeedCrouched`, so the engine handles the capsule resize and speed cap.
**Rejected:** A separate new character class (would duplicate the template's setup); a single
fixed speed (removes the core stealth trade-off).

### Run = hold, Crouch = toggle - 2026-07-11
**Decision:** Run is a hold input (hold to run, release to return to walk). Crouch is a toggle
(press to crouch, press again to stand).
**Reasoning:** Hold-to-run matches stealth-genre convention and can never get stuck "on," so
the player is at the safe/quiet default whenever they are not actively holding run. Toggle
crouch lets the player hold a crouched position through a long sneak without keeping a key
pressed, reducing finger strain and freeing that hand for look/throw.
**Rejected:** Toggle run (risk of leaving sprint on and getting spotted); hold crouch
(tiring over long stealth sections). Both are one-line changes if playtesting says otherwise;
revisit after the first playable stealth pass.

---

## Guard AI

_No decisions recorded yet. Planned: NavMesh patrol, event-driven AI Perception (vision
cone + line-of-sight), a Patrol / Investigate / Alerted state machine. See W3-W4 in CLAUDE.md._

---

## Detection

_No decisions recorded yet. Planned: a detection meter that fills while the player is in a
guard's sight; full meter = fail + restart from checkpoint. Detection/vision logic to run on
a ~0.1-0.2s timer via AI Perception, not per-frame Tick._

---

## Interaction

_No decisions recorded yet. Planned: an interaction interface plus a throwable/pickup actor;
object pooling for throwables to avoid spawn/destroy churn._

---

## Level

_No decisions recorded yet. Planned: one single-floor graybox museum built from the existing
asset packs._

---

## UI

_No decisions recorded yet. Planned: minimal HUD - detection meter, interaction prompts,
win/lose screens._

---

## Build

_No decisions recorded yet. Target: packaged Windows build._

---

## Version Control / Repo

### Git LFS without file locking - 2026-07-11
**Decision:** Route Unreal binary assets (`.uasset`, `.umap`) and raw binary types
(models, textures, audio, video, fonts) through Git LFS, but do NOT enable LFS file locking
(`lockable`) in `.gitattributes`.
**Reasoning:** LFS keeps large binaries out of git history so clones and diffs stay fast.
Locking exists to prevent conflicting edits to un-mergeable binary assets, but that only
matters with multiple developers. On a solo project, locking would only make asset files
read-only for no benefit.
**Rejected:** Locking on `*.uasset`/`*.umap`. Revisit if the repo ever becomes multi-developer.

### Trial assets sandboxed out of the repo - 2026-07-11
**Decision:** Evaluate/import candidate assets inside `Content/Developers/` (and optional
`Content/_Sandbox/`), with raw source files (`.fbx`/`.png`/`.psd`/`.wav`) kept in
`RawContent/`. All three are git-ignored. A keeper is promoted by MOVING it (Content Browser
Move, not copy) into a normal tracked `Content/` path, then committing.
**Reasoning:** Uncommitted candidates never cost LFS storage or pollute history. UE excludes
the Developers folder from packaged builds by default, and only needs raw source at import
time, not to run. This keeps the repo to assets actually in use.
**Rejected:** Committing every imported asset. Bloats LFS storage and history with
throwaway trial content.

### AI tooling excluded from the repo - 2026-07-11
**Decision:** Git-ignore `CLAUDE.md`, `CLAUDE.local.md`, `.claude/`, and `.mcp.json`.
**Reasoning:** These configure the local AI/dev workflow, not the game. They are personal,
change often, and add nothing to a build or to a collaborator. The durable project state
lives in the tracked `Docs/DECISIONS.md` and `Docs/SESSION_LOG.md` instead.
**Rejected:** Tracking the AI config in-repo. Couples the shipped project to a specific
tooling setup and leaks local paths/preferences into history.
