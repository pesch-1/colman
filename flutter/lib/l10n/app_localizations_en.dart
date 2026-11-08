// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026

// ignore: unused_import
import 'package:intl/intl.dart' as intl;
import 'app_localizations.dart';

// ignore_for_file: type=lint

/// The translations for English (`en`).
class AppLocalizationsEn extends AppLocalizations {
  AppLocalizationsEn([String locale = 'en']) : super(locale);

  @override
  String get appTitle => 'Colman';

  @override
  String get signOut => 'Sign out';

  @override
  String get navLend => 'Lend Item';

  @override
  String get navReturn => 'Return Item';

  @override
  String get navOpenLendings => 'Due';

  @override
  String get navMyLendings => 'Borrowed';

  @override
  String get navMyReservations => 'Reserved';

  @override
  String get myReservationsEmpty => 'No active reservations.';

  @override
  String get myReservationsColExpires => 'Expires';

  @override
  String get openLendingsPersonLabel => 'Person';

  @override
  String get openLendingsAllClasses => 'All classes';

  @override
  String get openLendingsLabelClass => 'Class';

  @override
  String get openLendingsClearFilter => 'Clear filter';

  @override
  String get openLendingsPrint => 'Print';

  @override
  String get openLendingsColDueDate => 'Due date';

  @override
  String get openLendingsColBorrower => 'Borrower';

  @override
  String get openLendingsColCount => 'Count';

  @override
  String get openLendingsColItem => 'Item';

  @override
  String get navHistory => 'View History';

  @override
  String get navPersons => 'Borrowers';

  @override
  String get navAcquire => 'Acquire';

  @override
  String get navLiquidate => 'Liquidate';

  @override
  String get navSearch => 'Search';

  @override
  String get navScan => 'Scan';

  @override
  String get navClasses => 'Classes';

  @override
  String get navCategories => 'Categories';

  @override
  String get categoriesTitle => 'Manage categories';

  @override
  String get categoriesNoResults => 'No categories defined.';

  @override
  String get categoriesAddTitle => 'New category';

  @override
  String get categoriesEditTitle => 'Edit category';

  @override
  String get categoriesNameLabel => 'Category name';

  @override
  String get categoriesSearchCategory => 'Search category';

  @override
  String get categoriesFields => 'Fields';

  @override
  String get categoriesNoFields => 'No fields yet.';

  @override
  String get categoriesAddField => 'Add field';

  @override
  String get categoriesEditFieldTitle => 'Field';

  @override
  String get categoriesFieldName => 'Field name';

  @override
  String get categoriesFieldType => 'Field type';

  @override
  String get categoriesFieldRepeatable => 'Repeatable';

  @override
  String get categoriesFieldDelete => 'Remove field';

  @override
  String get categoriesDeleteTitle => 'Delete category';

  @override
  String categoriesDeleteConfirm(String name) {
    return 'Delete category \'$name\'? All items of this type and their lending history will be permanently removed.';
  }

  @override
  String get categoriesSaveSuccess => 'Category saved.';

  @override
  String get categoriesSaveFailed =>
      'Could not save the category. The name may already be in use.';

  @override
  String get categoriesDeleteSuccess => 'Category deleted.';

  @override
  String get mediaKindUnknown => 'None';

  @override
  String get mediaKindBook => 'Book';

  @override
  String get mediaKindCd => 'CD';

  @override
  String get mediaKindDvd => 'DVD';

  @override
  String get mediaKindBluRay => 'Blu-ray';

  @override
  String get mediaKindBoardGame => 'Board game';

  @override
  String get dataTypeString => 'Text (multi-line)';

  @override
  String get dataTypeShortString => 'Text';

  @override
  String get dataTypeDate => 'Date';

  @override
  String get dataTypeTime => 'Time';

  @override
  String get dataTypeInteger => 'Integer';

  @override
  String get dataTypeFloat => 'Decimal';

  @override
  String get dataTypeDuration => 'Duration';

  @override
  String get dataTypeTable => 'List';

  @override
  String get dataTypeIntegerRanges => 'Integer ranges';

  @override
  String get navSettings => 'Settings';

  @override
  String get loginSubtitle => 'Library Management';

  @override
  String loginVersion(String version, String build) {
    return 'Version $version (build $build)';
  }

  @override
  String get loginSearching => 'Searching...';

  @override
  String get loginNoServers => 'No servers found';

  @override
  String get loginDiscoveryUnavailable => 'Discovery not available';

  @override
  String get loginDiscoverOnLan => 'Discover on LAN';

  @override
  String get loginRefresh => 'Refresh';

  @override
  String get loginEnterUrlManually => 'Enter URL manually';

  @override
  String get loginBackToDiscovery => 'Back to discovery';

  @override
  String get loginServerUrl => 'Server URL';

  @override
  String get loginServerUrlHint => 'https://colman.example.com';

  @override
  String get loginUsername => 'Username';

  @override
  String get loginPassword => 'Password';

