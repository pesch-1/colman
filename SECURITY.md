# Security Policy

colman stores personal data of pupils and staff.  Security problems are
therefore taken seriously — thank you for helping to keep colman
installations safe.

## Supported versions

colman is developed on a single main branch.  Security fixes are made
there and in the most recent release; older releases are not patched.

## Reporting a vulnerability

**Please do not report security problems in public issues, merge/pull
requests or discussions.**

Report them privately via the hosting platform instead:

- **GitHub:** open the repository's *Security* tab and choose
  *Report a vulnerability* (private security advisory).
- **GitLab:** open a new issue and tick *This issue is confidential*.

Please include:

- the affected component (desktop application, `colman_rest_server`,
  `colman_rest_client`, Flutter client) and version or commit,
- a description of the problem and its impact,
- steps or a proof of concept to reproduce it,
- if possible, a suggestion for a fix.

Do not include real personal data in the report.

You will receive an acknowledgement within one week.  After a fix is
available, the vulnerability is disclosed in the release notes; reporters
are credited unless they prefer to stay anonymous.

## Scope

Particularly relevant are:

- authentication and authorization of the REST server (local users with
  Argon2 password hashes, JWT/OIDC validation, barcode login, permission
  checks and permission delegation),
- TLS handling in the server and the clients,
- encryption of `.col` database files,
- injection issues (SQL, HTML in printed reports) and handling of untrusted
  data from external lookup services.

Out of scope are vulnerabilities in third-party services queried by
colman and in the libraries it links against (please report those to the
respective projects), as well as attacks that require prior administrative
access to the machine running colman.

## Operating colman securely

- `colman_rest_server` listens on all network interfaces.  Outside a
  trusted local network always enable TLS (`--cert` / `--key`); without
  TLS, passwords are transmitted in plain text.
- Only enable `--allow-barcode-login` if barcode-only access to the
  catalogue and reservations is acceptable for your installation.
- Store the file passed via `--key-file` (database encryption passphrase)
  with restrictive permissions, e.g. `chmod 600`, or provide the
  passphrase via the `COLMAN_DB_KEY` environment variable.
- Keep backups of `.col` files as protected as the database itself.
