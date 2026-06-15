# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Alfred Studio is a private AI music generation platform with two independent interfaces sharing the same Suno AI and OpenAI backends:

- **Python Discord bot** (`bot.py`, `agents.py`) — slash command `/generate` interface
- **Next.js web app** (`web/`) — password-protected browser UI with a song library

The core concept: users can generate music via natural language prompts **or** via C++ code interpreted by GPT-4o as musical directives (function names → song sections, loops → repeated patterns, variables → sonic elements, comments → lyrical themes).

## Environment Setup

**Python bot** — copy `.env.example` to `.env`:
```
DISCORD_TOKEN=
SUNO_API_KEY=
OPENAI_API_KEY=
```

**Next.js web** — copy `web/.env.local.example` to `web/.env.local`:
```
SUNO_API_KEY=
OPENAI_API_KEY=
PRIVATE_PASSWORD=
```

## Commands

### Python Bot
```bash
pip install -r requirements.txt
python bot.py
```

### Next.js Web App (run from `web/`)
```bash
npm install
npm run dev      # development server (localhost:3000)
npm run build    # production build
npm run start    # production server
```

There are no tests or linters configured in this project.

## Architecture

### Suno Generation Flow

Both interfaces share the same two-step pattern:
1. **Generate** — POST to Suno `POST /api/generate/v2/` with `prompt`, `tags`, `mv: "chirp-v3-5"` → returns a `clip.id`
2. **Poll** — GET Suno `GET /api/feed/?ids=<id>` every few seconds until `status === "complete"` and `audio_url` is populated (max ~2 min)

If `mode === "cpp"` (web) or `cpp_mode=True` (Discord), GPT-4o runs first with a hardcoded system prompt to translate the C++ into a Suno music prompt. The same system prompt text is duplicated in `agents.py:_interpret_cpp` and `web/src/app/api/generate/route.ts`.

### Web App Structure

**Authentication**: `web/src/middleware.ts` intercepts every request except `/login` and `/api/login`. It checks the `session` cookie value against `PRIVATE_PASSWORD` env var. The cookie is set to the string `"authenticated"` on successful login — there is no JWT or hashing.

**API routes** (`web/src/app/api/`):
- `POST /api/generate` — optional GPT-4o interpretation → calls `lib/suno.ts:generateSong()` → returns `{ songId }`
- `GET /api/status/[id]` — calls `lib/suno.ts:pollSong()` → returns status/urls
- `POST /api/login` — compares body password to `PRIVATE_PASSWORD`, sets cookie
- `GET /api/library` — reads `web/data/songs.json`
- `POST /api/library` — appends a song to `web/data/songs.json`

**Persistence**: `web/src/lib/songs.ts` reads/writes `web/data/songs.json` (auto-created). Songs are prepended (newest first). No database.

**Client polling**: `web/src/app/page.tsx` uses `setInterval` every 3 seconds to hit `/api/status/[id]`. The interval ref is stored in `pollRef` and cleared on completion, error, or 120-second timeout.

### Discord Bot Structure

`bot.py` registers a single slash command `/generate` that calls `SunoAgent` from `agents.py`. The bot polls every 5 seconds (vs. 3 seconds in the web app). `memory.py` exists as a stub but is not wired into the bot.

### PWA

`web/public/sw.js` is a service worker providing offline caching. `web/src/components/ServiceWorkerRegister.tsx` registers it client-side. The web manifest is at `web/public/manifest.json`.