  @override
  String get loginRequired => 'Required';

  @override
  String get loginSignIn => 'Sign in';

  @override
  String loginFailed(String error) {
    return 'Login failed: $error';
  }

  @override
  String get itemsTitle => 'Search Items';

  @override
  String get itemsSearchHint => 'Search by name, keyword…';

  @override
  String get itemsEmptyPrompt => 'Enter a search term above.';

  @override
  String get itemsNotFound => 'No items found.';

  @override
  String errorPrefix(String error) {
    return 'Error: $error';
  }

  @override
  String get errorDetails => 'Details';

  @override
  String get errorConnectionFailed => 'Could not connect to the server.';

  @override
  String get errorAccessDenied => 'You don\'t have permission for this action.';

  @override
  String get errorInvalidCredentials => 'Incorrect username or password.';

  @override
  String get errorServerError => 'The server reported an error.';

  @override
  String get errorRequestFailed => 'The request failed.';

  @override
  String errorIncompatibleServer(String serverVersion, String requiredVersion) {
    return 'This server is not compatible with this app (server $serverVersion, app requires $requiredVersion). Please update the app or the server.';
  }

  @override
  String get retry => 'Retry';

  @override
  String get itemDetailFallbackTitle => 'Item';

  @override
  String get labelName => 'Name';

  @override
  String get labelType => 'Type';

  @override
  String get labelPlace => 'Place';

  @override
  String get labelBarcode => 'Colman barcode';

  @override
  String get labelDescription => 'Description';

  @override
  String get labelAvailability => 'Availability';

  @override
  String availabilityOf(int available, int count) {
    return '$available of $count available';
  }

  @override
  String get lendingTitle => 'Lending';

  @override
  String get lendingComingSoon => 'Lending — coming soon';

  @override
  String get lendingTabNew => 'New';

  @override
  String get lendingTabOpen => 'Open';

  @override
  String get lendingTabDue => 'Due';

  @override
  String get lendingStepPerson => 'Person';

  @override
  String get lendingStepItem => 'Item';

  @override
  String get lendingStepDetails => 'Details';

  @override
  String get lendingStep1Title => 'Select borrower';

  @override
  String get lendingStep2Title => 'Select item';

  @override
  String get lendingStep3Title => 'Lending details';

  @override
  String get lendingItemSearchHint => 'Search by name…';

  @override
  String get lendingItemNotFound => 'No items found.';

  @override
  String get lendingItemSearchPrompt => 'Enter item name to search.';

  @override
  String lendingSuccessLendId(int id) {
    return 'Lending #$id created';
  }

  @override
  String lendingSuccessReserveId(int id) {
    return 'Reservation #$id created';
  }

  @override
  String get lendingNewLending => 'New lending';

  @override
  String get lendingNewItemSamePerson => 'New item, same person';

  @override
  String lendingConflictReserved(int count) {
    return 'Only $count unreserved copy/copies available';
  }

  @override
  String get lendingOpenNoResults => 'No open lendings.';

  @override
  String lendingOpenLentReturned(int lent, int returned) {
    return '$lent lent, $returned returned';
  }

  @override
  String get lendingDueNoResults => 'No due lendings.';

  @override
  String lendingDueOutstanding(int count) {
    return '$count outstanding';
  }

  @override
  String get returnTitle => 'Return';

  @override
  String get returnComingSoon => 'Return — coming soon';

  @override
  String get returnTabNew => 'New';

  @override
  String get returnTabOpen => 'Open';

  @override
  String get returnStepLending => 'Lending';

  @override
  String get returnStepDetails => 'Details';

  @override
  String get returnStep1Title => 'Select item';

  @override
  String get returnStep2Title => 'Select lending';

  @override
  String get returnStep3Title => 'Return details';

  @override
  String get returnStep2NoResults => 'No open lendings for this item.';

  @override
  String get returnOpenNoResults => 'No open lendings.';

  @override
  String returnSuccessId(int id) {
    return 'Return #$id recorded';
  }

  @override
  String get returnNewReturn => 'New return';

  @override
  String get returnSameItem => 'Same item';

  @override
  String get acquisitionTitle => 'Acquisition';

  @override
  String get acquisitionComingSoon => 'Acquisition — coming soon';

  @override
  String get acquisitionStep1Title => 'Select item';

  @override
  String get acquisitionStep2Title => 'Acquisition details';

  @override
  String acquisitionSuccessId(int id) {
    return 'Acquisition #$id recorded';
  }

  @override
  String get acquisitionNewAcquisition => 'New acquisition';

  @override
  String get acquisitionCreateItemTitle => 'New item';

  @override
  String get acquisitionCreateItemButton => 'Create new item';

  @override
  String get acquisitionBarcodeHint => 'Scan or enter barcode';

  @override
  String get acquisitionBarcodeSearchingInternet => 'Searching internet…';

  @override
  String get acquisitionBarcodeNotFound => 'Barcode not found';

