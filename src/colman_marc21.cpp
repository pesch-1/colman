// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// MARC21 field definitions and type templates.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_marc21.hpp"

#ifndef SYSTEM_QCOREAPPLICATION
#  include <QCoreApplication>
#  define SYSTEM_QCOREAPPLICATION
#endif

namespace
{

  QVector<colman::Marc21Field> make_fields()
  {
    using DT = colman::DataType;
    QVector<colman::Marc21Field> v;

    // Standard Numbers
    v.push_back({"020", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "ISBN"),
                 "International Standard Book Number",
                 QT_TRANSLATE_NOOP("colman_marc21", "Standard Numbers"),
                 DT::ShortString});
    v.push_back({"022", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "ISSN"),
                 "International Standard Serial Number",
                 QT_TRANSLATE_NOOP("colman_marc21", "Standard Numbers"),
                 DT::ShortString});
    v.push_back({"028", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Publisher Number"),
                 "Publisher or distributor number (e.g. for audio/video)",
                 QT_TRANSLATE_NOOP("colman_marc21", "Standard Numbers"),
                 DT::ShortString});

    // Language
    v.push_back({"041", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Language"),
                 "Language code or name of the item",
                 QT_TRANSLATE_NOOP("colman_marc21", "Language"),
                 DT::ShortString});

    // Main Entry
    v.push_back({"100", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Author"),
                 "Personal name of the primary author",
                 QT_TRANSLATE_NOOP("colman_marc21", "Main Entry"),
                 DT::ShortString, true});
    v.push_back({"110", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Corporate Author"),
                 "Corporate name (institution, organisation) as primary entry",
                 QT_TRANSLATE_NOOP("colman_marc21", "Main Entry"),
                 DT::ShortString});

    // Title
    v.push_back({"245", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Title"),
                 "Title proper of the work",
                 QT_TRANSLATE_NOOP("colman_marc21", "Title"),
                 DT::ShortString});
    v.push_back({"245", "b",
                 QT_TRANSLATE_NOOP("colman_marc21", "Subtitle"),
                 "Remainder of title (subtitle)",
                 QT_TRANSLATE_NOOP("colman_marc21", "Title"),
                 DT::ShortString});
    v.push_back({"245", "c",
                 QT_TRANSLATE_NOOP("colman_marc21", "Statement of Responsibility"),
                 "Statement of responsibility relating to title",
                 QT_TRANSLATE_NOOP("colman_marc21", "Title"),
                 DT::ShortString});

    // Edition
    v.push_back({"250", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Edition"),
                 "Edition statement (e.g. \"2nd ed.\")",
                 QT_TRANSLATE_NOOP("colman_marc21", "Edition"),
                 DT::ShortString});

    // Classification
    v.push_back({"082", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "DDC"),
                 "Dewey Decimal Classification number",
                 QT_TRANSLATE_NOOP("colman_marc21", "Classification"),
                 DT::ShortString});

    // Publication
    v.push_back({"260", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Place of Publication"),
                 "Place of publication, distribution, or manufacture",
                 QT_TRANSLATE_NOOP("colman_marc21", "Publication"),
                 DT::ShortString});
    v.push_back({"260", "b",
                 QT_TRANSLATE_NOOP("colman_marc21", "Publisher"),
                 "Name of publisher, distributor, or manufacturer",
                 QT_TRANSLATE_NOOP("colman_marc21", "Publication"),
                 DT::ShortString});
    v.push_back({"260", "c",
                 QT_TRANSLATE_NOOP("colman_marc21", "Year of Publication"),
                 "Date of publication, distribution, or manufacture",
                 QT_TRANSLATE_NOOP("colman_marc21", "Publication"),
                 DT::ShortString});

    // Physical Description
    v.push_back({"300", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Extent"),
                 "Number of pages, discs, items, etc.",
                 QT_TRANSLATE_NOOP("colman_marc21", "Physical Description"),
                 DT::ShortString});
    v.push_back({"300", "b",
                 QT_TRANSLATE_NOOP("colman_marc21", "Illustrations"),
                 "Illustration code or description",
                 QT_TRANSLATE_NOOP("colman_marc21", "Physical Description"),
                 DT::ShortString});
    v.push_back({"300", "c",
                 QT_TRANSLATE_NOOP("colman_marc21", "Dimensions"),
                 "Physical dimensions (e.g. \"21 cm\")",
                 QT_TRANSLATE_NOOP("colman_marc21", "Physical Description"),
                 DT::ShortString});

    // Content / Media Type
    v.push_back({"336", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Content Type"),
                 "Content type (e.g. \"text\", \"still image\")",
                 QT_TRANSLATE_NOOP("colman_marc21", "Content / Media Type"),
                 DT::ShortString});
    v.push_back({"337", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Media Type"),
                 "Media type (e.g. \"unmediated\", \"audio\", \"video\")",
                 QT_TRANSLATE_NOOP("colman_marc21", "Content / Media Type"),
                 DT::ShortString});
    v.push_back({"338", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Carrier Type"),
                 "Carrier type (e.g. \"volume\", \"audio disc\", \"videodisc\")",
                 QT_TRANSLATE_NOOP("colman_marc21", "Content / Media Type"),
                 DT::ShortString});

    // Series
    v.push_back({"490", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Series"),
                 "Series statement",
                 QT_TRANSLATE_NOOP("colman_marc21", "Series"),
                 DT::ShortString});

    // Notes
    v.push_back({"500", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "General Note"),
                 "General note (free text)",
                 QT_TRANSLATE_NOOP("colman_marc21", "Notes"),
                 DT::String});
    v.push_back({"505", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Contents Note"),
                 "Formatted contents note (table of contents, track list, etc.)",
                 QT_TRANSLATE_NOOP("colman_marc21", "Notes"),
                 DT::String});
    v.push_back({"511", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Participant Note"),
                 "Participant or performer note (cast, credits)",
                 QT_TRANSLATE_NOOP("colman_marc21", "Notes"),
                 DT::String});
    v.push_back({"518", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Date/Time of Event"),
                 "Date or time information about an event (recording, performance)",
                 QT_TRANSLATE_NOOP("colman_marc21", "Notes"),
                 DT::ShortString});
    v.push_back({"520", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Summary"),
                 "Summary, abstract, review, or scope note",
                 QT_TRANSLATE_NOOP("colman_marc21", "Notes"),
                 DT::String});
    v.push_back({"521", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Target Audience"),
                 "Target audience (e.g. age level, reading level)",
                 QT_TRANSLATE_NOOP("colman_marc21", "Notes"),
                 DT::ShortString});

    // Subjects
    v.push_back({"600", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Subject: Person"),
                 "Subject added entry — personal name",
                 QT_TRANSLATE_NOOP("colman_marc21", "Subjects"),
                 DT::ShortString, true});
    v.push_back({"650", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Subject: Topic"),
                 "Subject added entry — topical term (keyword)",
                 QT_TRANSLATE_NOOP("colman_marc21", "Subjects"),
                 DT::ShortString, true});
    v.push_back({"651", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Subject: Geographic"),
                 "Subject added entry — geographic name",
                 QT_TRANSLATE_NOOP("colman_marc21", "Subjects"),
                 DT::ShortString, true});

    // Added Entries
    v.push_back({"700", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Additional Person"),
                 "Added entry — personal name (co-author, editor, illustrator, etc.)",
                 QT_TRANSLATE_NOOP("colman_marc21", "Added Entries"),
                 DT::ShortString, true});
    v.push_back({"710", "a",
                 QT_TRANSLATE_NOOP("colman_marc21", "Additional Corporate"),
                 "Added entry — corporate name",
                 QT_TRANSLATE_NOOP("colman_marc21", "Added Entries"),
                 DT::ShortString, true});

    // Electronic Access
    v.push_back({"856", "u",
                 QT_TRANSLATE_NOOP("colman_marc21", "URL"),
                 "Uniform Resource Identifier (link to online resource)",
                 QT_TRANSLATE_NOOP("colman_marc21", "Electronic Access"),
                 DT::ShortString});

    return v;
  }


  QVector<colman::Marc21Template> make_templates()
  {
    QVector<colman::Marc21Template> t;

    t.push_back({"Book",
                 {"100$a", "020$a", "250$a", "260$b",
                  "260$c", "300$a", "082$a"}});

    t.push_back({"CD / Audio",
                 {"245$a", "100$a", "028$a", "260$b",
                  "260$c", "041$a", "300$a",
                  "336$a", "337$a", "338$a",
                  "520$a", "650$a"}});

    t.push_back({"DVD / Video",
                 {"245$a", "700$a", "260$c", "041$a",
                  "300$a", "336$a", "337$a", "338$a",
                  "511$a", "520$a", "521$a", "650$a"}});

    t.push_back({"Periodical",
                 {"022$a", "245$a", "260$b", "260$c",
                  "300$a", "650$a", "500$a"}});
    return t;
  }

} // anonymous namespace


