# Heist Gone Wrong - Development Setup Guide

## Required Software

### All Platforms:
- **Unreal Engine 5.x** (from Epic Games Launcher)
- **Git** + **Git LFS**
- **GitHub Account** (you'll receive an invite)

### Windows:
- **Visual Studio 2022 Community Edition**
  - Install with "Game development with C++" workload

### Mac:
- **Xcode** (from App Store)
- **Homebrew** (for Git LFS installation)

---

## First Time Setup

### Windows Users:

1. **Install Visual Studio 2022:**
   - Download from visualstudio.microsoft.com
   - Select "Game development with C++" workload
   - Install (takes 15-30 minutes)

2. **Install Git LFS:**
   ```bash
   # Download from git-lfs.github.com
   # Then run:
   git lfs install
   ```

3. **Clone the Repository:**
   ```bash
   git clone https://github.com/YOUR_USERNAME/heist-gone-wrong.git
   cd heist-gone-wrong
   git lfs pull
   ```

4. **Open the Project:**
   - Double-click `HeistGoneWrong.sln`
   - Visual Studio will open
   - Press **F5** to build and launch Unreal

---

### Mac Users:

1. **Install Xcode:**
   - Open App Store
   - Search "Xcode" and install (12GB, takes 30-60 min)
   - Open Xcode once to accept license
   - Install Command Line Tools:
   ```bash
   xcode-select --install
   ```

2. **Install Homebrew and Git LFS:**
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   brew install git-lfs
   git lfs install
   ```

3. **Clone the Repository:**
   ```bash
   git clone https://github.com/YOUR_USERNAME/heist-gone-wrong.git
   cd heist-gone-wrong
   git lfs pull
   ```

4. **Generate Xcode Project:**
   - Right-click `HeistGoneWrong.uproject`
   - Select "Services" → "Generate Xcode Project"

5. **Open the Project:**
   - Double-click `HeistGoneWrong.xcworkspace`
   - Press **⌘ + R** to build and launch Unreal

---

## Daily Workflow

### Before Starting Work (Every Day!):
```bash
git pull origin main
```

### While Working:
- Make your changes in Unreal Editor
- Work on your assigned Blueprints/systems
- Test your changes locally

### After Finishing Work:
```bash
git add .
git commit -m "Descriptive message about what you changed"
git push origin main
```

**Example commit messages:**
- ✅ "Added guard patrol blueprint with NavMesh"
- ✅ "Fixed detection meter UI scaling"
- ✅ "Implemented camera rotation system"
- ❌ "stuff" (too vague)
- ❌ "asdf" (meaningless)

---

## Important Git Rules

### ✅ DO:
- **Pull before you start** working every day
- **Commit frequently** (every 30-60 minutes)
- **Write clear commit messages**
- **Communicate in Discord/Slack** when working on shared systems
- **Test your changes** before pushing

### ❌ DON'T:
- **Never force push** (`git push -f`) - ask Project Lead first
- **Don't edit the same Blueprint** as someone else simultaneously
- **Don't commit broken/untested code**
- **Don't commit Saved/, Intermediate/, or DerivedDataCache/ folders** (already in .gitignore)

---

## Common Issues & Solutions

### Issue: "Git LFS files not downloading properly"
**Solution:**
```bash
git lfs pull
```

### Issue: "Unreal says project files are out of date"
**Solution (Windows):**
- Right-click `HeistGoneWrong.uproject` → "Generate Visual Studio project files"

**Solution (Mac):**
- Right-click `HeistGoneWrong.uproject` → "Generate Xcode Project"

### Issue: "Merge conflict in .uasset file"
**Solution:**
- Binary files can't be auto-merged
- Contact the other person who worked on that file
- Decide whose version to keep:
```bash
# Keep yours:
git checkout --ours path/to/file.uasset

# Keep theirs:
git checkout --theirs path/to/file.uasset

git add .
git commit -m "Resolved merge conflict"
```

### Issue: "Can't push - 'rejected' error"
**Solution:**
```bash
git pull origin main
# Resolve any conflicts
git push origin main
```

### Issue: "Unreal won't compile after pulling"
**Solution (Windows):**
- Open Visual Studio
- Build → Rebuild Solution
- Then open Unreal

**Solution (Mac):**
- Open Xcode
- Product → Clean Build Folder (⇧⌘K)
- Product → Build (⌘B)
- Then open Unreal

---

## Project Structure

```
HeistGoneWrong/
├── HeistGoneWrong.uproject    ← Unreal project file
├── HeistGoneWrong.sln         ← Visual Studio solution (Windows)
├── HeistGoneWrong.xcworkspace ← Xcode workspace (Mac)
├── Source/                    ← C++ source code
├── Content/                   ← YOUR WORK GOES HERE
│   ├── Blueprints/
│   ├── Maps/
│   ├── UI/
│   └── Audio/
├── Config/                    ← Project settings
└── Plugins/                   ← Any plugins
```

**You'll work primarily in the Content/ folder!**

---

## Where to Get Help

1. **Check Linear** for your assigned tasks
2. **Ask in Discord/Slack** team channel
3. **Contact Project Lead** for major issues
4. **Unreal Documentation:** docs.unrealengine.com

---

## Weekly Meetings

- **When:** [Day/Time - Fill in]
- **Where:** [Location/Link - Fill in]
- **Agenda:**
  - Review last week's progress
  - Assign new tasks from Linear
  - Discuss blockers
  - Demo completed features

---

## Development Tips

### For Beginners:
- Start with simple tasks in Linear
- Watch Unreal's official Blueprint tutorials
- Ask questions early and often
- Test frequently as you build
- Save your work often (Ctrl+S / ⌘S)

### Blueprint Best Practices:
- Use clear, descriptive names
- Add comments to complex logic
- Keep Blueprints organized and readable
- Don't copy-paste - use Blueprint functions
- Test in Play mode frequently

### Performance:
- Keep viewport at medium quality while working
- Close unused Blueprint windows
- Restart Unreal Editor if it gets slow

---

## Success Checklist

Before pushing your work, verify:

- [ ] Code/Blueprints compile without errors
- [ ] Tested in Play mode
- [ ] No placeholder/test content left behind
- [ ] Followed naming conventions
- [ ] Added comments where needed
- [ ] Pulled latest changes first
- [ ] Written clear commit message

---

**Questions?** Contact [Project Lead Name] at [Email/Discord]
