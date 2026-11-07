// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for generate_signature().
/**
   Covers Cutter-Sanborn number lookup, title initial extraction, article
   stripping, diacritic normalisation, DDC/year formatting, and collision
   avoidance.
   @file
   @author Christoph Pesch
*/

#include "colman_cutter.hpp"
#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif


namespace colman
{

  class ColmanCutterTest : public QObject
  {
    Q_OBJECT

  private Q_SLOTS:

  // -----------------------------------------------------------------------
  // Known Cutter-Sanborn reference values

    void test_hemingway_gives_h475()
    {
    // "Hemingway" is a well-known three-figure Cutter-Sanborn reference value.
      std::set<std::string> const none;
      std::string const sig = generate_signature("Hemingway", "", "", "", none);
      QVERIFY2(sig.rfind("H475", 0) != std::string::npos || sig.find("H475") != std::string::npos,
               sig.c_str());
    }

    void test_shakespeare_gives_s527()
    {
      std::set<std::string> const none;
      std::string const sig = generate_signature("Shakespeare", "Hamlet", "", "", none);
      QVERIFY2(sig.find("S527") != std::string::npos, sig.c_str());
    }

  // -----------------------------------------------------------------------
  // DDC and year formatting

    void test_ddc_prepended()
    {
      std::set<std::string> const none;
      std::string const sig =
          generate_signature("Hemingway", "Der alte Mann", "", "823.914", none);
      QVERIFY2(sig.substr(0, 7) == "823.914", sig.c_str());
    }

    void test_year_appended()
    {
      std::set<std::string> const none;
      std::string const sig =
          generate_signature("Hemingway", "Der alte Mann", "1952", "", none);
      QVERIFY2(sig.size() >= 4 && sig.substr(sig.size() - 4) == "1952",
               sig.c_str());
    }

    void test_full_format()
    {
    // Expected: "823.914 H475a 1952"
      std::set<std::string> const none;
      std::string const sig = generate_signature(
          "Hemingway", "Der alte Mann und das Meer", "1952", "823.914", none);
      QCOMPARE(sig, std::string("823.914 H475a 1952"));
    }

    void test_no_ddc_no_year()
    {
      std::set<std::string> const none;
      std::string const sig = generate_signature("Hemingway", "Fiesta", "", "", none);
    // Must not start with a space and must not contain trailing space.
      QVERIFY(!sig.empty());
      QVERIFY(sig.front() != ' ');
      QVERIFY(sig.back() != ' ');
    }

  // -----------------------------------------------------------------------
  // Title initial

    void test_title_initial_appended()
    {
      std::set<std::string> const none;
    // Title "Fiesta" → initial 'f'
      std::string const sig = generate_signature("Hemingway", "Fiesta", "", "", none);
      QVERIFY2(sig.find("H475f") != std::string::npos, sig.c_str());
    }

    void test_english_article_stripped_for_title_initial()
    {
      std::set<std::string> const none;
    // "The Old Man and the Sea" → strip "the " → initial 'o'
      std::string const sig =
          generate_signature("Hemingway", "The Old Man and the Sea", "", "", none);
      QVERIFY2(sig.find("H475o") != std::string::npos, sig.c_str());
    }

    void test_german_article_der_stripped_for_title_initial()
    {
      std::set<std::string> const none;
    // "Der alte Mann und das Meer" → strip "der " → initial 'a'
      std::string const sig =
          generate_signature("Hemingway", "Der alte Mann und das Meer", "", "", none);
      QVERIFY2(sig.find("H475a") != std::string::npos, sig.c_str());
    }

    void test_german_article_die_stripped_for_title_initial()
    {
      std::set<std::string> const none;
    // "Die Verwandlung" → strip "die " → initial 'v'
      std::string const sig =
          generate_signature("Kafka", "Die Verwandlung", "", "", none);
      QVERIFY2(sig.find('v') != std::string::npos, sig.c_str());
    }

    void test_german_article_das_stripped_for_title_initial()
    {
      std::set<std::string> const none;
    // "Das Schloss" → strip "das " → initial 's'
      std::string const sig =
          generate_signature("Kafka", "Das Schloss", "", "", none);
      QVERIFY2(sig.find('s') != std::string::npos, sig.c_str());
    }

  // -----------------------------------------------------------------------
  // Fallback to title when author is empty

    void test_no_author_uses_title()
    {
      std::set<std::string> const none;
      std::string const with_author =
          generate_signature("Bibel", "Die Bibel", "", "", none);
      std::string const without_author =
          generate_signature("", "Die Bibel", "", "", none);
    // With author "Bibel" and without author but title "Die Bibel" (after
    // stripping "die ") should produce the same Cutter letter+number.
      QCOMPARE(with_author, without_author);
    }

