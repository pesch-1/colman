// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Media kind used by internet title/barcode searches and stored per type.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_MEDIA_KIND_HPP
#  define COLMAN_MEDIA_KIND_HPP

namespace colman
{
  /// Media kind requested by a title search or detected in a MARC record.
  /**
     Stored as the integer @c media_kind column of the @c types table so that
     internet search can map a type to a search category independently of the
     type's (possibly localised) display name.
  */
  enum struct MediaKind { Unknown = 0,
                          Book = 1,
                          Cd = 2,
                          Dvd = 3,
                          BluRay = 4,
                          BoardGame = 5 };

} // namespace colman

#endif /* #ifndef COLMAN_MEDIA_KIND_HPP */

/* EOF */
