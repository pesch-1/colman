# Public SRU Servers for colman

SRU (Search/Retrieve via URL) is the HTTP-based successor to Z39.50.
It returns bibliographic records as XML over plain HTTPS — no special
client or port required.

colman already queries the **Deutsche Nationalbibliothek (DNB)** via SRU
for both ISBN (books) and EAN (AV media).  This document lists further
publicly accessible SRU endpoints that could be added as optional lookup
sources.

---

## German-language union catalogues

| Network | Endpoint | ISBN field | Coverage |
|---|---|---|---|
| **K10plus** (GBV + SWB) | `https://sru.k10plus.de/opac-de-627` | `pica.isb` | 80 M+ titles, 1000+ libraries |
| **KOBV** (Berlin–Brandenburg) | `https://sru.kobv.de/k2` | `dc.identifier` | State Library Berlin etc. |
| **HeBIS** (Hesse / Rhineland-Palatinate) | `https://www.hebis.de/dienste/sru/` | see explain | ~17 M titles |
| **BVB / B3Kat** (Bavaria + KOBV) | `http://bvbr.bib-bvb.de:5661/bvb01sru` | `marcxml.isbn` | Bavaria |

**K10plus** is the most relevant addition: it is the largest German-language
union catalogue, completely open, and covers materials that the DNB alone
may not index.

The other three (KOBV, HeBIS, BVB) overlap heavily with K10plus and are
only worth adding if regional completeness matters.

---

## Austria and Switzerland

Both networks run on Ex Libris Alma, share the same CQL query syntax, and
return MARCXML.

| Network | Endpoint | ISBN field |
|---|---|---|
| **ÖNB / OBVSG** (Austrian Library Network) | `https://obv-at-oenb.alma.exlibrisgroup.com/view/sru/43ACC_ONB` | `alma.isbn` |
| **Swisscovery / SLSP** (Swiss libraries) | `https://swisscovery.slsp.ch/view/sru/41SLSP_NETWORK` | `isbn` / `alma.isbn` |

The old OBVSG network endpoint (`https://services.obvsg.at/sru/...`) was
shut down after the migration to Alma and now answers every query with
"Database does not exist" — use the ONB Alma endpoint above instead (it
covers the national library only).

---

## International

| Network | Endpoint | ISBN field | Language / scope |
|---|---|---|---|
| **Library of Congress** (USA) | `https://lx2.loc.gov/sru/lcdb` | `bath.isbn` | English / worldwide |
| **BnF** (Bibliothèque nationale de France) | `http://catalogue.bnf.fr/api/SRU` | `bib.isbn` | French / worldwide |

Notes:
- **LoC** returns MARC21-XML (same format as DNB) — straightforward to parse.
  Remove hyphens from ISBN before querying.
- **BnF** returns UNIMARC rather than MARC21; field numbers differ from DNB.
  Supports up to 100 records per request; query operators are `all`, `any`,
  `adj`.

---

## Title search

In addition to the ISBN and EAN fields, every configured server has a
**title field**: the CQL index used for free-text title searches (books,
CDs, DVDs, Blu-rays).  A server with an empty title field is skipped for
title searches.

| Server | Title field |
|---|---|
| **DNB** | `tit` |
| **K10plus** | `pica.tit` |
| **Library of Congress** | `dc.title` |
| **ÖNB** | `alma.title` |
| **Swisscovery** | `alma.title` |
| **BnF** | `bib.title` (books only — UNIMARC has no AV parser) |

Use the `explain` operation to verify the exact index name of a server.
Title queries request multiple records (`maximumRecords=10`) and send the
title as a percent-encoded, quoted CQL phrase.  UNIMARC servers (BnF) are
queried with SRU version 1.2 because they reject version 1.1; all other
servers are queried with version 1.1 (DNB rejects 1.2).  Results are
filtered client-side by the detected media kind (MARC21 leader,
controlfield 007, and carrier/extent text); records whose kind cannot be
detected are kept.

Server lists saved by colman versions without the title field are
migrated automatically: for servers whose endpoint matches one of the
built-in defaults, the default title field is filled in; user-defined
servers keep an empty title field until it is set in the preferences.

---

## Not recommended

| Network | Reason |
|---|---|
| **WorldCat / OCLC** | Legacy SRU shut down end of 2024; new API requires paid OCLC membership |
| **British Library** | No public SRU endpoint found |

---

## Implementation notes

- All servers listed above are **free and open** (no API key required).
- All use **CQL (Contextual Query Language)** — the same syntax already used
  for the DNB queries.
- All return **UTF-8 XML**; most support both MARCXML and Dublin Core schemas.
- Pagination is controlled via `startRecord` and `maximumRecords` parameters.
- Use the SRU **explain** operation (`?operation=explain`) on any endpoint to
  discover available search indices and supported schemas at runtime.

---

## Priority ranking for colman

| Priority | Server | Rationale |
|---|---|---|
| 1 | **K10plus** | Largest German-language catalogue; same region as existing DNB integration |
| 2 | **Library of Congress** | English-language books; MARC21, well-documented, free |
| 3 | **ÖNB / OBVSG** | Austrian/Central-European materials |
| 4 | **Swisscovery** | Swiss materials; modern infrastructure |
| 5 | **BnF** | French materials; UNIMARC requires separate parser |
