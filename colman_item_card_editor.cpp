// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of ItemCardEditor.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_item_card_editor.hpp"
#ifndef COLMAN_CARD_ELEMENT_ITEM_HPP
#  include "colman_card_element_item.hpp"
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef SYSTEM_QSTRINGLIST
#  include <QStringList>
#  define SYSTEM_QSTRINGLIST
#endif

/**
   Constructs the editor and calls setup_editor() to build the UI.
   @param queries database queries.
   @param parent  parent widget.
*/
colman::ItemCardEditor::ItemCardEditor(IQueries& queries, QWidget* parent)
    : CardEditorBase(queries, parent)
{
  setup_editor(
      tr("Label size"),
      tr("Free margin around the label edge (0 \xe2\x80\x93 10 mm)"),
      "item_card_editor",
      colman::LabelLayoutPurpose::ItemLabel);
}


/**
   Returns the card dimensions for @p p as-is (no landscape reordering).
*/
std::pair<double, double>
colman::ItemCardEditor::card_dims(LabelLayoutPreset const& p) const
{
  return {p.label_width, p.label_height};
}


/**
   Returns a sample item barcode for the preview.
*/
QString colman::ItemCardEditor::preview_barcode(Collection const& col) const
{
  int const default_type = col.get_default_item_type();
  QString t;
  if(default_type >= 0) {
    t = queries_.compute_item_barcode(default_type, 1);
  }
  if(t.isEmpty()) {
    t = col.get_barcode_prefix() + "0100001";
  }
  return t;
}


/**
   Returns sample text lines: organisation, a sample title and author,
   and (for book types) a placeholder ISBN and DDC class.
*/
QStringList
colman::ItemCardEditor::preview_text_lines(Collection const& col) const
{
  int const default_type = col.get_default_item_type();
  bool const is_book =
      queries_.media_kind_for_type_id(default_type) == colman::MediaKind::Book;
  QStringList lines;
  lines << col.get_organization()
        << tr("Sample Title")
        << tr("Sample Author");
  if(is_book) {
    lines << tr("978-0-000-00000-0")
          << tr("DDC 0");
  }
  return lines;
}


/**
   Configures text item for left-aligned, italic-emphasised rendering.
*/
void colman::ItemCardEditor::configure_text_item(CardElementItem* tx) const
{
  tx->set_text_halign(Qt::AlignLeft);
  tx->set_text_emph_italic(true);
}

/* EOF */