QString colman::marc21_id(Marc21Field const& field)
{
  return field.tag + "$" + field.subfield;
}


QVector<colman::Marc21Field> const& colman::marc21_fields()
{
  static QVector<Marc21Field> const fields = make_fields();
  return fields;
}


colman::Marc21Field const* colman::find_marc21_field(QString const& id)
{
  for(auto const& f : marc21_fields()) {
    if(marc21_id(f) == id) {
      return &f;
    }
  }
  return nullptr;
}


QVector<colman::Marc21Template> const& colman::marc21_templates()
{
  static QVector<Marc21Template> const templates = make_templates();
  return templates;
}


QString colman::marc21_common_field_tag(QString const& sentinel)
{
  if(sentinel == "__name__") {
    return "245$a";
  }
  if(sentinel == "__description__") {
    return "520$a";
  }
  if(sentinel == "__keywords__") {
    return "650$a";
  }
  return {};
}


QString colman::marc21_translated_display_name(Marc21Field const& field)
{
  return QCoreApplication::translate("colman_marc21",
                                     field.display_name.toUtf8().constData());
}


QString colman::marc21_translated_category(Marc21Field const& field)
{
  return QCoreApplication::translate("colman_marc21",
                                     field.category.toUtf8().constData());
}

/* EOF */
