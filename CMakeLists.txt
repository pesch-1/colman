# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) Christoph Pesch 2026
cmake_minimum_required(VERSION 3.21)
project(colman VERSION 2.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wall -Wextra -Wno-psabi)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "ccache found: ${CCACHE_PROGRAM}")
  set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
else()
  message(STATUS "ccache not found — building without cache")
endif()

# Documentation only: configure just the doc and tutorial targets, without
# Qt or any other build dependency (used by the GitHub Pages workflow).
option(COLMAN_DOCS_ONLY
  "Configure only the documentation targets (doc, tutorial)" OFF)
if(COLMAN_DOCS_ONLY)
  include(cmake/colman_docs.cmake)
  return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Required for all targets including the headless REST server.
find_package(Qt6 REQUIRED COMPONENTS
  Core
  Concurrent
  Gui
  HttpServer
  Svg
  SvgWidgets
  Widgets
  PrintSupport
  Sql
  Network
  Xml
  Test
  LinguistTools
)

# Required for the desktop app and REST client; not needed on headless servers.
find_package(Qt6 COMPONENTS
  Multimedia
  WebChannel
  WebEngineWidgets
)
find_package(ZXing)

if(Qt6Multimedia_FOUND AND Qt6WebChannel_FOUND AND Qt6WebEngineWidgets_FOUND
   AND ZXing_FOUND)
  set(COLMAN_BUILD_GUI TRUE)
else()
  message(STATUS
    "Qt6 Multimedia/WebChannel/WebEngineWidgets or ZXing not found — "
    "skipping desktop app and REST client")
  set(COLMAN_BUILD_GUI FALSE)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(Argon2 REQUIRED libargon2)

find_package(OpenSSL REQUIRED)

# jwt-cpp: header-only JWT validation library (used by the REST server).
include(FetchContent)
FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
  GIT_TAG        v0.7.2
)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JWT_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(jwt-cpp)

# SQLite3MultipleCiphers: SQLite with encryption, used by colman's built-in
# Qt SQL driver (colman_sqlite_driver.cpp).  Linked statically with hidden
# symbols, so it cannot clash with a system libsqlite3 in the same process.
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()
FetchContent_Declare(
  sqlite3mc
  URL      https://github.com/utelle/SQLite3MultipleCiphers/releases/download/v2.6.1/sqlite3mc-2.6.1-sqlite-3.54.0-amalgamation.zip
  URL_HASH SHA256=3f25af603566a23c821d8e581778b5ece2a541d2435776de57b8d845c176815a
)
FetchContent_MakeAvailable(sqlite3mc)
find_package(Threads REQUIRED)
add_library(colman_sqlite3mc STATIC
  ${sqlite3mc_SOURCE_DIR}/sqlite3mc_amalgamation.c
)
target_include_directories(colman_sqlite3mc SYSTEM PUBLIC
  ${sqlite3mc_SOURCE_DIR}
)
target_compile_definitions(colman_sqlite3mc PRIVATE
  CODEC_TYPE=CODEC_TYPE_CHACHA20
  SQLITE_THREADSAFE=1
  SQLITE_ENABLE_FTS5=1
  SQLITE_TEMP_STORE=2
  SQLITE_SECURE_DELETE=1
  SQLITE_OMIT_LOAD_EXTENSION=1
)
# Third-party code: do not report its warnings.
target_compile_options(colman_sqlite3mc PRIVATE
  $<IF:$<C_COMPILER_ID:MSVC>,/w,-w>
)
set_target_properties(colman_sqlite3mc PROPERTIES C_VISIBILITY_PRESET hidden)
target_link_libraries(colman_sqlite3mc PUBLIC Threads::Threads)
if(UNIX)
  target_link_libraries(colman_sqlite3mc PUBLIC m)
endif()

enable_testing()

# Headers live in src/; tests in tests/ include them by plain file name.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Non-UI sources shared between application and tests.
set(COLMAN_CORE_SOURCES
  src/colman_acquisition.cpp
  src/colman_acquisition.hpp
  src/colman_actions.cpp
  src/colman_actions.hpp
  src/colman_assert.cpp
  src/colman_assert.hpp
  src/colman_assert_context.cpp
  src/colman_assert_context.hpp
  src/colman_collection.cpp
  src/colman_collection.hpp
  src/colman_error.cpp
  src/colman_error.hpp
  src/colman_trace.cpp
  src/colman_trace.hpp
  src/colman_id_card_layout.hpp
  src/colman_bgg_queries.cpp
  src/colman_bgg_queries.hpp
  src/colman_opengtindb.cpp
  src/colman_opengtindb.hpp
  src/colman_ean_queries.cpp
  src/colman_ean_queries.hpp
  src/colman_isbn_queries.cpp
  src/colman_isbn_queries.hpp
  src/colman_name_queries.cpp
  src/colman_name_queries.hpp
  src/colman_sru_server.cpp
  src/colman_sru_server.hpp
  src/colman_item.cpp
  src/colman_item.hpp
  src/colman_json.cpp
  src/colman_json.hpp
  src/colman_lending.cpp
  src/colman_lending.hpp
  src/colman_liquidation.cpp
  src/colman_liquidation.hpp
  src/colman_passwords.cpp
  src/colman_passwords.hpp
  src/colman_person.cpp
  src/colman_person.hpp
  src/colman_return.cpp
  src/colman_return.hpp
  src/colman_db_util.cpp
  src/colman_db_util.hpp
  src/colman_sql_queries.cpp
  src/colman_sql_queries.hpp
  src/colman_sql_queries_impl.hpp
  src/colman_sql_queries_history.cpp
  src/colman_sql_queries_history.hpp
  src/colman_sql_queries_items.cpp
  src/colman_sql_queries_items.hpp
  src/colman_sql_queries_movements.cpp
  src/colman_sql_queries_movements.hpp
  src/colman_sql_queries_persons.cpp
  src/colman_sql_queries_persons.hpp
  src/colman_sql_queries_types.cpp
  src/colman_sql_queries_types.hpp
  src/colman_type.cpp
  src/colman_type.hpp
  src/colman_media_kind.hpp
  src/colman_marc21.cpp
  src/colman_marc21.hpp
  src/colman_permission_deps.cpp
  src/colman_permission_deps.hpp
  src/colman_sqlite_driver.cpp
  src/colman_sqlite_driver.hpp
)

add_library(colman_core STATIC ${COLMAN_CORE_SOURCES})

