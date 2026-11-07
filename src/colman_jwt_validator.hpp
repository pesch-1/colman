// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// JwtValidator: RS256 JWT validation against a JWKS endpoint.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_JWT_VALIDATOR_HPP
#  define COLMAN_JWT_VALIDATOR_HPP

#  ifndef SYSTEM_QDATETIME
#    include <QDateTime>
#    define SYSTEM_QDATETIME
#  endif
#  ifndef SYSTEM_QMUTEX
#    include <QMutex>
#    define SYSTEM_QMUTEX
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  include <map>
#  include <string>

namespace colman
{

/// Claims extracted from a validated JWT.
  struct JwtClaims
  {
    QString sub;   ///< Subject claim (Authentik user UUID).
    QString email; ///< Email claim (may be empty).
  };

/// Outcome of a JWT validation attempt.
  enum struct JwtError {
    Ok,           ///< Token is valid; claims are populated.
    Malformed,    ///< Token cannot be decoded (bad format or missing fields).
    UnknownKey,   ///< Key id not in JWKS, even after a refresh attempt.
    BadSignature, ///< Cryptographic signature verification failed.
    Expired,      ///< Token exp claim is in the past.
    BadIssuer,    ///< iss claim does not match the configured issuer.
    BadAudience,  ///< aud claim does not match the configured audience.
    InternalError,///< JWKS fetch or parse failure.
  };

/// Result returned by JwtValidator::validate().
  struct JwtValidationResult
  {
    JwtError error = JwtError::Ok; ///< Error code; Ok means success.
    JwtClaims claims;             ///< Populated only when error == Ok.
  };

/// Validates RS256-signed JWTs against a cached JWKS key set.
/**
 * The validator caches RSA public keys parsed from a JWKS JSON document.
 * Keys are loaded either directly via load_jwks() (useful in tests) or
 * fetched from a remote URI via refresh_jwks().
 *
 * validate() is thread-safe: an internal mutex protects the key cache.
 * refresh_jwks() performs a synchronous HTTP GET using a temporary
 * QNetworkAccessManager and a QEventLoop; it must be called from a thread
 * that can run a local event loop (i.e. any thread, but not while an outer
 * QEventLoop for that thread is already running on the same stack).
 */
  class JwtValidator
  {
  public:
  /// Configuration passed to the constructor.
    struct Config
    {
      QString issuer;             ///< Expected iss claim value.
      QString audience;           ///< Expected aud claim (empty = not checked).
      QString jwks_uri;           ///< URI to fetch JWKS from (used by refresh_jwks()).
      int cache_ttl_secs = 3600;///< How long cached keys are considered fresh.
    };

    explicit JwtValidator(Config config);

  /// Parses \a json as a JWKS document and replaces the key cache.
  /**
   * Extracts every RSA key that carries both "n" and "e" components and
   * stores it as a PEM-encoded public key indexed by "kid".
   * @return true if at least one key was loaded, false on parse error or
   *         if no usable RSA keys were found.
   */
    bool load_jwks(QString const& json);

  /// Fetches a fresh JWKS document from the configured URI and loads it.
  /**
   * Performs a synchronous HTTP GET with a 10-second timeout.
   * @return true on success (at least one key loaded), false otherwise.
   */
    bool refresh_jwks();

  /// Validates a raw Bearer token string and returns the result.
  /**
   * If the token's key id is not in the cache, refresh_jwks() is called
   * once before giving up.
   */
    JwtValidationResult validate(QString const& token);

  private:
    bool key_known(std::string const& kid) const;

    Config config_;
    mutable QMutex jwks_mutex_;
    std::map<std::string, std::string> key_map_; ///< kid → PEM public key
    QDateTime jwks_fetched_at_;
  };

} // namespace colman

#endif /* #ifndef COLMAN_JWT_VALIDATOR_HPP */

/* EOF */
