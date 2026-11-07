// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Implementation of IdEditor.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_id_editor.hpp"
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#endif
#ifndef SYSTEM_QDATE
#  include <QDate>
#  define SYSTEM_QDATE
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
colman::IdEditor::IdEditor(IQueries& queries, QWidget* parent)
    : CardEditorBase(queries, parent)
{
  setup_editor(
      tr("Card size"),
      tr("Free margin around the card edge (0 \xe2\x80\x93 10 mm)"),
      "id_editor",
      colman::LabelLayoutPurpose::IdCard);
}


/**
   Returns the card dimensions for @p p as-is (no landscape reordering),
   so the placement editor honours the chosen orientation and matches both
   the layout preview and the printed output.
*/
std::pair<double, double>
colman::IdEditor::card_dims(LabelLayoutPreset const& p) const
{
  return {p.label_width, p.label_height};
}


/**
   Returns a sample person barcode for the preview.
*/
QString colman::IdEditor::preview_barcode(Collection const& col) const
{
  Q_UNUSED(col);
  QString t = queries_.compute_person_barcode(7);
  if(t.isEmpty()) {
    t = "0000007";
  }
  return t;
}


/**
   Returns four sample text lines: library name, organisation, issue
   date, and a placeholder person name.
*/
QStringList colman::IdEditor::preview_text_lines(Collection const& col) const
{
  QString const date_str =
      tr("issued on: ") + QDate::currentDate().toString(tr("yyyy-MM-dd"));
  return {col.get_name(), col.get_organization(), date_str,
          tr("Sample Person")};
}

/* EOF */
