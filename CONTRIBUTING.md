# Contributing to colman

Thank you for your interest in colman!  Bug reports, feature ideas,
translations and pull/merge requests are welcome.

## Reporting bugs and requesting features

- Search the existing issues first.
- For bugs, include the colman version (or commit), your operating system,
  the Qt version, the mode you use (single-machine, REST client, Flutter
  client) and the steps to reproduce the problem.
- **Never attach real library databases (`.col` files) or screenshots
  containing personal data of pupils or staff.**  Reproduce the problem with
  a test database instead (see below).
- Security issues must not be reported as public issues — see
  [SECURITY.md](SECURITY.md).

## Thermal label printers

So far colman has been tested with two thermal label printers, the Vevor
Y486 and the Munbyn RW403B.  Help with further models is very welcome —
and so is a printer lent to the author for testing (please get in touch
via an issue).

colman sends labels as raw TSPL jobs directly over USB
(`colman_label_printer.cpp`, device access in `colman_usb_printer.cpp`).
A new model is added as a profile in `tspl_device_profile()`, selected by
its USB vendor/product ID.  To add one, the following has to be found out
on the real printer:

- the USB ID (`lsusb`) and whether the printer understands TSPL;
- whether it answers the TSPL status request `ESC !?` — some firmwares do
  not, and the Munbyn RW403B even discards the following job when the
  request is sent, so models must never be probed blindly;
- the placement of the label (TSPL `REFERENCE` and the horizontal bitmap
  offset) and whether the `BITMAP` command works in mode 0 or only in
  mode 1;
- how the print darkness is set (TSPL `DENSITY` or a vendor command).

The interactive program `colman_test_label_printer` prints test labels and
shows the paper status reported by the connected printers.  Please ask
before sending any other raw commands to a printer you do not own.

## Internet lookup

The internet lookup of title data (by ISBN, EAN or title) has so far been
geared mainly towards the German-speaking countries: the preconfigured
SRU catalogues are the German National Library (DNB) and K10plus, and the
other services (Open Library, Google Books, MusicBrainz, BoardGameGeek,
opengtindb.org) have mainly been tested with German-language media.  The
lookup is not limited to these, though — any SRU catalogue can be added in
the settings, and further services can be implemented.

Anyone who would like to help is very welcome: reports on how well the
lookup works in other countries, suitable catalogues and their search
indexes (see [doc/sru_servers.md](doc/sru_servers.md)), or new lookup
services.  The lookup code lives in `colman_isbn_queries.cpp`,
`colman_ean_queries.cpp`, `colman_name_queries.cpp` (title search) and
`colman_bgg_queries.cpp`.

## Building and testing

C++ sources live in `src/`, unit tests in `tests/`, the Flutter client in
`flutter/`.  See [INSTALL.md](INSTALL.md) for dependencies.  In short:

```bash
cmake -B bld && cmake --build bld -j4
ctest --test-dir bld
```

Tests that need internet access are skipped by default; set
`COLMAN_RUN_NETWORK_TESTS=1` to enable them.  Always run tests via `ctest`,
not by calling the test binaries directly.

A database with generated test data can be created with
`colman_gen_testdb` (see [README.md](README.md)).

`tools/ui_check.sh` compares the rendered main window against the pixel
baseline in `tools/ui_baseline/`; run it when you touch the hand-written UI.

The Flutter client lives in `flutter/`:

```bash
cd flutter
flutter pub get
flutter analyze
flutter test
```

## Continuous integration

Every push and pull request runs the GitHub Actions workflow
`.github/workflows/ci.yml`:

- Qt build and `ctest` on Ubuntu 24.04 (Qt 6.4.2, the minimum supported
  Qt version — guard newer Qt APIs with `QT_VERSION_CHECK`),
- a check that every file in the repository is listed in
  `COLMAN_DIST_FILES`,
- Qt build and `ctest` on Windows with clang-cl (Qt 6.10, libraries from
  vcpkg), so that the code stays portable — keep operating-system specific
  code behind `#ifdef _WIN32` (see `colman_usb_printer.cpp`,
  `colman_console.cpp`); the Windows packages are a preview,
- `flutter analyze` and `flutter test`, plus a check that the generated
  Flutter localizations are up to date.

## Releases

Releases are built by `.github/workflows/release.yml`:

