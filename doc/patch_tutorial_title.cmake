# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) Christoph Pesch 2026
# Patches the Doxygen-generated LaTeX output before pdflatex runs.
#
# 1. Inserts the colman logo on the title page of refman.tex and
#    copies the logo JPEG into the latex output directory.
#
# 2. Patches doxygen.sty to load adjustbox with the 'export' option
#    so that 'max width' becomes a valid \includegraphics key.
#
# 3. Patches all tutorial_*.tex content files: replaces Doxygen's
#    hardcoded 'width=\textwidth' with 'max width=\textwidth' so that
#    images narrower than the text width keep their natural size
#    instead of being upscaled.
#
# Required variables (pass via -D):
#   REFMAN_TEX  — full path to refman.tex
#   LOGO_SRC    — full path to source colman_logo.jpeg
#   LOGO_DST    — full path to destination colman_logo.jpeg

get_filename_component(LATEX_DIR "${REFMAN_TEX}" DIRECTORY)

# --- 1. Title page logo ---
file(READ "${REFMAN_TEX}" content)
string(REPLACE
  "  \\begin{titlepage}\n  \\vspace*{7cm}\n  \\begin{center}%"
  "  \\begin{titlepage}\n  \\begin{center}%\n  \\includegraphics[height=5cm]{colman_logo}\\\\[1.5cm]"
  content "${content}"
)
file(WRITE "${REFMAN_TEX}" "${content}")
file(COPY_FILE "${LOGO_SRC}" "${LOGO_DST}")

# --- 2. Enable adjustbox 'max width' key in \includegraphics ---
file(READ "${LATEX_DIR}/doxygen.sty" doxy_content)
string(REPLACE
  [[RequirePackage{adjustbox}]]
  [[RequirePackage[export]{adjustbox}]]
  doxy_content "${doxy_content}"
)
file(WRITE "${LATEX_DIR}/doxygen.sty" "${doxy_content}")

# --- 3. Replace hardcoded width=\textwidth with max width=\textwidth ---
file(GLOB CONTENT_TEX_FILES "${LATEX_DIR}/tutorial_*.tex")
foreach(TEX_FILE ${CONTENT_TEX_FILES})
  file(READ "${TEX_FILE}" tex_content)
  string(REPLACE
    [[width=\textwidth,height=\textheight/2,keepaspectratio=true]]
    [[max width=\textwidth,max totalheight=0.5\textheight,keepaspectratio=true]]
    tex_content "${tex_content}"
  )
  file(WRITE "${TEX_FILE}" "${tex_content}")
endforeach()
