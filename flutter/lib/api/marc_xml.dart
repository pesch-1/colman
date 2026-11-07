// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:xml/xml.dart';

/// Shared MARC/SRU XML helpers used by the ISBN, EAN, and title lookups.
///
/// Both MARC21-xml (DNB) and marcxml (K10plus, LoC) namespace conventions
/// are supported by matching on local element names only.

/// Returns the local (namespace-free) element name of [el].
String localName(XmlElement el) {
  final name = el.name.local;
  return name.isEmpty ? el.name.qualified.split(':').last : name;
}

/// Returns the trimmed text of the subfield of [datafield] with [code].
String subfield(XmlElement datafield, String code) {
  for(final sf in datafield.childElements) {
    if(localName(sf) == 'subfield' && sf.getAttribute('code') == code) {
      return sf.innerText.trim();
    }
  }
  return '';
}

/// Removes MARC sorting marks (U+0098/U+009C) and trailing ISBD
/// punctuation (space, period, comma, slash, colon) from [s].
String stripMarc(String s) {
  var result = s.replaceAll('\u0098', '').replaceAll('\u009C', '');
  while(result.isNotEmpty) {
    final last = result[result.length - 1];
    if(last == ' ' || last == '.' || last == ',' ||
       last == '/' || last == ':') {
      result = result.substring(0, result.length - 1);
    } else {
      break;
    }
  }
  return result;
}

/// Returns the first child of [parent] whose local tag name is [localTag].
XmlElement? findChild(XmlElement parent, String localTag) {
  for(final c in parent.childElements) {
    if(localName(c) == localTag) return c;
  }
  return null;
}

/// Extracts all MARC record elements from a raw SRU response.
///
/// Walks `records/record/recordData/record` and returns every MARC record
/// element in response order. Returns an empty list on parse errors or
/// when the response contains no records (e.g. SRU diagnostics).
List<XmlElement> sruMarcRecords(String xml) {
  final records = <XmlElement>[];
  if(xml.isEmpty) return records;
  try {
    final doc = XmlDocument.parse(xml);
    final root = doc.rootElement;
    final recordsEl = findChild(root, 'records');
    if(recordsEl == null) return records;
    for(final sruRecord in recordsEl.childElements) {
      if(localName(sruRecord) != 'record') continue;
      final recordData = findChild(sruRecord, 'recordData');
      if(recordData == null) continue;
      final marc = findChild(recordData, 'record');
      if(marc != null) records.add(marc);
    }
  } catch (_) {}
  return records;
}
