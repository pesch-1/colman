// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:typed_data';
import 'dart:ui' as ui;

import 'package:dio/dio.dart';
import 'package:flutter/material.dart';
import 'package:webview_flutter/webview_flutter.dart';

import '../l10n/app_localizations.dart';

/// In-app browser that opens an Amazon.de product search.
///
/// Uses [WebViewWidget] so the Amazon app cannot intercept the navigation.
/// When [canPickImage] is true, a JavaScript channel is registered so that
/// tapping any `<img>` element downloads it and pops the screen with the PNG
/// bytes as the result.
class AmazonBrowserScreen extends StatefulWidget {
  const AmazonBrowserScreen({
    super.key,
    required this.searchTerm,
    this.canPickImage = false,
  });

  /// The term to search for on Amazon.de; empty opens the start page.
  final String searchTerm;

  /// When true the user can tap a cover image to download and return it.
  final bool canPickImage;

  @override
  State<AmazonBrowserScreen> createState() => _AmazonBrowserScreenState();
}

class _AmazonBrowserScreenState extends State<AmazonBrowserScreen> {
  late final WebViewController _controller;
  int _progress = 0;
  bool _downloading = false;

  // Intercepts clicks on <img> elements and posts the resolved src URL.
  static const _injectJs = '''
(function() {
  document.addEventListener('click', function(e) {
    var el = e.target;
    var depth = 0;
    while (el && el.tagName !== 'IMG' && depth < 5) {
      el = el.parentElement;
      depth++;
    }
    if (el && el.tagName === 'IMG' && el.src &&
        (el.src.startsWith('http://') || el.src.startsWith('https://'))) {
      e.preventDefault();
      e.stopPropagation();
      ImagePicker.postMessage(el.src);
    }
  }, true);
})();
''';

  @override
  void initState() {
    super.initState();
    final url = widget.searchTerm.isEmpty
        ? 'https://www.amazon.de/'
        : 'https://www.amazon.de/s?k='
            '${Uri.encodeQueryComponent(widget.searchTerm)}';
    final controller = WebViewController()
      ..setJavaScriptMode(JavaScriptMode.unrestricted)
      ..setNavigationDelegate(NavigationDelegate(
        onProgress: (p) => setState(() { _progress = p; }),
        onPageFinished: (_) {
          setState(() { _progress = 100; });
          if(widget.canPickImage) {
            _controller.runJavaScript(_injectJs);
          }
        },
      ));
    if(widget.canPickImage) {
      controller.addJavaScriptChannel(
        'ImagePicker',
        onMessageReceived: (msg) => _onImageUrl(msg.message),
      );
    }
    controller.loadRequest(Uri.parse(url));
    _controller = controller;
  }

  Future<void> _onImageUrl(String url) async {
    if(_downloading || !mounted) return;
    setState(() { _downloading = true; });
    final bytes = await _downloadAsPng(url);
    if(!mounted) return;
    if(bytes != null) {
      Navigator.pop(context, bytes);
    } else {
      setState(() { _downloading = false; });
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(
        content: Text(AppLocalizations.of(context)!.amazonBrowserDownloadError),
      ));
    }
  }

  static Future<Uint8List?> _downloadAsPng(String url) async {
    try {
      final dio = Dio(BaseOptions(
        connectTimeout: const Duration(seconds: 10),
        receiveTimeout: const Duration(seconds: 10),
      ));
      final response = await dio.get<List<int>>(
        url.replaceFirst('http://', 'https://'),
        options: Options(responseType: ResponseType.bytes),
      );
      final bytes = response.data;
      if(bytes == null || bytes.isEmpty) return null;
      final raw = Uint8List.fromList(bytes);
      final codec = await ui.instantiateImageCodec(raw);
      final frame = await codec.getNextFrame();
      final byteData =
          await frame.image.toByteData(format: ui.ImageByteFormat.png);
      return byteData?.buffer.asUint8List();
    } catch(_) {
      return null;
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    return Scaffold(
      appBar: AppBar(
        title: Text(l10n.thumbnailSearchAmazon),
        bottom: _progress < 100
            ? PreferredSize(
                preferredSize: const Size.fromHeight(4),
                child: LinearProgressIndicator(
                  value: _progress / 100,
                  backgroundColor: Colors.transparent,
                ),
              )
            : null,
        actions: [
          IconButton(
            icon: const Icon(Icons.refresh),
            onPressed: () => _controller.reload(),
          ),
        ],
      ),
      body: Stack(
        children: [
          WebViewWidget(controller: _controller),
          if(widget.canPickImage)
            Positioned(
              bottom: 0,
              left: 0,
              right: 0,
              child: Material(
                color: Theme.of(context)
                    .colorScheme
                    .secondaryContainer
                    .withValues(alpha: 0.92),
                child: Padding(
                  padding:
                      const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
                  child: Text(
                    l10n.amazonBrowserHint,
                    textAlign: TextAlign.center,
                    style: TextStyle(
                      color: Theme.of(context)
                          .colorScheme
                          .onSecondaryContainer,
                    ),
                  ),
                ),
              ),
            ),
          if(_downloading)
            const Center(child: CircularProgressIndicator()),
        ],
      ),
    );
  }
}
