// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:convert' show base64Encode;
import 'dart:typed_data';

import 'package:dio/dio.dart';

import '../models/item.dart';
import 'api_client.dart';

/// REST client for the `/api/items` endpoints.
class ItemsApi {
  /// Creates an [ItemsApi] backed by [_client].
  const ItemsApi(this._client);

  final ApiClient _client;

  /// Searches items using the given [query] parameters.
  ///
  /// Sends `POST /api/items/search` with all non-empty filter fields.
  /// Returns compact [SearchItemRow] objects suitable for list views.
  Future<List<SearchItemRow>> search(SearchQuery q) async {
    final data = <String, dynamic>{'type_id': q.typeId};
    if(q.general.isNotEmpty) data['general'] = q.general;
    if(q.name.isNotEmpty) data['name'] = q.name;
    if(q.subject.isNotEmpty) data['subject'] = q.subject;
    if(q.description.isNotEmpty) data['description'] = q.description;
    if(q.keywords.isNotEmpty) data['keywords'] = q.keywords;
    final response = await _client.dio.post('/api/items/search', data: data);
    final list = response.data as List;
    return list
        .map((e) => SearchItemRow.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  /// Returns the full [Item] with [id], including its ETag.
  Future<Item> getById(int id) async {
    final response = await _client.dio.get('/api/items/$id');
    final etag = response.headers.value('ETag');
    return Item.fromJson(
        response.data as Map<String, dynamic>, id: id, etag: etag);
  }

  /// Creates a new item and returns the server-assigned id.
  ///
  /// The server responds with `{"id": N}` (HTTP 201).
  Future<int> create(Map<String, dynamic> data) async {
    final response = await _client.dio.post('/api/items', data: data);
    return (response.data['id'] as num).toInt();
  }

  /// Updates item [id] with [data], using [etag] for optimistic concurrency.
  ///
  /// The server responds with an empty body on success, so the updated item
  /// is re-fetched to obtain the new ETag.
  Future<Item> update(int id, Map<String, dynamic> data, String etag) async {
    await _client.dio.put(
      '/api/items/$id',
      data: data,
      options: Options(headers: {'If-Match': etag}),
    );
    return getById(id);
  }

  /// Deletes item [id].
  Future<void> delete(int id) async {
    await _client.dio.delete('/api/items/$id');
  }

  /// Returns the first item matching [isbn], or `null` if none found.
  ///
  /// The server responds with a JSON array of integer item IDs.
  Future<Item?> getByIsbn(String isbn) async {
    try {
      final response = await _client.dio.get('/api/items/by-isbn/$isbn');
      final ids = (response.data as List)
          .map((e) => (e as num).toInt())
          .toList();
      if(ids.isEmpty) return null;
      try {
        return await getById(ids.first);
      } catch(_) {
        return null;
      }
    } on DioException catch(e) {
      if(e.response?.statusCode == 404) return null;
      rethrow;
    }
  }

  /// Returns the first item matching [ean], or `null` if none found.
  ///
  /// The server responds with a JSON array of integer item IDs.
  Future<Item?> getByEan(String ean) async {
    try {
      final response = await _client.dio.get('/api/items/by-ean/$ean');
      final ids = (response.data as List)
          .map((e) => (e as num).toInt())
          .toList();
      if(ids.isEmpty) return null;
      try {
        return await getById(ids.first);
      } catch(_) {
        return null;
      }
    } on DioException catch(e) {
      if(e.response?.statusCode == 404) return null;
      rethrow;
    }
  }

  /// Parses [barcode] as a colman-assigned item barcode.
  ///
  /// POSTs to `/api/items/barcode/parse`. Returns the decoded
  /// `type_id` and `item_id`, or `null` for non-colman barcodes.
  Future<({int typeId, int itemId})?> parseItemBarcode(String barcode) async {
    try {
      final response = await _client.dio.post(
        '/api/items/barcode/parse',
        data: {'barcode': barcode},
      );
      final data = response.data as Map<String, dynamic>;
      return (
        typeId: (data['type_id'] as num).toInt(),
        itemId: (data['item_id'] as num).toInt(),
      );
    } on DioException catch(e) {
      if(e.response?.statusCode == 400 || e.response?.statusCode == 404) {
        return null;
      }
      rethrow;
    }
  }

  /// Returns the next free barcode string for items of type [typeId].
  Future<String?> nextBarcode(int typeId) async {
    final response = await _client.dio
        .get('/api/items/barcode', queryParameters: {'type_id': typeId});
    return response.data['barcode'] as String?;
  }

  /// Returns the total number of copies acquired for item [id].
  ///
  /// Uses `/api/items/acquired?id=…` which returns a JSON array of item rows;
  /// the count is the length of that array.
  Future<int> getAcquired(int id) async {
    final response = await _client.dio
        .get('/api/items/acquired', queryParameters: {'id': id});
    return (response.data as List).length;
  }

  /// Returns the number of copies of item [id] currently available for lending.
  ///
  /// Uses `/api/items/available?id=…` which returns a JSON array of item rows;
  /// the count is the length of that array.
  Future<int> getAvailable(int id) async {
    final response = await _client.dio
        .get('/api/items/available', queryParameters: {'id': id});
    return (response.data as List).length;
  }

  /// Returns the total number of copies (stock size) of item [id].
  ///
  /// Uses `GET /api/items/$id/stock`, which returns `{"value": <int>}`.
  Future<int> getStock(int id) async {
    final response = await _client.dio.get('/api/items/$id/stock');
    return (response.data['value'] as num).toInt();
  }

  /// Uploads [pngBytes] to `/api/thumbnails` and returns the server-assigned
  /// thumbnail id, or null on any error.
  Future<int?> storeThumbnail(
      Uint8List pngBytes, String key, String url) async {
    try {
      final response = await _client.dio.post(
        '/api/thumbnails',
        data: {
          'png': base64Encode(pngBytes),
          'key': key,
          'url': url,
          'overwrite': false,
        },
      );
      final id = (response.data['id'] as num).toInt();
      return id >= 0 ? id : null;
    } catch(_) {
      return null;
    }
  }

  /// Returns the raw PNG bytes for thumbnail [id], or null on any error.
  Future<Uint8List?> getThumbnailBytes(int id) async {
    try {
      final response = await _client.dio.get<List<int>>(
        '/api/thumbnails/$id',
        options: Options(responseType: ResponseType.bytes),
      );
      final data = response.data;
      if(data == null) return null;
      return Uint8List.fromList(data);
    } catch(_) {
      return null;
    }
  }
}
