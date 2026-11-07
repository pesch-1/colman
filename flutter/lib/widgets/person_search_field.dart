// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';

import 'package:flutter/material.dart';

import '../api/persons_api.dart';
import '../models/person.dart';

/// Person search field with inline suggestion list for use inside dialogs.
///
/// Uses a debounced query against [personsApi] and renders suggestions as a
/// compact list below the text field — avoids overlay positioning problems
/// that arise when using [Autocomplete] inside an [AlertDialog].
class PersonSearchField extends StatefulWidget {
  const PersonSearchField({
    super.key,
    required this.personsApi,
    required this.labelText,
    required this.onSelected,
    this.initialText,
  });

  final PersonsApi personsApi;
  final String labelText;
  final void Function(Person) onSelected;

  /// Optional pre-filled text for the search field.
  final String? initialText;

  @override
  State<PersonSearchField> createState() => _PersonSearchFieldState();
}

class _PersonSearchFieldState extends State<PersonSearchField> {
  late final TextEditingController _controller;
  Timer? _debounce;
  List<Person> _suggestions = [];

  @override
  void initState() {
    super.initState();
    _controller = TextEditingController(text: widget.initialText ?? '');
  }

  @override
  void dispose() {
    _debounce?.cancel();
    _controller.dispose();
    super.dispose();
  }

  void _onChanged(String value) {
    _debounce?.cancel();
    if(value.length < 2) {
      setState(() { _suggestions = []; });
      return;
    }
    _debounce = Timer(
      const Duration(milliseconds: 300),
      () => _search(value),
    );
  }

  Future<void> _search(String query) async {
    try {
      final results = await widget.personsApi.search(query);
      if(!mounted) return;
      setState(() { _suggestions = results; });
    } catch(_) {
      // ignore search errors — user can retry by typing
    }
  }

  void _select(Person p) {
    _controller.text = '${p.firstname} ${p.lastname}';
    setState(() { _suggestions = []; });
    widget.onSelected(p);
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        TextField(
          controller: _controller,
          decoration: InputDecoration(
            labelText: widget.labelText,
            border: const OutlineInputBorder(),
          ),
          onChanged: _onChanged,
        ),
        if(_suggestions.isNotEmpty)
          Card(
            margin: const EdgeInsets.only(top: 2),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: _suggestions
                  .map(
                    (p) => ListTile(
                      dense: true,
                      title: Text('${p.firstname} ${p.lastname}'),
                      onTap: () => _select(p),
                    ),
                  )
                  .toList(),
            ),
          ),
      ],
    );
  }
}
