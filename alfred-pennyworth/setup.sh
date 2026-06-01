#!/bin/bash
set -e
echo "Setting up Alfred Pennyworth..."
if ! command -v xcodegen &> /dev/null; then
  echo "Installing XcodeGen..."
  brew install xcodegen
fi
xcodegen generate
echo "Done. Open AlfredPennyworth.xcodeproj in Xcode."
echo "Set your Team in Signing & Capabilities, then build."
