// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_JWT_VALIDATOR_HPP
#  include "colman_jwt_validator.hpp"
#endif

#ifndef THIRDPARTY_JWT_CPP_JWT_H
#  include <jwt-cpp/jwt.h>
#  define THIRDPARTY_JWT_CPP_JWT_H
#endif

#ifndef SYSTEM_QEVENTLOOP
#  include <QEventLoop>
#  define SYSTEM_QEVENTLOOP
#endif
#ifndef SYSTEM_QMUTEXLOCKER
#  include <QMutexLocker>
#  define SYSTEM_QMUTEXLOCKER
#endif
#ifndef SYSTEM_QNETWORKACCESSMANAGER
#  include <QNetworkAccessManager>
#  define SYSTEM_QNETWORKACCESSMANAGER
#endif
#ifndef SYSTEM_QNETWORKREPLY
#  include <QNetworkReply>
#  define SYSTEM_QNETWORKREPLY
#endif
#ifndef SYSTEM_QNETWORKREQUEST
#  include <QNetworkRequest>
#  define SYSTEM_QNETWORKREQUEST
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QURL
#  include <QUrl>
#  define SYSTEM_QURL
#endif

namespace
{

/// Maps a jwt-cpp token_verification_exception to a colman JwtError code.
  colman::JwtError map_verification_error(
      jwt::error::token_verification_exception const& ex)
  {
    auto const code =
        static_cast<jwt::error::token_verification_error>(ex.code().value());
    switch(code) {
    case jwt::error::token_verification_error::token_expired:
      return colman::JwtError::Expired;
    case jwt::error::token_verification_error::audience_missmatch:
      return colman::JwtError::BadAudience;
    case jwt::error::token_verification_error::claim_value_missmatch:
      return colman::JwtError::BadIssuer;
    default:
      return colman::JwtError::BadSignature;
    }
  }

} // namespace

colman::JwtValidator::JwtValidator(Config config)
    : config_(std::move(config))
{
}

/// Parses \a json as a JWKS document and replaces the key cache.
/**
 * Iterates over the keys array, extracts RSA "n" and "e" components for
 * every key that has a "kid", converts them to a PEM public key, and stores
 * the result in key_map_.
 * @param json  raw JWKS JSON string
 * @return true if at least one key was loaded, false on error
 */
bool colman::JwtValidator::load_jwks(QString const& json)
{
  try {
    auto const jwks = jwt::parse_jwks(json.toStdString());
    std::map<std::string, std::string> new_map;
    for(auto const& jwk : jwks) {
      if(!jwk.has_key_id()) {
        continue;
      }
      if(!jwk.has_jwk_claim("n") || !jwk.has_jwk_claim("e")) {
        continue;
      }
      auto const kid = jwk.get_key_id();
      auto const n = jwk.get_jwk_claim("n").as_string();
      auto const e = jwk.get_jwk_claim("e").as_string();
      auto pem = jwt::helper::create_public_key_from_rsa_components(n, e);
      new_map.emplace(kid, std::move(pem));
    }
    if(new_map.empty()) {
      qWarning() << "JwtValidator::load_jwks: no usable RSA keys found";
      return false;
    }
    QMutexLocker locker(&jwks_mutex_);
    key_map_ = std::move(new_map);
    jwks_fetched_at_ = QDateTime::currentDateTimeUtc();
    return true;
  } catch(std::exception const& ex) {
    qWarning() << "JwtValidator::load_jwks: failed:" << ex.what();
    return false;
  }
}

/// Fetches a fresh JWKS document from the configured URI via HTTP GET.
/**
 * Performs a synchronous request using a local QEventLoop with a 10-second
 * timeout.
 * @return true if the fetch and parse both succeeded
 */
bool colman::JwtValidator::refresh_jwks()
{
  if(config_.jwks_uri.isEmpty()) {
    qWarning() << "JwtValidator::refresh_jwks: no jwks_uri configured";
    return false;
  }
  QNetworkAccessManager nam;
  QNetworkRequest const req{QUrl(config_.jwks_uri)};
  auto* reply = nam.get(req);
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  QTimer::singleShot(10000, &loop, &QEventLoop::quit);
  loop.exec();
  if(!reply->isFinished() || reply->error() != QNetworkReply::NoError) {
    qWarning() << "JwtValidator::refresh_jwks: request failed:"
               << reply->errorString();
    reply->deleteLater();
    return false;
  }
  auto const data = reply->readAll();
  reply->deleteLater();
  return load_jwks(QString::fromUtf8(data));
}

bool colman::JwtValidator::key_known(std::string const& kid) const
{
  return key_map_.count(kid) > 0;
}

/// Validates a raw Bearer token string and extracts sub and email claims.
/**
 * Flow:
 *  1. Decode the JWT (structural parse, no crypto).
 *  2. Extract the key id ("kid" header).
 *  3. Look up the public key in the cache; refresh once if not found.
 *  4. Verify signature, expiry, issuer, and optionally audience.
 *  5. Extract sub (required) and email (optional) claims.
 *
 * @param token  raw JWT string (without "Bearer " prefix)
 * @return JwtValidationResult; error == JwtError::Ok on success
 */
colman::JwtValidationResult colman::JwtValidator::validate(
    QString const& token)
{
  JwtValidationResult result;

  // Outer try catches malformed tokens (decode throws on bad structure).
  try {
    auto decoded = jwt::decode(token.toStdString());

    // Extract kid.
    std::string kid;
    try {
      kid = decoded.get_key_id();
    } catch(std::exception const&) {
      result.error = JwtError::Malformed;
      return result;
    }

    // Look up key; refresh once on cache miss (only if URI is configured).
    bool const cached = [&] {
      QMutexLocker locker(&jwks_mutex_);
      return key_known(kid);
    }();
    if(!cached) {
      if(config_.jwks_uri.isEmpty()) {
        result.error = JwtError::UnknownKey;
        return result;
      }
      if(!refresh_jwks()) {
        result.error = JwtError::InternalError;
        return result;
      }
      QMutexLocker locker(&jwks_mutex_);
      if(!key_known(kid)) {
        result.error = JwtError::UnknownKey;
        return result;
      }
    }

    // Retrieve PEM under lock.
    std::string pem;
    {
      QMutexLocker locker(&jwks_mutex_);
      pem = key_map_.at(kid);
    }

    // Verify signature and standard claims.
    try {
      auto verifier =
          jwt::verify()
              .allow_algorithm(jwt::algorithm::rs256(pem))
              .with_issuer(config_.issuer.toStdString())
              .leeway(60UL);
      if(!config_.audience.isEmpty()) {
        verifier.with_audience(config_.audience.toStdString());
      }
      verifier.verify(decoded);
    } catch(jwt::error::token_verification_exception const& ex) {
      result.error = map_verification_error(ex);
      return result;
    } catch(std::exception const&) {
      result.error = JwtError::BadSignature;
      return result;
    }

    // Extract claims.
    result.claims.sub = QString::fromStdString(decoded.get_subject());
    try {
      result.claims.email = QString::fromStdString(
          decoded.get_payload_claim("email").as_string());
    } catch(std::exception const&) {
      // email is optional
    }
  } catch(std::exception const&) {
    result.error = JwtError::Malformed;
  }

  return result;
}

/* EOF */
