#!/bin/bash
#
# Alfred Pennyworth — project generator
# Generates AlfredPennyworth.xcodeproj from project.yml using XcodeGen.
#
set -e

echo "==> Alfred Pennyworth setup"

# 1. Xcode command-line tools must be present.
if ! xcode-select -p &> /dev/null; then
  echo "ERROR: Xcode command-line tools not found."
  echo "       Run:  xcode-select --install"
  echo "       Then install Xcode from the Mac App Store and re-run this script."
  exit 1
fi

# 2. Obtain XcodeGen — try in order: already installed, Homebrew, Mint, prebuilt binary.
if command -v xcodegen &> /dev/null; then
  echo "==> XcodeGen already installed ($(xcodegen --version))"
elif command -v brew &> /dev/null; then
  echo "==> Installing XcodeGen via Homebrew..."
  brew install xcodegen
elif command -v mint &> /dev/null; then
  echo "==> Installing XcodeGen via Mint..."
  mint install yonaskolb/XcodeGen
else
  echo "==> Homebrew/Mint not found. Downloading prebuilt XcodeGen..."
  XCG_VERSION="2.42.0"
  TMP="$(mktemp -d)"
  curl -fsSL "https://github.com/yonaskolb/XcodeGen/releases/download/${XCG_VERSION}/xcodegen.zip" -o "${TMP}/xcodegen.zip"
  unzip -q "${TMP}/xcodegen.zip" -d "${TMP}"
  XCODEGEN_BIN="${TMP}/xcodegen/bin/xcodegen"
  chmod +x "${XCODEGEN_BIN}"
  echo "==> Generating project with downloaded XcodeGen..."
  "${XCODEGEN_BIN}" generate
  echo ""
  echo "==> Done. Open AlfredPennyworth.xcodeproj in Xcode."
  echo "    Next: set your Team under Signing & Capabilities, then Product > Build."
  exit 0
fi

echo "==> Generating AlfredPennyworth.xcodeproj..."
xcodegen generate

echo ""
echo "==> Done. Open AlfredPennyworth.xcodeproj in Xcode."
echo "    Next: set your Team under Signing & Capabilities, then Product > Build."
echo "    See BUILD.md for the full walkthrough."
