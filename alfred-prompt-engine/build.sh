#!/bin/bash
set -e
echo "==> Building Alfred Prompt Engine..."

# Make sure Xcode CLT is present
if ! xcode-select -p &>/dev/null; then
  echo "ERROR: Run 'xcode-select --install' first."
  exit 1
fi

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

echo ""
echo "==> Done. Plugin copied to your AU/VST3 folder automatically."
echo "    Restart Logic Pro / your DAW to pick it up."
