// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';
import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';
import 'package:hashlib/hashlib.dart';

import 'package:colman_client/utils/password_hash.dart';

void main() {
  // Reference Argon2id hashes produced by the system libargon2 — the exact C
  // library the colman server links against (see colman_passwords.cpp). They
  // pin bit-for-bit interoperability: a hash produced by hashPassword() must be
  // accepted by argon2id_verify() on the server, and the client must in turn be
  // able to verify hashes the server created. Both vectors use the password
  // "secret" and a deterministic 16-byte salt 0x00..0x0F.
  const serverParamsVector = // t=3, m=65536, p=1 (server hashing parameters)
      r'$argon2id$v=19$m=65536,t=3,p=1$AAECAwQFBgcICQoLDA0ODw$'
      r'qTm/VWZ7pl/F3tgt8AK94HkAe1dcdoJUwoLx/ESzrT4';
  const clientParamsVector = // t=2, m=19456, p=1 (parameters hashPassword uses)
      r'$argon2id$v=19$m=19456,t=2,p=1$AAECAwQFBgcICQoLDA0ODw$'
      r'wf2/c/vlFte6hIN2Uyogmn1x/V/pPV40SfMbuFX/DGA';

  final fixedSalt = Uint8List.fromList(List<int>.generate(16, (i) => i));

  group('hashPassword (hashlib Argon2id)', () {
    test('produces a PHC string with the server-compatible parameters', () {
      final encoded = hashPassword('secret');
      expect(encoded, startsWith(r'$argon2id$v=19$m=19456,t=2,p=1$'));
    });

    test('output round-trips through argon2Verify', () {
      const password = 'correct horse battery staple';
      final encoded = hashPassword(password);
      expect(argon2Verify(encoded, utf8.encode(password)), isTrue);
      expect(argon2Verify(encoded, utf8.encode('wrong password')), isFalse);
    });

    test('matches libargon2 byte-for-byte for the client parameters', () {
      // Same parameters as hashPassword() but with the deterministic salt, so
      // the encoded result can be compared against the libargon2 reference.
      final encoded = Argon2(
        type: Argon2Type.argon2id,
        parallelism: 1,
        memorySizeKB: 19456,
        iterations: 2,
        hashLength: 32,
        salt: fixedSalt,
      ).encode(utf8.encode('secret'));
      expect(encoded, clientParamsVector);
    });
  });

  group('interoperability with the server libargon2', () {
    test('verifies a hash created by the server with its own parameters', () {
      expect(argon2Verify(serverParamsVector, utf8.encode('secret')), isTrue);
      expect(argon2Verify(serverParamsVector, utf8.encode('nope')), isFalse);
    });

    test('verifies a hash created with the client parameters', () {
      expect(argon2Verify(clientParamsVector, utf8.encode('secret')), isTrue);
    });
  });
}
