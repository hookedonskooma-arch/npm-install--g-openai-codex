/**
 * MELEGI PWA driver — Playwright smoke runner
 * Usage:  node driver.mjs [--screenshot-dir /tmp/shots] [--port 7890]
 * Starts a local static server, drives the app via Playwright, writes PNGs.
 */
import { chromium } from '/opt/node22/lib/node_modules/playwright/index.mjs';
import { createServer } from 'node:http';
import { readFileSync, existsSync, mkdirSync } from 'node:fs';
import { join, extname, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const __dir = dirname(fileURLToPath(import.meta.url));
const APP_ROOT = resolve(__dir, '../../..'); // melegi-app/
const args = process.argv.slice(2);
const sdIdx = args.indexOf('--screenshot-dir');
const ptIdx = args.indexOf('--port');
const shotDir = sdIdx !== -1 ? args[sdIdx + 1] : '/tmp/melegi-screenshots';
const port    = ptIdx !== -1 ? parseInt(args[ptIdx + 1], 10) : 7890;

mkdirSync(shotDir, { recursive: true });

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
  args: ['--no-sandbox', '--disable-dev-shm-usage', '--disable-gpu'],
});
const ctx = await browser.newContext({
  viewport: { width: 1280, height: 800 },
  // Silence Google Fonts SSL error — font falls back to system monospace, fine
  ignoreHTTPSErrors: true,
});
const page = await ctx.newPage();

const errors = [];
page.on('pageerror', e => errors.push(e.message));

const ss = async (name) => {
  const p = join(shotDir, name + '.png');
  await page.screenshot({ path: p });
  console.log('screenshot:', p);
  return p;
};

// ── main flow ──────────────────────────────────────────────────────────────────
await page.goto(`http://127.0.0.1:${port}/`, { waitUntil: 'domcontentloaded' });

// Splash animates for 2.8 s; wait for it then ensure BUILD tab is active
await page.waitForTimeout(3200);
await ss('01-initial');

// Trigger a build
await page.locator('#compile-btn').click();
// Wait for compile-btn to re-enable (BUILD complete)
await page.waitForFunction(() => !document.getElementById('compile-btn').disabled, { timeout: 15000 });
await ss('02-after-build');

// Check meters populated
const rmsText = await page.locator('#meter-rms').textContent();
console.log('RMS meter:', rmsText);
if (!rmsText || rmsText === '—') throw new Error('Build did not populate RMS meter');

// Play
await page.locator('#btn-play').click();
await page.waitForTimeout(800);
await ss('03-playing');
await page.locator('#btn-stop').click();

// Navigate each tab
for (const tab of ['refine','analyze','export']) {
  await page.locator(`.nav-tab[data-page="${tab}"]`).click();
  await page.waitForTimeout(400);
  await ss(`04-${tab}`);
}

// Export page: select Logic target
await page.locator('input[value="logic"]').click();
await page.waitForTimeout(200);
const fname = await page.locator('#filename-preview').textContent();
console.log('Export filename:', fname);
if (!fname || fname === '—') throw new Error('Filename preview not populated');

// ── report ─────────────────────────────────────────────────────────────────────
if (errors.length) {
  console.warn('Page JS errors:', errors);
} else {
  console.log('No page JS errors.');
}
console.log(`\nAll screenshots in: ${shotDir}/`);

await browser.close();
server.close();
