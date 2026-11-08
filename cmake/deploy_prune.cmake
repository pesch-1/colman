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
# It also removes the Qt SQL driver plugins, which colman does not use (it
# has its own SQLite driver); linuxdeploy would otherwise bundle whatever
# drivers are installed, including client libraries such as libmysqlclient.
#
# With -DSERVER=ON (REST server bundle) the user interface and image format
# plugins are removed as well — the server is a QCoreApplication and decodes
# no images — and then every bundled library that the server and the
# remaining plugins do not need.  libssl is kept: Qt's OpenSSL TLS backend
# loads it at run time.
#
# Usage: cmake -DAPPDIR=<AppDir> [-DSERVER=ON] -P deploy_prune.cmake

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

# Qt SQL drivers: not used by colman.
file(REMOVE_RECURSE "${APPDIR}/usr/plugins/sqldrivers")

if(NOT SERVER)
  return()
endif()

# REST server: plugins it does not need.
foreach(_type platforms platforminputcontexts xcbglintegrations
        wayland-decoration-client wayland-graphics-integration-client
        wayland-shell-integration imageformats iconengines styles
        egldeviceintegrations)
  file(REMOVE_RECURSE "${APPDIR}/usr/plugins/${_type}")
endforeach()

# Keep only the libraries the server and the remaining plugins need.
set(CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM "linux+elf")
set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL "objdump")
find_program(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND objdump REQUIRED)
file(GLOB _server_programs "${APPDIR}/usr/bin/colman_rest_server")
file(GLOB_RECURSE _server_plugins "${APPDIR}/usr/plugins/*.so")
file(GET_RUNTIME_DEPENDENCIES
  EXECUTABLES ${_server_programs}
  MODULES ${_server_plugins}
  DIRECTORIES "${APPDIR}/usr/lib"
  RESOLVED_DEPENDENCIES_VAR _needed
  UNRESOLVED_DEPENDENCIES_VAR _unresolved
)
set(_needed_names "")
foreach(_dep ${_needed})
  get_filename_component(_name "${_dep}" NAME)
  list(APPEND _needed_names "${_name}")
endforeach()
file(GLOB _bundled LIST_DIRECTORIES false "${APPDIR}/usr/lib/*.so*")
set(_removed 0)
foreach(_lib ${_bundled})
  get_filename_component(_name "${_lib}" NAME)
  if(_name MATCHES "^libssl\\.so" OR _name IN_LIST _needed_names)
    continue()
  endif()
  file(REMOVE "${_lib}")
  math(EXPR _removed "${_removed} + 1")
endforeach()
message(STATUS "Removed ${_removed} libraries the REST server does not need")
