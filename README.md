**English** | [Deutsch](README.de.md)

<p align="center">
  <img src="flutter/assets/icons/colman_logo.jpeg" alt="colman logo: an owl reading a book" width="200">
</p>

# colman — Library and Collection Manager

Colman is a desktop application for managing a school library.
It tracks items (books and other media), borrowers, lendings, returns,
acquisitions, and liquidations.
The application is built with Qt 6 and stores its data in a SQLite database.

**Platforms:** colman runs on **Linux** (including the Raspberry Pi); a
**Windows** version is available as a **preview** (see
[INSTALL.md](INSTALL.md#windows-preview)).  The mobile client runs on
**Android** devices.  macOS is not supported yet.

## Features at a glance

- **Fast operation with a barcode scanner:** scan the ID card, scan the
  items, done — lending and returning work without typing
- **Guided flows** for lending, returning, acquisition and liquidation,
  including whole class sets in a single step
- **Internet lookup** of books, CDs, DVDs, Blu-rays and board games by
  ISBN, EAN or title — title data and covers are filled in automatically
  (e.g. German National Library, K10plus, Open Library, MusicBrainz,
  BoardGameGeek); so far geared mainly towards the German-speaking
  countries, but not limited to them — anyone who would like to help with
  services for other countries and languages is very welcome (see
  [CONTRIBUTING.md](CONTRIBUTING.md#internet-lookup))
- **Label printing** of barcode labels and spine signatures, also directly
  on thermal label printers (see below)
- **Library ID cards** with barcode and a freely designable layout; the
  card can also be used to log in
- **Single machine, server and smartphone:** locally with a single
  database file, on the network with a server and several workstations
  (e.g. on a Raspberry Pi), or on the go with the Android app, including
  barcode scanning with the camera
- **Extensible with new item types** with custom fields (optionally mapped
  to MARC21) — from books and games to equipment
- Reservations, loan periods and renewals, lending history
- Class management and transition to the new school year
- Fine-grained permissions for library staff, teachers and pupils
- Optionally encrypted database file

Colman was originally developed to support the lending of class sets — for
example, workbooks of which a school owns many copies. Multiple copies of the
same item can be acquired, lent out, returned, or liquidated in a single step,
without having to register each copy individually.

Colman is not limited to school libraries: it is also suitable for managing
private collections of books, CDs, DVDs, board games or other items, and for
keeping track of what you have lent to whom.  Its main focus and most
thorough testing, however, have been on school library use; private
collections are supported but have not been tested as extensively.

In addition to the standard single-machine mode, colman supports a
**REST mode**: a dedicated server process exposes the collection via an HTTP
API, and one or more `colman_rest_client` instances connect to it.
Three authentication schemes are supported:

- **Local users** — username/password pairs stored in the database (Argon2
  hashes).  No external service required; suitable for development and
  small deployments.
- **JWT/OIDC** — RS256 bearer tokens issued by an OpenID Connect provider
  (e.g. [Authentik](https://goauthentik.io/)).  Recommended for production
  deployments where users are managed centrally.
- **Barcode login** — password-less login for reserve-only persons (pupils
  who may only view the catalogue and place reservations).  Enabled with
  `--allow-barcode-login`; the person's library ID-card barcode is used as
  the credential.

**Author:** Christoph Pesch  
**License:** [GNU General Public License v3.0 or later](https://www.gnu.org/licenses/gpl-3.0.txt)

For build and installation instructions see [INSTALL.md](INSTALL.md).
A step-by-step tutorial (English and German) is available online at
<https://pesch-1.github.io/colman/> (HTML and PDF).  It and the developer
documentation are generated with Doxygen
(`cmake --build bld --target tutorial_en`, `--target doc`; see
[INSTALL.md](INSTALL.md#generating-documentation)).
Third-party material and its licenses are listed in [NOTICE](NOTICE);
[ACKNOWLEDGEMENTS.md](ACKNOWLEDGEMENTS.md) thanks everyone whose work
colman is built upon.
Contributions are welcome — see [CONTRIBUTING.md](CONTRIBUTING.md); please
report security issues as described in [SECURITY.md](SECURITY.md).
How colman came about, and how and why AI assistants were used to develop
it, is told in [STORY.md](STORY.md).

---

## Thermal label printers

colman prints labels and spine signatures on thermal label printers
pixel-exact, as a raw TSPL job sent directly over USB.  So far two models
have been tested and are recognized automatically:

| Model | USB ID | Paper-out warning |
|---|---|---|
| Vevor Y486 | `5958:0486` | yes |
| Munbyn RW403B | `0d28:ccdd` | no — the firmware does not report its status |

Only these two are offered when they are connected via USB without a queue
in the print system.  Other label printers registered in CUPS are detected
by their name and label-sized paper formats and printed via CUPS, but have
not been tested.

Help with supporting and testing further models is very welcome — see
[CONTRIBUTING.md](CONTRIBUTING.md#thermal-label-printers).  And if you
would like to lend the author a specific printer model for this, please get
in touch via an issue :-)

---

## Single-machine mode

In the standard single-machine mode, colman stores everything in a local
`.col` SQLite database file.  No server process or network connection is
needed.

```bash
# Open an existing database
./bld/colman /path/to/school.col

# Create a new database (colman prompts for an admin password)
./bld/colman /path/to/new.col
```

If no path is supplied, colman opens a file-chooser dialog on startup.

---

## REST mode — server and client

In REST mode the SQLite database lives on one machine (the *server*) and any
number of `colman_rest_client` instances connect to it over HTTP.

### Architecture overview

```
┌────────────────────────┐        HTTP/JSON        ┌──────────────────────┐
│  colman_rest_client    │ ──────────────────────► │  colman_rest_server  │
│  (full colman UI)      │  Basic Auth  or  JWT     │  (SQLite database)   │
└────────────────────────┘                          └──────────────────────┘
                                                              ▲
                                                    RS256 JWT validation
                                                         (optional)
                                                              │
                                                    ┌─────────┴────────┐
                                                    │  OIDC provider   │
                                                    │  (e.g. Authentik)│
                                                    └──────────────────┘
```

---

### Local user authentication (no external provider)

This is the simplest setup and is ideal for local testing or small
deployments where an OIDC provider is not available.

#### Starting the server

On first run the server creates a new database and prompts for the
administrator password interactively:

```bash
./bld/colman_rest_server --db /tmp/school.col --port 8080
# Database not found — creating new database: /tmp/school.col
# Enter administrator password: ••••••••
# Confirm administrator password: ••••••••
# Database created. REST login: username=admin
# JWT authentication disabled — local credentials only
# Listening on port 8080
```

On subsequent runs the same command opens the existing database directly:

```bash
./bld/colman_rest_server --db /tmp/school.col --port 8080
# JWT authentication disabled — local credentials only
# Listening on port 8080
```

#### Starting the client — login dialog

Without any credential arguments the client shows a login dialog:

```bash
./bld/colman_rest_client
```

Enter `http://localhost:8080` as the server URL, `admin` as the username,
and the password chosen during the first server start.  The dialog
remembers the server URL for the next session.

#### Starting the client — command-line credentials

For scripting or automated testing, supply the credentials directly:

```bash
./bld/colman_rest_client \
  --server   http://localhost:8080 \
  --username admin \
  --password mysecret
```

#### Complete localhost example

Open two terminals on the same machine:

```
Terminal 1 — server
  $ ./bld/colman_rest_server --db /tmp/school.col --port 8080

Terminal 2 — client
  $ ./bld/colman_rest_client \
      --server   http://localhost:8080 \
      --username admin \
      --password mysecret
```

The client opens the full colman UI; all operations are forwarded to the
server over HTTP.

---

### JWT/OIDC authentication (optional, for production)

Pass `--jwks-uri` and `--issuer` to enable JWT validation.  Local Basic
Auth remains active alongside JWT, so admin access is never lost.

### Choosing an OIDC provider

[**Authentik**](https://goauthentik.io/) is the recommended provider.
It is open source (MIT), actively maintained, and supports the
Device Authorization (device-code) flow that works well for desktop
applications which cannot redirect a browser to `localhost`.

Other options:

| Provider | Notes |
|---|---|
| [Keycloak](https://www.keycloak.org/) | Very widely deployed; heavier (Java) |
| [Dex](https://dexidp.io/) | Minimalist; single Go binary |
| [Zitadel](https://zitadel.com/) | Modern UI; Docker-based |

#### Running Authentik locally for testing

Docker Compose is the quickest way to get a local Authentik instance:

```bash
curl -o docker-compose.yml https://goauthentik.io/docker-compose.yml
docker compose up -d
```

Authentik is then available at `http://localhost:9000`.
Open `http://localhost:9000/if/flow/initial-setup/` to set the admin
password on first run.

Configure a new **OAuth2/OpenID Connect Provider** in Authentik and note
the issuer URL and JWKS URI — these are the values for `--issuer` and
`--jwks-uri` when starting `colman_rest_server`.

#### Testing without an external provider

For local development without an OIDC provider, use the local user
authentication described above — no JWT configuration is needed at all.

The automated integration tests in `tests/colman_rest_queries_test.cpp` use a
hardcoded RSA-2048 key pair and load the JWKS directly via
`JwtValidator::load_jwks()` — no external service is needed for CI.

### Prerequisites

Each colman user must exist as a person in the collection **and** have their
OIDC subject (`sub` claim) stored as their external ID.  This links the JWT
identity to a colman account and its permission set.

### Starting the server with JWT

```bash
./bld/colman_rest_server \
  --db        /var/lib/colman/school.col \
  --port      8080 \
  --jwks-uri  https://auth.example.com/application/o/colman/jwks/ \
  --issuer    https://auth.example.com/application/o/colman/
```

| Option | Description |
|---|---|
| `--db` | Path to the `.col` SQLite database file (created on first run) |
| `--port` | TCP port to listen on (default: 8080) |
| `--jwks-uri` | *(optional)* URL from which the server fetches the OIDC provider's public keys; omit to disable JWT validation |
| `--issuer` | Expected `iss` claim in every JWT (required when `--jwks-uri` is set) |
| `--audience` | *(optional)* Expected `aud` claim; omit to skip audience validation |
| `--allow-barcode-login` | *(optional)* Enable password-less barcode login for reserve-only persons (see below) |

The server prints `Listening on port 8080` when ready.

### Barcode login (password-less, reserve-only persons)

When the server is started with `--allow-barcode-login`, persons whose
permission set is restricted to **at most** the following three permissions
may log in using only their library ID-card barcode — no password required:

| Permission | Meaning |
|---|---|
| `view_personal_data` | View own personal data |
| `reserve` | Place reservations |
| `view_items` | Browse the catalogue |

This is intended for pupils who use the self-service reservation feature.
The client sends the barcode as the HTTP Basic Auth username with an empty
password:

```
Authorization: Basic base64(<barcode>:)
```

If a person has any additional permission beyond the three listed above, the
barcode login attempt is rejected with HTTP 401.  Normal username/password
and JWT authentication are unaffected and always remain available.

### Obtaining a token

How you obtain a JWT depends on your OIDC provider.  With Authentik and the
`oauth2-device-code` flow (suitable for desktop apps):

```bash
# 1. Request a device code
curl -s -X POST https://auth.example.com/application/o/device/ \
  -d "client_id=colman" \
  | jq -r '.verification_uri_complete, .device_code'
# → open the URL in a browser and approve the login

# 2. Poll for the token (replace <device_code> with the value from step 1)
TOKEN=$(curl -s -X POST https://auth.example.com/application/o/token/ \
  -d "grant_type=urn:ietf:params:oauth:grant-type:device_code" \
  -d "client_id=colman" \
  -d "device_code=<device_code>" \
  | jq -r '.access_token')
```

### Starting the client

```bash
./bld/colman_rest_client \
  --server https://colman.example.com:8080 \
  --token  "$TOKEN"
```

| Option | Description |
|---|---|
| `--server` | Base URL of the running `colman_rest_server` (no trailing slash) |
| `--token` | Bearer JWT obtained from the OIDC provider (takes priority over `--username`/`--password`) |
| `--username` | Username for HTTP Basic Auth (local user) |
| `--password` | Password for HTTP Basic Auth (local user) |
| `--timeout-ms` | *(optional)* HTTP request timeout in milliseconds (default: 10000) |

If neither `--token` nor the full `--username`/`--password` pair is given,
a login dialog is shown on startup.

The client opens the standard colman interface.  All data operations are
forwarded to the server; no local database file is needed.

### Example: two clients sharing one database (JWT mode)

```
Machine A (server)
  $ ./colman_rest_server --db /data/school.col --port 8080 \
      --jwks-uri https://auth.example.com/.../jwks/ \
      --issuer   https://auth.example.com/.../

Machine B (librarian)
  $ ./colman_rest_client --server http://machine-a:8080 --token "$TOKEN_B"

Machine C (data collector)
  $ ./colman_rest_client --server http://machine-a:8080 --token "$TOKEN_C"
```

Both clients work simultaneously against the same collection.  Each user's
permissions (lend, acquire, view history, …) are enforced on the server based
on the colman permission set assigned to their account.

---

## Admin tools

### colman_transfer

`colman_transfer` is a command-line tool for bulk import and export of
persons (users) and items (media).  It operates directly on a `.col` database
file and does not require the colman GUI.

**Persons** are transferred as RFC 4180 CSV; **items** as a JSON array
(their structure is too complex for plain CSV due to type-specific and
repeatable fields).

```
Usage: colman_transfer [options] <database.col> <data-file>

Options:
  -u, --users        Operate on persons/users
  -i, --items        Operate on items/media
  -e, --export       Export data to file
  -m, --import       Import data from file
  -c, --clear        Delete all existing records before import
  -p, --password     Database encryption password (prompted if needed)
```

```bash
# Export all persons to CSV
./bld/colman_transfer --users --export school.col users.csv

# Import persons (add new, update existing by id)
./bld/colman_transfer --users --import school.col users.csv

# Replace all persons with CSV content
./bld/colman_transfer --users --import --clear school.col users.csv

# Export all items to JSON
./bld/colman_transfer --items --export school.col items.json

# Import items into an existing database
./bld/colman_transfer --items --import school.col items.json
```

For full documentation see [doc/colman_admin_tools.dox](doc/colman_admin_tools.dox).

---

### colman_gen_testdb

`colman_gen_testdb` is a command-line tool that populates a `.col` database
with randomly generated items.  It is intended for performance testing with
large datasets (e.g. to verify the search page pagination threshold).

```bash
# Create a new database with 2000 items (admin password defaults to "admin")
./bld/colman_gen_testdb /tmp/big.col 2000

# Create a new database with a custom admin password
./bld/colman_gen_testdb --password secret /tmp/big.col 2000

# Append 500 more items to an existing database
./bld/colman_gen_testdb /tmp/big.col 500
```

If the database file does not exist it is created and the administrator
password is set to `"admin"` (override with `--password`); otherwise items are
appended to the existing collection.  Each item gets a random name, type,
authors, publisher, publication date, and subject.  No GUI or running server is
needed.

---

## Code style

All C++ source files follow the rules documented in
[`src/colman_coding_style.hpp`](src/colman_coding_style.hpp) /
[`src/colman_coding_style.cpp`](src/colman_coding_style.cpp).
Key points:

- 2-space indent, 80-character line limit, C++14 or later
- East const (`int const&`), `*`/`&` adjacent to the type
- `CamelCase` for classes and type aliases; `snake_case` for functions
  and variables; trailing `_` for private data members
- No `using namespace`; anonymous namespace for file-local symbols
- `enum struct`, `using` instead of `typedef`, `constexpr` for constants
- Allman braces for functions/classes; same-line brace for single-line
  control conditions
- All includes use a guard pattern (`#ifndef / #include / #define`);
  do **not** sort includes

The project ships a `.clang-format` file that encodes the formatting
rules.  `clang-format` (version 17 or later) can be used to apply them
automatically.
