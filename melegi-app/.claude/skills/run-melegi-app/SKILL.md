---
name: run-melegi-app
description: >
  Run, build, screenshot, and test the MELEGI Audio Engine PWA. Use when
  asked to run the app, start the app, take a screenshot, verify a UI change,
  confirm a fix works, or test the MELEGI beat compiler in the browser.
---

MELEGI is a single-file static PWA (`melegi-app/index.html`) — no build step, no bundler. It runs in a browser and is driven headlessly with a Playwright driver script.

All paths in this skill are relative to `melegi-app/`.

## Prerequisites

Playwright and Chromium are pre-installed at system paths:

- Playwright: `/opt/node22/lib/node_modules/playwright`
- Chromium: `/opt/pw-browsers/chromium-1194/chrome-linux/chrome`

No `npm install` required.

## Run (agent path)

The driver at `.claude/skills/run-melegi-app/driver.mjs` spins up a Node.js static HTTP server, navigates a headless Chromium through the full BUILD → PLAY → REFINE → ANALYZE → EXPORT flow, asserts meters populate, and writes screenshots.

```bash
cd melegi-app
node .claude/skills/run-melegi-app/driver.mjs
```

Screenshots land in `/tmp/melegi-screenshots/` (or pass `--screenshot-dir /path`):

| File | What it shows |
|------|---------------|
| `01-initial.png` | BUILD page, no signal yet |
| `02-after-build.png` | Spectrum + meters populated post-BUILD |
| `03-playing.png` | Play button active |
| `04-refine.png` | REFINE page |
| `04-analyze.png` | ANALYZE page |
| `04-export.png` | EXPORT page |

Options:
```bash
node .claude/skills/run-melegi-app/driver.mjs \
  --screenshot-dir /tmp/my-shots \
  --port 7891
```

The driver exits non-zero and throws if the BUILD doesn't populate meters or if the filename preview stays `—`.

## Run (human path)

```bash
cd melegi-app
python3 -m http.server 7890
# Open http://localhost:7890/ in a browser
```

This path is useless headless — use the driver above.

## What to assert after a UI change

Look at `02-after-build.png` for:
- Spectrum bars (gold) rendered in center panel
- Bottom meters: **RMS**, **LUFS**, **CENTROID**, **PEAK** all showing values (not `—`)
- BUILD button re-enabled

The driver already checks `#meter-rms !== '—'` as a sanity gate. Add more `page.locator(...).textContent()` checks in the driver for whatever the PR touched.

## Gotchas

**Google Fonts SSL error in container.** `fonts.googleapis.com` fails with `ERR_CERT_AUTHORITY_INVALID` because the container has no outbound TLS trust for it. The font falls back to system monospace — font-family still renders fine. The driver passes `ignoreHTTPSErrors: true` to suppress it.

**Service Worker registration fails.** SW requires HTTPS or `localhost`; the driver uses `127.0.0.1` so the SW silently skips registration. No functional impact — SW only caches assets.

**Splash animation takes 2.8 s.** The driver waits 3.2 s after `domcontentloaded` before interacting. Don't cut this short or clicks on the BUILD button may fire into the splash overlay.

**BUILD is synchronous-looking but async.** The compile pipeline is `async` and disables the BUILD button while running. The driver waits for `#compile-btn` to be re-enabled (`waitForFunction`) — don't use a fixed timeout here.

**AudioContext requires user gesture.** `ensureAudioCtx()` is called inside `runCompile()` (triggered by the BUILD button click), which counts as a user gesture in Chromium. The driver's `locator('#compile-btn').click()` provides it.

**No `--screenshot-dir` flag needed for default use.** Omit it and screenshots land in `/tmp/melegi-screenshots/`.

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| `ERR_SOCKET_BAD_PORT: NaN` | Old driver cached — re-read driver.mjs; the arg parser had an off-by-one on missing flags |
| `Error: Build did not populate RMS meter` | BUILD threw an exception; check `errors[]` printed to stdout; likely an AudioContext or synthesis bug |
| `chromium` launch timeout | `/opt/pw-browsers/chromium-1194/chrome-linux/chrome` not found — check path with `find /opt/pw-browsers -name chrome -type f` |
| Port already in use | Previous run left a server alive — `pkill -f "http.server 7890"` or pass `--port 7891` |
