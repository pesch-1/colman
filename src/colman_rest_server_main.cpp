// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Entry point for the colman REST server.
/**
   @file
   @author Christoph Pesch
*/

#ifndef SYSTEM_QCOREAPPLICATION
#  include <QCoreApplication>
#  define SYSTEM_QCOREAPPLICATION
#endif
#ifndef SYSTEM_QLIBRARYINFO
#  include <QLibraryInfo>
#  define SYSTEM_QLIBRARYINFO
#endif
#ifndef SYSTEM_QLOCALE
#  include <QLocale>
#  define SYSTEM_QLOCALE
#endif
#ifndef SYSTEM_QTRANSLATOR
#  include <QTranslator>
#  define SYSTEM_QTRANSLATOR
#endif
#ifndef SYSTEM_QCOMMANDLINEPARSER
#  include <QCommandLineParser>
#  define SYSTEM_QCOMMANDLINEPARSER
#endif
#ifndef SYSTEM_QCOMMANDLINEOPTION
#  include <QCommandLineOption>
#  define SYSTEM_QCOMMANDLINEOPTION
#endif
#ifndef SYSTEM_QINFO
#  include <QDebug>
#  define SYSTEM_QINFO
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_QFILEINFO
#  include <QFileInfo>
#  define SYSTEM_QFILEINFO
#endif
#ifndef SYSTEM_QLOCKFILE
#  include <QLockFile>
#  define SYSTEM_QLOCKFILE
#endif
#ifndef SYSTEM_QDIR
#  include <QDir>
#  define SYSTEM_QDIR
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef COLMAN_PASSWORDS_HPP
#  include "colman_passwords.hpp"
#  define COLMAN_PASSWORDS_HPP
#endif
#ifndef COLMAN_JWT_VALIDATOR_HPP
#  include "colman_jwt_validator.hpp"
#endif
#ifndef COLMAN_AUTH_MIDDLEWARE_HPP
#  include "colman_auth_middleware.hpp"
#endif
#ifndef COLMAN_REST_SERVER_HPP
#  include "colman_rest_server.hpp"
#endif
#ifndef COLMAN_TRACE_HPP
#  include "colman_trace.hpp"
#endif
#ifndef COLMAN_MDNS_ANNOUNCER_HPP
#  include "colman_mdns_announcer.hpp"
#endif
#ifndef SYSTEM_QHOSTINFO
#  include <QHostInfo>
#  define SYSTEM_QHOSTINFO
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif
#ifndef SYSTEM_OPTIONAL
#  include <optional>
#  define SYSTEM_OPTIONAL
#endif
#ifndef SYSTEM_CSTDIO
#  include <cstdio>
#  define SYSTEM_CSTDIO
#endif
#ifndef SYSTEM_CSTRING
#  include <cstring>
#  define SYSTEM_CSTRING
#endif
#include <termios.h>
#include <unistd.h>

