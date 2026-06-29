#!/usr/bin/env bash
# ASHEN FRONTIER — Full pipeline runner
# Usage: OPENAI_API_KEY=sk-... ELEVENLABS_API_KEY=... ./render.sh
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

check_key() {
  local var="$1"
  if [ -z "${!var:-}" ]; then
    echo "ERROR: $var is not set. Export it and re-run."
    exit 1
  fi
}

echo "══════════════════════════════════════"
echo " ASHEN FRONTIER — Cinematic Trailer"
echo "══════════════════════════════════════"
echo ""

check_key OPENAI_API_KEY
check_key ELEVENLABS_API_KEY

# ── Step 1: DALL-E shots ──────────────────────────────────────────────────────
echo "[1/4] Generating DALL-E shots..."
cd "$REPO_ROOT"
pip install requests -q
python3 generate_shots.py

# ── Step 2: ElevenLabs audio ──────────────────────────────────────────────────
echo ""
echo "[2/4] Generating ElevenLabs audio..."
pip install pydub -q
python3 generate_audio.py

# ── Step 3: Remotion render ───────────────────────────────────────────────────
echo ""
echo "[3/4] Building Remotion composition..."
cd "$REPO_ROOT/remotion-composer"
npm install --silent

# Remotion needs a public/ dir to serve static assets from
mkdir -p public
ln -sf "$REPO_ROOT/assets/images" public/images 2>/dev/null || true
ln -sf "$REPO_ROOT/assets/audio"  public/audio  2>/dev/null || true

echo "Rendering trailer (this takes a few minutes)..."
npx remotion render AshenFrontierTrailer "$REPO_ROOT/out/trailer_raw.mp4" \
  --concurrency=2 \
  --log=verbose

# ── Step 4: FFmpeg post-process ───────────────────────────────────────────────
echo ""
echo "[4/4] FFmpeg post-process (film grain pass + loudness)..."
cd "$REPO_ROOT"

ffmpeg -y \
  -i out/trailer_raw.mp4 \
  -vf "noise=alls=8:allf=t+u,eq=contrast=1.05:saturation=0.3,vignette=PI/4" \
  -af "loudnorm=I=-16:LRA=11:TP=-1.5" \
  -c:v libx264 -preset slow -crf 18 -pix_fmt yuv420p \
  -c:a aac -b:a 192k \
  out/trailer.mp4

echo ""
echo "══════════════════════════════════════"
echo " DONE → out/trailer.mp4"
echo "══════════════════════════════════════"
