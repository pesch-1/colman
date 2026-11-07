// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of IdEditor.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ID_EDITOR_HPP
#  define COLMAN_ID_EDITOR_HPP

#  ifndef COLMAN_CARD_EDITOR_BASE_HPP
#    include "colman_card_editor_base.hpp"
#  endif

namespace colman
{

/// Widget for interactively designing the layout of library ID cards.
/**
   Embeds a LabelLayoutWidget at the top so the user can pick a card
   size.  Below it shows a scaled preview of the card (landscape
   orientation) with three draggable, resizable zones:
   - Barcode zone: displays a Code128 barcode.
   - Text zone: displays library name, issue date and a sample name.
   - Logo zone: displays the library logo (if one is stored); can be
     rotated in 90° steps via right-click and maintains the logo's
     aspect ratio.

   The three zones must not overlap; placement is reverted if a drop
   would cause an overlap.

   Call current_layout() to retrieve the resulting IdCardLayout.
*/
  class IdEditor : public CardEditorBase
  {
    Q_OBJECT

  public:
    explicit IdEditor(IQueries& queries, QWidget* parent = nullptr);

  protected:
    std::pair<double, double> card_dims(LabelLayoutPreset const& p) const override;
    QString preview_barcode(Collection const& col) const override;
    QStringList preview_text_lines(Collection const& col) const override;
    int text_emph_line() const override { return 3; }
    double default_text_height_fraction() const override { return 0.55; }
  };

} // namespace colman

#endif /* #ifndef COLMAN_ID_EDITOR_HPP */

/* EOF */