target_include_directories(colman_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(colman_core PUBLIC QT_NO_KEYWORDS
  COLMAN_VERSION="${PROJECT_VERSION}")
target_link_libraries(colman_core
  Qt6::Concurrent
  Qt6::Core
  Qt6::Gui
  Qt6::Network
  Qt6::Sql
  Qt6::Widgets
  Qt6::Xml
  ${Argon2_LIBRARIES}
  colman_sqlite3mc
)

# Owl icon set — shared by all GUI applications.
qt6_add_resources(COLMAN_OWL_RESOURCES src/colman_owls.qrc)

# Third-party logos (e.g. "Powered by BGG") — included in the main app.
qt6_add_resources(COLMAN_THIRDPARTY_RESOURCES src/colman_thirdparty.qrc)

set(COLMAN_APP_SOURCES
  src/colman_camera_scan_page.cpp
  src/colman_camera_scan_page.hpp
  src/colman_central_page.cpp
  src/colman_central_page.hpp
  src/colman_tile_button.cpp
  src/colman_tile_button.hpp
  src/colman_anonymous_variable.hpp
  src/colman_msgbox.cpp
  src/colman_msgbox.hpp
  src/colman_app_data.cpp
  src/colman_app_data.hpp
  src/colman_main_window_ui.cpp
  src/colman_main_window_ui.hpp
  src/colman_main_window_ui_management.cpp
  src/colman_main_window_ui_lending.cpp
  src/colman_main_window_ui_group_lending.cpp
  src/colman_main_window_ui_return.cpp
  src/colman_main_window_ui_acquisition.cpp
  src/colman_main_window_ui_liquidation.cpp
  src/colman_ui_build.cpp
  src/colman_ui_build.hpp
  src/colman_ui_snapshot.cpp
  src/colman_ui_snapshot.hpp
  src/colman_clear_selection_filter.cpp
  src/colman_clear_selection_filter.hpp
  src/colman_code128_item.cpp
  src/colman_code128_item.hpp
  src/colman_card_rendering.cpp
  src/colman_card_rendering.hpp
  src/colman_card_batch_print.cpp
  src/colman_card_batch_print.hpp
  src/colman_create_collection_page.cpp
  src/colman_create_collection_page.hpp
  src/colman_encryption_password_widget.cpp
  src/colman_encryption_password_widget.hpp
  src/colman_acquisition_page.cpp
  src/colman_acquisition_page.hpp
  src/colman_lending_page.cpp
  src/colman_lending_page.hpp
  src/colman_group_lending_page.cpp
  src/colman_group_lending_page.hpp
  src/colman_liquidation_page.cpp
  src/colman_liquidation_page.hpp
  src/colman_return_page.cpp
  src/colman_return_page.hpp
  src/colman_here.hpp
  src/colman_history_page.cpp
  src/colman_history_page.hpp
  src/colman_extend_lending_dialog.cpp
  src/colman_extend_lending_dialog.hpp
  src/colman_item_barcode_page.cpp
  src/colman_item_barcode_page.hpp
  src/colman_item_table.cpp
  src/colman_item_table.hpp
  src/colman_item_type_page.cpp
  src/colman_item_type_page.hpp
  src/colman_amazon_browser.cpp
  src/colman_amazon_browser.hpp
  src/colman_item_widget.cpp
  src/colman_item_widget.hpp
  src/colman_item_widget_ui.cpp
  src/colman_item_widget_ui.hpp
  src/colman_list_value_widget.cpp
  src/colman_list_value_widget.hpp
  src/colman_on_exit.hpp
  src/colman_page_layout.cpp
  src/colman_page_layout.hpp
  src/colman_partial_date_edit.cpp
  src/colman_partial_date_edit.hpp
  src/colman_printer_calibration.cpp
  src/colman_printer_calibration.hpp
  src/colman_printer_calibration_widget.cpp
  src/colman_printer_calibration_widget.hpp
  src/colman_label_layout_widget.cpp
  src/colman_label_layout_widget.hpp
  src/colman_label_printer.cpp
  src/colman_label_printer.hpp
  src/colman_label_selection_widget.cpp
  src/colman_label_selection_widget.hpp
  src/colman_card_element_item.cpp
  src/colman_card_element_item.hpp
  src/colman_card_editor_base.cpp
  src/colman_card_editor_base.hpp
  src/colman_id_editor.cpp
  src/colman_id_editor.hpp
  src/colman_item_card_editor.cpp
  src/colman_item_card_editor.hpp
  src/colman_signature_label_editor.cpp
  src/colman_signature_label_editor.hpp
  src/colman_signature_label_layout.hpp
  src/colman_signature_print_page.cpp
  src/colman_signature_print_page.hpp
  src/colman_person_barcode_page.cpp
  src/colman_person_barcode_page.hpp
  src/colman_person_list_widget.cpp
  src/colman_person_list_widget.hpp
  src/colman_person_check_list_widget.cpp
  src/colman_person_check_list_widget.hpp
  src/colman_person_page.cpp
  src/colman_person_page.hpp
  src/colman_person_view_page.cpp
  src/colman_person_view_page.hpp
  src/colman_person_widget.cpp
  src/colman_person_widget.hpp
  src/colman_preferences_page.cpp
  src/colman_preferences_page.hpp
  src/colman_preview_file_dialog.cpp
  src/colman_preview_file_dialog.hpp
  src/colman_screenshot.cpp
  src/colman_screenshot.hpp
  src/colman_search_page.cpp
  src/colman_search_page.hpp
  src/colman_class_management_page.cpp
  src/colman_class_management_page.hpp
  src/colman_class_names.hpp
  src/colman_settings_page.cpp
  src/colman_settings_page.hpp
  src/colman_pixmap_label.hpp
  src/colman_scaling_label.hpp
  src/colman_signal_helpers.hpp
  src/colman_sort_model.hpp
  src/colman_table_models.cpp
  src/colman_table_models.hpp
  src/colman_start_page.cpp
  src/colman_start_page.hpp
  src/colman_tableview_helpers.cpp
  src/colman_tableview_helpers.hpp
  src/colman_try_catch.hpp
  src/colman_permissions_widget.cpp
  src/colman_permissions_widget.hpp
)

set(COLMAN_APP_LIBS
  colman_core
  Qt6::Multimedia
  Qt6::Widgets
  Qt6::PrintSupport
  Qt6::Svg
  Qt6::SvgWidgets
  Qt6::Network
  Qt6::WebEngineWidgets
  Qt6::WebChannel
  Qt6::Xml
  Qt6::Concurrent
  ZXing::ZXing
)

# REST server support library (jwt-cpp + OpenSSL, no Qt UI).
add_library(colman_rest STATIC
  src/colman_auth_middleware.cpp
  src/colman_auth_middleware.hpp
  src/colman_jwt_validator.cpp
  src/colman_jwt_validator.hpp
  src/colman_rest_api_version.hpp
  src/colman_rest_queries.cpp
  src/colman_rest_queries.hpp
)
target_link_libraries(colman_rest
  colman_core
  jwt-cpp::jwt-cpp
  OpenSSL::SSL
  OpenSSL::Crypto
  Qt6::HttpServer
  Qt6::Network
)

# REST server executable.
add_executable(colman_rest_server
  src/colman_mdns_announcer.cpp
  src/colman_mdns_announcer.hpp
  src/colman_rest_server.cpp
  src/colman_rest_server.hpp
  src/colman_rest_server_main.cpp
)
target_link_libraries(colman_rest_server
  colman_rest
  Qt6::HttpServer
  Qt6::Network
)

# Convenience target: build only the REST server (for headless deployments).
add_custom_target(server DEPENDS colman_rest_server)

# REST client application (shares UI code with colman, uses RestQueries).
if(COLMAN_BUILD_GUI)
  add_executable(colman_rest_client
    ${COLMAN_APP_SOURCES}
    ${COLMAN_OWL_RESOURCES}
    ${COLMAN_THIRDPARTY_RESOURCES}
    src/colman_mdns_browser.cpp
    src/colman_mdns_browser.hpp
    src/colman_rest_client_main.cpp
  )
  target_link_libraries(colman_rest_client
    ${COLMAN_APP_LIBS}
    colman_rest
  )
endif()

# Auth middleware unit tests.
add_executable(colman_auth_middleware_test
  tests/colman_auth_middleware_test.cpp
)
target_link_libraries(colman_auth_middleware_test
  colman_rest
  jwt-cpp::jwt-cpp
  OpenSSL::SSL
  OpenSSL::Crypto
  Qt6::Test
  Qt6::Widgets
)

add_test(NAME colman_auth_middleware_test COMMAND colman_auth_middleware_test)
set_tests_properties(colman_auth_middleware_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# JWT validator unit tests.
add_executable(colman_jwt_validator_test
  tests/colman_jwt_validator_test.cpp
)
target_link_libraries(colman_jwt_validator_test
  colman_rest
  jwt-cpp::jwt-cpp
  OpenSSL::SSL
  OpenSSL::Crypto
  Qt6::Test
)

add_test(NAME colman_jwt_validator_test COMMAND colman_jwt_validator_test)
set_tests_properties(colman_jwt_validator_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Main application.
# JSON serialization unit tests.
add_executable(colman_json_test
  tests/colman_json_test.cpp
)
target_link_libraries(colman_json_test
  colman_core
  Qt6::Test
)

add_test(NAME colman_json_test COMMAND colman_json_test)
set_tests_properties(colman_json_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# mDNS announcer unit tests (legacy unicast responses, RFC 6762 §6.7).
add_executable(colman_mdns_announcer_test
  src/colman_mdns_announcer.cpp
  src/colman_mdns_announcer.hpp
  tests/colman_mdns_announcer_test.cpp
)
target_link_libraries(colman_mdns_announcer_test
  colman_core
  Qt6::Network
  Qt6::Test
)

add_test(NAME colman_mdns_announcer_test COMMAND colman_mdns_announcer_test)
set_tests_properties(colman_mdns_announcer_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# REST integration tests.
add_executable(colman_rest_queries_test
  tests/colman_rest_queries_test.cpp
  src/colman_rest_server.cpp
  src/colman_rest_server.hpp
)
target_link_libraries(colman_rest_queries_test
  colman_rest
  jwt-cpp::jwt-cpp
  OpenSSL::SSL
  OpenSSL::Crypto
  Qt6::HttpServer
  Qt6::Network
  Qt6::Test
  Qt6::Widgets
)
add_test(NAME colman_rest_queries_test
  COMMAND colman_rest_queries_test)
set_tests_properties(colman_rest_queries_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# REST authorization tests.
add_executable(colman_rest_authorization_test
  tests/colman_rest_authorization_test.cpp
  src/colman_rest_server.cpp
  src/colman_rest_server.hpp
)
target_link_libraries(colman_rest_authorization_test
  colman_rest
  jwt-cpp::jwt-cpp
  OpenSSL::SSL
  OpenSSL::Crypto
  Qt6::HttpServer
  Qt6::Network
  Qt6::Test
  Qt6::Widgets
)
add_test(NAME colman_rest_authorization_test
  COMMAND colman_rest_authorization_test)
set_tests_properties(colman_rest_authorization_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# REST concurrency stress tests.
add_executable(colman_rest_stress_test
  tests/colman_rest_stress_test.cpp
  src/colman_rest_server.cpp
  src/colman_rest_server.hpp
)
target_link_libraries(colman_rest_stress_test
  colman_rest
  jwt-cpp::jwt-cpp
  OpenSSL::SSL
  OpenSSL::Crypto
  Qt6::HttpServer
  Qt6::Network
  Qt6::Test
  Qt6::Widgets
)
add_test(NAME colman_rest_stress_test
  COMMAND colman_rest_stress_test)
set_tests_properties(colman_rest_stress_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
  TIMEOUT 180
)

if(COLMAN_BUILD_GUI)
  add_executable(colman
    ${COLMAN_APP_SOURCES}
    ${COLMAN_OWL_RESOURCES}
    ${COLMAN_THIRDPARTY_RESOURCES}
    src/colman_main.cpp
    src/colman_tutorial_screenshots.cpp
    src/colman_tutorial_screenshots.hpp
  )
  target_link_libraries(colman ${COLMAN_APP_LIBS})
  # Tutorial screenshots: renders doc/tutorial/<lang>/screenshots/*.jpg by
  # driving the application through tools/tutorial_screenshots.sh.
  foreach(lang de en)
    add_custom_target(tutorial_screenshots_${lang}
      COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/tutorial_screenshots.sh"
        --lang ${lang} --build-dir "${CMAKE_CURRENT_BINARY_DIR}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      DEPENDS colman
      COMMENT "Rendering ${lang} tutorial screenshots"
      VERBATIM
    )
  endforeach()
endif()

# Actions unit tests (colman_core).
add_executable(colman_actions_test tests/colman_actions_test.cpp)
target_link_libraries(colman_actions_test
  colman_core
  Qt6::Network
  Qt6::Test
  Qt6::Widgets
)

add_test(NAME colman_actions_test COMMAND colman_actions_test)
set_tests_properties(colman_actions_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Built-in SQLite driver tests (only colman_core, no UI needed).
add_executable(colman_sqlite_driver_test
  tests/colman_sqlite_driver_test.cpp
)
target_link_libraries(colman_sqlite_driver_test
  colman_core
  Qt6::Test
)

add_test(NAME colman_sqlite_driver_test COMMAND colman_sqlite_driver_test)

# Database utility unit tests (only colman_core, no UI needed).
add_executable(colman_db_util_test
  tests/colman_db_util_test.cpp
)
target_link_libraries(colman_db_util_test
  colman_core
  Qt6::Test
  Qt6::Widgets
)

add_test(NAME colman_db_util_test COMMAND colman_db_util_test)
set_tests_properties(colman_db_util_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Permission profiles and delegated authorization helpers.
add_executable(colman_permission_deps_test
  tests/colman_permission_deps_test.cpp
)
target_link_libraries(colman_permission_deps_test
  colman_core
  Qt6::Test
  Qt6::Widgets
)

add_test(NAME colman_permission_deps_test
  COMMAND colman_permission_deps_test)
set_tests_properties(colman_permission_deps_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Queries unit tests (SQLQueries + table models, needs Qt Widgets for QTableView).
add_executable(colman_queries_test
  tests/colman_queries_test.cpp
  src/colman_table_models.cpp
  src/colman_table_models.hpp
)
target_link_libraries(colman_queries_test
  colman_core
  Qt6::Widgets
  Qt6::Test
)

add_test(NAME colman_queries_test COMMAND colman_queries_test)
set_tests_properties(colman_queries_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(colman_reservations_test
  tests/colman_reservations_test.cpp
)
target_link_libraries(colman_reservations_test
  colman_core
  Qt6::Widgets
  Qt6::Test
)

add_test(NAME colman_reservations_test COMMAND colman_reservations_test)
set_tests_properties(colman_reservations_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# ISBN unit tests (isbn_queries + colman_core, no full UI needed).
add_executable(colman_isbn_test
  src/colman_isbn_queries.cpp
  tests/colman_isbn_test.cpp
)
target_link_libraries(colman_isbn_test
  colman_core
  Qt6::Widgets
  Qt6::Network
  Qt6::Xml
  Qt6::Concurrent
  Qt6::Test
)

add_test(NAME colman_isbn_test COMMAND colman_isbn_test)
set_tests_properties(colman_isbn_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(colman_bgg_test
  src/colman_opengtindb.cpp
  tests/colman_bgg_test.cpp
)
target_link_libraries(colman_bgg_test
  colman_core
  Qt6::Network
  Qt6::Xml
  Qt6::Test
)
add_test(NAME colman_bgg_test COMMAND colman_bgg_test)
set_tests_properties(colman_bgg_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Title (name) search unit tests (name_queries + colman_core).
add_executable(colman_name_test
  tests/colman_name_test.cpp
)
target_link_libraries(colman_name_test
  colman_core
  Qt6::Widgets
  Qt6::Network
  Qt6::Xml
  Qt6::Concurrent
  Qt6::Test
)
add_test(NAME colman_name_test COMMAND colman_name_test)
set_tests_properties(colman_name_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(colman_query_isbn
  src/colman_isbn_queries.cpp
  src/colman_query_isbn.cpp
)
target_link_libraries(colman_query_isbn
  colman_core
  Qt6::Widgets
  Qt6::Network
  Qt6::Xml
  Qt6::Concurrent
)

add_executable(colman_transfer src/colman_transfer.cpp)
target_link_libraries(colman_transfer
  colman_core
  Qt6::Core
  Qt6::Sql
  Qt6::Widgets
)
target_compile_definitions(colman_transfer PRIVATE QT_NO_KEYWORDS)

add_executable(colman_gen_testdb src/colman_gen_testdb.cpp)
target_link_libraries(colman_gen_testdb
  colman_core
  Qt6::Core
  Qt6::Sql
  Qt6::Widgets
)
target_compile_definitions(colman_gen_testdb PRIVATE QT_NO_KEYWORDS)

# Compile all .ts files to .qm (output goes to CMAKE_CURRENT_BINARY_DIR).
set(COLMAN_MAIN_TS_FILES
  localization/colman_de.ts
)
qt6_add_translation(COLMAN_MAIN_QM_FILES ${COLMAN_MAIN_TS_FILES})

# lupdate target — update .ts file from source code.
find_program(LUPDATE_PROGRAM lupdate)
if(LUPDATE_PROGRAM)
  # Qt include path so that lupdate can resolve Qt types from system headers.
  set(_lupdate_qt_inc "$<TARGET_PROPERTY:Qt6::Core,INTERFACE_INCLUDE_DIRECTORIES>")
  add_custom_target(lupdate_colman
    COMMAND ${LUPDATE_PROGRAM}
      -I "${_lupdate_qt_inc}"
      ${COLMAN_CORE_SOURCES}
      ${COLMAN_APP_SOURCES}
      src/colman_main.cpp
      src/colman_label_layout_app.cpp
      src/colman_label_selection_app.cpp
      src/colman_calibrate_printer.cpp
      src/colman_rest_client_main.cpp
      src/colman_rest_queries.cpp
      src/colman_rest_queries.hpp
      -ts ${COLMAN_MAIN_TS_FILES}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
    COMMENT "Updating ${COLMAN_MAIN_TS_FILES}"
  )
else()
  message(STATUS "lupdate not found — lupdate_colman target unavailable")
endif()

# Generate a config file for the lupdate staleness check so that
# cmake/check_lupdate.cmake can be called without knowing the source lists.
# Absolute paths are used because the script is invoked from the build tree.
set(_lupdate_main_sources)
foreach(_f ${COLMAN_CORE_SOURCES} ${COLMAN_APP_SOURCES}
         src/colman_main.cpp src/colman_rest_client_main.cpp
         src/colman_rest_queries.cpp src/colman_rest_queries.hpp)
  if(IS_ABSOLUTE "${_f}")
    list(APPEND _lupdate_main_sources "${_f}")
  else()
    list(APPEND _lupdate_main_sources "${CMAKE_CURRENT_SOURCE_DIR}/${_f}")
  endif()
endforeach()

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lupdate_check_config.cmake"
  CONTENT
"set(MAIN_TS    \"${CMAKE_CURRENT_SOURCE_DIR}/localization/colman_de.ts\")
set(BINARY_DIR \"${CMAKE_CURRENT_BINARY_DIR}\")
set(MAIN_SOURCES ${_lupdate_main_sources})
"
)

set(_check_lupdate_cmd
  ${CMAKE_COMMAND}
  "-DCONFIG_FILE=${CMAKE_CURRENT_BINARY_DIR}/lupdate_check_config.cmake"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_lupdate.cmake"
)

# Application translations.
configure_file(src/colman_app_translations.qrc colman_app_translations.qrc
  COPYONLY)
qt6_add_resources(COLMAN_APP_TRANSLATION_RESOURCES
  "${CMAKE_CURRENT_BINARY_DIR}/colman_app_translations.qrc"
)

# Embed translations in the REST server (data it creates, e.g. the
# default signature categories, follows the system language) ...
target_sources(colman_rest_server PRIVATE ${COLMAN_APP_TRANSLATION_RESOURCES})

# ... and in the main application and the REST client.
if(COLMAN_BUILD_GUI)
  target_sources(colman PRIVATE ${COLMAN_APP_TRANSLATION_RESOURCES})
  target_sources(colman_rest_client PRIVATE ${COLMAN_APP_TRANSLATION_RESOURCES})

  # Run the staleness check after the main application is built.
  add_custom_command(TARGET colman POST_BUILD
    COMMAND ${_check_lupdate_cmd}
    COMMENT "Checking whether lupdate is needed"
  )
endif()

# Interactive label layout manager.
add_executable(colman_label_layout_app
  src/colman_msgbox.cpp
  src/colman_msgbox.hpp
  src/colman_label_layout_widget.cpp
  src/colman_label_layout_widget.hpp
  src/colman_label_printer.cpp
  src/colman_label_printer.hpp
  src/colman_printer_calibration.cpp
  src/colman_printer_calibration.hpp
  src/colman_label_layout_app.cpp
  ${COLMAN_APP_TRANSLATION_RESOURCES}
)
target_link_libraries(colman_label_layout_app
  Qt6::Concurrent
  Qt6::Core
  Qt6::Network
  Qt6::Widgets
  Qt6::PrintSupport
  Qt6::Xml
)
target_compile_definitions(colman_label_layout_app PRIVATE QT_NO_KEYWORDS)


# Interactive label selection test.
add_executable(colman_label_selection_app
  src/colman_msgbox.cpp
  src/colman_msgbox.hpp
  src/colman_label_layout_widget.cpp
  src/colman_label_layout_widget.hpp
  src/colman_label_printer.cpp
  src/colman_label_printer.hpp
  src/colman_printer_calibration.cpp
  src/colman_printer_calibration.hpp
  src/colman_label_selection_widget.cpp
  src/colman_label_selection_widget.hpp
  src/colman_label_selection_app.cpp
  ${COLMAN_OWL_RESOURCES}
  ${COLMAN_APP_TRANSLATION_RESOURCES}
)
target_link_libraries(colman_label_selection_app
  Qt6::Concurrent
  Qt6::Core
  Qt6::Network
  Qt6::Widgets
  Qt6::PrintSupport
  Qt6::Xml
)
target_compile_definitions(colman_label_selection_app PRIVATE QT_NO_KEYWORDS)


# Interactive label printer test.
add_executable(colman_test_label_printer
  src/colman_label_printer.cpp
  src/colman_label_printer.hpp
  src/colman_printer_calibration.cpp
  src/colman_printer_calibration.hpp
  src/colman_code128_item.cpp
  src/colman_code128_item.hpp
  src/colman_test_label_printer.cpp
)
target_link_libraries(colman_test_label_printer
  colman_core
  Qt6::Concurrent
  Qt6::Network
  Qt6::PrintSupport
  Qt6::Widgets
)
target_compile_definitions(colman_test_label_printer PRIVATE QT_NO_KEYWORDS)

# Interactive printer calibration test.
add_executable(colman_calibrate_printer
  src/colman_page_layout.cpp
  src/colman_page_layout.hpp
  src/colman_printer_calibration.cpp
  src/colman_printer_calibration.hpp
  src/colman_printer_calibration_widget.cpp
  src/colman_printer_calibration_widget.hpp
  src/colman_calibrate_printer.cpp
  ${COLMAN_OWL_RESOURCES}
  ${COLMAN_APP_TRANSLATION_RESOURCES}
)
target_link_libraries(colman_calibrate_printer
  Qt6::Core
  Qt6::Concurrent
  Qt6::Network
  Qt6::PrintSupport
  Qt6::Widgets
  Qt6::Xml
)
target_compile_definitions(colman_calibrate_printer PRIVATE QT_NO_KEYWORDS)

# Password hashing unit tests (colman_core, no SQL or UI needed).
add_executable(colman_passwords_test
  tests/colman_passwords_test.cpp
)
target_link_libraries(colman_passwords_test
  colman_core
  Qt6::Concurrent
  Qt6::Network
  Qt6::Test
  Qt6::Xml
)

add_test(NAME colman_passwords_test COMMAND colman_passwords_test)
set_tests_properties(colman_passwords_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Cutter/signature unit tests (pure C++, no SQL or UI needed).
add_executable(colman_cutter_test
  src/colman_cutter.cpp
  tests/colman_cutter_test.cpp
)
target_link_libraries(colman_cutter_test
  Qt6::Core
  Qt6::Concurrent
  Qt6::Network
  Qt6::Test
  Qt6::Xml
)

add_test(NAME colman_cutter_test COMMAND colman_cutter_test)
set_tests_properties(colman_cutter_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(colman_class_management_test
  tests/colman_class_management_test.cpp
)
target_link_libraries(colman_class_management_test
  Qt6::Core
  Qt6::Test
)
target_compile_definitions(colman_class_management_test PRIVATE
  QT_NO_KEYWORDS
)

add_test(NAME colman_class_management_test
  COMMAND colman_class_management_test)
set_tests_properties(colman_class_management_test PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)


# Documentation.
include(cmake/colman_docs.cmake)

# Source distribution tarball.
# Image directories are globbed; everything else is listed explicitly.
file(GLOB COLMAN_DIST_IMAGES
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/flutter/assets/icons/*.jpeg"
  "${CMAKE_CURRENT_SOURCE_DIR}/doc/tutorial/*/screenshots/*.jpg"
  "${CMAKE_CURRENT_SOURCE_DIR}/tools/ui_baseline/*.png"
)
set(COLMAN_DIST_FILES
  README.md
  README.de.md
  INSTALL.md
  LICENSE
  NOTICE
  ACKNOWLEDGEMENTS.md
  CONTRIBUTING.md
  SECURITY.md
  STORY.md
  # Build and tooling
  CMakeLists.txt
  .github/workflows/ci.yml
  .github/workflows/tutorial.yml
  .github/workflows/release.yml
  .github/ISSUE_TEMPLATE/bug_report.yml
  .github/ISSUE_TEMPLATE/feature_request.yml
  .github/ISSUE_TEMPLATE/config.yml
  .gitignore
  .clang-format
  .clang-tidy
  colman_create_doc.sh
  tools/ui_check.sh
  cmake/check_lupdate.cmake
  cmake/colman_docs.cmake
  doc/patch_tutorial_title.cmake
  # flutter client — root
  flutter/README.md
  flutter/pubspec.yaml
  flutter/pubspec.lock
  flutter/analysis_options.yaml
  flutter/l10n.yaml
  flutter/.metadata
  flutter/.gitignore
  # flutter client — Dart sources
  flutter/lib/main.dart
  flutter/lib/permission_policy.dart
  flutter/lib/screens/categories_screen.dart
  flutter/lib/screens/category_edit_screen.dart
  flutter/lib/screens/my_reservations_screen.dart
  flutter/lib/utils/column_name.dart
  flutter/lib/utils/error_utils.dart
  flutter/lib/widgets/error_display.dart
  flutter/lib/widgets/item_detail_body.dart
  flutter/lib/widgets/server_error_dialog.dart
  flutter/lib/app.dart
  flutter/lib/router.dart
  flutter/lib/api/acquisitions_api.dart
  flutter/lib/api/api_client.dart
  flutter/lib/api/api_version.dart
  flutter/lib/api/classes_api.dart
  flutter/lib/api/collection_api.dart
  flutter/lib/api/api_config.dart
  flutter/lib/api/barcode_lookup.dart
  flutter/lib/api/ean_lookup.dart
  flutter/lib/api/history_api.dart
  flutter/lib/api/isbn_lookup.dart
  flutter/lib/api/items_api.dart
  flutter/lib/api/marc_xml.dart
  flutter/lib/api/title_lookup.dart
  flutter/lib/api/lendings_api.dart
  flutter/lib/api/liquidations_api.dart
  flutter/lib/api/persons_api.dart
  flutter/lib/api/reservations_api.dart
  flutter/lib/api/returns_api.dart
  flutter/lib/api/signatures_api.dart
  flutter/lib/api/table_versions_api.dart
  flutter/lib/api/types_api.dart
  flutter/lib/l10n/app_en.arb
  flutter/lib/l10n/app_de.arb
  flutter/lib/l10n/app_localizations.dart
  flutter/lib/l10n/app_localizations_de.dart
  flutter/lib/l10n/app_localizations_en.dart
  flutter/lib/models/acquisition.dart
  flutter/lib/models/affected_person.dart
  flutter/lib/models/change_password_person.dart
  flutter/lib/models/due_lending_row.dart
  flutter/lib/models/history_row.dart
  flutter/lib/models/id_card_layout.dart
  flutter/lib/models/internet_services_settings.dart
  flutter/lib/models/label_grid_settings.dart
  flutter/lib/models/label_preset.dart
  flutter/lib/models/signature_layout.dart
  flutter/lib/models/item.dart
  flutter/lib/models/item_prefill.dart
  flutter/lib/models/item_type.dart
  flutter/lib/models/lending.dart
  flutter/lib/models/liquidation.dart
  flutter/lib/models/open_lending_row.dart
  flutter/lib/models/person.dart
  flutter/lib/models/printer_calibration.dart
  flutter/lib/models/reservation.dart
  flutter/lib/models/return_entry.dart
  flutter/lib/printing/calibration_sheet_pdf.dart
  flutter/lib/printing/label_bitmap.dart
  flutter/lib/printing/label_sheet_pdf.dart
  flutter/lib/printing/usb_label_printer.dart
  flutter/lib/providers/api_client_provider.dart
  flutter/lib/providers/auth_provider.dart
  flutter/lib/providers/internet_services_provider.dart
  flutter/lib/providers/poll_interval_provider.dart
  flutter/lib/providers/table_versions_provider.dart
  flutter/lib/providers/table_watch.dart
  flutter/lib/screens/acquisition_screen.dart
  flutter/lib/screens/change_own_password_screen.dart
  flutter/lib/screens/change_passwords_screen.dart
  flutter/lib/screens/classes_screen.dart
  flutter/lib/screens/history_screen.dart
  flutter/lib/screens/new_school_year_screen.dart
  flutter/lib/screens/home_screen.dart
  flutter/lib/screens/layout_editor_screen.dart
  flutter/lib/screens/id_card_layout_screen.dart
  flutter/lib/screens/id_card_print_screen.dart
  flutter/lib/screens/item_detail_screen.dart
  flutter/lib/screens/item_edit_screen.dart
  flutter/lib/screens/items_screen.dart
  flutter/lib/screens/lending_screen.dart
  flutter/lib/screens/liquidation_screen.dart
  flutter/lib/screens/login_screen.dart
  flutter/lib/screens/open_lendings_screen.dart
  flutter/lib/screens/barcode_dispatch_screen.dart
  flutter/lib/screens/barcode_scan_screen.dart
  flutter/lib/screens/person_detail_screen.dart
  flutter/lib/screens/persons_screen.dart
  flutter/lib/screens/return_screen.dart
  flutter/lib/screens/amazon_browser_screen.dart
  flutter/lib/screens/settings_screen.dart
  flutter/lib/screens/signature_print_screen.dart
  flutter/lib/screens/label_print_screen.dart
  flutter/lib/screens/label_preset_editor_screen.dart
  flutter/lib/screens/printer_calibration_screen.dart
  flutter/lib/screens/sheet_positions_screen.dart
  flutter/lib/services/certificate_trust_store.dart
  flutter/lib/services/mdns_browser.dart
  flutter/lib/services/multicast_lock.dart
  flutter/lib/utils/password_hash.dart
  flutter/lib/widgets/app_bar_title.dart
  flutter/lib/widgets/class_dropdown.dart
  flutter/lib/widgets/extend_lending_dialog.dart
  flutter/lib/widgets/label_position_picker.dart
  flutter/lib/widgets/person_search_field.dart
  flutter/lib/widgets/thumbnail_widget.dart
  flutter/lib/widgets/title_search_dialog.dart
  # flutter client — tests
  flutter/test/api_version_test.dart
  flutter/test/barcode_lookup_test.dart
  flutter/test/ean_lookup_test.dart
  flutter/test/internet_services_settings_test.dart
  flutter/test/isbn_lookup_test.dart
  flutter/test/item_prefill_test.dart
  flutter/test/title_lookup_test.dart
  flutter/test/label_bitmap_test.dart
  flutter/test/label_preset_test.dart
  flutter/test/label_sheet_pdf_test.dart
  flutter/test/lending_period_test.dart
  flutter/test/person_barcode_test.dart
  flutter/test/password_hash_test.dart
  flutter/test/printer_calibration_test.dart
  flutter/test/widget_test.dart
  # flutter client — Linux runner
  flutter/linux/.gitignore
  flutter/linux/CMakeLists.txt
  flutter/linux/flutter/CMakeLists.txt
  flutter/linux/flutter/generated_plugin_registrant.cc
  flutter/linux/flutter/generated_plugin_registrant.h
  flutter/linux/flutter/generated_plugins.cmake
  flutter/linux/runner/CMakeLists.txt
  flutter/linux/runner/main.cc
  flutter/linux/runner/my_application.cc
  flutter/linux/runner/my_application.h
  # flutter client — Windows runner
  flutter/windows/.gitignore
  flutter/windows/CMakeLists.txt
  flutter/windows/flutter/CMakeLists.txt
  flutter/windows/flutter/generated_plugin_registrant.cc
  flutter/windows/flutter/generated_plugin_registrant.h
  flutter/windows/flutter/generated_plugins.cmake
  flutter/windows/runner/CMakeLists.txt
  flutter/windows/runner/flutter_window.cpp
  flutter/windows/runner/flutter_window.h
  flutter/windows/runner/main.cpp
  flutter/windows/runner/resource.h
  flutter/windows/runner/resources/app_icon.ico
  flutter/windows/runner/runner.exe.manifest
  flutter/windows/runner/Runner.rc
  flutter/windows/runner/utils.cpp
  flutter/windows/runner/utils.h
  flutter/windows/runner/win32_window.cpp
  flutter/windows/runner/win32_window.h
  # flutter client — Android runner
  flutter/android/.gitignore
  flutter/android/app/src/main/res/drawable-hdpi/ic_launcher_foreground.png
  flutter/android/app/src/main/res/drawable-mdpi/ic_launcher_foreground.png
  flutter/android/app/src/main/res/drawable-xhdpi/ic_launcher_foreground.png
  flutter/android/app/src/main/res/drawable-xxhdpi/ic_launcher_foreground.png
  flutter/android/app/src/main/res/drawable-xxxhdpi/ic_launcher_foreground.png
  flutter/android/app/src/main/res/mipmap-anydpi-v26/ic_launcher.xml
  flutter/android/app/src/main/res/values/colors.xml
  flutter/android/build.gradle.kts
  flutter/android/settings.gradle.kts
  flutter/android/gradle.properties
  flutter/android/gradle/wrapper/gradle-wrapper.properties
  flutter/android/app/build.gradle.kts
  flutter/android/app/src/debug/AndroidManifest.xml
  flutter/android/app/src/main/AndroidManifest.xml
  flutter/android/app/src/main/kotlin/de/colman/colman_client/MainActivity.kt
  flutter/android/app/src/main/kotlin/de/colman/colman_client/UsbPrinterChannel.kt
  flutter/android/app/src/main/res/xml/usb_device_filter.xml
  flutter/android/app/src/main/res/drawable/launch_background.xml
  flutter/android/app/src/main/res/drawable-v21/launch_background.xml
  flutter/android/app/src/main/res/mipmap-hdpi/ic_launcher.png
  flutter/android/app/src/main/res/mipmap-mdpi/ic_launcher.png
  flutter/android/app/src/main/res/mipmap-xhdpi/ic_launcher.png
  flutter/android/app/src/main/res/mipmap-xxhdpi/ic_launcher.png
  flutter/android/app/src/main/res/mipmap-xxxhdpi/ic_launcher.png
  flutter/android/app/src/main/res/values-night/styles.xml
  flutter/android/app/src/main/res/values/styles.xml
  flutter/android/app/src/profile/AndroidManifest.xml
  flutter/android/app/proguard-rules.pro
  # flutter client — iOS runner
  flutter/ios/.gitignore
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-50x50@1x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-50x50@2x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-57x57@1x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-57x57@2x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-72x72@1x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-72x72@2x.png
  flutter/ios/Flutter/AppFrameworkInfo.plist
  flutter/ios/Flutter/Debug.xcconfig
  flutter/ios/Flutter/Release.xcconfig
  flutter/ios/Runner/AppDelegate.swift
  flutter/ios/Runner/Info.plist
  flutter/ios/Runner/Runner-Bridging-Header.h
  flutter/ios/Runner/SceneDelegate.swift
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Contents.json
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-1024x1024@1x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-20x20@1x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-20x20@2x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-20x20@3x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-29x29@1x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-29x29@2x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-29x29@3x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-40x40@1x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-40x40@2x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-40x40@3x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-60x60@2x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-60x60@3x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-76x76@1x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-76x76@2x.png
  flutter/ios/Runner/Assets.xcassets/AppIcon.appiconset/Icon-App-83.5x83.5@2x.png
  flutter/ios/Runner/Assets.xcassets/LaunchImage.imageset/Contents.json
  flutter/ios/Runner/Assets.xcassets/LaunchImage.imageset/LaunchImage.png
  flutter/ios/Runner/Assets.xcassets/LaunchImage.imageset/LaunchImage@2x.png
  flutter/ios/Runner/Assets.xcassets/LaunchImage.imageset/LaunchImage@3x.png
  flutter/ios/Runner/Assets.xcassets/LaunchImage.imageset/README.md
  flutter/ios/Runner/Base.lproj/LaunchScreen.storyboard
  flutter/ios/Runner/Base.lproj/Main.storyboard
  flutter/ios/RunnerTests/RunnerTests.swift
  flutter/ios/Runner.xcodeproj/project.pbxproj
  flutter/ios/Runner.xcodeproj/project.xcworkspace/contents.xcworkspacedata
  flutter/ios/Runner.xcodeproj/project.xcworkspace/xcshareddata/IDEWorkspaceChecks.plist
  flutter/ios/Runner.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings
  flutter/ios/Runner.xcodeproj/xcshareddata/xcschemes/Runner.xcscheme
  flutter/ios/Runner.xcworkspace/contents.xcworkspacedata
  flutter/ios/Runner.xcworkspace/xcshareddata/IDEWorkspaceChecks.plist
  flutter/ios/Runner.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings
  # flutter client — macOS runner
  flutter/macos/.gitignore
  flutter/macos/Flutter/GeneratedPluginRegistrant.swift
  flutter/macos/Flutter/Flutter-Debug.xcconfig
  flutter/macos/Flutter/Flutter-Release.xcconfig
  flutter/macos/Runner/AppDelegate.swift
  flutter/macos/Runner/Assets.xcassets/AppIcon.appiconset/Contents.json
  flutter/macos/Runner/Assets.xcassets/AppIcon.appiconset/app_icon_1024.png
  flutter/macos/Runner/Assets.xcassets/AppIcon.appiconset/app_icon_128.png
  flutter/macos/Runner/Assets.xcassets/AppIcon.appiconset/app_icon_16.png
  flutter/macos/Runner/Assets.xcassets/AppIcon.appiconset/app_icon_256.png
  flutter/macos/Runner/Assets.xcassets/AppIcon.appiconset/app_icon_32.png
  flutter/macos/Runner/Assets.xcassets/AppIcon.appiconset/app_icon_512.png
  flutter/macos/Runner/Assets.xcassets/AppIcon.appiconset/app_icon_64.png
  flutter/macos/Runner/Base.lproj/MainMenu.xib
  flutter/macos/Runner/Configs/AppInfo.xcconfig
  flutter/macos/Runner/Configs/Debug.xcconfig
  flutter/macos/Runner/Configs/Release.xcconfig
  flutter/macos/Runner/Configs/Warnings.xcconfig
  flutter/macos/Runner/DebugProfile.entitlements
  flutter/macos/Runner/Info.plist
  flutter/macos/Runner/MainFlutterWindow.swift
  flutter/macos/Runner/Release.entitlements
  flutter/macos/RunnerTests/RunnerTests.swift
  flutter/macos/Runner.xcodeproj/project.pbxproj
  flutter/macos/Runner.xcodeproj/project.xcworkspace/xcshareddata/IDEWorkspaceChecks.plist
  flutter/macos/Runner.xcodeproj/xcshareddata/xcschemes/Runner.xcscheme
  flutter/macos/Runner.xcworkspace/contents.xcworkspacedata
  flutter/macos/Runner.xcworkspace/xcshareddata/IDEWorkspaceChecks.plist
  # flutter client — assets (owl icons are globbed below; the desktop
  # application uses the same files via aliases in the .qrc files)
  flutter/assets/thirdparty/powered-by-bgg-rgb.svg
  # flutter client — web
  flutter/web/favicon.png
  flutter/web/icons/Icon-192.png
  flutter/web/icons/Icon-512.png
  flutter/web/icons/Icon-maskable-192.png
  flutter/web/icons/Icon-maskable-512.png
  flutter/web/index.html
  flutter/web/manifest.json
  # documentation
  doc/Doxyfile.in
  doc/Doxyfile_tutorial.in
  doc/colman_doxy_extra.css
  doc/doxygen_custom.sty
  doc/colman_tutorial.sty
  doc/latex_header.tex
  doc/latex_footer.tex
  doc/colman_overview.dox
  doc/colman_rest_api.dox
  doc/colman_sql_database.dox
  doc/tutorial/index.html
  doc/tutorial/de/colman_tutorial.dox
  doc/tutorial/de/sample_data.json
  doc/tutorial/en/colman_tutorial.dox
  doc/tutorial/en/sample_data.json
  doc/colman_admin_tools.dox
  doc/raspberry_pi.md
  doc/sru_servers.md
  doc/release_notes/2.0.0.md
  doc/release_notes/2.0.1.md
  packaging/linux/colman.desktop
  packaging/linux/colman.png
  packaging/licenses/SQLite3MultipleCiphers.txt
  # Core library sources
  ${COLMAN_CORE_SOURCES}
  src/colman_iqueries.hpp
  # REST library sources
  src/colman_auth_middleware.cpp
  src/colman_auth_middleware.hpp
  src/colman_jwt_validator.cpp
  src/colman_jwt_validator.hpp
  src/colman_rest_api_version.hpp
  src/colman_rest_queries.cpp
  src/colman_rest_queries.hpp
  # REST server executable sources
  src/colman_mdns_announcer.cpp
  src/colman_mdns_announcer.hpp
  src/colman_rest_server.cpp
  src/colman_rest_server.hpp
  src/colman_rest_server_main.cpp
  # REST client executable sources
  src/colman_mdns_browser.cpp
  src/colman_mdns_browser.hpp
  src/colman_rest_client_main.cpp
  # Application sources
  ${COLMAN_APP_SOURCES}
  src/colman_main.cpp
  src/colman_tutorial_screenshots.cpp
  src/colman_tutorial_screenshots.hpp
  tools/tutorial_screenshots.sh
  # Standalone helper applications
  src/colman_calibrate_printer.cpp
  src/colman_gen_testdb.cpp
  src/colman_label_layout_app.cpp
  src/colman_label_selection_app.cpp
  src/colman_query_isbn.cpp
  src/colman_transfer.cpp
  # Coding style reference
  src/colman_coding_style.cpp
  src/colman_coding_style.hpp
  # Utility library
  src/colman_cutter.cpp
  src/colman_cutter.hpp
  # Tests
  tests/colman_actions_test.cpp
  tests/colman_class_management_test.cpp
  tests/colman_cutter_test.cpp
  tests/colman_bgg_test.cpp
  tests/colman_isbn_test.cpp
  tests/colman_name_test.cpp
  tests/colman_passwords_test.cpp
  tests/colman_queries_test.cpp
  tests/colman_reservations_test.cpp
  tests/colman_json_test.cpp
  tests/colman_mdns_announcer_test.cpp
  tests/colman_auth_middleware_test.cpp
  tests/colman_jwt_validator_test.cpp
  tests/colman_rest_authorization_test.cpp
  tests/colman_rest_queries_test.cpp
  tests/colman_rest_stress_test.cpp
  tests/colman_db_util_test.cpp
  tests/colman_sqlite_driver_test.cpp
  tests/colman_permission_deps_test.cpp
  tests/colman_test_helpers.hpp
  src/colman_test_label_printer.cpp
  # Resources
  src/colman_app_translations.qrc
  src/colman_owls.qrc
  src/colman_thirdparty.qrc
  # Images: owl icons (app and Flutter client), tutorial screenshots,
  # UI baseline for tools/ui_check.sh
  ${COLMAN_DIST_IMAGES}
  # Localization
  localization/colman_de.ts
)

set(COLMAN_DIST_NAME colman-${PROJECT_VERSION})

add_custom_target(dist
  COMMAND tar czf
    "${CMAKE_BINARY_DIR}/${COLMAN_DIST_NAME}.tar.gz"
    "--transform=s|^|${COLMAN_DIST_NAME}/|S"
    ${COLMAN_DIST_FILES}
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  VERBATIM
  COMMENT "Creating ${COLMAN_DIST_NAME}.tar.gz"
)

# Self-contained deployment tarball via linuxdeploy + linuxdeploy-plugin-qt.
# Place both AppImages (executable) in tools/ then run:
#   cmake --build bld --target deploy
find_program(LINUXDEPLOY
  NAMES linuxdeploy linuxdeploy-x86_64.AppImage
  HINTS "${CMAKE_SOURCE_DIR}/tools"
)
find_program(LINUXDEPLOY_PLUGIN_QT
  NAMES linuxdeploy-plugin-qt linuxdeploy-plugin-qt-x86_64.AppImage
  HINTS "${CMAKE_SOURCE_DIR}/tools"
)

if(LINUXDEPLOY AND LINUXDEPLOY_PLUGIN_QT)
  get_target_property(_colman_qmake Qt6::qmake IMPORTED_LOCATION)
  set(_deploy_dir "${CMAKE_BINARY_DIR}/AppDir")
  set(_deploy_tarball
    "${CMAKE_BINARY_DIR}/${COLMAN_DIST_NAME}-linux-x86_64.tar.gz"
  )
  add_custom_target(deploy
    DEPENDS colman
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${_deploy_dir}"
    COMMAND ${CMAKE_COMMAND} -E env
      QMAKE=${_colman_qmake}
      ${LINUXDEPLOY}
        --appdir "${_deploy_dir}"
        --executable $<TARGET_FILE:colman>
        --desktop-file "${CMAKE_SOURCE_DIR}/packaging/linux/colman.desktop"
        --icon-file "${CMAKE_SOURCE_DIR}/packaging/linux/colman.png"
        --plugin qt
    # License texts of colman and of the statically linked libraries.
    COMMAND ${CMAKE_COMMAND} -E make_directory
      "${_deploy_dir}/usr/share/doc/colman/licenses"
    COMMAND ${CMAKE_COMMAND} -E copy
      "${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_SOURCE_DIR}/NOTICE"
      "${_deploy_dir}/usr/share/doc/colman/"
    COMMAND ${CMAKE_COMMAND} -E copy
      "${CMAKE_SOURCE_DIR}/packaging/licenses/SQLite3MultipleCiphers.txt"
      "${_deploy_dir}/usr/share/doc/colman/licenses/"
    COMMAND tar czf "${_deploy_tarball}" -C "${_deploy_dir}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    VERBATIM
    COMMENT "Creating ${COLMAN_DIST_NAME}-linux-x86_64.tar.gz"
  )
  message(STATUS "linuxdeploy found — 'deploy' target available")
else()
  message(STATUS "linuxdeploy not found — 'deploy' target unavailable")
  message(STATUS "  Download to tools/ and make executable:")
  message(STATUS "  tools/linuxdeploy-x86_64.AppImage")
  message(STATUS "  tools/linuxdeploy-plugin-qt-x86_64.AppImage")
endif()
