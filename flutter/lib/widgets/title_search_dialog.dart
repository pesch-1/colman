// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:colman_client/l10n/app_localizations.dart';

import '../api/ean_lookup.dart' show queryBggThing;
import '../api/title_lookup.dart';
import '../models/internet_services_settings.dart';
import '../models/item_prefill.dart';
import '../utils/error_utils.dart';

/// Modal dialog for a multi-source internet title search.
///
/// Shows kind chips (book, CD, DVD, Blu-ray, board game), a search field,
/// and a result list with entries of the form "name (year) — source".
/// Tapping a result adopts it: the dialog pops with an [ItemPrefill] for
/// the create-item dialog. Board-game results trigger a lazy BGG detail
/// fetch before the dialog closes.
class TitleSearchDialog extends StatefulWidget {
  const TitleSearchDialog({
    super.key,
    required this.settings,
    this.initialQuery = '',
  });

  /// Internet service settings selecting the queried sources.
  final InternetServicesSettings settings;

  /// Pre-filled search term; a non-empty value triggers an automatic
  /// search when the dialog opens.
  final String initialQuery;

  @override
  State<TitleSearchDialog> createState() => _TitleSearchDialogState();
}

class _TitleSearchDialogState extends State<TitleSearchDialog> {
  late final TextEditingController _queryCtrl;

  /// Currently selected media kind.
  MediaKind _kind = MediaKind.book;

  /// Results of the last search, or null before the first search.
  List<TitleSearchResult>? _results;

  /// Query string of the last search (for the no-results message).
  String _lastQuery = '';

  /// True while a search is in flight.
  bool _searching = false;

  /// True while a BGG detail fetch is in flight (blocks the list).
  bool _resolving = false;

  /// Error text shown below the list, empty when there is no error.
  String _error = '';

  @override
  void initState() {
    super.initState();
    _queryCtrl = TextEditingController(text: widget.initialQuery);
    _kind = MediaKind.values.firstWhere(
      (kind) => titleSearchAvailableFor(kind, widget.settings),
      orElse: () => MediaKind.book,
    );
    if(widget.initialQuery.trim().isNotEmpty) {
      WidgetsBinding.instance.addPostFrameCallback((_) => _search());
    }
  }

  @override
  void dispose() {
    _queryCtrl.dispose();
    super.dispose();
  }

  Future<void> _search() async {
    final query = _queryCtrl.text.trim();
    if(query.isEmpty || _searching || _resolving) return;
    final l10n = AppLocalizations.of(context)!;
    setState(() {
      _searching = true;
      _error = '';
    });
    try {
      final results = await queryTitle(query, _kind, widget.settings);
      if(!mounted) return;
      setState(() {
        _results = results;
        _lastQuery = query;
        _searching = false;
      });
    } catch(e) {
      if(!mounted) return;
      setState(() {
        _searching = false;
        _error = friendlyHttpError(e, l10n);
      });
    }
  }

  void _selectKind(MediaKind kind) {
    if(kind == _kind || _searching || _resolving) return;
    setState(() {
      _kind = kind;
      _results = null;
      _error = '';
    });
    if(_queryCtrl.text.trim().isNotEmpty) _search();
  }

  Future<void> _adopt(TitleSearchResult result) async {
    if(_resolving || _searching) return;
    final l10n = AppLocalizations.of(context)!;
    if(result is BggTitleResult) {
      setState(() {
        _resolving = true;
        _error = '';
      });
      try {
        final detail = await queryBggThing(
          result.bggId,
          widget.settings.bggToken,
          widget.settings.timeoutMs,
        );
        if(!mounted) return;
        if(detail.isEmpty) {
          setState(() {
            _resolving = false;
            _error = l10n.titleSearchDetailFailed;
          });
          return;
        }
        Navigator.of(context).pop(bggToItemPrefill(detail, result.bggId));
      } catch(e) {
        if(!mounted) return;
        setState(() {
          _resolving = false;
          _error = friendlyHttpError(e, l10n);
        });
      }
      return;
    }
    Navigator.of(context).pop(titleResultToItemPrefill(result, _kind));
  }