  @override
  String get barcodeScanTitle => 'Scan Barcode';

  @override
  String get barcodeScanHint => 'Hold camera over barcode';

  @override
  String get barcodeScanTooltip => 'Scan with camera';

  @override
  String get liquidationTitle => 'Liquidation';

  @override
  String get liquidationComingSoon => 'Liquidation — coming soon';

  @override
  String get liquidationStep1Title => 'Select item';

  @override
  String get liquidationStep2Title => 'Liquidation details';

  @override
  String liquidationSuccessId(int id) {
    return 'Liquidation #$id recorded';
  }

  @override
  String get liquidationNewLiquidation => 'New liquidation';

  @override
  String get historyTitle => 'History';

  @override
  String get historyComingSoon => 'History — coming soon';

  @override
  String get personsTitle => 'Persons';

  @override
  String get personsComingSoon => 'Persons — coming soon';

  @override
  String get classesTitle => 'Classes';

  @override
  String get classesComingSoon => 'Classes — coming soon';

  @override
  String get classesNoResults => 'No classes defined.';

  @override
  String get classesAddTooltip => 'New class';

  @override
  String get classesAddTitle => 'New class';

  @override
  String get classesAddNameLabel => 'Class name';

  @override
  String get classesRenameTitle => 'Rename class';

  @override
  String get classesRenameNewNameLabel => 'New name';

  @override
  String classesDeleteConfirm(String name) {
    return 'Delete class \'$name\'?';
  }

  @override
  String get classesGradeRange => 'Grade range';

  @override
  String get classesGradeRangeMin => 'Min';

  @override
  String get classesGradeRangeMax => 'Max';

  @override
  String get classesNewSchoolYear => 'New school year';

  @override
  String get classesNewSchoolYearStep1 => 'Class name changes';

  @override
  String get classesNewSchoolYearStep2 => 'Affected persons';

  @override
  String get classesNewSchoolYearDissolved => '(dissolved)';

  @override
  String get classesNewSchoolYearAddClass => 'Add class';

  @override
  String get classesNewSchoolYearNoPersons => 'No persons affected.';

  @override
  String get classesNewSchoolYearNoClass => '(no class)';

  @override
  String get classesNewSchoolYearDeletePerson => 'Delete person';

  @override
  String get classesApply => 'Apply';

  @override
  String get classesNext => 'Next';

  @override
  String get settingsTitle => 'Settings';

  @override
  String get settingsComingSoon => 'Settings — coming soon';

  @override
  String get settingsInternetServicesSection => 'Internet services';

  @override
  String get settingsNetworkSection => 'Network';

  @override
  String get settingsSruServersSection => 'SRU library servers';

  @override
  String get settingsOpenLibrary => 'Open Library';

  @override
  String get settingsGoogleBooks => 'Google Books';

  @override
  String get settingsGoogleApisKey => 'API key';

  @override
  String get settingsMusicBrainz => 'MusicBrainz';

  @override
  String get settingsBgg => 'BoardGameGeek';

  @override
  String get settingsBggToken => 'BGG token';

  @override
  String get settingsOpenGtindb => 'OpenGTINdb';

  @override
  String get settingsOpenGtindbUserId => 'User ID';

  @override
  String get settingsDownloadThumbnails => 'Download cover images';

  @override
  String get settingsTimeoutSeconds => 'Timeout (s)';

  @override
  String get settingsPollIntervalSeconds => 'Refresh interval (s)';

  @override
  String get settingsPollIntervalHint =>
      'How often the app checks the server for changes (5–300 s).';

  @override
  String get settingsSruName => 'Name';

  @override
  String get settingsSruEndpoint => 'Endpoint';

  @override
  String get settingsSruIsbnField => 'ISBN field';

  @override
  String get settingsSruEanField => 'EAN field';

  @override
  String get settingsSruSchema => 'Schema';

  @override
  String get settingsSruAddServer => 'Add server';

  @override
  String get settingsSruDeleteServer => 'Delete server';

  @override
  String get settingsSruEditServer => 'Edit server';

  @override
  String get settingsSave => 'Save';

  @override
  String get settingsRevert => 'Revert';

  @override
  String get settingsSaved => 'Settings saved';

  @override
  String get serverUnreachableBanner =>
      'Server unreachable — the data shown may be out of date.';

  @override
  String get dismiss => 'Dismiss';

  @override
  String get personDetailFallbackTitle => 'Person';

  @override
  String personDetailComingSoon(int id) {
    return 'Person $id — coming soon';
  }

  @override
  String get labelSignatureTop => 'Signature (top)';

  @override
  String get labelSignatureBottom => 'Signature (bottom)';

  @override
  String get labelKeywords => 'Keywords';

  @override
  String get labelSubject => 'Subject';

  @override
  String get labelAcademicYear => 'Academic year';

  @override
  String get itemsAdvancedSearch => 'Advanced search';

  @override
  String get labelAllTypes => 'All types';

  @override
  String get labelCount => 'Count';