1. Set the new version in `CMakeLists.txt` (`project(colman VERSION …)`)
   and `flutter/pubspec.yaml` (`version: <version>+<build>`; always
   increase the build number), and write the release notes in
   `doc/release_notes/<version>.md` (add it to `COLMAN_DIST_FILES`).
2. Commit, then tag and push: `git tag -a v<version> -m "colman <version>"`
   and `git push origin v<version>`.
3. The workflow checks that the tag matches both versions, builds the
   source tarball, the self-contained Linux bundle and — if the Android
   signing secrets `ANDROID_KEYSTORE_BASE64`, `ANDROID_KEYSTORE_PASSWORD`,
   `ANDROID_KEY_ALIAS` and `ANDROID_KEY_PASSWORD` are set — the signed APK,
   and creates a **draft** release with these files.
4. Test the files, then publish the draft on GitHub.

Running the workflow manually rebuilds the files of an existing tag; unless
"dry run" is unchecked, they are only kept as workflow artifacts.

## Pull / merge requests

- Keep changes focused: one feature or fix per request.
- The build must be **free of compiler warnings**, and all tests must pass.
- Add or update tests for changed behaviour where practical.
- Commit messages follow [Conventional Commits](https://www.conventionalcommits.org/),
  e.g. `feat(lendings): extend due dates`, `fix(sql): ...`, `docs: ...`.
- By contributing you agree that your contribution is licensed under the
  GNU General Public License v3.0 or later (see [LICENSE](LICENSE)).

## Coding style (C++)

The authoritative reference is `src/colman_coding_style.hpp` /
`src/colman_coding_style.cpp`; `.clang-format` encodes the formatting rules.
Please run `clang-format` on changed files.

- 2 spaces indentation, no tabs, 80 columns.
- Classes, type aliases and `enum struct` values in `CamelCase`; functions
  and variables in `snake_case`; **all** data members (class and struct) end
  with `_`.
- East const (`int const& value`); `*` and `&` attach to the type.
- Allman braces for functions, classes and namespaces; control statements
  with a single-line condition keep `{` on the same line, `else` starts on a
  new line.  No space between keyword and parenthesis (`if(`, `for(`).
- Always use braces, also for single-statement bodies; one statement per
  line.
- No global or file-scope mutable variables; file-local symbols go into an
  anonymous namespace in the `.cpp` file (never in headers).
- No `using namespace`, `enum struct` instead of `enum class`, `using`
  instead of `typedef`, `constexpr` for constants; prefer `auto`.
- Includes: corresponding header first; every include is wrapped in a guard
  (`#ifndef SYSTEM_QSTRING` / `#  include <QString>` / `#  define ...`);
  includes are not sorted.
- `QT_NO_KEYWORDS` is set: use `Q_EMIT`, `Q_SIGNALS`, `Q_SLOTS`.
- Identifiers, comments and string literals are in English.
- Doxygen comments: `///` brief, `/** ... */` long, `///<` after members.
- Every source file starts with an SPDX header:

  ```cpp
  // SPDX-License-Identifier: GPL-3.0-or-later
  // Copyright (C) <year> <your name>
  ```

Flutter code follows `flutter analyze` and the Dart style guide.

## Translations

- User-visible strings in the desktop application use `tr()` with
  **English** source text.
- German translations live in `localization/colman_de.ts`.  New or changed
  strings must be translated in the same change; the build checks that no
  translation is left `unfinished`.
- Flutter strings are maintained only in `flutter/lib/l10n/app_en.arb` and
  `app_de.arb`; the `app_localizations*.dart` files are generated
  (`flutter gen-l10n`) and must not be edited by hand.
- New languages are welcome — please open an issue first.

## Project bookkeeping

- When adding, removing or renaming files (sources, icons in `.qrc` files,
  CMake scripts, documentation), update `COLMAN_DIST_FILES` in
  `CMakeLists.txt`.
- For user-visible changes, check and update the documentation in `doc/`:
  - `doc/tutorial/<lang>/colman_tutorial.dox` — the end-user tutorial
    (German and English, keep both in sync; screenshots via
    `tools/tutorial_screenshots.sh`),
  - `colman_overview.dox` — developer overview,
  - `colman_rest_api.dox` — REST API (bump the API version in
    `colman_rest_api_version.hpp`: minor for compatible changes, major for
    breaking changes).