  String _kindLabel(MediaKind kind, AppLocalizations l10n) {
    switch(kind) {
      case MediaKind.book:
        return l10n.titleSearchKindBook;
      case MediaKind.cd:
        return l10n.titleSearchKindCd;
      case MediaKind.dvd:
        return l10n.titleSearchKindDvd;
      case MediaKind.bluray:
        return l10n.titleSearchKindBluray;
      case MediaKind.boardGame:
        return l10n.titleSearchKindBoardGame;
    }
  }

  String _resultLabel(TitleSearchResult result) {
    var label = result.name;
    if(result.year.isNotEmpty) label += ' (${result.year})';
    if(result.source.isNotEmpty) label += ' — ${result.source}';
    return label;
  }

  Widget _buildResults(AppLocalizations l10n) {
    if(_searching || _resolving) {
      return const Center(
        child: Padding(
          padding: EdgeInsets.all(24),
          child: CircularProgressIndicator(),
        ),
      );
    }
    final results = _results;
    if(results == null) {
      return Padding(
        padding: const EdgeInsets.all(16),
        child: Text(l10n.titleSearchEmptyPrompt),
      );
    }
    if(results.isEmpty) {
      return Padding(
        padding: const EdgeInsets.all(16),
        child: Text(l10n.titleSearchNoResults(_lastQuery)),
      );
    }
    return ListView.separated(
      shrinkWrap: true,
      itemCount: results.length,
      separatorBuilder: (_, _) => const Divider(height: 1),
      itemBuilder: (context, index) {
        final result = results[index];
        return ListTile(
          dense: true,
          title: Text(_resultLabel(result)),
          onTap: () => _adopt(result),
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    final l10n = AppLocalizations.of(context)!;
    final busy = _searching || _resolving;
    return AlertDialog(
      title: Text(l10n.titleSearchTitle),
      content: SizedBox(
        width: 480,
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            Wrap(
              spacing: 8,
              runSpacing: 4,
              children: [
                for(final kind in MediaKind.values)
                  ChoiceChip(
                    label: Text(_kindLabel(kind, l10n)),
                    selected: _kind == kind,
                    onSelected:
                        titleSearchAvailableFor(kind, widget.settings) &&
                                !busy
                            ? (_) => _selectKind(kind)
                            : null,
                  ),
              ],
            ),
            const SizedBox(height: 12),
            Row(
              children: [
                Expanded(
                  child: TextField(
                    controller: _queryCtrl,
                    enabled: !busy,
                    autofocus: widget.initialQuery.trim().isEmpty,
                    decoration: InputDecoration(
                      hintText: l10n.titleSearchFieldHint,
                      isDense: true,
                    ),
                    onSubmitted: (_) => _search(),
                  ),
                ),
                IconButton(
                  icon: const Icon(Icons.search),
                  onPressed: busy ? null : _search,
                  tooltip: l10n.titleSearchButton,
                ),
              ],
            ),
            const SizedBox(height: 8),
            Flexible(
              child: ConstrainedBox(
                constraints: const BoxConstraints(maxHeight: 400),
                child: _buildResults(l10n),
              ),
            ),
            if(_error.isNotEmpty)
              Padding(
                padding: const EdgeInsets.only(top: 8),
                child: Text(
                  _error,
                  style: TextStyle(
                    color: Theme.of(context).colorScheme.error,
                  ),
                ),
              ),
          ],
        ),
      ),
      actions: [
        TextButton(
          onPressed:
              _resolving ? null : () => Navigator.of(context).pop(),
          child: Text(MaterialLocalizations.of(context).cancelButtonLabel),
        ),
      ],
    );
  }
}
