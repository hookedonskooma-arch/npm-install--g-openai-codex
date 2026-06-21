/**
 * MELEGI 8-bar guitar riff — Guitar Bible 9-string C# earthquake
 *
 * Progression (4 × 2 bars): C# → D → C# → A
 * Parameters derived from the MELEGI 9-String Guitar Bible:
 *   - Tuning:       9-string low C# (C# F# B E A D G B E)
 *   - Progression:  C#5 → D5 → C#5 → A5  (Guitar Bible opening riff)
 *   - Voicings:     root + fifth, sub (2nd oct) + mid (3rd oct)
 *   - Guitar-first: cash.drums=0, cash.soul=1.0  — pure KS guitar texture
 *   - BPM:          75  (Guitar Bible half-time breakdown tempo)
 *   - Duration:     8 bars × 4 beats × (60/75s) = 25.6s
 *
 * Usage:
 *   node guitar-riff.mjs
 *   node guitar-riff.mjs --out /tmp/my_riff.wav --port 7903
 */
import { chromium } from '/opt/node22/lib/node_modules/playwright/index.mjs';
import { createServer } from 'node:http';
import { readFileSync, writeFileSync, existsSync } from 'node:fs';
import { join, extname, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const __dir    = dirname(fileURLToPath(import.meta.url));
const APP_ROOT = resolve(__dir, '../../..'); // melegi-app/
const args     = process.argv.slice(2);
const outIdx   = args.indexOf('--out');
const ptIdx    = args.indexOf('--port');
const OUT_FILE = outIdx !== -1 ? args[outIdx + 1] : '/tmp/guitar_riff_8bar.wav';
const PORT     = ptIdx  !== -1 ? parseInt(args[ptIdx + 1], 10) : 7903;

const MIME = {
  '.html':'text/html','.js':'application/javascript',
  '.css':'text/css','.png':'image/png','.webmanifest':'application/manifest+json',
  '.json':'application/json','.ico':'image/x-icon',
};

// Guitar Bible 8-bar riff: C# earthquake → D panic → C# return → A tension
// Voicings: root + fifth, sub (2nd oct) + mid (3rd oct)
const PROGRESSION = [
  { name:'C#', notation:'C#2 G#2 C#3 G#3', label:'earthquake home' },
  { name:'D',  notation:'D2 A2 D3 A3',     label:'panic/knife-twist' },
  { name:'C#', notation:'C#2 G#2 C#3 G#3', label:'return' },
  { name:'A',  notation:'A2 E3 A3',         label:'tension/resolve' },
];

// ── static server ───────────────────────────────────────────────────────────
const server = createServer((req, res) => {
  const safePath = req.url.split('?')[0].replace(/\.\./g, '');
  const filePath = join(APP_ROOT, safePath === '/' ? '/index.html' : safePath);
  if (!existsSync(filePath)) { res.writeHead(404); res.end('Not found'); return; }
  res.writeHead(200, { 'Content-Type': MIME[extname(filePath)] ?? 'application/octet-stream' });
  res.end(readFileSync(filePath));
});
await new Promise(r => server.listen(PORT, '127.0.0.1', r));
console.log(`Server: http://127.0.0.1:${PORT}/`);

const browser = await chromium.launch({
  executablePath: '/opt/pw-browsers/chromium-1194/chrome-linux/chrome',
  args: ['--no-sandbox', '--disable-dev-shm-usage', '--disable-gpu'],
});
const ctx = await browser.newContext({
  viewport: { width: 1280, height: 800 },
  ignoreHTTPSErrors: true,
  acceptDownloads: true,
});

// WAV helpers — standard 44-byte PCM header
function readWavPCM(buf) { return buf.slice(44); }

function makeWavHeader(pcmByteLen, sr, channels, bitsPerSample) {
  const byteRate   = sr * channels * bitsPerSample / 8;
  const blockAlign = channels * bitsPerSample / 8;
  const hdr = Buffer.alloc(44);
  hdr.write('RIFF', 0);
  hdr.writeUInt32LE(36 + pcmByteLen, 4);
  hdr.write('WAVE', 8);
  hdr.write('fmt ', 12);
  hdr.writeUInt32LE(16, 16);
  hdr.writeUInt16LE(1, 20);              // PCM
  hdr.writeUInt16LE(channels, 22);
  hdr.writeUInt32LE(sr, 24);
  hdr.writeUInt32LE(byteRate, 28);
  hdr.writeUInt16LE(blockAlign, 32);
  hdr.writeUInt16LE(bitsPerSample, 34);
  hdr.write('data', 36);
  hdr.writeUInt32LE(pcmByteLen, 40);
  return hdr;
}

const pcmChunks = [];
let wavMeta = null;

for (const [i, chord] of PROGRESSION.entries()) {
  console.log(`\n── [${i+1}/${PROGRESSION.length}] ${chord.name} (${chord.label}) bars ${i*2+1}-${i*2+2}`);

  const page = await ctx.newPage();
  const errors = [];
  page.on('pageerror', e => errors.push(e.message));

  await page.goto(`http://127.0.0.1:${PORT}/`, { waitUntil: 'domcontentloaded' });
  await page.waitForTimeout(3200); // splash animation

  // Song title
  await page.locator('#song-title-input').fill(`9STR RIFF ${chord.name}`);

  // BPM: 75 (Guitar Bible half-time breakdown)
  await page.locator('input[oninput*="state.bpm"]').evaluate(el => {
    el.value = 75; el.dispatchEvent(new Event('input', { bubbles: true }));
  });

  // BARS: 4 per render (minimum for export pill; we trim to 2 bars in stitch step)
  await page.locator('input[oninput*="state.bars"]').evaluate(el => {
    el.value = 4; el.dispatchEvent(new Event('input', { bubbles: true }));
  });

  // Style chips: DARK only (raw earthquake, no Memphis colour this time)
  const ALL_CHIPS = ['dark','lo-fi','boom-bap','dusty','soul','trap','jazz','ambient'];
  const WANT_ON   = new Set(['dark']);
  for (const style of ALL_CHIPS) {
    const chip = page.locator(`.style-chip[data-style="${style}"]`);
    if (!await chip.count()) continue;
    const isActive = await chip.evaluate(el => el.classList.contains('active'));
    if (WANT_ON.has(style) && !isActive) await chip.click();
    if (!WANT_ON.has(style) && isActive) await chip.click();
  }

  // Advanced panel
  const advVisible = await page.locator('#adv-panel').evaluate(el => el.style.display !== 'none');
  if (!advVisible) await page.locator('#adv-toggle').click();
  await page.waitForTimeout(200);

  // Guitar-first mix: max SOUL (KS layer), zero DRUMS, moderate SUB
  await page.locator('input[oninput*="state.cash.soul"]').evaluate(el => {
    el.value = 1.0; el.dispatchEvent(new Event('input', { bubbles: true }));
  });
  await page.locator('input[oninput*="state.cash.sub"]').evaluate(el => {
    el.value = 0.65; el.dispatchEvent(new Event('input', { bubbles: true }));
  });
  await page.locator('input[oninput*="state.cash.drums"]').evaluate(el => {
    el.value = 0.0; el.dispatchEvent(new Event('input', { bubbles: true }));
  });
  await page.locator('input[oninput*="state.cash.vinyl"]').evaluate(el => {
    el.value = 0.25; el.dispatchEvent(new Event('input', { bubbles: true }));
  });
  await page.locator('input[oninput*="state.lofi.warmth"]').evaluate(el => {
    el.value = 0.70; el.dispatchEvent(new Event('input', { bubbles: true }));
  });
  await page.locator('input[oninput*="state.lofi.crackle"]').evaluate(el => {
    el.value = 0.10; el.dispatchEvent(new Event('input', { bubbles: true }));
  });

  // KS notation for this chord
  const moreBody = page.locator('#more-opts-body');
  const moreOpen = await moreBody.evaluate(el => el.classList.contains('open'));
  if (!moreOpen) await page.locator('#more-opts-hdr').click();
  await page.waitForTimeout(200);
  await page.locator('#ks-notation').fill(chord.notation);
  console.log(`  KS: ${chord.notation}`);

  // BUILD
  await page.locator('#compile-btn').click();
  await page.waitForFunction(
    () => !document.getElementById('compile-btn').disabled,
    { timeout: 30000 }
  );
  const rms = await page.locator('#meter-rms').textContent();
  if (!rms || rms === '—') throw new Error(`BUILD failed for chord ${chord.name}`);
  console.log(`  RMS: ${rms}`);

  // EXPORT → BeatStars, 4 BARS (minimum available pill; trimmed to 2 bars in stitch step)
  await page.locator('.nav-tab[data-page="export"]').click();
  await page.waitForTimeout(500);
  await page.locator('input[value="beatstars"]').click();
  await page.waitForTimeout(200);
  const fourBarPill = page.locator('.radio-pill[data-bars="4"]');
  if (await fourBarPill.count()) await fourBarPill.click();
  await page.waitForTimeout(200);

  const [download] = await Promise.all([
    page.waitForEvent('download', { timeout: 45000 }),
    page.locator('#render-btn').click(),
  ]);
  const tmpPath = `/tmp/riff_chord_${i}_${chord.name.replace('#','s')}.wav`;
  await download.saveAs(tmpPath);
  console.log(`  Clip saved: ${tmpPath}`);

  if (errors.length) console.warn(`  JS errors: ${errors.join('; ')}`);

  // Extract PCM and remember header info from first clip
  const wavBuf = readFileSync(tmpPath);
  if (i === 0) {
    wavMeta = {
      sr:           wavBuf.readUInt32LE(24),
      channels:     wavBuf.readUInt16LE(22),
      bitsPerSample:wavBuf.readUInt16LE(34),
    };
    console.log(`  WAV format: ${wavMeta.sr} Hz, ${wavMeta.channels}ch, ${wavMeta.bitsPerSample}-bit`);
  }
  // Trim to exactly 2 bars: 2 bars × 4 beats × (60/75s) = 6.4s
  const twoBarSamples = Math.floor(2 * 4 * (60 / 75) * wavMeta.sr);
  const bytesPerSample = wavMeta.channels * wavMeta.bitsPerSample / 8;
  const twoBarBytes   = twoBarSamples * bytesPerSample;
  const fullPCM = readWavPCM(wavBuf);
  pcmChunks.push(fullPCM.slice(0, twoBarBytes));
  console.log(`  Trimmed: ${twoBarSamples} samples (${(twoBarBytes/1024).toFixed(0)} KB, 2 bars)`);


  await page.close();
}

// Stitch 4 × 2-bar clips → 8-bar riff
console.log('\n── Stitching 4 clips into 8-bar riff...');
const totalPCM     = Buffer.concat(pcmChunks);
const durationSec  = totalPCM.length / (wavMeta.sr * wavMeta.channels * wavMeta.bitsPerSample / 8);
console.log(`  Total PCM: ${totalPCM.length} bytes → ${durationSec.toFixed(2)}s (expect ~25.6s: 4 chords × 2 bars at 75 BPM)`);

const outHeader = makeWavHeader(totalPCM.length, wavMeta.sr, wavMeta.channels, wavMeta.bitsPerSample);
writeFileSync(OUT_FILE, Buffer.concat([outHeader, totalPCM]));
console.log(`\nDone: ${OUT_FILE}`);
console.log('  Progression: C# (bars 1-2) → D (3-4) → C# (5-6) → A (7-8)');
console.log('  Guitar Bible: earthquake root → panic/knife-twist → return → tension');

await browser.close();
server.close();