namespace
{

/// Reads a line from stdin without echoing characters.
/**
 * Falls back to plain reading if stdin is not a terminal (e.g. piped input).
 * Strips trailing CR/LF.
 * @return the entered line, or std::nullopt on end of input.
 */
  std::optional<QString> read_password(char const* prompt)
  {
    struct termios old_term;
    bool const is_tty = (tcgetattr(STDIN_FILENO, &old_term) == 0);
    if(is_tty) {
      struct termios no_echo = old_term;
      no_echo.c_lflag &= ~static_cast<tcflag_t>(ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &no_echo);
    }
    std::fprintf(stderr, "%s", prompt);
    std::fflush(stderr);
    char buf[1024];
    std::memset(buf, 0, sizeof(buf));
    bool const got_line =
        std::fgets(buf, static_cast<int>(sizeof(buf)), stdin) != nullptr;
    if(is_tty) {
      tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
      std::fprintf(stderr, "\n");
    }
    if(!got_line) {
      return std::nullopt;
    }
    QString pw = QString::fromUtf8(buf);
    while(pw.endsWith('\n') || pw.endsWith('\r')) {
      pw.chop(1);
    }
    return pw;
  }

/// Reads a plain text line from stdin with echo, stripping trailing CR/LF.
/** @return the entered line, or std::nullopt on end of input. */
  std::optional<QString> read_line(char const* prompt)
  {
    std::fprintf(stderr, "%s", prompt);
    std::fflush(stderr);
    char buf[1024];
    std::memset(buf, 0, sizeof(buf));
    if(!std::fgets(buf, static_cast<int>(sizeof(buf)), stdin)) {
      return std::nullopt;
    }
    QString line = QString::fromUtf8(buf);
    while(line.endsWith('\n') || line.endsWith('\r')) {
      line.chop(1);
    }
    return line;
  }

/// Returns the database encryption key from a key file or environment variable.
/**
 * If \a key_file_path is non-empty the key is read from that file (full
 * content, stripped of surrounding whitespace).  Otherwise the environment
 * variable @c COLMAN_DB_KEY is used.  Returns an empty string when neither
 * source is configured (unencrypted database).
 * Returns std::nullopt when the key file was specified but could not be read.
 * @param key_file_path  path passed via --key-file, or empty string
 */
  std::optional<QString> resolve_db_key(QString const& key_file_path)
  {
    if(!key_file_path.isEmpty()) {
      QFile f(key_file_path);
      if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Cannot read key file:" << key_file_path;
        return std::nullopt;
      }
      return QString::fromUtf8(f.readAll()).trimmed();
    }
    return QString::fromUtf8(qgetenv("COLMAN_DB_KEY"));
  }

