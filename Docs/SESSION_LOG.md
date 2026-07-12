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
- First commit and GitHub repo creation are pending developer confirmation at time of writing.

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
- Get developer confirmation of the staged file list, then make the first commit.
- Create the private GitHub repo `heist-gone-wrong`, add the remote, and push `main`.
- Verify LFS with `git lfs ls-files` after the first asset commit (no assets committed yet at
  setup time, so this will be empty until Content is added).
- Then begin W1 gameplay work: the C++ Character class (walk/run in all directions, crouch)
  and the walkable graybox museum level.

### Editor-side steps still needed from me
- None for this session (version-control setup only).