  @override
  String get labelExpiryDate => 'Expires on';

  @override
  String get labelDueDate => 'Due date';

  @override
  String get labelPerson => 'Person';

  @override
  String get labelPersonSearch => 'Search person…';

  @override
  String get actionReserve => 'Reserve';

  @override
  String get actionLend => 'Lend';

  @override
  String get actionCancelReservation => 'Cancel reservation';

  @override
  String get actionEditItem => 'Edit item';

  @override
  String get actionDeleteItem => 'Delete item';

  @override
  String deleteItemConfirmMessage(String name) {
    return 'Permanently delete \"$name\" and all its past acquisitions, liquidations, lendings and returns?\n\nThis cannot be undone.';
  }

  @override
  String get itemDeleted => 'Item deleted.';

  @override
  String get itemsScanNotFound => 'No item found for this barcode';

  @override
  String get itemEditKeywordsHint => 'comma-separated';

  @override
  String get itemSaved => 'Item saved.';

  @override
  String get reserveDialogTitle => 'Reserve item';

  @override
  String get reserveSelfHint =>
      'The item will be reserved for your own account.';

  @override
  String get lendDialogTitle => 'Lend item';

  @override
  String get cancelReservationDialogTitle => 'Cancel reservation';

  @override
  String get actionConfirm => 'Confirm';

  @override
  String get reservationCreated => 'Reservation created';

  @override
  String get lendingCreated => 'Lending created';

  @override
  String get reservationCancelled => 'Reservation cancelled';

  @override
  String get errorAlreadyReserved =>
      'A reservation already exists for this person';

  @override
  String get errorNotEnoughAvailable => 'Not enough copies available';

  @override
  String get errorReservationsDisabled =>
      'Reservations are disabled for this item';

  @override
  String get noActiveReservations => 'No active reservations';

  @override
  String get selectReservationToCancel => 'Select reservation to cancel';

  @override
  String get historyFromDate => 'From';

  @override
  String get historyToDate => 'To';

  @override
  String get historyApply => 'Apply';

  @override
  String get historyShowAll => 'Show all';

  @override
  String get historyFilterAllActions => 'All actions';

  @override
  String get extendDueDateButton => 'Extend due date';

  @override
  String get extendDueDateTitle => 'Extend due date';

  @override
  String get extendDueDateAction => 'Extend';

  @override
  String get extendDueDateNoLendings => 'This person has no open lendings.';

  @override
  String get extendDueDateSelectHint =>
      'Select the lendings whose due date shall be changed:';

  @override
  String get extendDueDateNewDate => 'New due date';

  @override
  String extendDueDateFixedHint(int count) {
    String _temp0 = intl.Intl.pluralLogic(
      count,
      locale: localeName,
      other: '$count lendings will be extended:',
      one: '1 lending will be extended:',
    );
    return '$_temp0';
  }

  @override
  String openLendingsSelectedCount(int count) {
    return '$count selected';
  }

  @override
  String get openLendingsSelectAll => 'Select all';

  @override
  String get openLendingsDeselectAll => 'Deselect all';

  @override
  String get openLendingsSelectHint =>
      'Select the lendings whose due date shall be extended.';

  @override
  String openLendingsExtendSelected(int count) {
    return 'Extend ($count)';
  }

  @override
  String historyActionsShown(int count) {
    return '$count action(s) shown';
  }

  @override
  String get historyLoadMore => 'Load more';

  @override
  String get historyNoResults => 'No history entries found.';

  @override
  String get historyActionAcquisition => 'acquisition';

  @override
  String get historyActionLiquidation => 'liquidation';

  @override
  String get historyActionLending => 'lending';

  @override
  String get historyActionReturn => 'return';

  @override
  String get historyActionReservation => 'reservation';

  @override
  String get historyLabelDateTime => 'Date/time';

  @override
  String get historyLabelAction => 'Action';

  @override
  String get historyLabelItem => 'Item';

  @override
  String get historyLabelUser => 'Person';

  @override
  String get historyLabelCollector => 'Collector';

  @override
  String get historyLabelRemarks => 'Remarks';

  @override
  String get historyDetailTitle => 'Action details';

  @override
  String get historyDeleteTitle => 'Delete action';

  @override
  String get historyDeleteConfirm => 'Delete this action permanently?';

  @override
  String historyDeleteWithConflicts(int n) {
    return 'Deleting this action requires also deleting $n dependent action(s):';
  }

  @override
  String get historyDeleteAll => 'Delete all';

  @override
  String get historyDeleteSuccess => 'Action(s) deleted.';

  @override
  String get historyDeleteFailed => 'Failed to delete the action.';

  @override
  String get historyEditTitle => 'Edit action';

  @override
  String get historyEditSave => 'Save';

  @override
  String get historyEditSuccess => 'Action updated.';

  @override
  String get historyEditFailed => 'Failed to update the action.';

