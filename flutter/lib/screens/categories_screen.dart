// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'package:colman_client/l10n/app_localizations.dart';

import '../api/types_api.dart';
import '../providers/api_client_provider.dart';
import '../providers/auth_provider.dart';
import '../providers/table_watch.dart';
import '../utils/error_utils.dart';
import '../widgets/app_bar_title.dart';
import '../widgets/error_display.dart';
import '../widgets/server_error_dialog.dart';
import 'category_edit_screen.dart';

/// Lists item types ("categories") with localized names and provides
/// create / edit / delete for users holding the `edit_categories` permission.
///
/// Mobile-friendly counterpart of the Qt "Kategorien verwalten" page.
class CategoriesScreen extends ConsumerStatefulWidget {
  const CategoriesScreen({super.key});

  @override
  ConsumerState<CategoriesScreen> createState() => _CategoriesScreenState();
}

class _CategoriesScreenState extends ConsumerState<CategoriesScreen> {
  List<({int id, String name})> _categories = [];
  bool _loading = false;
  Object? _error;

  @override
  void initState() {
    super.initState();
    _load();
  }

  Future<void> _load() async {
    if (!mounted) return;
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final list = await ref.read(typesApiProvider).getLocale();
      if (!mounted) return;
      setState(() {
        _categories = list;
        _loading = false;
      });
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _error = e;
        _loading = false;
      });
    }
  }

  Future<void> _openEditor(AppLocalizations l10n, {int? typeId}) async {
    final saved = await Navigator.of(context).push<bool>(
      MaterialPageRoute(builder: (_) => CategoryEditScreen(typeId: typeId)),
    );
    if (saved == true && mounted) {
      _load();
    }
  }

  Future<void> _confirmDelete(
    AppLocalizations l10n,
    TypesApi api,
    ({int id, String name}) category,
  ) async {
    final confirmed = await showDialog<bool>(
      context: context,
      builder: (ctx) => AlertDialog(
        title: Text(l10n.categoriesDeleteTitle),
        content: Text(l10n.categoriesDeleteConfirm(category.name)),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: Text(MaterialLocalizations.of(ctx).cancelButtonLabel),
          ),
          TextButton(
            onPressed: () => Navigator.pop(ctx, true),
            child: Text(l10n.personDelete),
          ),
        ],
      ),
    );
    if (confirmed != true || !mounted) return;
    try {
      await api.delete(category.id);
      if (!mounted) return;
      ScaffoldMessenger.of(
        context,
      ).showSnackBar(SnackBar(content: Text(l10n.categoriesDeleteSuccess)));
      _load();
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(friendlyHttpError(e, l10n)),
          action: SnackBarAction(
            label: l10n.errorDetails,
            onPressed: () {
              if (!mounted) return;
              ServerErrorDialog.show(context, e, l10n);
            },
          ),
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    watchTables(ref, const ['types'], _load);
    final l10n = AppLocalizations.of(context)!;
    final auth = ref.watch(authProvider).value;
    final canManage =
        auth is Authenticated && auth.hasPermission('edit_categories');
    final api = ref.read(typesApiProvider);
    return Scaffold(
      appBar: AppBar(
        title: AppBarTitle(
          icon: 'assets/icons/edit_types.jpeg',
          title: l10n.categoriesTitle,
        ),
      ),
      body: _buildBody(l10n, canManage, api),
      floatingActionButton: canManage
          ? FloatingActionButton(
              onPressed: () => _openEditor(l10n),
              tooltip: l10n.categoriesAddTitle,
              child: const Icon(Icons.add),
            )
          : null,
    );
  }

  Widget _buildBody(AppLocalizations l10n, bool canManage, TypesApi api) {
    if (_loading && _categories.isEmpty) {
      return const Center(child: CircularProgressIndicator());
    }
    if (_error != null && _categories.isEmpty) {
      return Center(
        child: ErrorDisplay(error: _error!, l10n: l10n),
      );
    }
    if (_categories.isEmpty) {
      return Center(child: Text(l10n.categoriesNoResults));
    }
    return RefreshIndicator(
      onRefresh: _load,
      child: ListView.separated(
        itemCount: _categories.length,
        separatorBuilder: (_, __) => const Divider(height: 1),
        itemBuilder: (context, i) {
          final category = _categories[i];
          return ListTile(
            title: Text(category.name),
            onTap: canManage
                ? () => _openEditor(l10n, typeId: category.id)
                : null,
            trailing: canManage
                ? PopupMenuButton<_CategoryAction>(
                    onSelected: (action) {
                      if (action == _CategoryAction.edit) {
                        _openEditor(l10n, typeId: category.id);
                      } else {
                        _confirmDelete(l10n, api, category);
                      }
                    },
                    itemBuilder: (_) => [
                      PopupMenuItem(
                        value: _CategoryAction.edit,
                        child: Text(l10n.categoriesEditTitle),
                      ),
                      PopupMenuItem(
                        value: _CategoryAction.delete,
                        child: Text(l10n.personDelete),
                      ),
                    ],
                  )
                : null,
          );
        },
      ),
    );
  }
}

enum _CategoryAction { edit, delete }
