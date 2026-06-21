#!/bin/bash
set -euo pipefail

# Only run in remote (Claude Code on the web) environments
if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
  exit 0
fi

echo "[session-start] Installing Python dependencies..."
pip install -r "$CLAUDE_PROJECT_DIR/requirements.txt" --quiet

# Build MELEGI C++ engine on first run (cached after that).
# Soft-fail: a broken C++ build should not block the bot or PWA.
MELEGI_BINARY="$CLAUDE_PROJECT_DIR/melegi/build/melegi_pipeline"
if [ ! -f "$MELEGI_BINARY" ]; then
  echo "[session-start] Building MELEGI C++ engine (first run)..."
  if cmake -B "$CLAUDE_PROJECT_DIR/melegi/build" \
           -S "$CLAUDE_PROJECT_DIR/melegi" \
           -DCMAKE_BUILD_TYPE=Release \
           -Wno-dev 2>&1 \
     && cmake --build "$CLAUDE_PROJECT_DIR/melegi/build" -j"$(nproc)" 2>&1; then
    echo "[session-start] MELEGI C++ engine built."
  else
    echo "[session-start] WARNING: MELEGI C++ build failed — native engine unavailable. Bot and PWA unaffected."
  fi
else
  echo "[session-start] MELEGI C++ engine binary already present, skipping build."
fi

echo "[session-start] Done."
