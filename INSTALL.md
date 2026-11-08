# Building and installing colman

colman currently runs on **Linux** only (desktop application, REST server
and client, command-line tools; also on the Raspberry Pi, see
[doc/raspberry_pi.md](doc/raspberry_pi.md)).  The mobile client in
`flutter/` runs on **Android** devices.  Windows and macOS are not
supported yet.

## Prerequisites

| Dependency | Version | Notes |
|---|---|---|
| CMake | ≥ 3.21 | |
| Qt 6 | Core, Concurrent, Gui, HttpServer, Svg, SvgWidgets, Widgets, PrintSupport, Sql, Network, Multimedia, WebChannel, WebEngineWidgets, Xml, WebSockets, Test, LinguistTools | ≥ 6.4.2. HttpServer and WebSockets required for REST mode; Multimedia, WebChannel and WebEngineWidgets required for the desktop app and REST client (camera scanning, Amazon cover-image browser) |
| Argon2 | libargon2 | |
| ZXing-C++ | libzxing | Barcode decoding for camera scanning; desktop app and REST client only |
| OpenSSL | ≥ 1.0.1 | Required by jwt-cpp |
| pkg-config | any | |
| jwt-cpp | 0.7.2 | Fetched automatically by CMake via FetchContent |
| SQLite3MultipleCiphers | 2.6.1 | SQLite with encryption; fetched automatically by CMake via FetchContent and linked statically |

### OpenSUSE

```bash
sudo zypper install cmake pkg-config argon2-devel openssl-devel \
  qt6-base-devel qt6-svg-devel qt6-tools-devel \
  qt6-webengine-devel qt6-webchannel-devel
```

### Debian / Ubuntu

```bash
sudo apt install cmake pkg-config libargon2-dev libssl-dev \
  libzxing-dev qt6-base-dev qt6-svg-dev qt6-tools-dev qt6-l10n-tools \
  qt6-httpserver-dev qt6-websockets-dev qt6-multimedia-dev \
  qt6-webengine-dev qt6-webchannel-dev
```

The CI workflow (`.github/workflows/ci.yml`) builds with exactly these
packages on Ubuntu 24.04.

---

## Building from source

### From a source tarball

```bash
tar xzf colman-2.0.1.tar.gz
cd colman-2.0.1
cmake -B bld && cmake --build bld -j4
./bld/colman
```

### From a git checkout

```bash
cmake -B bld && cmake --build bld -j4
./bld/colman
```

---

## Generating documentation

Requires [Doxygen](https://www.doxygen.nl/) to be installed.

```bash
cmake -B bld && cmake --build bld --target doc
```

Output is written to `bld/doc/html/index.html` (HTML) and `bld/doc/latex/`
(LaTeX / PDF).

A separate tutorial for new users is generated per language
(`tutorial_de`, `tutorial_en`; `tutorial` builds all languages):

```bash
cmake --build bld --target tutorial_en
```

Output is written to `bld/tutorial/<lang>/html/index.html` (HTML) and
`bld/tutorial/<lang>/latex/colman_tutorial.pdf` (PDF).  The PDF needs
`pdflatex` (TeX Live with the `latex-extra` and German language packages).

To build only the documentation without installing Qt or any other build
dependency, configure with `COLMAN_DOCS_ONLY` (only the `doc` and
`tutorial*` targets are available then):

```bash
cmake -B bld-docs -DCOLMAN_DOCS_ONLY=ON
cmake --build bld-docs --target tutorial
```

The published tutorial at <https://pesch-1.github.io/colman/> is built
this way by `.github/workflows/tutorial.yml` on every push to `main` that
touches `doc/`; the landing page is `doc/tutorial/index.html`.

---

## Creating a source distribution

```bash
cmake -B bld && cmake --build bld --target dist
```

This produces `bld/colman-2.0.1.tar.gz` containing all source files.

---

## Creating self-contained binary tarballs

The `deploy` target bundles the programs together with all required shared
libraries (including Qt) so the result runs on other Linux machines without
any additional installation.  It creates two tarballs:

- `colman-<version>-linux-x86_64.tar.gz` — the desktop application and the
  REST client, which share the user interface libraries;
- `colman-server-<version>-linux-x86_64.tar.gz` — the REST server only,
  without user interface libraries (about a seventh of the size).

### One-time setup — download linuxdeploy

```bash
mkdir -p tools
wget -P tools https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget -P tools https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x tools/linuxdeploy-x86_64.AppImage tools/linuxdeploy-plugin-qt-x86_64.AppImage
```

Both tools are available under the [MIT License](https://opensource.org/licenses/MIT).

### Build the tarballs

```bash
cmake -B bld && cmake --build bld -j4 --target deploy
```

This produces `bld/colman-2.0.1-linux-x86_64.tar.gz` and
`bld/colman-server-2.0.1-linux-x86_64.tar.gz`.  The desktop entry and icon
of the application bundle are taken from `packaging/linux/`.  `LICENSE`,
`NOTICE` and the license texts from `packaging/licenses/` are copied to
`usr/share/doc/colman/` in both bundles.

`cmake/deploy_prune.cmake` adjusts the bundles after linuxdeploy:

- GStreamer and GLib are not bundled: they are taken from the target
  system, whose GStreamer plugins Qt Multimedia needs to find the camera.
  For camera scanning the target system needs GStreamer with the "good"
  plugins (`v4l2src`), e.g. `gstreamer1.0-plugins-good` on Debian/Ubuntu.
- The Qt SQL driver plugins are removed; colman has its own SQLite driver.
- The server bundle keeps only the plugins and libraries the REST server
  needs (it has no user interface and decodes no images).

### Run on the target machine

```bash
tar xzf colman-2.0.1-linux-x86_64.tar.gz
./AppRun                        # desktop application
./usr/bin/colman_rest_client    # REST client

tar xzf colman-server-2.0.1-linux-x86_64.tar.gz
./usr/bin/colman_rest_server --db school.col
```

`AppRun` is a link to `usr/bin/colman`; the bundled libraries are found
via the run path (rpath) and the Qt plugins via `usr/bin/qt.conf`.  See the
[README](README.md#rest-mode--server-and-client) for the server and client
options.

Requirements of the target machine:

- x86_64 Linux with **glibc 2.38 or newer**, i.e. distributions from 2024
  on, such as Ubuntu 24.04, Debian 13, Fedora 39 or current openSUSE
  Tumbleweed / Arch Linux.  The bundle built by the release workflow on
  Ubuntu 24.04 needs glibc 2.38; a bundle built elsewhere needs at least
  the glibc of its build machine.  Older systems such as Ubuntu 22.04 or
  Debian 12 are not supported.
- GLib (always present on a Linux desktop) and, for camera scanning,
  GStreamer with the "good" plugins, as described above.