  @override
  String historyChangeConflicts(int n) {
    return 'Saving this change affects $n other action(s). Continue?';
  }

  @override
  String get historyChangeConflictsTitle => 'Dependent actions';

  @override
  String get personsSearchHint => 'Search by name…';

  @override
  String get personsNoResults => 'No persons found.';

  @override
  String get personsAddTooltip => 'New person';

  @override
  String get personCreateTitle => 'New person';

  @override
  String get personSectionDetails => 'Person details';

  @override
  String get personFormLastname => 'Last name';

  @override
  String get personFormFirstname => 'First name';

  @override
  String get personFormTitle => 'Title';

  @override
  String get personFormSchoolClass => 'Class';

  @override
  String get personFormBarcode => 'Barcode';

  @override
  String get personFormEmail => 'Email (comma-separated)';

  @override
  String get personFormTelephone => 'Telephone';

  @override
  String get personFormStreet => 'Street';

  @override
  String get personFormZip => 'ZIP';

  @override
  String get personFormCity => 'City';

  @override
  String get personFormCountry => 'Country';

  @override
  String get personFormRemarks => 'Remarks';

  @override
  String get personFormExternalId => 'External ID (UUID)';

  @override
  String get personSave => 'Save';

  @override
  String get personSaved => 'Person saved.';

  @override
  String get personConflict =>
      'Conflict — the record was modified externally. Please reload.';

  @override
  String get personSaveFailed => 'Failed to save person.';

  @override
  String get personDelete => 'Delete';

  @override
  String get personDeleteConfirm => 'Delete this person permanently?';

  @override
  String get personDeleteFailed =>
      'Cannot delete — return all open lendings first.';

  @override
  String get personDeleteSuccess => 'Person deleted.';

  @override
  String get personLoginSection => 'Login';

  @override
  String get personUsernameLabel => 'Username';

  @override
  String get personUsernameNotSet => 'No username set';

  @override
  String get personUsernameSetButton => 'Set';

  @override
  String get personUsernameClearButton => 'Remove';

  @override
  String get personUsernameSaved => 'Username saved.';

  @override
  String get personUsernameRemoved => 'Username removed.';

  @override
  String get personUsernameTaken => 'Username already taken.';

  @override
  String get personPasswordSection => 'Password';

  @override
  String get personPasswordIsSet => 'Password is set';

  @override
  String get personPasswordNotSet => 'No password set';

  @override
  String get personPasswordSetButton => 'Set password';

  @override
  String get personPasswordRemoveButton => 'Remove password';

  @override
  String get personPasswordSetTitle => 'Set password';

  @override
  String get personPasswordNew => 'New password';

  @override
  String get personPasswordConfirm => 'Confirm password';

  @override
  String get personPasswordMismatch => 'Passwords do not match.';

  @override
  String get personPasswordSaved => 'Password saved.';

  @override
  String get personPasswordRemoved => 'Password removed.';

  @override
  String get navChangePassword => 'Change password';

  @override
  String get navChangePasswords => 'Manage passwords';

  @override
  String get changePasswordTitle => 'Change password';

  @override
  String get changePasswordCurrent => 'Current password';

  @override
  String get changePasswordWrong => 'Wrong password.';

  @override
  String get changePasswordSubmit => 'Change password';

  @override
  String get changePasswordsTitle => 'Manage passwords';

  @override
  String get changePasswordsSelectPrompt =>
      'Select a person to manage their credentials.';

  @override
  String get changePasswordsNoUsername => 'No username set';

  @override
  String changePasswordsDeleteConfirm(String name) {
    return 'Delete the password for $name?';
  }

  @override
  String get personPermissionsSection => 'Permissions';

  @override
  String get personPermissionsApply => 'Apply';

  @override
  String get personPermissionsSaved => 'Permissions saved.';

  @override
  String get permEditPersons => 'Edit persons';

  @override
  String get permCreateLenders => 'Create borrowers';

  @override
  String get permCreateDataCollectors => 'Create data collectors';

  @override
  String get permCollectLendings => 'Record lendings';

  @override
  String get permCollectReturns => 'Record returns';

  @override
  String get permCollectAcquisitions => 'Record acquisitions';

  @override
  String get permCollectLiquidations => 'Record liquidations';

  @override
  String get permViewHistory => 'View history';

  @override
  String get permViewPersons => 'View person lists';

  @override
  String get permViewItems => 'View item lists';

  @override
  String get permEditPreferences => 'Edit preferences';

  @override
  String get permPrintIds => 'Print ID cards';

  @override
  String get permPrintBarcodes => 'Print barcodes';

  @override
  String get permEditCategories => 'Edit categories';

  @override
  String get permViewPersonalData => 'View personal data';

  @override
  String get permManageLabelLayouts => 'Manage label layouts';

  @override
  String get permCalibratePrinters => 'Calibrate printers';

  @override
  String get permEditIdLayout => 'Edit ID card layout';

  @override
  String get permPrintSignatures => 'Print signature labels';

