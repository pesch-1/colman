// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Client-side mirror of the server permission-delegation policy
/// (`colman_permission_deps.cpp`).
///
/// The REST server is the security boundary and enforces all of these rules;
/// these helpers exist only to keep the UI in sync, so the app never offers a
/// control that the server would reject with HTTP 403.
library;

/// Permission columns a "borrower" (lender) account may hold.
const List<String> kBorrowerProfile = ['view_items', 'reserve'];

/// Permission columns a "data collector" account may hold.
const List<String> kDataCollectorProfile = [
  'collect_acquisitions',
  'collect_liquidations',
  'view_items',
];

/// Permissions every account holds by default (ignored by the containment
/// check; grantable by any person manager).
const List<String> kBaselinePermissions = ['view_personal_data', 'reserve'];

bool _isBaseline(String column) => kBaselinePermissions.contains(column);

/// Columns the delegation rights held in [perms] allow granting.
Set<String> _grantableProfile(Map<String, bool> perms) {
  final cols = <String>{};
  if (perms['create_lenders'] == true) {
    cols.addAll(kBorrowerProfile);
  }
  if (perms['create_data_collectors'] == true) {
    cols.addAll(kDataCollectorProfile);
  }
  return cols;
}

/// Whether [perms] may create or edit persons at all.
///
/// [isAdmin] short-circuits to `true` (the administrator holds everything).
bool mayManagePersons(Map<String, bool> perms, {required bool isAdmin}) {
  if (isAdmin) return true;
  return perms['edit_persons'] == true ||
      perms['create_lenders'] == true ||
      perms['create_data_collectors'] == true;
}

/// Whether the user described by [perms] may grant [column] to someone.
///
/// Anti-escalation: a user may never grant a permission they do not hold
/// themselves (this also applies to `edit_persons` holders); only the
/// administrator holds every permission.
bool mayGrantPermission(
  Map<String, bool> perms,
  String column, {
  required bool isAdmin,
}) {
  if (isAdmin) return true;
  if (perms[column] != true) return false;
  if (perms['edit_persons'] == true) return true;
  if (_isBaseline(column)) {
    return mayManagePersons(perms, isAdmin: false);
  }
  return _grantableProfile(perms).contains(column);
}

/// Whether the user [myPerms] may create or edit a target whose current
/// permission map is [targetPerms].
///
/// The user must be able to grant every (non-baseline) permission the target
/// currently holds.  Besides the containment scope this closes the
/// credential-reset escalation: a manager may only manage accounts whose
/// rights are a subset of their own.
bool mayManageTarget(
  Map<String, bool> myPerms,
  Map<String, bool> targetPerms, {
  required bool isAdmin,
}) {
  if (isAdmin) return true;
  if (!mayManagePersons(myPerms, isAdmin: false)) return false;
  for (final entry in targetPerms.entries) {
    if (entry.value != true || _isBaseline(entry.key)) continue;
    if (!mayGrantPermission(myPerms, entry.key, isAdmin: false)) {
      return false;
    }
  }
  return true;
}
