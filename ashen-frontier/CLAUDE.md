# Ashen Frontier

Survival + creature-taming game. Single-file HTML prototype, no bundler, runs directly in browser.

## Philosophy
Build the smallest fun version first. Each phase adds one system on top of a working foundation.

## Core Loop
Gather → Build → Tame → Fight → Upgrade → Explore → Repeat

## Project Structure

```
ashen-frontier/
  src/prototype.html    ← Playable prototype (open in browser, no server needed)
  docs/                 ← Design documents (01–05)
  art/                  ← SDXL concept art assets
    terra_stoneshell.png   — earth turtle creature (Phase 2 starter tame)
    flame_magmapede.png    — fire centipede (Phase 4 dungeon boss)
    env_forest.png         — forest sanctuary environment
    portrait_wanderer.png  — NPC portrait (Wanderer role)
```

---

## Visual Style — Game Boy Ash Gray

Strict 4-color monochrome palette:

| Constant | Hex | Role |
|----------|-----|------|
| C0 / PAL[1] | `#e8e8d8` | Lightest — highlights, UI accents |
| C1 / PAL[2] | `#a8a890` | Light gray — mid detail, badges |
| C2 / PAL[3] | `#585850` | Dark gray — base tiles, shadows |
| C3 / PAL[4] | `#101010` | Near-black — outlines, night sky |

**Pixel sprite system:** `drawSpr(spr, cx, cy, px)` — centered at (cx,cy), each cell = px×px screen pixels. Arrays use indices 0 (transparent) through 4 (PAL). `ctx.imageSmoothingEnabled = false` on canvas for crisp scaling.

**Sprites defined:**
- `TREE_SPR` — 8×8 @ PX=4 → 32×32 screen (one tile)
- `ROCK_SPR` — 8×8 @ PX=4 → 32×32 screen
- `ASH_SPR`  — 8×8 @ PX=4 → 32×32 screen
- `PLAYER_SPR` — 8×8 @ PX=3 → 24×24 screen

**World tiles** pre-baked into an offscreen canvas (`bakeWorld()`) using pixel fill patterns: checkerboard grass, brick rock, ash speckle, dark forest. Drawn once via `ctx.drawImage(worldCanvas, 0, 0)` each frame.

All rings, particles, build ghosts, and UI use `fillRect` only — no strokes. Keeps pixel-art feel consistent.

---

## Terra Stoneshell — Missingno Glitch Sprite

Creature renders as a corrupted Gen-1 Pokémon-style glitch sprite.

1. `SHELL_MASK` — 16×16 binary turtle silhouette (body rows 0–8, legs rows 9–12)
2. `makeMissingnoBmp(seed)` — fills mask with random PAL values (1–4), seeded per creature
3. Drawn at PX=2 (16×16 → 32×32 screen) centered on creature
4. `updateGlitch()` every 10 ticks: scrambles 3 random pixels; 20% chance of scanline row-shift ±1px for 7 ticks
5. Tamed/Penned creatures are NOT scrambled (they've stabilized)

State rings (pixel dots): ALERT=C2 dotted, STUNNED=C1 progress arc, TAMED=C1 solid ring
Badge text: `!!` / `??` / `++` / `<>`

---

## Day/Night Cycle

Full cycle = 3600 ticks (~60s at 60fps).

| Phase | Ticks | Range |
|-------|-------|-------|
| DAY   | 1440  | 0–1440 |
| DUSK  | 360   | 1440–1800 |
| NIGHT | 1440  | 1800–3240 |
| DAWN  | 360   | 3240–3600 |

`getNightAlpha()` → 0.0 (day) to 1.0 (night), linear transitions.

`drawNightOverlay()` runs last in `draw()`:
1. Campfire halos drawn BEFORE overlay (punch through darkness): pulsing C1 inner circle r=T×3 + dim outer r=T×5.5
2. Dark `#08080e` overlay at `na × 0.78` opacity
3. Pixel sun (fades at dusk) / crescent moon (fades in at night) icon — bottom-right corner

HUD label: `— DAY —` / `— DUSK —` / `— NIGHT —` / `— DAWN —` (updated every frame).

---

## Phase 1 Status — IN PROGRESS

- [x] Tile-based world (grass / forest / rock / ash terrain) — pre-baked pixel art
- [x] Resource nodes with HP (wood / stone / ash)
- [x] Player movement (WASD) — pixel top-down sprite
- [x] Gather mechanic (E key, ~1.5 tile radius)
- [x] Build mode (1/2/3/4) — campfire, foundation, wall, creature pen
- [x] Resource cost system + build ghost
- [x] Day/night cycle — 60s full cycle, overlay + campfire glow + moon/sun icon
- [x] Campfire — 3-frame pixel fire, firefly particles, night halo
- [ ] Campfire warmth / rest mechanic (penalty at night without warmth)
- [ ] Save/load (localStorage)

---

## Phase 2 — Starter Creature (PARTIAL)

- [x] Terra Stoneshell spawned at 4 corners — Missingno glitch sprite
- [x] Follow AI (walks toward player when tamed)
- [x] Creature state machine: IDLE → ALERT → FLEE → STUNNED → TAMED → PENNED
- [x] Capture mechanic: F key near wild creature + Pen built → stuns 150 ticks → tames
- [x] Pen assignment: tamed creature navigates to nearest unoccupied pen
- [ ] Basic creature combat stats (HP / attack / defense)
- [ ] Creature inventory panel

---

## NPC Roles (Phase 3)
Merchant / Brujo / Hunter / Engineer / Wanderer / Faction Rep
Dialogue loop: Approach → Interact → Dialogue → Choice → Reward

---

## Phase 4 — Dungeon Instance
Flow: World → Entrance → Save → Load Instance → Complete Dungeon → Return
Flame Magmapede is the Phase 4 boss candidate.

---

## Future Target Platform
Keep in HTML prototype until core loop is fun. Ship paths:
- **GB ROM**: GB Studio (separate port — exports `.gb`)
- **Desktop**: Electron (wraps HTML, zero rewrite)
- **Mobile**: PWA or Capacitor

Do NOT move to UE5 — wrong tool for this vibe.

---

## Run
```
open ashen-frontier/src/prototype.html   # any browser, no server
```

## Controls
- WASD: move
- E: gather (within ~1.5 tiles)
- F: capture (within ~2.2 tiles, requires Pen)
- 1: Campfire (2 wood + 1 stone)
- 2: Foundation (3 wood)
- 3: Wall (2 wood)
- 4: Creature Pen (4 wood + 2 stone)
- Esc: cancel build
- Click: place structure (build mode)

## Music (MELEGI — Phase 5+)
- Explore: SOUL + LO-FI, ~75 BPM, F minor
- Encounter: DARK + SOUL, ~95 BPM, G minor
- Combat: DARK + TRAP, ~130 BPM, heavy sub