  @override
  String get permChangeActions => 'Change and delete actions';

  @override
  String get permEditProgramSettings => 'Edit program settings';

  @override
  String get permManageClasses => 'Manage classes';

  @override
  String get permReserve => 'Reserve media';

  @override
  String get permEditReservations => 'Edit reservations';

  @override
  String get permAdminPermissions => 'Edit item settings';

  @override
  String get barcodeDispatchTitle => 'Scan Result';

  @override
  String get barcodeDispatchLoading => 'Looking up barcode…';

  @override
  String barcodeDispatchPersonSubtitle(String name) {
    return 'Actions for $name';
  }

  @override
  String barcodeDispatchItemSubtitle(String name) {
    return 'Actions for: $name';
  }

  @override
  String get barcodeDispatchActionViewLendings =>
      'Open lendings & reservations';

  @override
  String get barcodeDispatchActionEditPerson => 'Edit person';

  @override
  String get barcodeDispatchActionReturnPersonItems => 'Return items';

  @override
  String get barcodeDispatchActionLendToPerson => 'Lend item to this person';

  @override
  String get barcodeDispatchActionReturnItem => 'Return this item';

  @override
  String get barcodeDispatchActionLendItem => 'Lend this item';

  @override
  String get barcodeDispatchActionAcquireItem => 'Acquire / edit item';

  @override
  String get barcodeDispatchActionLiquidateItem => 'Liquidate item';

  @override
  String get barcodeDispatchNotRecognized => 'Barcode not recognized';

  @override
  String get barcodeDispatchNoOpenLendings => 'No open lendings';

  @override
  String get thumbnailFromFile => 'From file';

  @override
  String get thumbnailSearchAmazon => 'Search on Amazon';

  @override
  String get thumbnailRemove => 'Remove';

  @override
  String get thumbnailNone => 'No cover image';

  @override
  String get amazonBrowserHint => 'Tap a cover image to use it';

  @override
  String get amazonBrowserDownloadError => 'Could not download image';

  @override
  String get certDialogTitle => 'Untrusted Certificate';

  @override
  String certDialogBody(
    String host,
    String subject,
    String validFrom,
    String validUntil,
    String fingerprint,
  ) {
    return 'The server \"$host\" uses a certificate that could not be verified.\n\nSubject: $subject\nValid: $validFrom – $validUntil\nSHA-256:\n$fingerprint\n\nTrust this server?';
  }

  @override
  String get certDialogTrust => 'Trust';

  @override
  String get certDialogCancel => 'Cancel';

  @override
  String get titleSearchTitle => 'Title search';

  @override
  String get titleSearchButton => 'Search internet by title';

  @override
  String titleSearchOfferButton(String query) {
    return 'Search the internet for \"$query\"';
  }

  @override
  String get titleSearchFieldHint => 'Title…';

  @override
  String get titleSearchKindBook => 'Book';

  @override
  String get titleSearchKindCd => 'CD';

  @override
  String get titleSearchKindDvd => 'DVD';

  @override
  String get titleSearchKindBluray => 'Blu-ray';

  @override
  String get titleSearchKindBoardGame => 'Board game';

  @override
  String get titleSearchEmptyPrompt => 'Enter a title and start the search.';

  @override
  String titleSearchNoResults(String query) {
    return 'No results found for \"$query\".';
  }

  @override
  String get titleSearchDetailFailed =>
      'Could not retrieve details from the selected source.';

  @override
  String get settingsSruTitleField => 'Title field';

  @override
  String get navIdCardLayout => 'ID card layout';

  @override
  String get navIdCardPrint => 'Print ID cards';

  @override
  String get idCardLayoutTitle => 'ID card layout';

  @override
  String get idCardLayoutCardSize => 'Card size';

  @override
  String get idCardLayoutWidthMm => 'Width (mm)';

  @override
  String get idCardLayoutHeightMm => 'Height (mm)';

  @override
  String get idCardLayoutUseLogo => 'Use logo';

  @override
  String get idCardLayoutMarginMm => 'Free margin (mm)';

  @override
  String get idCardLayoutZoneBarcode => 'Barcode';

  @override
  String get idCardLayoutZoneText => 'Text';

  @override
  String get idCardLayoutZoneLogo => 'Logo';

  @override
  String get idCardLayoutRotate => 'Rotate 90°';

  @override
  String get idCardLayoutMaximize => 'Maximize';

  @override
  String get idCardLayoutAlignLeft => 'Align left';

  @override
  String get idCardLayoutAlignHCenter => 'Centre horizontally';

  @override
  String get idCardLayoutAlignRight => 'Align right';

  @override
  String get idCardLayoutAlignTop => 'Align top';

  @override
  String get idCardLayoutAlignVCenter => 'Centre vertically';

  @override
  String get idCardLayoutAlignBottom => 'Align bottom';

  @override
  String get idCardLayoutRotateBlocked =>
      'The zone does not fit on the card after rotation.';

