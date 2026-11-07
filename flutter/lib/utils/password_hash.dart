// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert';
import 'dart:math';
import 'dart:typed_data';

import 'package:hashlib/hashlib.dart';

/// Hashes [password] using Argon2id with parameters matching
/// `colman_passwords.cpp` (t=2, m=19456 KB, p=1, len=32).
///
/// Returns a PHC-encoded string (`$argon2id$v=19$m=19456,t=2,p=1$…`)
/// that is directly accepted by `argon2id_verify` on the server.
String hashPassword(String password) {
  final salt = Uint8List.fromList(
    List.generate(16, (_) => Random.secure().nextInt(256)),
  );
  return Argon2(
    type: Argon2Type.argon2id,
    parallelism: 1,
    memorySizeKB: 19456,
    iterations: 2,
    hashLength: 32,
    salt: salt,
  ).encode(utf8.encode(password));
}
