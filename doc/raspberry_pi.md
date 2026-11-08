# Raspberry Pi – REST Server Setup

This guide explains how to install the **colman REST server** on a
Raspberry Pi 4 (Raspberry Pi OS, Bookworm) and configure it to
start automatically on every boot.

---

## System requirements

- Raspberry Pi 4 (4 GB RAM recommended)
- Raspberry Pi OS, Debian Bookworm (Raspberry Pi OS 12), 32-bit (armhf) or 64-bit (arm64)
- Internet access during initial setup (cmake downloads jwt-cpp and
  SQLite3MultipleCiphers)

---

## Step 1 – Install dependencies

```bash
sudo apt update
sudo apt install -y build-essential cmake git libssl-dev libargon2-dev \
  qt6-base-dev qt6-httpserver-dev qt6-websockets-dev qt6-svg-dev \
  linguist-qt6
```

| Package | Purpose |
|---|---|
| `build-essential` | GCC, Make |
| `cmake` | Build system |
| `git` | Version control |
| `libssl-dev` | OpenSSL (JWT signature verification) |
| `libargon2-dev` | Argon2 password hashing |
| `qt6-base-dev` | Qt 6 (Core, Network, Sql, Widgets, …) |
| `qt6-httpserver-dev` | Qt 6 HTTP server module |
| `qt6-websockets-dev` | Qt 6 WebSockets (required by HttpServer) |
| `qt6-svg-dev` | Qt 6 SVG support |
| `linguist-qt6` | Qt 6 Linguist tools (translation support) |

---

## Step 2 – Get the source code

```bash
git clone <repo-url> colman
cd colman
```

Replace `<repo-url>` with the actual repository URL.

---

## Step 3 – Build the REST server

```bash
cmake -B bld
cmake --build bld --target server -j1
```

The cmake configuration step downloads `jwt-cpp` and SQLite3MultipleCiphers
(SQLite with encryption, linked into colman) automatically (FetchContent,
one-time, requires internet access). The `server` target
builds only `colman_rest_server` and its dependencies, without compiling
the desktop application.

Building needs a lot of main memory on the Raspberry Pi; compiling the
SQLite3MultipleCiphers amalgamation and the large colman sources in
parallel can exhaust the RAM and make the build fail or the system stall.
It is therefore recommended to build serially with a single process
(`-j1`, as above), even though this takes longer.

---

## Step 4 – Install the binary

```bash
sudo install -m 755 bld/colman_rest_server /usr/local/bin/
```

---

## Step 5 – Create system user and database directory

```bash
sudo useradd -r -s /usr/sbin/nologin colman
sudo mkdir -p /var/lib/colman
sudo chown colman:colman /var/lib/colman
```

The `colman` system user has no login shell and no home directory — it
exists solely to run the server.

---

## Step 6a – Set up an unencrypted database

On the first start with a new database file the server prompts for a
server display name (shown to clients on the LAN) and an administrator
password:

```bash
sudo -u colman /usr/local/bin/colman_rest_server \
  --db            /var/lib/colman/library.col \
  --thumbnail-dir /var/lib/colman/thumbnails
```

After the prompts the server runs normally. Press **Ctrl+C** to stop it
once setup is complete.

The REST login is then: username `admin`, the password chosen above.

---

## Step 6b – Set up an encrypted database (recommended)

An encrypted database guards against accidental use of the wrong collection
file and protects data when a backup is stolen.

### Create a key file

```bash
sudo sh -c 'openssl rand -base64 32 > /var/lib/colman/library.key'
sudo chmod 400 /var/lib/colman/library.key
sudo chown colman /var/lib/colman/library.key
```

### Bootstrap the database with encryption

```bash
sudo -u colman /usr/local/bin/colman_rest_server \
  --db            /var/lib/colman/library.col \
  --key-file      /var/lib/colman/library.key \
  --thumbnail-dir /var/lib/colman/thumbnails
```

After the prompts press **Ctrl+C** to stop the server.

If the database already exists unencrypted, open it with the desktop
application and use **Settings → Encrypt database** to add encryption
before running the server.

The server will refuse to start if the wrong key (or no key) is supplied,
so a mismatch is detected immediately at startup — not silently.

---

## Step 6c – Set up TLS/HTTPS (optional)

By default the server listens on plain HTTP.  Passing `--cert` and `--key`
switches it to HTTPS.  Both flags must be specified together.

### Option A – Self-signed certificate (LAN / school network)

A self-signed certificate is sufficient for a local network where clients
can be told to accept it once.

```bash
sudo mkdir -p /var/lib/colman/tls
sudo openssl req -x509 -newkey rsa:4096 -sha256 -days 3650 -nodes \
  -keyout /var/lib/colman/tls/server.key \
  -out    /var/lib/colman/tls/server.crt \
  -subj   "/CN=colman-nas" \
  -addext "subjectAltName=IP:$(hostname -I | awk '{print $1}')"
sudo chmod 400 /var/lib/colman/tls/server.key
sudo chmod 444 /var/lib/colman/tls/server.crt
sudo chown colman /var/lib/colman/tls/server.key
sudo chown colman /var/lib/colman/tls/server.crt
```