  @override
  String get idCardLayoutSave => 'Save';

  @override
  String get idCardLayoutReset => 'Reset';

  @override
  String get idCardLayoutSaved => 'Layout saved.';

  @override
  String get idCardSampleName => 'Sample Name';

  @override
  String idCardIssuedOn(String date) {
    return 'issued on: $date';
  }

  @override
  String get idCardPrintTitle => 'Print ID cards';

  @override
  String get idCardPrintSearchHint => 'Search persons';

  @override
  String get idCardPrintSelectAll => 'Select all';

  @override
  String get idCardPrintDeselectAll => 'Deselect all';

  @override
  String idCardPrintSelectedCount(int count) {
    return '$count selected';
  }

  @override
  String get idCardPrintButton => 'Print';

  @override
  String get idCardPrintNoSelection => 'Select at least one person.';

  @override
  String get idCardPrintNoLayout => 'No ID-card layout configured.';

  @override
  String get idCardPrintArrangement => 'Label arrangement';

  @override
  String get idCardPrintPaperSize => 'Paper size';

  @override
  String get idCardPrintOrientation => 'Orientation';

  @override
  String get idCardPrintPortrait => 'Portrait';

  @override
  String get idCardPrintLandscape => 'Landscape';

  @override
  String get idCardPrintColumns => 'Columns';

  @override
  String get idCardPrintRows => 'Rows';

  @override
  String get idCardPrintMarginLeft => 'Left margin (mm)';

  @override
  String get idCardPrintMarginTop => 'Top margin (mm)';

  @override
  String get idCardPrintGapH => 'Horizontal gap (mm)';

  @override
  String get idCardPrintGapV => 'Vertical gap (mm)';

  @override
  String get idCardPrintDrawBorder => 'Draw border around each card';

  @override
  String get navThermalLabelPrint => 'Print labels';

  @override
  String get navLabelPrint => 'Print labels';

  @override
  String get navLabelPresets => 'Label layouts';

  @override
  String get navSignaturePrint => 'Print signatures';

  @override
  String get signaturePrintTitle => 'Print signatures';

  @override
  String get signaturePrintTop => 'Top signature';

  @override
  String get signaturePrintBottom => 'Bottom signature';

  @override
  String get signaturePrintCount => 'Count';

  @override
  String get signaturePrintButton => 'Print';

  @override
  String get signaturePrintPickItem =>
      'Pick an item to prefill the signatures (optional)';

  @override
  String get signaturePrintEmpty => 'Enter a signature first.';

  @override
  String get labelPrintTitle => 'Print labels';

  @override
  String get labelPrintButton => 'Print';

  @override
  String get labelPrintNext => 'Next: positions';

  @override
  String get labelPrintSearchHint => 'Search items';

  @override
  String get labelPrintCopies => 'Copies per item';

  @override
  String get labelPrintSelectAll => 'Select all';

  @override
  String get labelPrintDeselectAll => 'Deselect all';

  @override
  String labelPrintSelectedCount(int count) {
    return '$count selected';
  }

  @override
  String get labelPrintNoItems => 'No items found.';

  @override
  String get labelPrintNoSelection => 'Select at least one item.';

  @override
  String get labelPrintNoPrinter => 'No USB printer connected.';

  @override
  String get labelPrintPrinterConnected => 'Connected via USB';

  @override
  String get labelPrintRefresh => 'Rescan for printer';

  @override
  String get labelPrintPreset => 'Layout';

  @override
  String get labelPrintPositions => 'Label positions on the sheet';

  @override
  String get labelPrintPositionsHint =>
      'Pick which positions on the sheet to use — at most as many as selected items.';

  @override
  String labelPrintCellsSelected(int count, int limit) {
    return '$count of $limit positions selected';
  }

  @override
  String get labelPrintTooManyCells =>
      'You cannot select more positions than selected items.';

  @override
  String labelPrintRemaining(int count) {
    return '$count item(s) remaining — pick positions for the next sheet.';
  }

  @override
  String get labelPrintNoCells => 'Select at least one label position.';

  @override
  String get labelPrintPermissionDenied => 'USB permission denied.';

  @override
  String labelPrintSent(int count) {
    return '$count label(s) sent to the printer.';
  }

  @override
  String get cancel => 'Cancel';

  @override
  String get ok => 'OK';

  @override
  String get signatureSample => 'Sample 4';

  @override
  String get labelPresetTitle => 'Label layouts';

  @override
  String get labelPresetPreset => 'Layout';

  @override
  String get labelPresetNew => 'New layout';

  @override
  String get labelPresetRename => 'Rename';

  @override
  String get labelPresetDelete => 'Delete';

  @override
  String get labelPresetName => 'Name';

  @override
  String get labelPresetPurpose => 'Purpose';

  @override
  String get labelPresetPrinter => 'Printer';

  @override
  String get labelPresetBarWidthReduction => 'Bar width reduction (mm)';

  @override
  String get labelBarcodeUnit => 'Barcode bar width (mm)';

