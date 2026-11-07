#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) Christoph Pesch 2026
# colman_create_doc.sh
# Erstellt die Colman-Dokumentation als HTML und PDF.
#
# Voraussetzungen:
#   doxygen >= 1.9   (/usr/bin/doxygen)
#   pdflatex         (texlive)
#   makeindex        (texlive)
#   dot              (graphviz)
#
# Ausgabe:
#   doc/html/index.html  – HTML-Dokumentation
#   doc/colman.pdf       – PDF-Dokumentation

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== Schritt 1/3: Doxygen läuft ==="
doxygen Doxyfile

# echo "=== Schritt 2/3: PDF wird aus LaTeX kompiliert ==="
# cd doc/latex
# make pdf

# echo "=== Schritt 3/3: PDF wird kopiert ==="
# cp refman.pdf ../colman.pdf

echo ""
echo "Fertig."
echo "  HTML: $SCRIPT_DIR/doc/html/index.html"
# echo "  PDF:  $SCRIPT_DIR/doc/colman.pdf"
