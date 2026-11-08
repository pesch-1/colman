# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) Christoph Pesch 2026
#
# deploy_prune.cmake — removes libraries from the deployment AppDir that
# must be taken from the target system (run by the 'deploy' target after
# linuxdeploy):
#
# - GStreamer: a bundled libgstreamer does not find the system's GStreamer
#   plugins (e.g. v4l2src), so Qt Multimedia would see no camera.  Qt's
#   GStreamer media plugin therefore uses the system's GStreamer.
# - GLib: the system's GStreamer plugins must run with the system's GLib,
#   which is backwards compatible and present on every Linux desktop.
#
# Usage: cmake -DAPPDIR=<AppDir> -P deploy_prune.cmake

if(NOT APPDIR)
  message(FATAL_ERROR "APPDIR not set")
endif()

file(GLOB _system_libs
  "${APPDIR}/usr/lib/libgst*.so*"
  "${APPDIR}/usr/lib/libglib-2.0.so*"
  "${APPDIR}/usr/lib/libgobject-2.0.so*"
  "${APPDIR}/usr/lib/libgio-2.0.so*"
  "${APPDIR}/usr/lib/libgmodule-2.0.so*"
  "${APPDIR}/usr/lib/libgthread-2.0.so*"
)
foreach(_lib ${_system_libs})
  get_filename_component(_name "${_lib}" NAME)
  message(STATUS "Using the system's ${_name}")
endforeach()
if(_system_libs)
  file(REMOVE ${_system_libs})
endif()

# Copyright files that linuxdeploy copied for the removed libraries.
file(GLOB _system_docs LIST_DIRECTORIES true
  "${APPDIR}/usr/share/doc/libgstreamer*"
  "${APPDIR}/usr/share/doc/libglib2.0-*"
)
if(_system_docs)
  file(REMOVE_RECURSE ${_system_docs})
endif()
