// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'app.dart';
import 'services/certificate_trust_store.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await certificateTrustStore.initialize();
  runApp(const ProviderScope(child: ColmanApp()));
}
