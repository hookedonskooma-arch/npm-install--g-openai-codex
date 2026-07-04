# Ashen Frontier Lab

Handoff 12 structural fork of Mina's World's builder tooling, for prototyping
separately from the kid-safe app. This is a **technical fork only** — no
mature/dark content has been added. Renamed concepts (per the handoff doc):

| Mina's World       | Ashen Frontier Lab       |
|---------------------|---------------------------|
| World Builder        | Biome/Base Planner        |
| Kid Quest             | Quest Template Tool       |
| Chibi Avatar          | Ashen Character Prototype (not built here yet) |

## Keep Separate (per the handoff)

- **Repo**: this currently lives as a sibling folder in the same scratch repo
  as `minas-world/`, same as everything else built in this session — move it
  to its own repo before doing anything real with it.
- **Supabase project**: `schema.sql` here is a copy of Mina's World's schema.
  Do **not** point it at Mina's World's Supabase project — create a separate
  one. `.env.local.example` is a placeholder; there is no real `.env.local`
  checked in or copied over.
- **Assets / content rating**: whatever gets built here is not reviewed for
  or intended for Mina. Keep it out of the kid-safe app entirely.

## Running it

Same as Mina's World:

```bash
npm install
cp .env.local.example .env.local  # fill in this lab's own Supabase project
npm run dev
```
