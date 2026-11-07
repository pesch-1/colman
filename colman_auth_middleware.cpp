// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_AUTH_MIDDLEWARE_HPP
#  include "colman_auth_middleware.hpp"
#endif
#ifndef COLMAN_JWT_VALIDATOR_HPP
#  include "colman_jwt_validator.hpp"
#endif
#ifndef COLMAN_IQUERIES_HPP
#  include "colman_iqueries.hpp"
#endif
#ifndef SYSTEM_QBYTEARRAY
#  include <QByteArray>
#  define SYSTEM_QBYTEARRAY
#endif
#ifndef SYSTEM_QCRYPTOGRAPHICHASH
#  include <QCryptographicHash>
#  define SYSTEM_QCRYPTOGRAPHICHASH
#endif
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
#endif

namespace
{
  char const* const kBearerPrefix = "Bearer ";
  int constexpr kBearerLen = 7;
  char const* const kBasicPrefix = "Basic ";
  int constexpr kBasicLen = 6;

/// Decodes a "Basic <base64>" header into username and password.
  bool decode_basic(QString const& header, QString& username, QString& password)
  {
    QByteArray const decoded =
        QByteArray::fromBase64(header.mid(kBasicLen).toUtf8());
    int const colon = decoded.indexOf(':');
    if(colon < 0) {
      return false;
    }
    username = QString::fromUtf8(decoded.left(colon));
    password = QString::fromUtf8(decoded.mid(colon + 1));
    return true;
  }
} // namespace

colman::AuthMiddleware::AuthMiddleware(JwtValidator& validator,
                                       IQueries& queries,
                                       bool allow_barcode_login)
    : validator_(&validator),
      queries_(queries),
      allow_barcode_login_(allow_barcode_login)
{
}

colman::AuthMiddleware::AuthMiddleware(IQueries& queries,
                                       bool allow_barcode_login)
    : validator_(nullptr),
      queries_(queries),
      allow_barcode_login_(allow_barcode_login)
{
}

/// Authenticates the request and checks a single colman permission.
/**
 * Dispatches on the Authorization header scheme:
 *  - Basic  → decode base64, call IQueries::check_local_credentials().
 *    Wrong credentials → InvalidToken.
 *  - Bearer → validate JWT via JwtValidator (InvalidToken if none configured),
 *    resolve sub → person_id via get_person_by_external_id().
 *    Unknown sub → NoAccount.
 *  - absent / other → MissingToken.
 *
 * On success, checks has_permission() if \a required_permission is non-empty.
 */
colman::AuthMiddleware::AuthResult colman::AuthMiddleware::authenticate(
    QString const& authorization_header,
    QString const& required_permission)
{
  AuthResult result;

  if(authorization_header.startsWith(
         QString::fromUtf8(kBasicPrefix))) {
    QString username, password;
    if(!decode_basic(authorization_header, username, password)) {
      result.error = AuthError::MissingToken;
      return result;
    }
    if(password.isEmpty() && allow_barcode_login_) {
      int barcode_id = -1;
      if(queries_.parse_person_barcode(username, barcode_id) &&
         queries_.is_reserve_only(barcode_id)) {
        if(!required_permission.isEmpty() &&
           !queries_.has_permission(barcode_id, required_permission)) {
          result.error = AuthError::Forbidden;
          return result;
        }
        result.person_id = barcode_id;
        return result;
      }
    }
  // Cache lookup: SHA-256(username+":"+password) → person_id.
  // Avoids running Argon2id on every authenticated request.
    QByteArray const cache_key = QCryptographicHash::hash(
        (username + QStringLiteral(":") + password).toUtf8(),
        QCryptographicHash::Sha256);
    {
      auto const it = auth_cache_.constFind(cache_key);
      if(it != auth_cache_.constEnd() &&
         it->expires_at > QDateTime::currentDateTime()) {
        int const person_id = it->person_id;
        if(!required_permission.isEmpty() &&
           !queries_.has_permission(person_id, required_permission)) {
          result.error = AuthError::Forbidden;
          return result;
        }
        result.person_id = person_id;
        return result;
      }
    }
    int const person_id =
        queries_.check_local_credentials(username, password);
    if(person_id < 0) {
      result.error = AuthError::InvalidToken;
      return result;
    }
    auth_cache_[cache_key] =
        CachedAuth{person_id, QDateTime::currentDateTime().addSecs(600)};
    if(!required_permission.isEmpty() &&
       !queries_.has_permission(person_id, required_permission)) {
      result.error = AuthError::Forbidden;
      return result;
    }
    result.person_id = person_id;
    return result;
  }

  if(!authorization_header.startsWith(
         QString::fromUtf8(kBearerPrefix))) {
    result.error = AuthError::MissingToken;
    return result;
  }
  if(validator_ == nullptr) {
    result.error = AuthError::InvalidToken;
    return result;
  }
  QString const token = authorization_header.mid(kBearerLen).trimmed();
  if(token.isEmpty()) {
    result.error = AuthError::MissingToken;
    return result;
  }

  JwtValidationResult const jwt = validator_->validate(token);
  if(jwt.error != JwtError::Ok) {
    result.error = AuthError::InvalidToken;
    return result;
  }

  int const person_id = queries_.get_person_by_external_id(jwt.claims.sub);
  if(person_id < 0) {
    result.error = AuthError::NoAccount;
    return result;
  }

  if(!required_permission.isEmpty() &&
     !queries_.has_permission(person_id, required_permission)) {
    result.error = AuthError::Forbidden;
    return result;
  }

  result.person_id = person_id;
  return result;
}

/// Returns the HTTP status code that corresponds to \a error.
int colman::AuthMiddleware::http_status(AuthError error)
{
  switch(error) {
  case AuthError::Ok:
    return 200;
  case AuthError::MissingToken:
  case AuthError::InvalidToken:
    return 401;
  case AuthError::NoAccount:
  case AuthError::Forbidden:
    return 403;
  }
  return 500;
}

/* EOF */
