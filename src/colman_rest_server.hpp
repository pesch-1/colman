// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// RestServer: QHttpServer-based HTTP API for colman.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_REST_SERVER_HPP
#  define COLMAN_REST_SERVER_HPP

#  ifndef SYSTEM_QHTTPSERVER
#    include <QHttpServer>
#    define SYSTEM_QHTTPSERVER
#  endif
#  ifndef SYSTEM_QOBJECT
#    include <QObject>
#    define SYSTEM_QOBJECT
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef COLMAN_AUTH_MIDDLEWARE_HPP
#    include "colman_auth_middleware.hpp"
#  endif

namespace colman
{

  class IQueries;

/// HTTP REST server for colman.
/**
 * Wraps QHttpServer and registers all /api/ routes.
 * Authenticates each request via AuthMiddleware.
 */
  class RestServer : public QObject
  {
    Q_OBJECT
  public:
  /// Constructs a RestServer with the given queries and auth middleware.
    explicit RestServer(IQueries& queries,
                        AuthMiddleware& auth,
                        QObject* parent = nullptr);

    virtual ~RestServer() = default;

  /// Starts listening on \a port (plain HTTP).
  /** @return true on success */
    bool listen(quint16 port);

  /// Starts listening on \a port with TLS (HTTPS).
  /**
   * Loads the PEM certificate from \a cert_path and the private key from
   * \a key_path.  Returns false and logs an error if either file cannot be
   * read or parsed, or if the server fails to bind.
   * @return true on success
   */
    bool listen_tls(quint16 port,
                    QString const& cert_path,
                    QString const& key_path);

  /// Starts listening on localhost with an OS-assigned port (for tests).
  /** @return the assigned port, or 0 on failure */
    quint16 listen_on_local_port();

  /// Returns the port the server is currently listening on, or 0.
    quint16 server_port() const;

  protected:
  /// Returns the underlying HTTP server (for use by subclasses).
    QHttpServer& http_server();

  /// Called at the end of setup_routes(); override to register extra routes.
    virtual void setup_extra_routes() {}

  private:
  /// Registers all route handlers on server_.
    void setup_routes();

  /// Returns an error response for the given auth error.
    QHttpServerResponse auth_error(
        AuthMiddleware::AuthError e) const;

    IQueries& queries_;       ///<
    AuthMiddleware& auth_;    ///<
    QHttpServer server_;      ///<
  };

} // namespace colman

#endif /* #ifndef COLMAN_REST_SERVER_HPP */

/* EOF */
