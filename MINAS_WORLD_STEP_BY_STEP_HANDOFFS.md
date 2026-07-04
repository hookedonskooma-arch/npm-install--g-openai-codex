# Mina’s World — Step-by-Step Build Handoffs

Purpose: keep the project organized, kid-safe, and built in the same order as the original plan.

Core rule: **Mina’s World stays clean, cute, and safe. Ashen Frontier Lab stays separate.**

---

# Master Build Order

1. Foundation check
2. Avatar Studio cleanup
3. World Builder shell
4. Tile Grid v0.1
5. Tile Palette v0.1
6. Save / Load world locally
7. One playable test world
8. Play Mode movement
9. Simple object interaction
10. First kid-safe quest
11. Profile / database save
12. Ashen Frontier Lab fork

---

# Handoff 01 — Foundation Check

## Goal
Confirm the project runs locally before changing anything.

## Do This
```bash
git clone https://github.com/erickgrau/minas-1st-platform.git
cd minas-1st-platform
npm install
cp .env.local.example .env.local
npm run dev
```

## Verify
- App opens at `http://localhost:3000`
- No fatal console errors
- Existing pages load
- Avatar Studio is reachable

## Do Not
- Add Ashen Frontier content
- Change database structure yet
- Start multiplayer
- Start AI features

## Done When
The app runs and the current project state is documented.

---

# Handoff 02 — Avatar Studio Cleanup

## Goal
Make sure the avatar creator is stable before building worlds.

## Do This
- Test every avatar option
- Confirm the chibi SVG renders correctly
- Confirm colors update properly
- Confirm no broken UI states
- Add a simple “Save Avatar” placeholder button if missing

## Files To Inspect
```text
src/components/avatar/
src/app/studio/
src/types/avatar.ts
src/store/avatarStore.ts
```

## Done When
A kid can make a character without breaking the app.

---

# Handoff 03 — World Builder Shell

## Goal
Create the world builder page without real map logic yet.

## Build
```text
src/app/worlds/page.tsx
src/components/world/WorldToolbar.tsx
src/components/world/TilePalette.tsx
src/components/world/TileGrid.tsx
src/types/world.ts
src/store/worldStore.ts
```

## Page Should Show
- Title: Mina’s World Builder
- Empty grid area
- Tile palette area
- Toolbar area

## Done When
`/worlds` loads and shows the empty builder layout.

---

# Handoff 04 — Tile Grid v0.1

## Goal
Build a clickable grid.

## Rules
- Start with 16 x 16 tiles
- Each tile is square
- Each tile has x/y coordinates
- Default tile type: grass

## World Type
```ts
export type TileType = 'grass' | 'path' | 'water' | 'house' | 'tree';

export type WorldTile = {
  x: number;
  y: number;
  type: TileType;
};
```

## Done When
The screen displays a full 16 x 16 grass grid.

---

# Handoff 05 — Tile Palette v0.1

## Goal
Let the user choose a tile and paint it onto the grid.

## Tiles
- Grass
- Path
- Water
- House
- Tree
- Eraser

## Behavior
- Click palette tile
- Click grid square
- Grid square changes type

## Done When
A kid can paint a simple map.

---

# Handoff 06 — Local Save / Load

## Goal
Save the world locally before connecting Supabase.

## Use
```text
localStorage
```

## Buttons
- Save World
- Load World
- Clear World

## Storage Key
```text
minas_world_local_save_v1
```

## Done When
Refreshing the browser does not lose the saved world.

---

# Handoff 07 — One Playable Test World

## Goal
Make one cute test map.

## Map Requirements
- A path
- A house
- Trees
- Water
- One open grass area

## Done When
There is a saved test world that looks intentional.

---

# Handoff 08 — Play Mode Movement

## Goal
Let the avatar walk around the test world.

## Controls
- Arrow keys or WASD
- Block movement into water and houses
- Allow walking on grass and path

## Done When
The player can move around the world grid.

---

# Handoff 09 — Object Interaction

## Goal
Add one interaction.

## First Interaction
Click or press space near a tree:

```text
“You found a shiny leaf!”
```

## Done When
The world responds to the player.

---

# Handoff 10 — First Kid-Safe Quest

## Goal
Add one simple quest.

## Quest
```text
Find 3 shiny leaves for the garden.
```

## Needs
- Quest text
- Counter: 0/3
- Completion message
- Simple reward: flower sticker

## Done When
The player can complete one full quest loop.

---

# Handoff 11 — Database Save

## Goal
Move from local save to Supabase save.

## Save
- Avatar
- World map
- Quest progress

## Rule
Do not delete local save. Keep it as backup/dev mode.

## Done When
A saved world can be loaded from the database.

---

# Handoff 12 — Ashen Frontier Lab Fork

## Goal
Create the separate darker test project without touching Mina’s safe project.

## Create
```bash
git checkout -b ashen-lab
```

## Rename Conceptually
```text
Mina’s World Builder → Ashen Frontier Lab
Chibi Avatar → Ashen Character Prototype
World Builder → Biome/Base Planner
Kid Quest → Quest Template Tool
```

## Keep Separate
- Different branch or repo
- Different Supabase project
- Different `.env.local`
- Different assets
- Different content rating

## Done When
Ashen Lab exists separately and Mina’s main branch remains clean.

---

# Daily Work Rule

Each session should end with:

```text
1. What changed?
2. What works?
3. What broke?
4. What is the next handoff?
```

---

# Forbidden For Mina’s World

Do not add:
- Gore
- Horror monsters
- Ashen Frontier lore
- Combat violence
- Adult language
- Dark spiritual themes
- Anything Mina should not casually open

---

# North Star

Mina’s World is not just a game.

It is a safe imagination engine where Mina can make characters, places, and stories.
