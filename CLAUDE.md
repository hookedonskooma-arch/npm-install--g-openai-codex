# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

This repo contains three independent projects:

| Directory | Language | What it is |
|-----------|----------|------------|
| `bot.py` / `agents.py` / `memory.py` | Python | Alfred Studio — Discord bot with OpenAI integration |
| `melegi/` | C++20 | MELEGI Audio Engine — native DSP pipeline (CMake) |
| `melegi-app/` | HTML/CSS/JS (single file) | MELEGI PWA — browser-based beat compiler |

---

## Alfred Studio (Discord Bot)

**Run:**
```bash
pip install -r requirements.txt
cp .env.example .env  # populate DISCORD_TOKEN and OPENAI_API_KEY
python bot.py
```

**Architecture:** `bot.py` is the entry point; `agents.py` holds modular task agents; `memory.py` handles persistence. Slash command extensions load via `bot.load_extension()` patterns inside `bot.py`.

---

## MELEGI C++ Engine (`melegi/`)

**Build:**
```bash
cd melegi
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

**Run:**
```bash
./melegi/build/melegi_pipeline              # generate from scratch → exports 3 WAVs
./melegi/build/melegi_pipeline input.wav    # process existing WAV through pipeline
```

**Pipeline order:** `GenerationEngine` → `AnalysisEngine` → `TimingEngine` → `AuthenticityEngine` → `ShoegazeEngine` → `AgentFramework` → `ExportEngine`

**Key concepts:**
- `StyleDNA` (defined in `include/melegi/StyleDNA.hpp`) is the central parameter object passed through every engine stage. `makeGROUXX()` is the default preset.
- `GeoAstralEngine` modulates `StyleDNA` from lat/lon + weather inputs before generation.
- `ExportEngine` writes three simultaneous targets: BeatStars (–10 LUFS 44.1k/16b), SunoRef (–14 LUFS 44.1k/16b), Logic (–18 LUFS 48k/24b).
- Spectral centroid target is 757 Hz — the "GROUXX fingerprint." `AnalysisEngine::analyze()` reports `onTarget` against this.
- Build artifacts (WAV + JSON sidecar) land in `melegi/build/`.

**Headers** live in `melegi/include/melegi/`. Each `.hpp` corresponds to a `.cpp` in `melegi/src/`. Compiled flags: C++20, `-O2 -ffast-math -march=native` on Linux x86-64.

---

## MELEGI PWA (`melegi-app/`)

**All code is inline in `melegi-app/index.html`** — no bundler, no build step. Edit that file directly.

**Run / test headlessly:**
```bash
cd melegi-app
node .claude/skills/run-melegi-app/driver.mjs          # smoke test: BUILD → PLAY → EXPORT
node .claude/skills/run-melegi-app/build-arrangement.mjs  # render "THE FLOOR GAVE UP" → /tmp/the_floor_gave_up.wav
```

Screenshots land in `/tmp/melegi-screenshots/`. Playwright + Chromium are pre-installed at:
- `/opt/node22/lib/node_modules/playwright`
- `/opt/pw-browsers/chromium-1194/chrome-linux/chrome`

**Visual theme:** Dark obsidian + electric purple. CSS custom properties in `:root` control the palette — `--gold` maps to `#9333ea` (purple), `--green` is reserved for UI indicators only (API status dot, 757 Hz target line).

**Synthesis chain (all in-browser Web Audio API):**
- `karplusStrong()` — plucked string / chord layer
- `synth808Kick()`, `synthSnare()`, `synthHiHat()` — drum synthesis
- `aiGenerateNotation()` — calls `claude-haiku-4-5-20251001` to generate 12 KS chord notes

**State:** All runtime state lives in a single `state` object. Style chip UI state must be kept in sync with `state.activeStyles` (a `Set`) whenever chips are changed programmatically.

**DEMO preset:** `loadDemo()` pre-loads "THE FLOOR GAVE UP" 9-string C# arrangement. The `▶ DEMO` button above BUILD triggers it.

**Export:** The EXPORT tab renders a WAV via the Web Audio API offline context. `acceptDownloads: true` is required in the Playwright browser context to capture the download.

---

## Session Workflow Rule

**After every test or render that produces a WAV, send the file to the user immediately.**

This applies to:
- `build-arrangement.mjs` → `/tmp/the_floor_gave_up.wav`
- `guitar-riff.mjs` → `/tmp/guitar_riff_8bar.wav`
- Any new render script → whatever path it outputs

Use `SendUserFile` right after the render completes. Do not wait until end of session.

---

## Environment

`.env` (gitignored) holds `DISCORD_TOKEN` and `OPENAI_API_KEY`. The PWA reads an Anthropic API key from a UI input field (not from `.env`).
