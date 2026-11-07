// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../api/acquisitions_api.dart';
import '../api/api_client.dart';
import '../api/classes_api.dart';
import '../api/collection_api.dart';
import '../api/history_api.dart';
import '../api/items_api.dart';
import '../api/lendings_api.dart';
import '../api/liquidations_api.dart';
import '../api/persons_api.dart';
import '../api/reservations_api.dart';
import '../api/returns_api.dart';
import '../api/signatures_api.dart';
import '../api/table_versions_api.dart';
import '../api/types_api.dart';
import 'auth_provider.dart';

final apiClientProvider = Provider<ApiClient>((ref) {
  final auth = ref.watch(authProvider).value;
  if (auth is! Authenticated) {
    throw StateError('Not authenticated');
  }
  return ApiClient(auth.config);
});

final itemsApiProvider = Provider<ItemsApi>((ref) {
  return ItemsApi(ref.watch(apiClientProvider));
});

final personsApiProvider = Provider<PersonsApi>((ref) {
  return PersonsApi(ref.watch(apiClientProvider));
});

final lendingsApiProvider = Provider<LendingsApi>((ref) {
  return LendingsApi(ref.watch(apiClientProvider));
});

final returnsApiProvider = Provider<ReturnsApi>((ref) {
  return ReturnsApi(ref.watch(apiClientProvider));
});

final acquisitionsApiProvider = Provider<AcquisitionsApi>((ref) {
  return AcquisitionsApi(ref.watch(apiClientProvider));
});

final liquidationsApiProvider = Provider<LiquidationsApi>((ref) {
  return LiquidationsApi(ref.watch(apiClientProvider));
});

final typesApiProvider = Provider<TypesApi>((ref) {
  return TypesApi(ref.watch(apiClientProvider));
});

final tableVersionsApiProvider = Provider<TableVersionsApi>((ref) {
  return TableVersionsApi(ref.watch(apiClientProvider));
});

final reservationsApiProvider = Provider<ReservationsApi>((ref) {
  return ReservationsApi(ref.watch(apiClientProvider));
});

final historyApiProvider = Provider<HistoryApi>((ref) {
  return HistoryApi(ref.watch(apiClientProvider));
});

final classesApiProvider = Provider<ClassesApi>((ref) {
  return ClassesApi(ref.watch(apiClientProvider));
});

final collectionApiProvider = Provider<CollectionApi>((ref) {
  return CollectionApi(ref.watch(apiClientProvider));
});

final signaturesApiProvider = Provider<SignaturesApi>((ref) {
  return SignaturesApi(ref.watch(apiClientProvider));
});
