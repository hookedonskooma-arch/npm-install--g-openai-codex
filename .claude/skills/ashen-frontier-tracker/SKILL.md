---
name: ashen-frontier-tracker
description: "Track Ashen Frontier game development phase progress and surface the next recommended action. Use when asked about game status, what to build next, what's done, phase progress, or to update the checklist after completing a feature. Reads ashen-frontier/CLAUDE.md as the single source of truth for phase state. Triggers on: 'what's next for ashen', 'game status', 'ashen status', 'phase progress', 'what did we build', 'mark X done', 'update ashen tracker', 'ashen frontier status', 'what should I work on'. Do NOT use for MELEGI audio engine, Alfred bot, or anything outside the ashen-frontier/ directory."
---

# Ashen Frontier Project Tracker

## Overview

This skill reads `ashen-frontier/CLAUDE.md` as the single source of truth, parses all phase checklists, surfaces what's done vs pending, and recommends the single highest-value next action. It can also update the checklist in CLAUDE.md when features are completed.

## Quick Reference

| Task | Approach |
|------|----------|
| Show full project status | Read CLAUDE.md → parse all phases → render status table |
| Surface next action | Find first unchecked item in lowest incomplete phase |
| Mark feature complete | Edit CLAUDE.md checklist item `[ ]` → `[x]` + commit |
| See what's left in a phase | Filter checklist to that phase only |
| See what was built this session | Read git log for ashen-frontier/ since last session |

---

## Workflow

### Step 1 — Read the source of truth

Always start by reading the checklist sections from `ashen-frontier/CLAUDE.md`. That file is the canonical record — not this skill, not your memory.

```
Read /home/user/npm-install--g-openai-codex/ashen-frontier/CLAUDE.md
```

### Step 2 — Parse phase state

Extract every checklist item (`- [x]` = done, `- [ ]` = pending) grouped by phase header. A phase is **complete** when all its items are `[x]`. A phase is **active** when it has at least one `[x]` and at least one `[ ]`. A phase is **locked** when no prior phase is complete.

**Phase order:**
1. Phase 1 — Core Prototype
2. Phase 2 — Starter Creature
3. Phase 3 — NPC System
4. Phase 4 — Dungeon Instance

### Step 3 — Render status report

Output this format every time:

```
## Ashen Frontier — Project Status

### Phase 1 — Core Prototype  [ACTIVE / COMPLETE / LOCKED]
✅ done item
✅ done item
⬜ pending item   ← NEXT ACTION if this is the active phase
⬜ pending item

### Phase 2 — Starter Creature  [ACTIVE / COMPLETE / LOCKED]
...

---
🎯 NEXT ACTION: [specific pending item in plain language]
Why: [one sentence on why this is highest priority]
```

### Step 4 — Recommend next action

The next action is always:
1. The **first unchecked item** in the **lowest numbered phase** that has unchecked items
2. If Phase 1 has unchecked items, those come before Phase 2, always

**Priority tiebreaker within a phase:** gameplay feel > visual polish > QoL features. If two items are both unchecked and feel equal, pick the one that unblocks the most downstream work.

### Step 5 — Optionally update the checklist

If the user says "mark X done" or "we finished Y":
1. Edit `ashen-frontier/CLAUDE.md`: change the matching `- [ ]` to `- [x]`
2. If no exact match, ask the user to clarify rather than guessing
3. Commit the change: `git add ashen-frontier/CLAUDE.md && git commit -m "chore(ashen-frontier): mark [feature] complete in CLAUDE.md"`
4. Re-render the status report so the user sees the updated state

---

## Anti-Patterns

**Don't report from memory.** Always re-read CLAUDE.md. Context from earlier in the conversation may be stale — the file is the truth.

**Don't recommend Phase 2 work while Phase 1 has unchecked items.** The project philosophy is "build the smallest working version first." Jumping ahead undermines the foundation.

**Don't recommend multiple next actions.** One clear action beats a menu. The user can ask "what else?" if they want more.

**Don't invent checklist items.** If a feature was built but isn't on the checklist, add it to CLAUDE.md before marking it done. Don't silently skip it.

**Don't conflate MELEGI with Ashen Frontier.** They're separate projects in the same repo. This skill only covers `ashen-frontier/`.

---

## Quality Checklist

Before delivering a status report:
- [ ] Read CLAUDE.md fresh (not from memory)
- [ ] All phases parsed, not just the current one
- [ ] Completion percentages calculated correctly (count `[x]` / total per phase)
- [ ] NEXT ACTION is a single concrete thing, not a category
- [ ] If updating the checklist: file edited + committed before re-rendering

---

## Source of Truth

`ashen-frontier/CLAUDE.md` — contains:
- Phase 1 checklist (Core Prototype + Day/Night)
- Phase 2 checklist (Starter Creature + AI)
- Phase 3 design notes (NPC System)
- Phase 4 design notes (Dungeon Instance)
- Visual style reference (GB Ash Gray palette, Missingno sprite system)
- Day/night cycle implementation notes

When CLAUDE.md and the actual code disagree, the code wins — update CLAUDE.md to match reality.
