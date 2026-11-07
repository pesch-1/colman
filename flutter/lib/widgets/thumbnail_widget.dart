// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:typed_data';
import 'dart:ui' as ui;

import 'package:image_picker/image_picker.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import '../l10n/app_localizations.dart';
import '../providers/api_client_provider.dart';
import '../screens/amazon_browser_screen.dart';

/// Displays a thumbnail image and optional action buttons.
///
/// Shows the image identified by [thumbnailId], or a placeholder icon when
/// no thumbnail is available.  When [onPickFile] is provided a "From file"
/// button lets the user pick a local image; when [amazonSearchTerm] is
/// provided a "Search on Amazon" button opens an Amazon.de search; when
/// [onRemove] is provided and a thumbnail is shown a "Remove" button lets
/// the user discard it.
class ThumbnailWidget extends ConsumerStatefulWidget {
  const ThumbnailWidget({
    super.key,
    this.thumbnailId,
    this.previewBytes,
    this.loading = false,
    this.amazonSearchTerm,
    this.onPickFile,
    this.onRemove,
  });

  /// Id of the thumbnail to load from the server, or null for placeholder.
  final int? thumbnailId;

  /// Pre-downloaded image bytes shown as a preview when [thumbnailId] is null.
  final Uint8List? previewBytes;

  /// When true a loading spinner is shown in the image area.
  final bool loading;

  /// When non-null an Amazon.de search button is shown.
  ///
  /// Called when the button is pressed; the returned term is searched on
  /// Amazon.de.  An empty term opens the Amazon.de start page instead.
  final String Function()? amazonSearchTerm;

  /// When non-null a "From file" button is shown.  Called with PNG-converted
  /// bytes after the user picks an image file; the caller handles the upload.
  final Future<void> Function(Uint8List pngBytes)? onPickFile;

  /// When non-null a "Remove" button is shown while a thumbnail is displayed.
  ///
  /// The caller must clear [thumbnailId] / [previewBytes] so the placeholder
  /// reappears.
  final VoidCallback? onRemove;

  @override
  ConsumerState<ThumbnailWidget> createState() => _ThumbnailWidgetState();
}

class _ThumbnailWidgetState extends ConsumerState<ThumbnailWidget> {
  Uint8List? _bytes;
  bool _loading = false;
  bool _picking = false;

  @override
  void initState() {
    super.initState();
    if(widget.thumbnailId != null) {
      _load(widget.thumbnailId!);
    }
  }

  @override
  void didUpdateWidget(ThumbnailWidget old) {
    super.didUpdateWidget(old);
    if(old.thumbnailId != widget.thumbnailId) {
      setState(() { _bytes = null; });
      if(widget.thumbnailId != null) {
        _load(widget.thumbnailId!);
      }
    }
  }

  Future<void> _load(int id) async {
    setState(() { _loading = true; });
    final bytes =
        await ref.read(itemsApiProvider).getThumbnailBytes(id);
    if(!mounted) return;
    setState(() {
      _bytes = bytes;
      _loading = false;
    });
  }

  Future<void> _openAmazon() async {
    final canPick = widget.onPickFile != null;
    final bytes = await Navigator.push<Uint8List>(
      context,
      MaterialPageRoute(
        builder: (_) => AmazonBrowserScreen(
          searchTerm: widget.amazonSearchTerm!(),
          canPickImage: canPick,
        ),
      ),
    );
    if(bytes != null && mounted && canPick) {
      await widget.onPickFile!(bytes);
    }
  }

  void _showZoomed(BuildContext context) {
    final bytes = _bytes ?? widget.previewBytes;
    if(bytes == null) {
      return;
    }
    showDialog<void>(
      context: context,
      builder: (ctx) => Dialog(
        backgroundColor: Colors.black87,
        insetPadding: const EdgeInsets.all(16),
        child: GestureDetector(
          onTap: () => Navigator.of(ctx).pop(),
          child: InteractiveViewer(
            child: Image.memory(bytes, fit: BoxFit.contain),
          ),
        ),
      ),
    );
  }

  Future<void> _pickFile() async {
    setState(() { _picking = true; });
    try {
      final xfile = await ImagePicker().pickImage(
        source: ImageSource.gallery,
      );
      if(!mounted || xfile == null) return;
      final raw = await xfile.readAsBytes();
      final codec = await ui.instantiateImageCodec(raw);
      final frame = await codec.getNextFrame();
      final byteData =
          await frame.image.toByteData(format: ui.ImageByteFormat.png);
      if(!mounted || byteData == null) return;
      final png = byteData.buffer.asUint8List();
      await widget.onPickFile!(png);
    } finally {
      if(mounted) setState(() { _picking = false; });
    }
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final hasPickFile = widget.onPickFile != null;
    final hasAmazon = widget.amazonSearchTerm != null;
    final hasRemove = widget.onRemove != null &&
        (widget.thumbnailId != null || widget.previewBytes != null);

    return Padding(
      padding: const EdgeInsets.only(bottom: 12),
      child: Column(
        children: [
          SizedBox(
            height: 180,
            child: Center(
              child: (_loading || widget.loading)
                  ? const CircularProgressIndicator()
                  : (_bytes ?? widget.previewBytes) != null
                      ? InkWell(
                          onTap: () => _showZoomed(context),
                          child: Image.memory(
                            (_bytes ?? widget.previewBytes)!,
                            fit: BoxFit.contain,
                            height: 180,
                            errorBuilder: (_, __, ___) =>
                                _placeholder(context, l10n),
                          ),
                        )
                      : _placeholder(context, l10n),
            ),
          ),
          if(hasPickFile || hasAmazon || hasRemove) ...[
            const SizedBox(height: 8),
            Wrap(
              spacing: 8,
              children: [
                if(hasPickFile)
                  OutlinedButton.icon(
                    icon: _picking
                        ? const SizedBox(
                            width: 16,
                            height: 16,
                            child: CircularProgressIndicator(
                                strokeWidth: 2),
                          )
                        : const Icon(Icons.image_outlined, size: 18),
                    label: Text(l10n.thumbnailFromFile),
                    onPressed:
                        (_picking || _loading) ? null : _pickFile,
                  ),
                if(hasAmazon)
                  OutlinedButton.icon(
                    icon: const Icon(Icons.open_in_browser, size: 18),
                    label: Text(l10n.thumbnailSearchAmazon),
                    onPressed: _openAmazon,
                  ),
                if(hasRemove)
                  OutlinedButton.icon(
                    icon: const Icon(Icons.delete_outline, size: 18),
                    label: Text(l10n.thumbnailRemove),
                    onPressed: (_picking || _loading)
                        ? null
                        : widget.onRemove,
                  ),
              ],
            ),
          ],
        ],
      ),
    );
  }

  Widget _placeholder(BuildContext context, AppLocalizations l10n) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        Icon(
          Icons.image_not_supported_outlined,
          size: 64,
          color: Theme.of(context).colorScheme.onSurfaceVariant,
        ),
        const SizedBox(height: 4),
        Text(
          l10n.thumbnailNone,
          style: TextStyle(
            color: Theme.of(context).colorScheme.onSurfaceVariant,
          ),
        ),
      ],
    );
  }
}
