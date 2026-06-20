/**
 * MELEGI arrangement driver — THE FLOOR GAVE UP
 * Feeds 9-string C# arrangement into MELEGI, builds, renders, and saves the WAV.
 * Usage: node build-arrangement.mjs [--out /tmp/output.wav] [--port 7890]
 */
import { chromium } from '/opt/node22/lib/node_modules/playwright/index.mjs';
import { createServer } from 'node:http';
import { readFileSync, existsSync, mkdirSync, createWriteStream } from 'node:fs';
import { join, extname, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const __dir = dirname(fileURLToPath(import.meta.url));
const APP_ROOT = resolve(__dir, '../../..'); // melegi-app/
const args = process.argv.slice(2);
const outIdx = args.indexOf('--out');
const ptIdx  = args.indexOf('--port');
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

// ── navigate ───────────────────────────────────────────────────────────────────
await page.goto(`http://127.0.0.1:${port}/`, { waitUntil: 'domcontentloaded' });
console.log('Waiting for splash...');
await page.waitForTimeout(3200);

// ── fill lyrics / prompt ───────────────────────────────────────────────────────
const LYRICS = `THE FLOOR GAVE UP
9-string C# tuning earthquake hardcore.
Progression: C#5 D5 C#5 A5 G5 D5
Memphis horror tape. Hardcore two-step bounce. Mega slow half-time breakdown.
Chopped screwed outro. Sub rumble. No happy chorus. No clean hook.`;

await page.locator('#lyrics-prompt').fill(LYRICS);
console.log('Lyrics/prompt set.');

// ── fill styles ────────────────────────────────────────────────────────────────
const STYLES = 'dark, Memphis horror tape, 9-string earthquake sub bass, hardcore two-step, chopped screwed slowed, 808 kick, half-time breakdown, violent pit-ready';
await page.locator('#styles-prompt').fill(STYLES);
console.log('Styles set.');

// ── set song title ─────────────────────────────────────────────────────────────
await page.locator('#song-title-input').fill('THE FLOOR GAVE UP');
console.log('Song title set.');

// ── open MORE OPTIONS and set notation override ────────────────────────────────
const moreHdr = page.locator('#more-opts-hdr');
const moreBody = page.locator('#more-opts-body');
const isOpen = await moreBody.evaluate(el => el.classList.contains('open'));
if (!isOpen) await moreHdr.click();
await page.waitForTimeout(200);

// C# minor chord voicing spanning the C#→D→A→G→D progression
// C#2 G#2 = C# power chord bass / D2 A2 = D chord / E3 C#3 = upper color
// G#3 D3 = dissonant cluster capturing darkness of the full riff
const NOTATION = 'C#2 G#2 D2 A2 C#3 E3 G#3 D3';
await page.locator('#ks-notation').fill(NOTATION);
console.log('KS notation set:', NOTATION);

// ── activate DARK style chip, deactivate LO-FI and BOOM-BAP ──────────────────
// DARK should already be active; turn off the lighter vibes
const lofiBtnActive = await page.locator('.style-chip[data-style="lo-fi"]').evaluate(el => el.classList.contains('active'));
if (lofiBtnActive) {
  await page.locator('.style-chip[data-style="lo-fi"]').click();
}
const boomBapActive = await page.locator('.style-chip[data-style="boom-bap"]').evaluate(el => el.classList.contains('active'));
if (boomBapActive) {
  await page.locator('.style-chip[data-style="boom-bap"]').click();
}
// Activate SOUL (Memphis soul undertone)
const soulActive = await page.locator('.style-chip[data-style="soul"]').evaluate(el => el.classList.contains('active'));
if (!soulActive) {
  await page.locator('.style-chip[data-style="soul"]').click();
}
console.log('Style chips configured: DARK + SOUL');

// ── BUILD ──────────────────────────────────────────────────────────────────────
console.log('Triggering BUILD...');
await page.locator('#compile-btn').click();
await page.waitForFunction(() => !document.getElementById('compile-btn').disabled, { timeout: 20000 });
console.log('BUILD complete.');

const rms = await page.locator('#meter-rms').textContent();
console.log('RMS meter:', rms);
if (!rms || rms === '—') throw new Error('BUILD did not populate RMS meter');

// ── navigate to EXPORT ─────────────────────────────────────────────────────────
await page.locator('.nav-tab[data-page="export"]').click();
await page.waitForTimeout(500);

// Select BEATSTARS SALE (44.1kHz 24-bit, -10 LUFS)
await page.locator('input[value="beatstars"]').click();
await page.waitForTimeout(200);

// 8 bars is already the default (selected class) — no click needed

const fname = await page.locator('#filename-preview').textContent();
console.log('Export filename preview:', fname);

// ── RENDER & DOWNLOAD ──────────────────────────────────────────────────────────
console.log('Starting render...');
const [download] = await Promise.all([
  page.waitForEvent('download', { timeout: 30000 }),
  page.locator('#render-btn').click(),
]);

console.log('Download received:', download.suggestedFilename());
await download.saveAs(outFile);
console.log('Saved to:', outFile);

// ── report ─────────────────────────────────────────────────────────────────────
if (errors.length) {
  console.warn('Page JS errors:', errors);
} else {
  console.log('No page JS errors.');
}

await browser.close();
server.close();
console.log('\nDone. Output file:', outFile);
