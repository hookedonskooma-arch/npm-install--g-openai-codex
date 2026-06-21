/**
 * MELEGI arrangement driver — THE FLOOR GAVE UP
 *
 * Parameters derived from the MELEGI 9-String Guitar Bible:
 *   - Tuning:    9-string low C# (C# F# B E A D G B E)
 *   - Progression: C#5 → D5 → C#5 → A5 → G5 → D5
 *   - Vibe:      Memphis horror tape · hardcore two-step · 9-string earthquake
 *   - GROUXX thumbprint: 757 Hz spectral centroid target
 *
 * Usage:
 *   node build-arrangement.mjs
 *   node build-arrangement.mjs --out /tmp/my.wav --port 7893
 */
import { chromium } from '/opt/node22/lib/node_modules/playwright/index.mjs';
import { createServer } from 'node:http';
import { readFileSync, existsSync } from 'node:fs';
import { join, extname, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const __dir   = dirname(fileURLToPath(import.meta.url));
const APP_ROOT = resolve(__dir, '../../..'); // melegi-app/
const args    = process.argv.slice(2);
const outIdx  = args.indexOf('--out');
const ptIdx   = args.indexOf('--port');
const outFile = outIdx !== -1 ? args[outIdx + 1] : '/tmp/the_floor_gave_up.wav';
const port    = ptIdx  !== -1 ? parseInt(args[ptIdx + 1], 10) : 7893;

const MIME = {
  '.html':'text/html','.js':'application/javascript',
  '.css':'text/css','.png':'image/png','.webmanifest':'application/manifest+json',
  '.json':'application/json','.ico':'image/x-icon',
};

// ── static server ──────────────────────────────────────────────────────────────
const server = createServer((req, res) => {
  const safePath = req.url.split('?')[0].replace(/\.\./g,'');
  const filePath = join(APP_ROOT, safePath === '/' ? '/index.html' : safePath);
  if (!existsSync(filePath)) { res.writeHead(404); res.end('Not found'); return; }
  res.writeHead(200, { 'Content-Type': MIME[extname(filePath)] ?? 'application/octet-stream' });
  res.end(readFileSync(filePath));
});
await new Promise(r => server.listen(port, '127.0.0.1', r));
console.log(`Server: http://127.0.0.1:${port}/`);

// ── browser ────────────────────────────────────────────────────────────────────
const browser = await chromium.launch({
  executablePath: '/opt/pw-browsers/chromium-1194/chrome-linux/chrome',
  args: ['--no-sandbox','--disable-dev-shm-usage','--disable-gpu'],
});
const ctx = await browser.newContext({
  viewport: { width: 1280, height: 800 },
  ignoreHTTPSErrors: true,
  acceptDownloads: true,
});
const page = await ctx.newPage();
const errors = [];
page.on('pageerror', e => errors.push(e.message));

await page.goto(`http://127.0.0.1:${port}/`, { waitUntil: 'domcontentloaded' });
console.log('Waiting for splash...');
await page.waitForTimeout(3200);

// ── SONG TITLE ─────────────────────────────────────────────────────────────────
await page.locator('#song-title-input').fill('THE FLOOR GAVE UP');

// ── LYRICS / PROMPT ────────────────────────────────────────────────────────────
// Full Guitar Bible intent condensed for the AI notation generator
await page.locator('#lyrics-prompt').fill(
  `THE FLOOR GAVE UP
9-string low C# earthquake arrangement. Progression: C#5 D5 C#5 A5 G5 D5.
Memphis horror tape intro. Hardcore two-step bounce. Mega slow half-time breakdown.
Chopped screwed collapse outro. Sub rumble. Dead stops. No happy chorus.
Every element has a job.`
);

// ── STYLES PROMPT ──────────────────────────────────────────────────────────────
await page.locator('#styles-prompt').fill(
  'dark, Memphis horror tape, 9-string earthquake sub bass, hardcore two-step, ' +
  'chopped screwed slowed haunted, 808 sub kick, half-time breakdown, ' +
  'violent pit-ready, shoegaze fog layer, no djent, no solo, no happy chorus'
);

// ── BPM: 75 (half-time breakdown tempo from Guitar Bible) ─────────────────────
// "Breakdown drops into 75 BPM half-time. Chopped/screwed effect makes it feel 55-60 BPM."
await page.locator('input[oninput*="state.bpm"]').evaluate(el => {
  el.value = 75; el.dispatchEvent(new Event('input', { bubbles: true }));
});
console.log('BPM: 75 (breakdown half-time)');

// ── SWING: heavy (two-step lurch, beatdown gravity) ───────────────────────────
await page.locator('input[oninput*="state.swing"]').evaluate(el => {
  el.value = 0.5; el.dispatchEvent(new Event('input', { bubbles: true }));
});

// ── STYLE CHIPS ────────────────────────────────────────────────────────────────
// Guitar Bible: DARK (earthquake fear) + SOUL (Memphis soul undertone) + DUSTY (tape)
// OFF: lo-fi, boom-bap, trap, jazz, ambient
const ALL_CHIPS  = ['dark','lo-fi','boom-bap','dusty','soul','trap','jazz','ambient'];
const WANT_ON    = new Set(['dark','soul','dusty']);
for (const style of ALL_CHIPS) {
  const chip     = page.locator(`.style-chip[data-style="${style}"]`);
  if (!await chip.count()) continue;
  const isActive = await chip.evaluate(el => el.classList.contains('active'));
  const wantOn   = WANT_ON.has(style);
  if (wantOn && !isActive) await chip.click();
  if (!wantOn && isActive) await chip.click();
}
console.log('Style chips: DARK + SOUL + DUSTY');

// ── ADVANCED PANEL ─────────────────────────────────────────────────────────────
// Open if not already visible
const advPanel   = page.locator('#adv-panel');
const advVisible = await advPanel.evaluate(el => el.style.display !== 'none');
if (!advVisible) await page.locator('#adv-toggle').click();
await page.waitForTimeout(200);

// CASH SOUL → 0.92 (Memphis soul backbone)
await page.locator('input[oninput*="state.cash.soul"]').evaluate(el => {
  el.value = 0.92; el.dispatchEvent(new Event('input', { bubbles: true }));
});
// CASH SUB → 0.95 (9-string earthquake sub — maximum sub threat)
await page.locator('input[oninput*="state.cash.sub"]').evaluate(el => {
  el.value = 0.95; el.dispatchEvent(new Event('input', { bubbles: true }));
});
// CASH DRUMS → 0.88 (heavy beatdown kick, no trap polish)
await page.locator('input[oninput*="state.cash.drums"]').evaluate(el => {
  el.value = 0.88; el.dispatchEvent(new Event('input', { bubbles: true }));
});
// CASH VINYL → 0.45 (Memphis cassette tape grime)
await page.locator('input[oninput*="state.cash.vinyl"]').evaluate(el => {
  el.value = 0.45; el.dispatchEvent(new Event('input', { bubbles: true }));
});
// LO-FI CRACKLE → 0.5 (heavy tape grime, haunted cassette)
await page.locator('input[oninput*="state.lofi.crackle"]').evaluate(el => {
  el.value = 0.5; el.dispatchEvent(new Event('input', { bubbles: true }));
});
// LO-FI WARMTH → 0.75 (shoegaze reverb wash under the 9-string)
await page.locator('input[oninput*="state.lofi.warmth"]').evaluate(el => {
  el.value = 0.75; el.dispatchEvent(new Event('input', { bubbles: true }));
});
console.log('Advanced: SOUL=0.92 SUB=0.95 DRUMS=0.88 VINYL=0.45 CRACKLE=0.5 WARMTH=0.75');

// ── KS NOTATION (from Guitar Bible) ───────────────────────────────────────────
// Progression roots: C# D C# A G D (power chord roots, all in C# minor)
// Sub register (9-string earthquake): C#2 D2 G#2 A2
//   C#2 = 69.3 Hz — home/concrete impact
//   D2  = 73.4 Hz — panic/knife-twist (minor 2nd above C#)
//   G#2 = 103.8 Hz — perfect fifth of C# (power chord fifth)
//   A2  = 110.0 Hz — tension (D chord fifth)
// Upper register (6-string shoegaze fog):
//   C#3 = 138.6 Hz — octave root
//   E3  = 164.8 Hz — C# minor third
//   G#3 = 207.7 Hz — upper fifth / shoegaze shimmer
//   D3  = 146.8 Hz — minor second color in mid range
const NOTATION = 'C#2 D2 G#2 A2 C#3 E3 G#3 D3';
const moreHdr  = page.locator('#more-opts-hdr');
const moreBody = page.locator('#more-opts-body');
const moreOpen = await moreBody.evaluate(el => el.classList.contains('open'));
if (!moreOpen) await moreHdr.click();
await page.waitForTimeout(200);
await page.locator('#ks-notation').fill(NOTATION);
console.log('KS notation:', NOTATION);
console.log('  C#2=earthquake root  D2=panic knife-twist  G#2=power-chord fifth');
console.log('  A2=D-chord tension   C#3=octave            E3=minor-third');
console.log('  G#3=upper-fifth fog  D3=mid-range color');

// ── BUILD ──────────────────────────────────────────────────────────────────────
console.log('\nBuilding...');
await page.locator('#compile-btn').click();
await page.waitForFunction(
  () => !document.getElementById('compile-btn').disabled,
  { timeout: 30000 }
);
const rms = await page.locator('#meter-rms').textContent();
console.log('RMS:', rms);
if (!rms || rms === '—') throw new Error('BUILD did not populate RMS meter');

// ── EXPORT: BeatStars target (-10 LUFS, 44.1 kHz, 24-bit) ────────────────────
await page.locator('.nav-tab[data-page="export"]').click();
await page.waitForTimeout(500);
await page.locator('input[value="beatstars"]').click();
await page.waitForTimeout(200);

const fname = await page.locator('#filename-preview').textContent();
console.log('Export file:', fname);

const [download] = await Promise.all([
  page.waitForEvent('download', { timeout: 45000 }),
  page.locator('#render-btn').click(),
]);
await download.saveAs(outFile);
console.log('Saved:', outFile);

if (errors.length) console.warn('JS errors:', errors);
else               console.log('No JS errors.');

await browser.close();
server.close();
console.log('\nDone:', outFile);