/// Creates a new database at \a db_path and configures the administrator.
/**
 * Uses the file stem as the collection name.
 * Prompts for a server display name and a password twice; loops until both
 * password entries match and are non-empty.  Aborts when stdin reaches end
 * of input (e.g. redirected from an exhausted pipe), so headless starts
 * fail cleanly instead of looping.  The database is only created after all
 * input has been gathered.  Sets the password for both desktop login and
 * the REST API account (username "admin").
 * If \a key is non-empty the database is encrypted with that passphrase.
 * @return true on success
 */
  bool bootstrap_database(QString const& db_path, colman::SQLQueries& queries,
                          QString const& key)
  {
    qInfo() << "Database not found — creating new database:" << db_path;
    colman::Collection col;
    col.set_name(QFileInfo(db_path).completeBaseName());
    QString server_name;
    while(server_name.trimmed().isEmpty()) {
      auto const line = read_line(
          "Enter a name for this server "
          "(shown to clients on the LAN): ");
      if(!line) {
        std::fprintf(stderr, "\nEnd of input — aborting setup.\n");
        return false;
      }
      server_name = *line;
    }
    col.set_server_name(server_name.trimmed());
    QString hash;
    for(;;) {
      auto const pw1 = read_password("Enter administrator password: ");
      if(!pw1) {
        std::fprintf(stderr, "\nEnd of input — aborting setup.\n");
        return false;
      }
      if(pw1->isEmpty()) {
        std::fprintf(stderr, "Password must not be empty.\n");
        continue;
      }
      auto const pw2 = read_password("Confirm administrator password: ");
      if(!pw2) {
        std::fprintf(stderr, "\nEnd of input — aborting setup.\n");
        return false;
      }
      if(*pw1 != *pw2) {
        std::fprintf(stderr, "Passwords do not match. Try again.\n");
        continue;
      }
      hash = colman::hash_password(*pw1);
      break;
    }
    if(!queries.create_new_database(col, db_path, key)) {
      qCritical() << "Failed to create database:" << db_path;
      return false;
    }
    if(!queries.set_password(0, hash)) {
      qCritical() << "Failed to store administrator password.";
      return false;
    }
    if(!queries.set_local_credentials(0, "admin", hash)) {
      qCritical() << "Failed to store administrator REST credentials.";
      return false;
    }
    qInfo() << "Database created. REST login: username=admin";
    return true;
  }

} // namespace

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  // Translations as in the desktop application, so that data the server
  // creates (e.g. the default signature categories of a new collection)
  // follows the system language.
  QTranslator translator_qt;
  static_cast<void>(
      translator_qt.load(QLocale::system(), "qtbase", "_",
                         QLibraryInfo::path(QLibraryInfo::TranslationsPath),
                         ".qm"));
  QTranslator translator;
  static_cast<void>(
      translator.load(QLocale::system(), "colman", "_", ":/translations",
                      ".qm"));
  app.installTranslator(&translator_qt);
  app.installTranslator(&translator);
  QCoreApplication::setApplicationName(
      QStringLiteral("colman-rest-server"));
  QCoreApplication::setApplicationVersion(
      QStringLiteral("1.0"));

  QCommandLineParser parser;
  parser.setApplicationDescription(
      QStringLiteral("colman REST API server"));
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption db_option(
      QStringLiteral("db"),
      QStringLiteral("Path to .col database file "
                     "(created automatically if it does not exist)"),
      QStringLiteral("path"));
  parser.addOption(db_option);

  QCommandLineOption db_key_file_option(
      QStringLiteral("key-file"),
      QStringLiteral(
          "Path to a file containing the database encryption passphrase. "
          "Alternative: set the COLMAN_DB_KEY environment variable. "
          "Omit both for an unencrypted database."),
      QStringLiteral("path"));
  parser.addOption(db_key_file_option);

  QCommandLineOption port_option(
      QStringLiteral("port"),
      QStringLiteral("TCP port to listen on (default: 8080)"),
      QStringLiteral("port"),
      QStringLiteral("8080"));
  parser.addOption(port_option);

  QCommandLineOption jwks_option(
      QStringLiteral("jwks-uri"),
      QStringLiteral("JWKS endpoint URI for Authentik JWT validation "
                     "(optional; omit to use local credentials only)"),
      QStringLiteral("uri"));
  parser.addOption(jwks_option);

  QCommandLineOption issuer_option(
      QStringLiteral("issuer"),
      QStringLiteral("Expected JWT issuer claim (required with --jwks-uri)"),
      QStringLiteral("iss"));
  parser.addOption(issuer_option);

  QCommandLineOption audience_option(
      QStringLiteral("audience"),
      QStringLiteral("Expected JWT audience claim (optional)"),
      QStringLiteral("aud"));
  parser.addOption(audience_option);

  QCommandLineOption barcode_login_option(
      QStringLiteral("allow-barcode-login"),
      QStringLiteral("Allow reserve-only persons to authenticate "
                     "using only their ID-card barcode (no password)"));
  parser.addOption(barcode_login_option);

  QCommandLineOption cert_option(
      QStringLiteral("cert"),
      QStringLiteral("Path to PEM certificate file for HTTPS "
                     "(requires --key; omit for plain HTTP)"),
      QStringLiteral("path"));
  parser.addOption(cert_option);

  QCommandLineOption key_option(
      QStringLiteral("key"),
      QStringLiteral("Path to PEM private key file for HTTPS "
                     "(requires --cert; omit for plain HTTP)"),
      QStringLiteral("path"));
  parser.addOption(key_option);

  QCommandLineOption trace_level_option(
      QStringLiteral("trace-level"),
      QStringLiteral("Print diagnostic traces on the console "
                     "(0 = silent, 2 = internet search; default: 0)"),
      QStringLiteral("number"),
      QStringLiteral("0"));
  parser.addOption(trace_level_option);

  QCommandLineOption thumbnail_dir_option(
      QStringLiteral("thumbnail-dir"),
      QStringLiteral("Directory for storing thumbnail image files"),
      QStringLiteral("path"));
  parser.addOption(thumbnail_dir_option);

  parser.process(app);

  if(!colman::set_trace_level(parser.value(trace_level_option))) {
    qCritical() << "Invalid trace level:" << parser.value(trace_level_option);
    return 1;
  }

  auto const db_path = parser.value(db_option);
  auto const key_file_path = parser.value(db_key_file_option);
  auto const port_str = parser.value(port_option);
  auto const jwks_uri = parser.value(jwks_option);
  auto const issuer = parser.value(issuer_option);
  auto const audience = parser.value(audience_option);
  auto const allow_barcode_login = parser.isSet(barcode_login_option);
  auto const cert_path = parser.value(cert_option);
  auto const key_path = parser.value(key_option);
  auto const thumbnail_dir = parser.value(thumbnail_dir_option);

  if(db_path.isEmpty()) {
    qCritical() << "--db is required";
    return 1;
  }
  if(!jwks_uri.isEmpty() && issuer.isEmpty()) {
    qCritical() << "--issuer is required when --jwks-uri is specified";
    return 1;
  }
  bool const use_tls = !cert_path.isEmpty() || !key_path.isEmpty();
  if(use_tls && (cert_path.isEmpty() || key_path.isEmpty())) {
    qCritical() << "--cert and --key must be specified together";
    return 1;
  }

  auto const db_key_opt = resolve_db_key(key_file_path);
  if(!db_key_opt.has_value()) {
    return 1;
  }
  auto const db_key = *db_key_opt;
  if(!db_key.isEmpty()) {
    qInfo() << "Database encryption key loaded";
  }

  auto const port = static_cast<quint16>(port_str.toUInt());

  QLockFile db_lock(db_path + QStringLiteral(".lock"));
  db_lock.setStaleLockTime(0);
  if(!db_lock.tryLock()) {
    auto const err = db_lock.error();
    if(err == QLockFile::LockFailedError) {
      qCritical() << "Database is already open in another process:"
                  << db_path;
      return 1;
    }
    if(err == QLockFile::PermissionError) {
      qWarning() << "Could not create lock file — concurrent access "
                    "is not protected.";
    }
  }

  if(!thumbnail_dir.isEmpty()) {
    if(!QDir().mkpath(thumbnail_dir)) {
      qCritical() << "Cannot create thumbnail directory:" << thumbnail_dir;
      return 1;
    }
  }

  colman::SQLQueries queries{colman::SQLQueries::FileDb{},
      []{ qWarning() << "could not write backup file"; }};
  if(!thumbnail_dir.isEmpty()) {
    queries.set_thumbnail_dir(thumbnail_dir);
  }
  if(!QFile::exists(db_path)) {
    if(!bootstrap_database(db_path, queries, db_key)) {
      return 1;
    }
  }
  else {
    if(!queries.load_database(db_path, db_key)) {
      qCritical() << "Failed to open database:" << db_path;
      return 1;
    }
  }

  std::optional<colman::JwtValidator> validator;
  if(!jwks_uri.isEmpty()) {
    colman::JwtValidator::Config jwt_config;
    jwt_config.issuer = issuer;
    jwt_config.audience = audience;
    jwt_config.jwks_uri = jwks_uri;
    validator.emplace(jwt_config);
    if(!validator->refresh_jwks()) {
      qCritical() << "Failed to fetch JWKS from" << jwks_uri;
      return 1;
    }
    qInfo() << "JWT authentication enabled (issuer:" << issuer << ")";
  }
  else {
    qInfo() << "JWT authentication disabled — local credentials only";
  }

  if(allow_barcode_login) {
    qInfo() << "Barcode login enabled for reserve-only persons";
  }
  std::unique_ptr<colman::AuthMiddleware> auth;
  if(validator) {
    auth = std::make_unique<colman::AuthMiddleware>(
        *validator, queries, allow_barcode_login);
  }
  else {
    auth = std::make_unique<colman::AuthMiddleware>(
        queries, allow_barcode_login);
  }

  colman::RestServer server(queries, *auth);
  if(use_tls) {
    if(!server.listen_tls(port, cert_path, key_path)) {
      qCritical() << "Failed to start HTTPS server on port" << port;
      return 1;
    }
    qInfo() << "HTTPS (TLS) enabled";
  }
  else {
    if(!server.listen(port)) {
      qCritical() << "Failed to listen on port" << port;
      return 1;
    }
    qWarning() << "Running without TLS — use --cert and --key for HTTPS";
  }

  colman::MdnsAnnouncer::Config mdns_cfg;
  mdns_cfg.service_name =
      queries.read_collection().get_server_name();
  mdns_cfg.host_name =
      QHostInfo::localHostName() + QStringLiteral(".local");
  if(mdns_cfg.service_name.isEmpty()) {
    mdns_cfg.service_name = QHostInfo::localHostName();
  }
  mdns_cfg.port = port;
  mdns_cfg.tls = use_tls;
  colman::MdnsAnnouncer announcer(mdns_cfg);
  if(!announcer.start()) {
    qWarning() << "mDNS announcement failed (non-fatal)";
  }

  qInfo() << "Listening on port" << port;
  return app.exec();
}

/* EOF */