    void test_no_author_no_title_does_not_crash()
    {
      std::set<std::string> const none;
    // Should return something without crashing.
      std::string const sig = generate_signature("", "", "", "", none);
      QVERIFY(true); // just must not crash
      (void)sig;
    }

  // -----------------------------------------------------------------------
  // Diacritic normalisation

    void test_umlaut_ae_normalised()
    {
      std::set<std::string> const none;
    // "Äsop" normalises to "aesop"; lookup on "aesop" vs "Aesop"
      std::string const sig1 = generate_signature("\xC3\x84sop", "Fabeln", "", "", none);
      std::string const sig2 = generate_signature("Aesop", "Fabeln", "", "", none);
      QCOMPARE(sig1, sig2);
    }

    void test_umlaut_oe_normalised()
    {
      std::set<std::string> const none;
      std::string const sig1 = generate_signature("G\xC3\xB6the", "Faust", "", "", none);
      std::string const sig2 = generate_signature("Goethe", "Faust", "", "", none);
      QCOMPARE(sig1, sig2);
    }

    void test_umlaut_ue_normalised()
    {
      std::set<std::string> const none;
      std::string const sig1 =
          generate_signature("M\xC3\xBCller", "Gedichte", "", "", none);
      std::string const sig2 =
          generate_signature("Mueller", "Gedichte", "", "", none);
      QCOMPARE(sig1, sig2);
    }

    void test_sharp_s_normalised()
    {
      std::set<std::string> const none;
    // "Straße" → "strasse"
      std::string const sig1 =
          generate_signature("Stra\xC3\x9F"
                             "e",
                             "Titel", "", "", none);
      std::string const sig2 = generate_signature("Strasse", "Titel", "", "", none);
      QCOMPARE(sig1, sig2);
    }

  // -----------------------------------------------------------------------
  // Collision avoidance

    void test_collision_appends_dot_2()
    {
      std::set<std::string> existing;
      std::string const first =
          generate_signature("Hemingway", "Fiesta", "1926", "", existing);
      existing.insert(first);
      std::string const second =
          generate_signature("Hemingway", "Fiesta", "1926", "", existing);
      QVERIFY(first != second);
      QVERIFY2(second.find(".2") != std::string::npos, second.c_str());
    }

    void test_collision_appends_dot_3_when_dot_2_taken()
    {
      std::set<std::string> existing;
      std::string const first =
          generate_signature("Hemingway", "Fiesta", "1926", "", existing);
      existing.insert(first);
      std::string const second =
          generate_signature("Hemingway", "Fiesta", "1926", "", existing);
      existing.insert(second);
      std::string const third =
          generate_signature("Hemingway", "Fiesta", "1926", "", existing);
      QVERIFY(third != first);
      QVERIFY(third != second);
      QVERIFY2(third.find(".3") != std::string::npos, third.c_str());
    }

    void test_no_collision_when_not_in_existing()
    {
      std::set<std::string> existing;
      existing.insert("unrelated entry");
      std::string const sig =
          generate_signature("Hemingway", "Fiesta", "1926", "", existing);
      QVERIFY(sig.find(".2") == std::string::npos);
    }

    void test_result_not_in_existing()
    {
      std::set<std::string> existing;
      for(int i = 0; i < 5; ++i) {
        std::string const sig =
            generate_signature("Hemingway", "Fiesta", "", "", existing);
        QVERIFY(existing.find(sig) == existing.end());
        existing.insert(sig);
      }
    }

  // -----------------------------------------------------------------------
  // Cutter letter is uppercase first letter of lookup word

    void test_cutter_letter_is_uppercase()
    {
      std::set<std::string> const none;
      std::string const sig = generate_signature("kafka", "Der Prozess", "", "", none);
    // After DDC (none here) the first character must be 'K'.
      QVERIFY(!sig.empty());
      QCOMPARE(sig.front(), 'K');
    }

    void test_different_authors_different_letters()
    {
      std::set<std::string> const none;
      std::string const ha = generate_signature("Hemingway", "X", "", "", none);
      std::string const ka = generate_signature("Kafka", "X", "", "", none);
      QVERIFY(ha.front() == 'H');
      QVERIFY(ka.front() == 'K');
    }
  };

} // namespace colman

QTEST_MAIN(colman::ColmanCutterTest)
#include "colman_cutter_test.moc"

/* EOF */
