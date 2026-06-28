# Ashen Frontier

Survival + creature-taming game. Single-file HTML prototype, no bundler, runs directly in browser.

## Philosophy
Build the smallest fun version first. Each phase adds one system on top of a working foundation.

## Core Loop
Gather → Build → Tame → Fight → Upgrade → Explore → Repeat

## Project Structure

```
ashen-frontier/
  src/prototype.html    ← Phase 1 playable prototype (open in browser)
  docs/                 ← Design documents (01–05)
  art/                  ← SDXL concept art assets
    terra_stoneshell.png   — earth turtle creature (Phase 2 starter tame)
    flame_magmapede.png    — fire centipede (Phase 4 dungeon boss)
    env_forest.png         — forest sanctuary environment
    portrait_wanderer.png  — NPC portrait (Wanderer role)
```

## Phase 1 Status — IN PROGRESS

Prototype implements:
- [x] Tile-based world (grass / forest / rock / ash terrain)
- [x] Resource nodes with HP (wood from forest, stone from rock, ash deposits)
- [x] Player movement (WASD)
- [x] Gather mechanic (E key, proximity-based)
- [x] Build mode (1/2/3/4 hotkeys) — campfire, foundation, wall, creature pen
- [x] Resource cost system
- [x] Build ghost (green=can place, red=can't afford)
- [ ] Campfire warmth / rest mechanic
- [ ] Day/night cycle
- [ ] Save/load

## Phase 2 Next — Starter Creature

- Terra Stoneshell: slow, defensive, earth-type
- Follow AI (pathfinding to player)
- Basic creature combat stats
- Creature pen assigns home tile

## NPC Roles (Phase 3)
Merchant / Brujo / Hunter / Engineer / Wanderer / Faction Rep
Dialogue loop: Approach → Interact → Dialogue → Choice → Reward

## Phase 4 — Dungeon Instance
Flow: World → Entrance → Save → Load Instance → Complete Dungeon → Return
Flame Magmapede is the Phase 4 boss candidate.

## Run
Open `src/prototype.html` directly in any browser. No server needed.

## Controls
- WASD: move
- E: gather (must be within ~1.5 tiles)
- 1: place Campfire (2 wood + 1 stone)
- 2: place Foundation (3 wood)
- 3: place Wall (2 wood)
- 4: place Creature Pen (4 wood + 2 stone)
- Esc: cancel build mode
- Click: place structure (in build mode)

## Music (MELEGI integration)
Three soundtrack contexts for later:
- Explore: SOUL + LO-FI, ~75 BPM, F minor — ambient base building
- Encounter: DARK + SOUL, ~95 BPM, G minor — creature taming
- Combat: DARK + TRAP, ~130 BPM, heavy sub — dungeon/boss fight
