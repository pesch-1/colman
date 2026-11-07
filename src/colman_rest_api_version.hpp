// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// REST API version constants and compatibility check.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_REST_API_VERSION_HPP
#  define COLMAN_REST_API_VERSION_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

namespace colman
{

/// Major version of the REST API.
/** Incremented on backwards-incompatible changes. */
  int constexpr ApiMajorVersion = 2;

/// Minor version of the REST API.
/** Incremented on backwards-compatible additions.  Minor 2: barcode
    format 2 (generated and parsed).  Minor 3: POST /api/lendings/extend
    and the action filter of GET /api/history.  Minor 4: school_class in
    the rows of GET /api/lendings/due.  Minor 5: POST /api/lendings/batch
    (group lending).  Minor 6: POST /api/returns/batch (group return) and
    school_class in the rows of POST /api/persons/lending-query. */
  int constexpr ApiMinorVersion = 6;

/// REST API version as reported by the server or required by the client.
  struct ApiVersion
  {
    int major_ = 0;    ///<
    int minor_ = 0;    ///<
    QString name_;   ///< server display name (added in minor 1)
  };

/// Returns true when \a server_v is compatible with the built-in client version.
/**
   Compatible means: same major number and server minor ≥ client minor.
   @param server_v Version reported by the server.
*/
  inline bool api_version_compatible(ApiVersion const& server_v)
  {
    return server_v.major_ == ApiMajorVersion &&
           server_v.minor_ >= ApiMinorVersion;
  }

} // namespace colman

#endif /* #ifndef COLMAN_REST_API_VERSION_HPP */

/* EOF */
