#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) Christoph Pesch 2026
# Render the current main-window UI offscreen and compare every page against
# the committed pixel baseline in tools/ui_baseline/.  Used to guard the
# hand-written main-window UI against layout regressions during refactoring.
#
# Usage: run from the project root after building ./bld/colman
#   sh tools/ui_check.sh
# Exit status 0 if all pages are bit-identical to the baseline, 1 otherwise.
set -e
root=$(pwd)
base="$root/tools/ui_baseline"
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT

QT_QPA_PLATFORM=offscreen "$root/bld/colman" --ui-snapshot="$out" >/dev/null 2>&1

diffs=$(diff <(cd "$base" && md5sum *.png) <(cd "$out" && md5sum *.png) || true)
if [ -z "$diffs" ]; then
  echo "UI snapshots identical to baseline ($(ls "$out"/*.png | wc -l) pages)"
  exit 0
fi
echo "UI snapshots DIFFER from baseline:"
echo "$diffs" | grep '^[<>]' | awk '{print $1, $3}' | sort -u
exit 1
