// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Tests for hash_password() and verify_password().
/**
   Covers hash format, salt uniqueness, correct/wrong password
   verification, empty inputs, and non-ASCII (UTF-8) passwords.
   @file
   @author Christoph Pesch
*/

#include "colman_passwords.hpp"
#ifndef SYSTEM_QTTEST
#  include <QtTest>
#  define SYSTEM_QTTEST
#endif


namespace colman
{

  class ColmanPasswordsTest : public QObject
  {
    Q_OBJECT

  private Q_SLOTS:

  // -----------------------------------------------------------------------
  // hash_password

    void hash_returns_non_empty()
    {
      QString const h = hash_password("secret");
      QVERIFY(!h.isEmpty());
    }

    void hash_is_argon2id_encoded()
    {
    // The encoded format produced by libargon2 always starts with "$argon2id$".
      QString const h = hash_password("secret");
      QVERIFY2(h.startsWith("$argon2id$"), qPrintable(h));
    }

    void hash_same_password_gives_different_hashes()
    {
    // Each call generates a fresh random salt, so two hashes must differ.
      QString const h1 = hash_password("secret");
      QString const h2 = hash_password("secret");
      QVERIFY(h1 != h2);
    }

    void hash_different_passwords_give_different_hashes()
    {
      QString const h1 = hash_password("password1");
      QString const h2 = hash_password("password2");
      QVERIFY(h1 != h2);
    }

    void hash_empty_password_returns_non_empty()
    {
    // An empty password is technically valid; libargon2 accepts it.
      QString const h = hash_password("");
      QVERIFY(!h.isEmpty());
      QVERIFY(h.startsWith("$argon2id$"));
    }

    void hash_unicode_password_returns_non_empty()
    {
    // German umlauts and other non-ASCII characters are encoded as UTF-8.
      QString const h = hash_password("P\u00e4ssw\u00f6rter");
      QVERIFY(!h.isEmpty());
      QVERIFY(h.startsWith("$argon2id$"));
    }

  // -----------------------------------------------------------------------
  // verify_password

    void verify_correct_password_returns_true()
    {
      QString const h = hash_password("correct horse battery staple");
      QVERIFY(verify_password("correct horse battery staple", h));
    }

    void verify_wrong_password_returns_false()
    {
      QString const h = hash_password("correct");
      QVERIFY(!verify_password("wrong", h));
    }

    void verify_empty_password_against_non_empty_hash_returns_false()
    {
      QString const h = hash_password("nonempty");
      QVERIFY(!verify_password("", h));
    }

    void verify_non_empty_password_against_empty_hash_returns_false()
    {
      QVERIFY(!verify_password("secret", ""));
    }

    void verify_empty_password_against_its_own_hash_returns_true()
    {
      QString const h = hash_password("");
      QVERIFY(verify_password("", h));
    }

    void verify_unicode_password_returns_true()
    {
      QString const pwd = "P\u00e4ssw\u00f6rter";
      QString const h = hash_password(pwd);
      QVERIFY(verify_password(pwd, h));
    }

    void verify_unicode_wrong_password_returns_false()
    {
      QString const h = hash_password("P\u00e4ssw\u00f6rter");
      QVERIFY(!verify_password("Passworter", h));
    }

    void verify_hash_from_different_call_still_matches()
    {
    // Verify that verification works across independent hash/verify calls,
    // not just immediately after hashing.
      QString const h = hash_password("persistent");
      QString const h_copy = h;
      QVERIFY(verify_password("persistent", h_copy));
    }
  };

} // namespace colman

QTEST_MAIN(colman::ColmanPasswordsTest)
#include "colman_passwords_test.moc"

/* EOF */