  @override
  String get labelPresetEditArrangement => 'Edit label arrangement';

  @override
  String get labelPresetArrangement => 'Label arrangement';

  @override
  String get labelPresetSave => 'Save';

  @override
  String get labelPresetSaved => 'Layout saved.';

  @override
  String get labelPresetDarkness => 'Darkness';

  @override
  String get labelPresetGap => 'Gap (mm)';

  @override
  String get labelPresetXOffset => 'Horizontal offset (mm)';

  @override
  String get labelPresetColumns => 'Columns';

  @override
  String get labelPresetRows => 'Rows';

  @override
  String get labelPresetLeftMargin => 'Left margin (mm)';

  @override
  String get labelPresetTopMargin => 'Top margin (mm)';

  @override
  String get labelPresetHGap => 'Horizontal gap (mm)';

  @override
  String get labelPresetVGap => 'Vertical gap (mm)';

  @override
  String get labelPresetPaperSize => 'Paper size';

  @override
  String get labelPresetLandscape => 'Landscape';

  @override
  String get labelPurposeBookLabel => 'Book label';

  @override
  String get labelPurposeSignature => 'Signature';

  @override
  String get labelPurposeIdCard => 'ID card';

  @override
  String get labelPrinterThermal => 'USB thermal printer';

  @override
  String get labelPrinterSheet => 'Normal/sheet printer';

  @override
  String get navBookLabelLayout => 'Book label layout';

  @override
  String get bookLabelLayoutTitle => 'Book label layout';

  @override
  String get bookLabelLayoutSaved => 'Book label layout saved.';

  @override
  String get bookLabelSampleOrganization => 'Sample Library';

  @override
  String get bookLabelSampleTitle => 'Sample Title';

  @override
  String get bookLabelSampleAuthor => 'Sample Author';

  @override
  String get thermalPrintTitle => 'Print item labels';

  @override
  String get thermalPrintButton => 'Print on USB printer';

  @override
  String get thermalPrintSystemButton => 'Print on a normal printer';

  @override
  String get thermalPrintSearchHint => 'Search items';

  @override
  String get thermalPrintSelectAll => 'Select all';

  @override
  String get thermalPrintDeselectAll => 'Deselect all';

  @override
  String thermalPrintSelectedCount(int count) {
    return '$count selected';
  }

  @override
  String get thermalPrintNoItems => 'No items found.';

  @override
  String get thermalPrintNoSelection => 'Select at least one item.';

  @override
  String get thermalPrintNoPrinter => 'No USB printer connected.';

  @override
  String get thermalPrintPrinterConnected => 'Connected via USB';

  @override
  String get thermalPrintRefresh => 'Rescan for printer';

  @override
  String get thermalPrintLabelSettings => 'Label & printer settings';

  @override
  String get thermalPrintWidthMm => 'Label width (mm)';

  @override
  String get thermalPrintHeightMm => 'Label height (mm)';

  @override
  String get thermalPrintGapMm => 'Gap between labels (mm)';

  @override
  String get thermalPrintXOffsetMm => 'Horizontal offset (mm)';

  @override
  String get thermalPrintDarkness => 'Darkness';

  @override
  String get thermalPrintPermissionDenied => 'USB permission denied.';

  @override
  String thermalPrintSent(int count) {
    return '$count label(s) sent to the printer.';
  }

  @override
  String get thermalPrintAndroidOnly =>
      'Direct USB label printing is only available on Android.';

  @override
  String get calibrationButton => 'Calibrate printer';

  @override
  String get calibrationTitle => 'Calibrate printer';

  @override
  String get calibrationIntro =>
      'Correct the print position of a normal (sheet) printer. Print the calibration sheet at 100 % (no fit-to-page), measure the three crosses with a ruler and enter the values. The correction is saved with this preset and applied to every sheet print.';

  @override
  String get calibrationPrintSheet => 'Print calibration sheet';

  @override
  String get calibrationMeasureHeading => 'Measured positions';

  @override
  String get calibrationMeasureHint =>
      'Measure the distance from the top/left paper edge to the centre of each cross, in millimetres.';

  @override
  String get calibrationMeasureAX => 'Cross A – X';

  @override
  String get calibrationMeasureAY => 'Cross A – Y';

  @override
  String get calibrationMeasureBX => 'Cross B – X';

  @override
  String get calibrationMeasureCY => 'Cross C – Y';

  @override
  String get calibrationCompute => 'Compute correction';

  @override
  String get calibrationResultHeading => 'Computed correction';

  @override
  String get calibrationOffsetX => 'Offset X';

  @override
  String get calibrationOffsetY => 'Offset Y';

  @override
  String get calibrationScaleX => 'Scale X';

  @override
  String get calibrationScaleY => 'Scale Y';

  @override
  String get calibrationReset => 'Reset';

  @override
  String get calibrationSave => 'Save';

  @override
  String get calibrationSaved => 'Calibration saved.';
}
