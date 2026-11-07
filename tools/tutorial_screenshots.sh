#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) Christoph Pesch 2026
# Render the tutorial screenshots for one language.
#
# Builds a sample collection from scratch (doc/tutorial/<lang>/sample_data.json),
# drives colman through the tutorial and writes one JPEG per tutorial image
# into doc/tutorial/<lang>/screenshots/.  The UI language follows the locale
# set here.  Settings of the run go to a temporary directory; the user's own
# colman configuration is not touched.
#
# Usage: run from the project root after building ./bld/colman
#   tools/tutorial_screenshots.sh --lang de|en [--out DIR] [--display]
#                                 [--network] [--build-dir DIR]
#
#   --lang LANG      language of UI and sample data (required)
#   --out DIR        output directory (default doc/tutorial/LANG/screenshots)
#   --display        render on the real desktop (native style) instead of
#                    the offscreen platform; do not touch mouse/keyboard
#   --network        also take screenshots that need internet access
#   --build-dir DIR  build directory containing colman (default bld)
set -e
root=$(cd "$(dirname "$0")/.." && pwd)
lang=
out=
display=
network=
build_dir="$root/bld"

usage() {
  sed -n '12,21p' "$0" | sed 's/^# \{0,1\}//'
  exit "$1"
}

while [ $# -gt 0 ]; do
  case "$1" in
    --lang) lang=$2; shift 2 ;;
    --out) out=$2; shift 2 ;;
    --display) display=1; shift ;;
    --network) network=--tutorial-network; shift ;;
    --build-dir) build_dir=$2; shift 2 ;;
    -h|--help) usage 0 ;;
    *) echo "unknown argument: $1" >&2; usage 1 ;;
  esac
done

case "$lang" in
  de) locale_name=de_DE.UTF-8 ;;
  en) locale_name=en_US.UTF-8 ;;
  "") echo "--lang is required" >&2; usage 1 ;;
  *) echo "unsupported language: $lang" >&2; exit 1 ;;
esac

data="$root/doc/tutorial/$lang/sample_data.json"
if [ ! -f "$data" ]; then
  echo "missing sample data: $data" >&2
  exit 1
fi
[ -n "$out" ] || out="$root/doc/tutorial/$lang/screenshots"
mkdir -p "$out"

# The locale must exist, otherwise Qt falls back to "C" (English UI and
# date formats).  English is the source language, so C.UTF-8 will do.
normalized=$(echo "$locale_name" | sed 's/UTF-8/utf8/')
if ! locale -a | grep -qx "$normalized"; then
  if [ "$lang" = en ]; then
    locale_name=C.UTF-8
  else
    echo "locale $locale_name is not installed (see locale-gen)" >&2
    exit 1
  fi
fi

if [ -z "$display" ]; then
  export QT_QPA_PLATFORM=offscreen
fi
export LC_ALL=$locale_name
export LANG=$locale_name
export LANGUAGE=$lang

exec "$build_dir/colman" --tutorial-screenshots="$out" \
  --tutorial-data="$data" $network
