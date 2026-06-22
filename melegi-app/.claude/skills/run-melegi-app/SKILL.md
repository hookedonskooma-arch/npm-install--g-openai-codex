---
name: run-melegi-app
description: >
  Run, smoke-test, screenshot, or render audio with the MELEGI Audio Engine
  PWA. Use when asked to run the app, start the app, take a screenshot, verify
  a UI change, confirm a fix works, test the MELEGI beat compiler in the
  browser, render a WAV, produce audio output, export a beat, smoke test after
  an index.html edit, or run the e2e test. Also triggers on: "build the
  arrangement", "render the floor gave up", "guitar riff WAV", "generate
  audio", "headless build". Do NOT use for general Node.js debugging,
  non-MELEGI browser tasks, or C++ engine work in melegi/.
---

MELEGI is a single-file static PWA (`melegi-app/index.html`) — no build step, no bundler. All three scripts below drive it headlessly via Playwright.

All paths in this skill are relative to `melegi-app/`.

## Quick Reference

| Goal | Script | Default output |
|------|--------|---------------|
| Smoke test BUILD → PLAY → EXPORT flow | `driver.mjs` | Pass/fail + screenshots in `/tmp/melegi-screenshots/` |
| Render "THE FLOOR GAVE UP" full arrangement | `build-arrangement.mjs` | `/tmp/the_floor_gave_up.wav` |
| Render 8-bar Guitar Bible riff (C# → D → C# → A) | `guitar-riff.mjs` | `/tmp/guitar_riff_8bar.wav` |
| Custom genre render (Griselda, shoegaze, etc.) | Write a new script modelled on `build-arrangement.mjs` | Caller-defined path |

**After any WAV render, send the file to the user immediately** — this is a standing CLAUDE.md rule.

## Smoke Test (driver.mjs)

```bash
cd melegi-app
node .claude/skills/run-melegi-app/driver.mjs
```

Options:
```bash
node .claude/skills/run-melegi-app/driver.mjs --port 7891 --screenshot-dir /tmp/my-shots
```

The driver navigates the full BUILD → PLAY → REFINE → ANALYZE → EXPORT flow, asserts meters populate, and exits non-zero on failure.

Screenshots written:

| File | What it shows |
|------|---------------|
| `01-initial.png` | BUILD page before signal |
| `02-after-build.png` | Spectrum + meters populated post-BUILD |
| `03-playing.png` | Play button active |
| `04-refine.png` | REFINE page |
| `04-analyze.png` | ANALYZE page |
| `04-export.png` | EXPORT page |

## Render Scripts

### build-arrangement.mjs — "THE FLOOR GAVE UP"

Full arrangement: 75 BPM, C# minor, Memphis horror tape vibe, 9-string earthquake sub. Chips: DARK + SOUL + DUSTY.

```bash
cd melegi-app
node .claude/skills/run-melegi-app/build-arrangement.mjs
# Output: /tmp/the_floor_gave_up.wav

node .claude/skills/run-melegi-app/build-arrangement.mjs --out /tmp/my.wav --port 7894
```

### guitar-riff.mjs — 8-bar Guitar Bible riff

Four chord changes (C# → D → C# → A), stitched into a single 25.6s WAV. Guitar-first mix: drums=0, soul=1.0.

```bash
cd melegi-app
node .claude/skills/run-melegi-app/guitar-riff.mjs
# Output: /tmp/guitar_riff_8bar.wav

node .claude/skills/run-melegi-app/guitar-riff.mjs --out /tmp/my_riff.wav --port 7903
```

## What to Assert After a Code Change

Check `02-after-build.png` for:
- Spectrum bars (gold) rendered in center panel
- Bottom meters: **RMS**, **LUFS**, **CENTROID**, **PEAK** all showing values (not `—`)
- BUILD button re-enabled

The driver checks `#meter-rms !== '—'` as a sanity gate. Add more `page.locator(...).textContent()` checks for whatever the PR touched.

## Gotchas

**Google Fonts SSL error.** `fonts.googleapis.com` fails with `ERR_CERT_AUTHORITY_INVALID` — no outbound TLS trust in the container. Font falls back to system monospace. The driver passes `ignoreHTTPSErrors: true`; this is expected and harmless.

**Service Worker silently skips.** SW requires HTTPS or `localhost`; the driver uses `127.0.0.1`. No functional impact — SW only caches assets.

**Splash animation takes 2.8 s.** The driver waits 3.2 s after `domcontentloaded`. Don't cut this — clicks before splash clears land on the overlay, not the BUILD button.

**BUILD is async, not instant.** The compile pipeline disables `#compile-btn` while running. Wait for it to re-enable:
```js
// Right: wait for the button, not a fixed timeout
await page.waitForFunction(() => !document.getElementById('compile-btn').disabled, { timeout: 30000 });

// Wrong: fixed sleep races with compile duration
await page.waitForTimeout(5000);
```

**AudioContext requires a user gesture.** `ensureAudioCtx()` fires inside `runCompile()`, which is triggered by the BUILD click. Playwright's `.click()` counts as a gesture — don't call `runCompile()` directly via `evaluate`.

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| `ERR_SOCKET_BAD_PORT: NaN` | Old driver cached — re-read `driver.mjs`; arg parser had off-by-one on missing flags |
| `Error: Build did not populate RMS meter` | BUILD threw a silent exception; check `errors[]` in stdout and `02-after-build.png` for visual state |
| Chromium launch timeout | Binary moved — find it: `find /opt/pw-browsers -name chrome -type f` |
| Port already in use | Previous run leaked a server — pass `--port 7891` (preferred) or `pkill -f "http.server 7890"` |
| WAV file is silence (0 bytes or flat) | Synthesis threw inside AudioContext; check JS errors in render script stdout |
| `guitar-riff.mjs` produces 4 clips but no stitched WAV | One chord's BUILD failed RMS guard and threw; check per-chord RMS log lines |

## Quality Checklist

- [ ] RMS meter shows a value (not `—`)
- [ ] No entries in `errors[]` (JS errors printed to stdout)
- [ ] Export filename preview populated (not `—`)
- [ ] Screenshots written to `--screenshot-dir`
- [ ] WAV file exists at declared output path and is non-empty (for render scripts)
- [ ] WAV sent to user via `SendUserFile` immediately after render (CLAUDE.md session rule)
