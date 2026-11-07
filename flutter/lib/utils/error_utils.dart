// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:dio/dio.dart';
import 'package:colman_client/l10n/app_localizations.dart';

import '../api/api_version.dart';

/// Extracts a technical description from [e].
///
/// For [DioException], includes the HTTP status and the server's JSON
/// `error` or `message` field when present.
String extractHttpDetail(Object e) {
  if(e is DioException) {
    final body = e.response?.data;
    String? bodyMsg;
    if(body is Map) {
      bodyMsg = body['error'] as String? ?? body['message'] as String?;
      if(bodyMsg != null && bodyMsg.isEmpty) bodyMsg = null;
    }
    else if(body is String && body.isNotEmpty) {
      bodyMsg = body;
    }
    final status = e.response?.statusCode;
    if(status != null) {
      return bodyMsg != null ? 'HTTP $status – $bodyMsg' : 'HTTP $status';
    }
    return e.message ?? e.toString();
  }
  return e.toString();
}

/// Returns a user-facing error message for [e].
String friendlyHttpError(Object e, AppLocalizations l10n) {
  if(e is IncompatibleServerException) {
    return l10n.errorIncompatibleServer(e.server.label, e.requiredLabel);
  }
  if(e is DioException) {
    final status = e.response?.statusCode;
    if(status == 401) return l10n.errorInvalidCredentials;
    if(status == 403) return l10n.errorAccessDenied;
    if(status != null && status >= 500) return l10n.errorServerError;
    if(e.type == DioExceptionType.connectionTimeout ||
       e.type == DioExceptionType.receiveTimeout ||
       e.type == DioExceptionType.connectionError) {
      return l10n.errorConnectionFailed;
    }
  }
  return l10n.errorRequestFailed;
}
