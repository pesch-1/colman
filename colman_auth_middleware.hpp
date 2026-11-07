// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// AuthMiddleware: per-request JWT authentication and permission check.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_AUTH_MIDDLEWARE_HPP
#  define COLMAN_AUTH_MIDDLEWARE_HPP

#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif
#  ifndef SYSTEM_QDATETIME
#    include <QDateTime>
#    define SYSTEM_QDATETIME
#  endif
#  ifndef SYSTEM_QHASH
#    include <QHash>
#    define SYSTEM_QHASH
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{

  class IQueries;
  class JwtValidator;

/// Per-request authentication and authorization gate.
/**
 * Supports three auth schemes on each request:
 *  - `Authorization: Basic <base64>` — local username/password (Argon2).
 *  - `Authorization: Basic <base64>` with empty password — barcode-only login
 *    (requires allow_barcode_login=true at construction; person must be
 *    reserve-only, i.e. only view_personal_data / reserve / view_items set).
 *  - `Authorization: Bearer <jwt>`   — Authentik JWT (optional; requires
 *    a JwtValidator to be passed at construction time).
 *
 * Typical call sequence per HTTP request:
 * @code
 *   auto result = middleware.authenticate(
 *       request.headers().value("Authorization"),
 *       "collect_lendings");
 *   if(result.error != AuthMiddleware::AuthError::Ok)
 *       return respond(AuthMiddleware::http_status(result.error));
 *   // use result.person_id for business logic
 * @endcode
 *
 * AuthMiddleware holds non-owning references to IQueries and optionally
 * JwtValidator; both must outlive every authenticate() call.
 */
  class AuthMiddleware
  {
  public:
  /// Outcome of an authentication + authorization attempt.
    enum struct AuthError {
      Ok,           ///< Authenticated and authorized; person_id is valid.
      MissingToken, ///< Authorization header absent or scheme unsupported.
      InvalidToken, ///< Wrong password, or JWT malformed/expired/mis-issuer.
      NoAccount,    ///< Valid JWT but sub not linked to any colman person.
      Forbidden,    ///< Person exists but lacks the required permission.
    };

  /// Result returned by authenticate().
    struct AuthResult
    {
      AuthError error = AuthError::Ok; ///< Ok means proceed.
      int person_id = -1;        ///< Set only when error == Ok.
    };

  /// Constructs a middleware supporting both local and Authentik auth.
  /**
   * @param allow_barcode_login  When true, persons with at most reserve-only
   *   permissions may authenticate via Basic Auth with empty password and
   *   their ID-card barcode as username.
   */
    AuthMiddleware(JwtValidator& validator, IQueries& queries,
                   bool allow_barcode_login = false);
  /// Constructs a middleware with local auth only (Authentik disabled).
  /**
   * @param allow_barcode_login  When true, persons with at most reserve-only
   *   permissions may authenticate via Basic Auth with empty password and
   *   their ID-card barcode as username.
   */
    explicit AuthMiddleware(IQueries& queries,
                            bool allow_barcode_login = false);

  /// Authenticates the request and checks a single colman permission.
  /**
   * Dispatches on the Authorization header scheme:
   *  - `Basic`  → local credential lookup via IQueries::check_local_credentials.
   *  - `Bearer` → JWT validation via JwtValidator (401 if no validator set).
   *  - absent / other → MissingToken (401).
   *
   * @param authorization_header  Value of the HTTP Authorization header.
   * @param required_permission   Colman permission column name
   *        (e.g. "collect_lendings").  Pass an empty string to skip the
   *        permission check (authentication only).
   * @return AuthResult; person_id is populated when error == Ok.
   */
    AuthResult authenticate(QString const& authorization_header,
                            QString const& required_permission);

  /// Returns the HTTP status code that corresponds to \a error.
  /**
   * Ok → 200, MissingToken/InvalidToken → 401, NoAccount/Forbidden → 403.
   */
    static int http_status(AuthError error);

  private:
  /// Entry in the per-session credential cache.
    struct CachedAuth
    {
      int person_id = -1;   ///< Verified person id.
      QDateTime expires_at; ///< Wall-clock expiry time.
    };

    JwtValidator* validator_; ///< null if Authentik is disabled
    IQueries& queries_;
    bool allow_barcode_login_ = false;
  /// SHA-256(username+":"+password) → cached person_id + expiry.
    QHash<QByteArray, CachedAuth> auth_cache_;
  };

} // namespace colman

#endif /* #ifndef COLMAN_AUTH_MIDDLEWARE_HPP */

/* EOF */
