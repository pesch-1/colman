# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) Christoph Pesch 2026
# check_lupdate.cmake — run after building to detect stale translations.
#
# Reads the generated config file (lupdate_check_config.cmake) which
# provides MAIN_TS and MAIN_SOURCES.  Checks whether any source file is
# newer than the TS file and prints a hint if so.  No lupdate invocation —
# timestamps only.

include("${CONFIG_FILE}")

function(_any_source_newer_than ts sources out_var)
  file(TIMESTAMP "${ts}" ts_time "%s" UTC)
  foreach(src ${sources})
    file(TIMESTAMP "${src}" src_time "%s" UTC)
    if(src_time GREATER ts_time)
      set(${out_var} ON PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${out_var} OFF PARENT_SCOPE)
endfunction()

_any_source_newer_than("${MAIN_TS}" "${MAIN_SOURCES}" _main_stale)

if(_main_stale)
  message(STATUS
    "note: translations may be stale — consider to run:\n"
    "    cmake --build \"${BINARY_DIR}\" --target lupdate_colman")
endif()