`CN=colman-nas` can be any label; it appears in the certificate details shown
by browsers or the Flutter app when accepting the certificate for the first
time.

The SAN extension (`subjectAltName`) is required by modern TLS clients and is
automatically set to the Pi's current IP address.  If the IP is fixed via DHCP
reservation or a static address, this value will remain valid.

### Option B – Let's Encrypt (internet-facing deployment)

If the Pi is reachable under a public domain name, obtain a certificate with
certbot:

```bash
sudo apt install -y certbot
sudo certbot certonly --standalone -d your.domain.example
```

The certificate and key are placed in
`/etc/letsencrypt/live/your.domain.example/`.  Grant the `colman` user read
access to the key:

```bash
sudo chmod 750 /etc/letsencrypt/live /etc/letsencrypt/archive
sudo chgrp colman /etc/letsencrypt/live /etc/letsencrypt/archive
```

Use `fullchain.pem` and `privkey.pem` as the values for `--cert` and `--key`
in Step 7 below.  Remember to reload the service after every renewal
(`ExecStartPost` or a renewal hook in `/etc/letsencrypt/renewal-hooks/deploy/`
calling `systemctl restart colman-rest`).

### Testing TLS before configuring the service

```bash
sudo -u colman /usr/local/bin/colman_rest_server \
  --db            /var/lib/colman/library.col \
  --cert          /var/lib/colman/tls/server.crt \
  --key           /var/lib/colman/tls/server.key \
  --port          8443 \
  --thumbnail-dir /var/lib/colman/thumbnails
```

From another machine:

```bash
curl -k https://<pi-ip>:8443/api/version
```

`-k` disables certificate verification for self-signed certificates.  Press
**Ctrl+C** to stop the test run.

---

## Step 7 – Set up the systemd service

Create `/etc/systemd/system/colman-rest.service`:

```bash
sudo nano /etc/systemd/system/colman-rest.service
```

Contents:

```ini
[Unit]
Description=Colman Library REST Server
After=network.target

[Service]
Type=simple
User=colman
Group=colman
ExecStart=/usr/local/bin/colman_rest_server --db /var/lib/colman/library.col --port 8080 --thumbnail-dir /var/lib/colman/thumbnails
Restart=on-failure
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

For an **encrypted database**, add `--key-file` to `ExecStart`:

```ini
ExecStart=/usr/local/bin/colman_rest_server \
    --db            /var/lib/colman/library.col \
    --port          8080 \
    --key-file      /var/lib/colman/library.key \
    --thumbnail-dir /var/lib/colman/thumbnails
```

For **TLS/HTTPS** (see Step 6c), add `--cert` and `--key` and use a different
port (conventionally 443 or 8443):

```ini
ExecStart=/usr/local/bin/colman_rest_server \
    --db            /var/lib/colman/library.col \
    --port          8443 \
    --cert          /var/lib/colman/tls/server.crt \
    --key           /var/lib/colman/tls/server.key \
    --thumbnail-dir /var/lib/colman/thumbnails
```

Optional parameters to append to `ExecStart`:

| Parameter | Description |
|---|---|
| `--port 8080` | TCP port (default: 8080) |
| `--key-file <path>` | Path to a file containing the database encryption passphrase |
| `--thumbnail-dir <path>` | Directory for storing cover image files (created on first start if absent) |
| `--cert <path>` | PEM certificate file — enables HTTPS (requires `--key`) |
| `--key <path>` | PEM private key file — enables HTTPS (requires `--cert`) |
| `--jwks-uri <uri>` | JWKS endpoint for Authentik JWT validation |
| `--issuer <iss>` | Expected JWT issuer claim (required with `--jwks-uri`) |
| `--audience <aud>` | Expected JWT audience claim (optional) |
| `--trace-level <n>` | Console diagnostics (default 0 = silent; 2 and above log internet search requests) |

**Alternative for multiple instances — shell script wrapper:**
When running several server instances (different databases, different ports)
on the same machine, each instance can get its own wrapper script that sets
`COLMAN_DB_KEY` and then calls the server executable.  The script must be
`chmod 500`, owned by `colman`, and must not be world-readable.  Note: with
this approach the key is present in the process environment and visible in
`/proc/<pid>/environ`; `--key-file` avoids that.

Enable and start the service:

```bash
sudo systemctl daemon-reload
sudo systemctl enable colman-rest
sudo systemctl start colman-rest
```

The server now starts automatically on every boot.

---

## Managing the service

```bash
sudo systemctl status colman-rest    # show status
sudo journalctl -u colman-rest -f    # follow live log
sudo systemctl restart colman-rest   # restart
sudo systemctl stop colman-rest      # stop
```

---

## Applying updates

```bash
cd colman
git pull
cmake --build bld --target server -j1
sudo install -m 755 bld/colman_rest_server /usr/local/bin/
sudo systemctl restart colman-rest
```
