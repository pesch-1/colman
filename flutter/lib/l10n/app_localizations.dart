// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:intl/intl.dart' as intl;

import 'app_localizations_de.dart';
import 'app_localizations_en.dart';

// ignore_for_file: type=lint

/// Callers can lookup localized strings with an instance of AppLocalizations
/// returned by `AppLocalizations.of(context)`.
///
/// Applications need to include `AppLocalizations.delegate()` in their app's
/// `localizationDelegates` list, and the locales they support in the app's
/// `supportedLocales` list. For example:
///
/// ```dart
/// import 'l10n/app_localizations.dart';
///
/// return MaterialApp(
///   localizationsDelegates: AppLocalizations.localizationsDelegates,
///   supportedLocales: AppLocalizations.supportedLocales,
///   home: MyApplicationHome(),
/// );
/// ```
///
/// ## Update pubspec.yaml
///
/// Please make sure to update your pubspec.yaml to include the following
/// packages:
///
/// ```yaml
/// dependencies:
///   # Internationalization support.
///   flutter_localizations:
///     sdk: flutter
///   intl: any # Use the pinned version from flutter_localizations
///
///   # Rest of dependencies
/// ```
///
/// ## iOS Applications
///
/// iOS applications define key application metadata, including supported
/// locales, in an Info.plist file that is built into the application bundle.
/// To configure the locales supported by your app, you’ll need to edit this
/// file.
///
/// First, open your project’s ios/Runner.xcworkspace Xcode workspace file.
/// Then, in the Project Navigator, open the Info.plist file under the Runner
/// project’s Runner folder.
///
/// Next, select the Information Property List item, select Add Item from the
/// Editor menu, then select Localizations from the pop-up menu.
///
/// Select and expand the newly-created Localizations item then, for each
/// locale your application supports, add a new item and select the locale
/// you wish to add from the pop-up menu in the Value field. This list should
/// be consistent with the languages listed in the AppLocalizations.supportedLocales
/// property.
abstract class AppLocalizations {
  AppLocalizations(String locale)
    : localeName = intl.Intl.canonicalizedLocale(locale.toString());

  final String localeName;

  static AppLocalizations? of(BuildContext context) {
    return Localizations.of<AppLocalizations>(context, AppLocalizations);
  }

  static const LocalizationsDelegate<AppLocalizations> delegate =
      _AppLocalizationsDelegate();

  /// A list of this localizations delegate along with the default localizations
  /// delegates.
  ///
  /// Returns a list of localizations delegates containing this delegate along with
  /// GlobalMaterialLocalizations.delegate, GlobalCupertinoLocalizations.delegate,
  /// and GlobalWidgetsLocalizations.delegate.
  ///
  /// Additional delegates can be added by appending to this list in
  /// MaterialApp. This list does not have to be used at all if a custom list
  /// of delegates is preferred or required.
  static const List<LocalizationsDelegate<dynamic>> localizationsDelegates =
      <LocalizationsDelegate<dynamic>>[
        delegate,
        GlobalMaterialLocalizations.delegate,
        GlobalCupertinoLocalizations.delegate,
        GlobalWidgetsLocalizations.delegate,
      ];

  /// A list of this localizations delegate's supported locales.
  static const List<Locale> supportedLocales = <Locale>[
    Locale('de'),
    Locale('en'),
  ];

  /// No description provided for @appTitle.
  ///
  /// In en, this message translates to:
  /// **'Colman'**
  String get appTitle;

  /// No description provided for @signOut.
  ///
  /// In en, this message translates to:
  /// **'Sign out'**
  String get signOut;

  /// No description provided for @navLend.
  ///
  /// In en, this message translates to:
  /// **'Lend Item'**
  String get navLend;

  /// No description provided for @navReturn.
  ///
  /// In en, this message translates to:
  /// **'Return Item'**
  String get navReturn;

  /// No description provided for @navOpenLendings.
  ///
  /// In en, this message translates to:
  /// **'Due'**
  String get navOpenLendings;

  /// No description provided for @navMyLendings.
  ///
  /// In en, this message translates to:
  /// **'Borrowed'**
  String get navMyLendings;

  /// No description provided for @navMyReservations.
  ///
  /// In en, this message translates to:
  /// **'Reserved'**
  String get navMyReservations;

  /// No description provided for @myReservationsEmpty.
  ///
  /// In en, this message translates to:
  /// **'No active reservations.'**
  String get myReservationsEmpty;

  /// No description provided for @myReservationsColExpires.
  ///
  /// In en, this message translates to:
  /// **'Expires'**
  String get myReservationsColExpires;

  /// No description provided for @openLendingsPersonLabel.
  ///
  /// In en, this message translates to:
  /// **'Person'**
  String get openLendingsPersonLabel;

  /// No description provided for @openLendingsAllClasses.
  ///
  /// In en, this message translates to:
  /// **'All classes'**
  String get openLendingsAllClasses;

  /// No description provided for @openLendingsLabelClass.
  ///
  /// In en, this message translates to:
  /// **'Class'**
  String get openLendingsLabelClass;

  /// No description provided for @openLendingsClearFilter.
  ///
  /// In en, this message translates to:
  /// **'Clear filter'**
  String get openLendingsClearFilter;

  /// No description provided for @openLendingsPrint.
  ///
  /// In en, this message translates to:
  /// **'Print'**
  String get openLendingsPrint;

  /// No description provided for @openLendingsColDueDate.
  ///
  /// In en, this message translates to:
  /// **'Due date'**
  String get openLendingsColDueDate;

  /// No description provided for @openLendingsColBorrower.
  ///
  /// In en, this message translates to:
  /// **'Borrower'**
  String get openLendingsColBorrower;

  /// No description provided for @openLendingsColCount.
  ///
  /// In en, this message translates to:
  /// **'Count'**
  String get openLendingsColCount;

  /// No description provided for @openLendingsColItem.
  ///
  /// In en, this message translates to:
  /// **'Item'**
  String get openLendingsColItem;

  /// No description provided for @navHistory.
  ///
  /// In en, this message translates to:
  /// **'View History'**
  String get navHistory;

  /// No description provided for @navPersons.
  ///
  /// In en, this message translates to:
  /// **'Borrowers'**
  String get navPersons;

  /// No description provided for @navAcquire.
  ///
  /// In en, this message translates to:
  /// **'Acquire'**
  String get navAcquire;

  /// No description provided for @navLiquidate.
  ///
  /// In en, this message translates to:
  /// **'Liquidate'**
  String get navLiquidate;

  /// No description provided for @navSearch.
  ///
  /// In en, this message translates to:
  /// **'Search'**
  String get navSearch;

  /// No description provided for @navScan.
  ///
  /// In en, this message translates to:
  /// **'Scan'**
  String get navScan;

  /// No description provided for @navClasses.
  ///
  /// In en, this message translates to:
  /// **'Classes'**
  String get navClasses;

  /// No description provided for @navCategories.
  ///
  /// In en, this message translates to:
  /// **'Categories'**
  String get navCategories;

  /// No description provided for @categoriesTitle.
  ///
  /// In en, this message translates to:
  /// **'Manage categories'**
  String get categoriesTitle;

  /// No description provided for @categoriesNoResults.
  ///
  /// In en, this message translates to:
  /// **'No categories defined.'**
  String get categoriesNoResults;

  /// No description provided for @categoriesAddTitle.
  ///
  /// In en, this message translates to:
  /// **'New category'**
  String get categoriesAddTitle;

  /// No description provided for @categoriesEditTitle.
  ///
  /// In en, this message translates to:
  /// **'Edit category'**
  String get categoriesEditTitle;

  /// No description provided for @categoriesNameLabel.
  ///
  /// In en, this message translates to:
  /// **'Category name'**
  String get categoriesNameLabel;

  /// No description provided for @categoriesSearchCategory.
  ///
  /// In en, this message translates to:
  /// **'Search category'**
  String get categoriesSearchCategory;

  /// No description provided for @categoriesFields.
  ///
  /// In en, this message translates to:
  /// **'Fields'**
  String get categoriesFields;

  /// No description provided for @categoriesNoFields.
  ///
  /// In en, this message translates to:
  /// **'No fields yet.'**
  String get categoriesNoFields;

  /// No description provided for @categoriesAddField.
  ///
  /// In en, this message translates to:
  /// **'Add field'**
  String get categoriesAddField;

  /// No description provided for @categoriesEditFieldTitle.
  ///
  /// In en, this message translates to:
  /// **'Field'**
  String get categoriesEditFieldTitle;

  /// No description provided for @categoriesFieldName.
  ///
  /// In en, this message translates to:
  /// **'Field name'**
  String get categoriesFieldName;

  /// No description provided for @categoriesFieldType.
  ///
  /// In en, this message translates to:
  /// **'Field type'**
  String get categoriesFieldType;

  /// No description provided for @categoriesFieldRepeatable.
  ///
  /// In en, this message translates to:
  /// **'Repeatable'**
  String get categoriesFieldRepeatable;

  /// No description provided for @categoriesFieldDelete.
  ///
  /// In en, this message translates to:
  /// **'Remove field'**
  String get categoriesFieldDelete;

  /// No description provided for @categoriesDeleteTitle.
  ///
  /// In en, this message translates to:
  /// **'Delete category'**
  String get categoriesDeleteTitle;

  /// No description provided for @categoriesDeleteConfirm.
  ///
  /// In en, this message translates to:
  /// **'Delete category \'{name}\'? All items of this type and their lending history will be permanently removed.'**
  String categoriesDeleteConfirm(String name);

  /// No description provided for @categoriesSaveSuccess.
  ///
  /// In en, this message translates to:
  /// **'Category saved.'**
  String get categoriesSaveSuccess;

  /// No description provided for @categoriesSaveFailed.
  ///
  /// In en, this message translates to:
  /// **'Could not save the category. The name may already be in use.'**
  String get categoriesSaveFailed;

  /// No description provided for @categoriesDeleteSuccess.
  ///
  /// In en, this message translates to:
  /// **'Category deleted.'**
  String get categoriesDeleteSuccess;

  /// No description provided for @mediaKindUnknown.
  ///
  /// In en, this message translates to:
  /// **'None'**
  String get mediaKindUnknown;

  /// No description provided for @mediaKindBook.
  ///
  /// In en, this message translates to:
  /// **'Book'**
  String get mediaKindBook;

  /// No description provided for @mediaKindCd.
  ///
  /// In en, this message translates to:
  /// **'CD'**
  String get mediaKindCd;

  /// No description provided for @mediaKindDvd.
  ///
  /// In en, this message translates to:
  /// **'DVD'**
  String get mediaKindDvd;

  /// No description provided for @mediaKindBluRay.
  ///
  /// In en, this message translates to:
  /// **'Blu-ray'**
  String get mediaKindBluRay;

  /// No description provided for @mediaKindBoardGame.
  ///
  /// In en, this message translates to:
  /// **'Board game'**
  String get mediaKindBoardGame;

  /// No description provided for @dataTypeString.
  ///
  /// In en, this message translates to:
  /// **'Text (multi-line)'**
  String get dataTypeString;

  /// No description provided for @dataTypeShortString.
  ///
  /// In en, this message translates to:
  /// **'Text'**
  String get dataTypeShortString;

  /// No description provided for @dataTypeDate.
  ///
  /// In en, this message translates to:
  /// **'Date'**
  String get dataTypeDate;

  /// No description provided for @dataTypeTime.
  ///
  /// In en, this message translates to:
  /// **'Time'**
  String get dataTypeTime;

  /// No description provided for @dataTypeInteger.
  ///
  /// In en, this message translates to:
  /// **'Integer'**
  String get dataTypeInteger;

  /// No description provided for @dataTypeFloat.
  ///
  /// In en, this message translates to:
  /// **'Decimal'**
  String get dataTypeFloat;

  /// No description provided for @dataTypeDuration.
  ///
  /// In en, this message translates to:
  /// **'Duration'**
  String get dataTypeDuration;

  /// No description provided for @dataTypeTable.
  ///
  /// In en, this message translates to:
  /// **'List'**
  String get dataTypeTable;

  /// No description provided for @dataTypeIntegerRanges.
  ///
  /// In en, this message translates to:
  /// **'Integer ranges'**
  String get dataTypeIntegerRanges;

  /// No description provided for @navSettings.
  ///
  /// In en, this message translates to:
  /// **'Settings'**
  String get navSettings;

  /// No description provided for @loginSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Library Management'**
  String get loginSubtitle;

  /// No description provided for @loginSearching.
  ///
  /// In en, this message translates to:
  /// **'Searching...'**
  String get loginSearching;

  /// No description provided for @loginNoServers.
  ///
  /// In en, this message translates to:
  /// **'No servers found'**
  String get loginNoServers;

  /// No description provided for @loginDiscoveryUnavailable.
  ///
  /// In en, this message translates to:
  /// **'Discovery not available'**
  String get loginDiscoveryUnavailable;

  /// No description provided for @loginDiscoverOnLan.
  ///
  /// In en, this message translates to:
  /// **'Discover on LAN'**
  String get loginDiscoverOnLan;

  /// No description provided for @loginRefresh.
  ///
  /// In en, this message translates to:
  /// **'Refresh'**
  String get loginRefresh;

  /// No description provided for @loginEnterUrlManually.
  ///
  /// In en, this message translates to:
  /// **'Enter URL manually'**
  String get loginEnterUrlManually;

  /// No description provided for @loginBackToDiscovery.
  ///
  /// In en, this message translates to:
  /// **'Back to discovery'**
  String get loginBackToDiscovery;

  /// No description provided for @loginServerUrl.
  ///
  /// In en, this message translates to:
  /// **'Server URL'**
  String get loginServerUrl;

  /// No description provided for @loginServerUrlHint.
  ///
  /// In en, this message translates to:
  /// **'https://colman.example.com'**
  String get loginServerUrlHint;

  /// No description provided for @loginUsername.
  ///
  /// In en, this message translates to:
  /// **'Username'**
  String get loginUsername;

  /// No description provided for @loginPassword.
  ///
  /// In en, this message translates to:
  /// **'Password'**
  String get loginPassword;

  /// No description provided for @loginRequired.
  ///
  /// In en, this message translates to:
  /// **'Required'**
  String get loginRequired;

  /// No description provided for @loginSignIn.
  ///
  /// In en, this message translates to:
  /// **'Sign in'**
  String get loginSignIn;

  /// No description provided for @loginFailed.
  ///
  /// In en, this message translates to:
  /// **'Login failed: {error}'**
  String loginFailed(String error);

  /// No description provided for @itemsTitle.
  ///
  /// In en, this message translates to:
  /// **'Search Items'**
  String get itemsTitle;

  /// No description provided for @itemsSearchHint.
  ///
  /// In en, this message translates to:
  /// **'Search by name, keyword…'**
  String get itemsSearchHint;

  /// No description provided for @itemsEmptyPrompt.
  ///
  /// In en, this message translates to:
  /// **'Enter a search term above.'**
  String get itemsEmptyPrompt;

  /// No description provided for @itemsNotFound.
  ///
  /// In en, this message translates to:
  /// **'No items found.'**
  String get itemsNotFound;

  /// No description provided for @errorPrefix.
  ///
  /// In en, this message translates to:
  /// **'Error: {error}'**
  String errorPrefix(String error);

  /// No description provided for @errorDetails.
  ///
  /// In en, this message translates to:
  /// **'Details'**
  String get errorDetails;

  /// No description provided for @errorConnectionFailed.
  ///
  /// In en, this message translates to:
  /// **'Could not connect to the server.'**
  String get errorConnectionFailed;

  /// No description provided for @errorAccessDenied.
  ///
  /// In en, this message translates to:
  /// **'You don\'t have permission for this action.'**
  String get errorAccessDenied;

  /// No description provided for @errorInvalidCredentials.
  ///
  /// In en, this message translates to:
  /// **'Incorrect username or password.'**
  String get errorInvalidCredentials;

  /// No description provided for @errorServerError.
  ///
  /// In en, this message translates to:
  /// **'The server reported an error.'**
  String get errorServerError;

  /// No description provided for @errorRequestFailed.
  ///
  /// In en, this message translates to:
  /// **'The request failed.'**
  String get errorRequestFailed;

  /// No description provided for @errorIncompatibleServer.
  ///
  /// In en, this message translates to:
  /// **'This server is not compatible with this app (server {serverVersion}, app requires {requiredVersion}). Please update the app or the server.'**
  String errorIncompatibleServer(String serverVersion, String requiredVersion);

  /// No description provided for @retry.
  ///
  /// In en, this message translates to:
  /// **'Retry'**
  String get retry;

  /// No description provided for @itemDetailFallbackTitle.
  ///
  /// In en, this message translates to:
  /// **'Item'**
  String get itemDetailFallbackTitle;

  /// No description provided for @labelName.
  ///
  /// In en, this message translates to:
  /// **'Name'**
  String get labelName;

  /// No description provided for @labelType.
  ///
  /// In en, this message translates to:
  /// **'Type'**
  String get labelType;

  /// No description provided for @labelPlace.
  ///
  /// In en, this message translates to:
  /// **'Place'**
  String get labelPlace;

  /// No description provided for @labelBarcode.
  ///
  /// In en, this message translates to:
  /// **'Colman barcode'**
  String get labelBarcode;

  /// No description provided for @labelDescription.
  ///
  /// In en, this message translates to:
  /// **'Description'**
  String get labelDescription;

  /// No description provided for @labelAvailability.
  ///
  /// In en, this message translates to:
  /// **'Availability'**
  String get labelAvailability;

  /// No description provided for @availabilityOf.
  ///
  /// In en, this message translates to:
  /// **'{available} of {count} available'**
  String availabilityOf(int available, int count);

  /// No description provided for @lendingTitle.
  ///
  /// In en, this message translates to:
  /// **'Lending'**
  String get lendingTitle;

  /// No description provided for @lendingComingSoon.
  ///
  /// In en, this message translates to:
  /// **'Lending — coming soon'**
  String get lendingComingSoon;

  /// No description provided for @lendingTabNew.
  ///
  /// In en, this message translates to:
  /// **'New'**
  String get lendingTabNew;

  /// No description provided for @lendingTabOpen.
  ///
  /// In en, this message translates to:
  /// **'Open'**
  String get lendingTabOpen;

  /// No description provided for @lendingTabDue.
  ///
  /// In en, this message translates to:
  /// **'Due'**
  String get lendingTabDue;

  /// No description provided for @lendingStepPerson.
  ///
  /// In en, this message translates to:
  /// **'Person'**
  String get lendingStepPerson;

  /// No description provided for @lendingStepItem.
  ///
  /// In en, this message translates to:
  /// **'Item'**
  String get lendingStepItem;

  /// No description provided for @lendingStepDetails.
  ///
  /// In en, this message translates to:
  /// **'Details'**
  String get lendingStepDetails;

  /// No description provided for @lendingStep1Title.
  ///
  /// In en, this message translates to:
  /// **'Select borrower'**
  String get lendingStep1Title;

  /// No description provided for @lendingStep2Title.
  ///
  /// In en, this message translates to:
  /// **'Select item'**
  String get lendingStep2Title;

  /// No description provided for @lendingStep3Title.
  ///
  /// In en, this message translates to:
  /// **'Lending details'**
  String get lendingStep3Title;

  /// No description provided for @lendingItemSearchHint.
  ///
  /// In en, this message translates to:
  /// **'Search by name…'**
  String get lendingItemSearchHint;

  /// No description provided for @lendingItemNotFound.
  ///
  /// In en, this message translates to:
  /// **'No items found.'**
  String get lendingItemNotFound;

  /// No description provided for @lendingItemSearchPrompt.
  ///
  /// In en, this message translates to:
  /// **'Enter item name to search.'**
  String get lendingItemSearchPrompt;

  /// No description provided for @lendingSuccessLendId.
  ///
  /// In en, this message translates to:
  /// **'Lending #{id} created'**
  String lendingSuccessLendId(int id);

  /// No description provided for @lendingSuccessReserveId.
  ///
  /// In en, this message translates to:
  /// **'Reservation #{id} created'**
  String lendingSuccessReserveId(int id);

  /// No description provided for @lendingNewLending.
  ///
  /// In en, this message translates to:
  /// **'New lending'**
  String get lendingNewLending;

  /// No description provided for @lendingNewItemSamePerson.
  ///
  /// In en, this message translates to:
  /// **'New item, same person'**
  String get lendingNewItemSamePerson;

  /// No description provided for @lendingConflictReserved.
  ///
  /// In en, this message translates to:
  /// **'Only {count} unreserved copy/copies available'**
  String lendingConflictReserved(int count);

  /// No description provided for @lendingOpenNoResults.
  ///
  /// In en, this message translates to:
  /// **'No open lendings.'**
  String get lendingOpenNoResults;

  /// No description provided for @lendingOpenLentReturned.
  ///
  /// In en, this message translates to:
  /// **'{lent} lent, {returned} returned'**
  String lendingOpenLentReturned(int lent, int returned);

  /// No description provided for @lendingDueNoResults.
  ///
  /// In en, this message translates to:
  /// **'No due lendings.'**
  String get lendingDueNoResults;

  /// No description provided for @lendingDueOutstanding.
  ///
  /// In en, this message translates to:
  /// **'{count} outstanding'**
  String lendingDueOutstanding(int count);

  /// No description provided for @returnTitle.
  ///
  /// In en, this message translates to:
  /// **'Return'**
  String get returnTitle;

  /// No description provided for @returnComingSoon.
  ///
  /// In en, this message translates to:
  /// **'Return — coming soon'**
  String get returnComingSoon;

  /// No description provided for @returnTabNew.
  ///
  /// In en, this message translates to:
  /// **'New'**
  String get returnTabNew;

  /// No description provided for @returnTabOpen.
  ///
  /// In en, this message translates to:
  /// **'Open'**
  String get returnTabOpen;

  /// No description provided for @returnStepLending.
  ///
  /// In en, this message translates to:
  /// **'Lending'**
  String get returnStepLending;

  /// No description provided for @returnStepDetails.
  ///
  /// In en, this message translates to:
  /// **'Details'**
  String get returnStepDetails;

  /// No description provided for @returnStep1Title.
  ///
  /// In en, this message translates to:
  /// **'Select item'**
  String get returnStep1Title;

  /// No description provided for @returnStep2Title.
  ///
  /// In en, this message translates to:
  /// **'Select lending'**
  String get returnStep2Title;

  /// No description provided for @returnStep3Title.
  ///
  /// In en, this message translates to:
  /// **'Return details'**
  String get returnStep3Title;

  /// No description provided for @returnStep2NoResults.
  ///
  /// In en, this message translates to:
  /// **'No open lendings for this item.'**
  String get returnStep2NoResults;

  /// No description provided for @returnOpenNoResults.
  ///
  /// In en, this message translates to:
  /// **'No open lendings.'**
  String get returnOpenNoResults;

  /// No description provided for @returnSuccessId.
  ///
  /// In en, this message translates to:
  /// **'Return #{id} recorded'**
  String returnSuccessId(int id);

  /// No description provided for @returnNewReturn.
  ///
  /// In en, this message translates to:
  /// **'New return'**
  String get returnNewReturn;

  /// No description provided for @returnSameItem.
  ///
  /// In en, this message translates to:
  /// **'Same item'**
  String get returnSameItem;

  /// No description provided for @acquisitionTitle.
  ///
  /// In en, this message translates to:
  /// **'Acquisition'**
  String get acquisitionTitle;

  /// No description provided for @acquisitionComingSoon.
  ///
  /// In en, this message translates to:
  /// **'Acquisition — coming soon'**
  String get acquisitionComingSoon;

  /// No description provided for @acquisitionStep1Title.
  ///
  /// In en, this message translates to:
  /// **'Select item'**
  String get acquisitionStep1Title;

  /// No description provided for @acquisitionStep2Title.
  ///
  /// In en, this message translates to:
  /// **'Acquisition details'**
  String get acquisitionStep2Title;

  /// No description provided for @acquisitionSuccessId.
  ///
  /// In en, this message translates to:
  /// **'Acquisition #{id} recorded'**
  String acquisitionSuccessId(int id);

  /// No description provided for @acquisitionNewAcquisition.
  ///
  /// In en, this message translates to:
  /// **'New acquisition'**
  String get acquisitionNewAcquisition;

  /// No description provided for @acquisitionCreateItemTitle.
  ///
  /// In en, this message translates to:
  /// **'New item'**
  String get acquisitionCreateItemTitle;

  /// No description provided for @acquisitionCreateItemButton.
  ///
  /// In en, this message translates to:
  /// **'Create new item'**
  String get acquisitionCreateItemButton;

  /// No description provided for @acquisitionBarcodeHint.
  ///
  /// In en, this message translates to:
  /// **'Scan or enter barcode'**
  String get acquisitionBarcodeHint;

  /// No description provided for @acquisitionBarcodeSearchingInternet.
  ///
  /// In en, this message translates to:
  /// **'Searching internet…'**
  String get acquisitionBarcodeSearchingInternet;

  /// No description provided for @acquisitionBarcodeNotFound.
  ///
  /// In en, this message translates to:
  /// **'Barcode not found'**
  String get acquisitionBarcodeNotFound;

  /// No description provided for @barcodeScanTitle.
  ///
  /// In en, this message translates to:
  /// **'Scan Barcode'**
  String get barcodeScanTitle;

  /// No description provided for @barcodeScanHint.
  ///
  /// In en, this message translates to:
  /// **'Hold camera over barcode'**
  String get barcodeScanHint;

  /// No description provided for @barcodeScanTooltip.
  ///
  /// In en, this message translates to:
  /// **'Scan with camera'**
  String get barcodeScanTooltip;

  /// No description provided for @liquidationTitle.
  ///
  /// In en, this message translates to:
  /// **'Liquidation'**
  String get liquidationTitle;

  /// No description provided for @liquidationComingSoon.
  ///
  /// In en, this message translates to:
  /// **'Liquidation — coming soon'**
  String get liquidationComingSoon;

  /// No description provided for @liquidationStep1Title.
  ///
  /// In en, this message translates to:
  /// **'Select item'**
  String get liquidationStep1Title;

  /// No description provided for @liquidationStep2Title.
  ///
  /// In en, this message translates to:
  /// **'Liquidation details'**
  String get liquidationStep2Title;

  /// No description provided for @liquidationSuccessId.
  ///
  /// In en, this message translates to:
  /// **'Liquidation #{id} recorded'**
  String liquidationSuccessId(int id);

  /// No description provided for @liquidationNewLiquidation.
  ///
  /// In en, this message translates to:
  /// **'New liquidation'**
  String get liquidationNewLiquidation;

  /// No description provided for @historyTitle.
  ///
  /// In en, this message translates to:
  /// **'History'**
  String get historyTitle;

  /// No description provided for @historyComingSoon.
  ///
  /// In en, this message translates to:
  /// **'History — coming soon'**
  String get historyComingSoon;

  /// No description provided for @personsTitle.
  ///
  /// In en, this message translates to:
  /// **'Persons'**
  String get personsTitle;

  /// No description provided for @personsComingSoon.
  ///
  /// In en, this message translates to:
  /// **'Persons — coming soon'**
  String get personsComingSoon;

  /// No description provided for @classesTitle.
  ///
  /// In en, this message translates to:
  /// **'Classes'**
  String get classesTitle;

  /// No description provided for @classesComingSoon.
  ///
  /// In en, this message translates to:
  /// **'Classes — coming soon'**
  String get classesComingSoon;

  /// No description provided for @classesNoResults.
  ///
  /// In en, this message translates to:
  /// **'No classes defined.'**
  String get classesNoResults;

  /// No description provided for @classesAddTooltip.
  ///
  /// In en, this message translates to:
  /// **'New class'**
  String get classesAddTooltip;

  /// No description provided for @classesAddTitle.
  ///
  /// In en, this message translates to:
  /// **'New class'**
  String get classesAddTitle;

  /// No description provided for @classesAddNameLabel.
  ///
  /// In en, this message translates to:
  /// **'Class name'**
  String get classesAddNameLabel;

  /// No description provided for @classesRenameTitle.
  ///
  /// In en, this message translates to:
  /// **'Rename class'**
  String get classesRenameTitle;

  /// No description provided for @classesRenameNewNameLabel.
  ///
  /// In en, this message translates to:
  /// **'New name'**
  String get classesRenameNewNameLabel;

  /// No description provided for @classesDeleteConfirm.
  ///
  /// In en, this message translates to:
  /// **'Delete class \'{name}\'?'**
  String classesDeleteConfirm(String name);

  /// No description provided for @classesGradeRange.
  ///
  /// In en, this message translates to:
  /// **'Grade range'**
  String get classesGradeRange;

  /// No description provided for @classesGradeRangeMin.
  ///
  /// In en, this message translates to:
  /// **'Min'**
  String get classesGradeRangeMin;

  /// No description provided for @classesGradeRangeMax.
  ///
  /// In en, this message translates to:
  /// **'Max'**
  String get classesGradeRangeMax;

  /// No description provided for @classesNewSchoolYear.
  ///
  /// In en, this message translates to:
  /// **'New school year'**
  String get classesNewSchoolYear;

  /// No description provided for @classesNewSchoolYearStep1.
  ///
  /// In en, this message translates to:
  /// **'Class name changes'**
  String get classesNewSchoolYearStep1;

  /// No description provided for @classesNewSchoolYearStep2.
  ///
  /// In en, this message translates to:
  /// **'Affected persons'**
  String get classesNewSchoolYearStep2;

  /// No description provided for @classesNewSchoolYearDissolved.
  ///
  /// In en, this message translates to:
  /// **'(dissolved)'**
  String get classesNewSchoolYearDissolved;

  /// No description provided for @classesNewSchoolYearAddClass.
  ///
  /// In en, this message translates to:
  /// **'Add class'**
  String get classesNewSchoolYearAddClass;

  /// No description provided for @classesNewSchoolYearNoPersons.
  ///
  /// In en, this message translates to:
  /// **'No persons affected.'**
  String get classesNewSchoolYearNoPersons;

  /// No description provided for @classesNewSchoolYearNoClass.
  ///
  /// In en, this message translates to:
  /// **'(no class)'**
  String get classesNewSchoolYearNoClass;

  /// No description provided for @classesNewSchoolYearDeletePerson.
  ///
  /// In en, this message translates to:
  /// **'Delete person'**
  String get classesNewSchoolYearDeletePerson;

  /// No description provided for @classesApply.
  ///
  /// In en, this message translates to:
  /// **'Apply'**
  String get classesApply;

  /// No description provided for @classesNext.
  ///
  /// In en, this message translates to:
  /// **'Next'**
  String get classesNext;

  /// No description provided for @settingsTitle.
  ///
  /// In en, this message translates to:
  /// **'Settings'**
  String get settingsTitle;

  /// No description provided for @settingsComingSoon.
  ///
  /// In en, this message translates to:
  /// **'Settings — coming soon'**
  String get settingsComingSoon;

  /// No description provided for @settingsInternetServicesSection.
  ///
  /// In en, this message translates to:
  /// **'Internet services'**
  String get settingsInternetServicesSection;

  /// No description provided for @settingsNetworkSection.
  ///
  /// In en, this message translates to:
  /// **'Network'**
  String get settingsNetworkSection;

  /// No description provided for @settingsSruServersSection.
  ///
  /// In en, this message translates to:
  /// **'SRU library servers'**
  String get settingsSruServersSection;

  /// No description provided for @settingsOpenLibrary.
  ///
  /// In en, this message translates to:
  /// **'Open Library'**
  String get settingsOpenLibrary;

  /// No description provided for @settingsGoogleBooks.
  ///
  /// In en, this message translates to:
  /// **'Google Books'**
  String get settingsGoogleBooks;

  /// No description provided for @settingsGoogleApisKey.
  ///
  /// In en, this message translates to:
  /// **'API key'**
  String get settingsGoogleApisKey;

  /// No description provided for @settingsMusicBrainz.
  ///
  /// In en, this message translates to:
  /// **'MusicBrainz'**
  String get settingsMusicBrainz;

  /// No description provided for @settingsBgg.
  ///
  /// In en, this message translates to:
  /// **'BoardGameGeek'**
  String get settingsBgg;

  /// No description provided for @settingsBggToken.
  ///
  /// In en, this message translates to:
  /// **'BGG token'**
  String get settingsBggToken;

  /// No description provided for @settingsOpenGtindb.
  ///
  /// In en, this message translates to:
  /// **'OpenGTINdb'**
  String get settingsOpenGtindb;

  /// No description provided for @settingsOpenGtindbUserId.
  ///
  /// In en, this message translates to:
  /// **'User ID'**
  String get settingsOpenGtindbUserId;

  /// No description provided for @settingsDownloadThumbnails.
  ///
  /// In en, this message translates to:
  /// **'Download cover images'**
  String get settingsDownloadThumbnails;

  /// No description provided for @settingsTimeoutSeconds.
  ///
  /// In en, this message translates to:
  /// **'Timeout (s)'**
  String get settingsTimeoutSeconds;

  /// No description provided for @settingsPollIntervalSeconds.
  ///
  /// In en, this message translates to:
  /// **'Refresh interval (s)'**
  String get settingsPollIntervalSeconds;

  /// No description provided for @settingsPollIntervalHint.
  ///
  /// In en, this message translates to:
  /// **'How often the app checks the server for changes (5–300 s).'**
  String get settingsPollIntervalHint;

  /// No description provided for @settingsSruName.
  ///
  /// In en, this message translates to:
  /// **'Name'**
  String get settingsSruName;

  /// No description provided for @settingsSruEndpoint.
  ///
  /// In en, this message translates to:
  /// **'Endpoint'**
  String get settingsSruEndpoint;

  /// No description provided for @settingsSruIsbnField.
  ///
  /// In en, this message translates to:
  /// **'ISBN field'**
  String get settingsSruIsbnField;

  /// No description provided for @settingsSruEanField.
  ///
  /// In en, this message translates to:
  /// **'EAN field'**
  String get settingsSruEanField;

  /// No description provided for @settingsSruSchema.
  ///
  /// In en, this message translates to:
  /// **'Schema'**
  String get settingsSruSchema;

  /// No description provided for @settingsSruAddServer.
  ///
  /// In en, this message translates to:
  /// **'Add server'**
  String get settingsSruAddServer;

  /// No description provided for @settingsSruDeleteServer.
  ///
  /// In en, this message translates to:
  /// **'Delete server'**
  String get settingsSruDeleteServer;

  /// No description provided for @settingsSruEditServer.
  ///
  /// In en, this message translates to:
  /// **'Edit server'**
  String get settingsSruEditServer;

  /// No description provided for @settingsSave.
  ///
  /// In en, this message translates to:
  /// **'Save'**
  String get settingsSave;

  /// No description provided for @settingsRevert.
  ///
  /// In en, this message translates to:
  /// **'Revert'**
  String get settingsRevert;

  /// No description provided for @settingsSaved.
  ///
  /// In en, this message translates to:
  /// **'Settings saved'**
  String get settingsSaved;

  /// No description provided for @serverUnreachableBanner.
  ///
  /// In en, this message translates to:
  /// **'Server unreachable — the data shown may be out of date.'**
  String get serverUnreachableBanner;

  /// No description provided for @dismiss.
  ///
  /// In en, this message translates to:
  /// **'Dismiss'**
  String get dismiss;

  /// No description provided for @personDetailFallbackTitle.
  ///
  /// In en, this message translates to:
  /// **'Person'**
  String get personDetailFallbackTitle;

  /// No description provided for @personDetailComingSoon.
  ///
  /// In en, this message translates to:
  /// **'Person {id} — coming soon'**
  String personDetailComingSoon(int id);

  /// No description provided for @labelSignatureTop.
  ///
  /// In en, this message translates to:
  /// **'Signature (top)'**
  String get labelSignatureTop;

  /// No description provided for @labelSignatureBottom.
  ///
  /// In en, this message translates to:
  /// **'Signature (bottom)'**
  String get labelSignatureBottom;

  /// No description provided for @labelKeywords.
  ///
  /// In en, this message translates to:
  /// **'Keywords'**
  String get labelKeywords;

  /// No description provided for @labelSubject.
  ///
  /// In en, this message translates to:
  /// **'Subject'**
  String get labelSubject;

  /// No description provided for @labelAcademicYear.
  ///
  /// In en, this message translates to:
  /// **'Academic year'**
  String get labelAcademicYear;

  /// No description provided for @itemsAdvancedSearch.
  ///
  /// In en, this message translates to:
  /// **'Advanced search'**
  String get itemsAdvancedSearch;

  /// No description provided for @labelAllTypes.
  ///
  /// In en, this message translates to:
  /// **'All types'**
  String get labelAllTypes;

  /// No description provided for @labelCount.
  ///
  /// In en, this message translates to:
  /// **'Count'**
  String get labelCount;

  /// No description provided for @labelExpiryDate.
  ///
  /// In en, this message translates to:
  /// **'Expires on'**
  String get labelExpiryDate;

  /// No description provided for @labelDueDate.
  ///
  /// In en, this message translates to:
  /// **'Due date'**
  String get labelDueDate;

  /// No description provided for @labelPerson.
  ///
  /// In en, this message translates to:
  /// **'Person'**
  String get labelPerson;

  /// No description provided for @labelPersonSearch.
  ///
  /// In en, this message translates to:
  /// **'Search person…'**
  String get labelPersonSearch;

  /// No description provided for @actionReserve.
  ///
  /// In en, this message translates to:
  /// **'Reserve'**
  String get actionReserve;

  /// No description provided for @actionLend.
  ///
  /// In en, this message translates to:
  /// **'Lend'**
  String get actionLend;

  /// No description provided for @actionCancelReservation.
  ///
  /// In en, this message translates to:
  /// **'Cancel reservation'**
  String get actionCancelReservation;

  /// No description provided for @actionEditItem.
  ///
  /// In en, this message translates to:
  /// **'Edit item'**
  String get actionEditItem;

  /// No description provided for @actionDeleteItem.
  ///
  /// In en, this message translates to:
  /// **'Delete item'**
  String get actionDeleteItem;

  /// No description provided for @deleteItemConfirmMessage.
  ///
  /// In en, this message translates to:
  /// **'Permanently delete \"{name}\" and all its past acquisitions, liquidations, lendings and returns?\n\nThis cannot be undone.'**
  String deleteItemConfirmMessage(String name);

  /// No description provided for @itemDeleted.
  ///
  /// In en, this message translates to:
  /// **'Item deleted.'**
  String get itemDeleted;

  /// No description provided for @itemsScanNotFound.
  ///
  /// In en, this message translates to:
  /// **'No item found for this barcode'**
  String get itemsScanNotFound;

  /// No description provided for @itemEditKeywordsHint.
  ///
  /// In en, this message translates to:
  /// **'comma-separated'**
  String get itemEditKeywordsHint;

  /// No description provided for @itemSaved.
  ///
  /// In en, this message translates to:
  /// **'Item saved.'**
  String get itemSaved;

  /// No description provided for @reserveDialogTitle.
  ///
  /// In en, this message translates to:
  /// **'Reserve item'**
  String get reserveDialogTitle;

  /// No description provided for @reserveSelfHint.
  ///
  /// In en, this message translates to:
  /// **'The item will be reserved for your own account.'**
  String get reserveSelfHint;

  /// No description provided for @lendDialogTitle.
  ///
  /// In en, this message translates to:
  /// **'Lend item'**
  String get lendDialogTitle;

  /// No description provided for @cancelReservationDialogTitle.
  ///
  /// In en, this message translates to:
  /// **'Cancel reservation'**
  String get cancelReservationDialogTitle;

  /// No description provided for @actionConfirm.
  ///
  /// In en, this message translates to:
  /// **'Confirm'**
  String get actionConfirm;

  /// No description provided for @reservationCreated.
  ///
  /// In en, this message translates to:
  /// **'Reservation created'**
  String get reservationCreated;

  /// No description provided for @lendingCreated.
  ///
  /// In en, this message translates to:
  /// **'Lending created'**
  String get lendingCreated;

  /// No description provided for @reservationCancelled.
  ///
  /// In en, this message translates to:
  /// **'Reservation cancelled'**
  String get reservationCancelled;

  /// No description provided for @errorAlreadyReserved.
  ///
  /// In en, this message translates to:
  /// **'A reservation already exists for this person'**
  String get errorAlreadyReserved;

  /// No description provided for @errorNotEnoughAvailable.
  ///
  /// In en, this message translates to:
  /// **'Not enough copies available'**
  String get errorNotEnoughAvailable;

  /// No description provided for @errorReservationsDisabled.
  ///
  /// In en, this message translates to:
  /// **'Reservations are disabled for this item'**
  String get errorReservationsDisabled;

  /// No description provided for @noActiveReservations.
  ///
  /// In en, this message translates to:
  /// **'No active reservations'**
  String get noActiveReservations;

  /// No description provided for @selectReservationToCancel.
  ///
  /// In en, this message translates to:
  /// **'Select reservation to cancel'**
  String get selectReservationToCancel;

  /// No description provided for @historyFromDate.
  ///
  /// In en, this message translates to:
  /// **'From'**
  String get historyFromDate;

  /// No description provided for @historyToDate.
  ///
  /// In en, this message translates to:
  /// **'To'**
  String get historyToDate;

  /// No description provided for @historyApply.
  ///
  /// In en, this message translates to:
  /// **'Apply'**
  String get historyApply;

  /// No description provided for @historyShowAll.
  ///
  /// In en, this message translates to:
  /// **'Show all'**
  String get historyShowAll;

  /// No description provided for @historyFilterAllActions.
  ///
  /// In en, this message translates to:
  /// **'All actions'**
  String get historyFilterAllActions;

  /// No description provided for @extendDueDateButton.
  ///
  /// In en, this message translates to:
  /// **'Extend due date'**
  String get extendDueDateButton;

  /// No description provided for @extendDueDateTitle.
  ///
  /// In en, this message translates to:
  /// **'Extend due date'**
  String get extendDueDateTitle;

  /// No description provided for @extendDueDateAction.
  ///
  /// In en, this message translates to:
  /// **'Extend'**
  String get extendDueDateAction;

  /// No description provided for @extendDueDateNoLendings.
  ///
  /// In en, this message translates to:
  /// **'This person has no open lendings.'**
  String get extendDueDateNoLendings;

  /// No description provided for @extendDueDateSelectHint.
  ///
  /// In en, this message translates to:
  /// **'Select the lendings whose due date shall be changed:'**
  String get extendDueDateSelectHint;

  /// No description provided for @extendDueDateNewDate.
  ///
  /// In en, this message translates to:
  /// **'New due date'**
  String get extendDueDateNewDate;

  /// No description provided for @extendDueDateFixedHint.
  ///
  /// In en, this message translates to:
  /// **'{count, plural, =1{1 lending will be extended:} other{{count} lendings will be extended:}}'**
  String extendDueDateFixedHint(int count);

  /// No description provided for @openLendingsSelectedCount.
  ///
  /// In en, this message translates to:
  /// **'{count} selected'**
  String openLendingsSelectedCount(int count);

  /// No description provided for @openLendingsSelectAll.
  ///
  /// In en, this message translates to:
  /// **'Select all'**
  String get openLendingsSelectAll;

  /// No description provided for @openLendingsDeselectAll.
  ///
  /// In en, this message translates to:
  /// **'Deselect all'**
  String get openLendingsDeselectAll;

  /// No description provided for @openLendingsSelectHint.
  ///
  /// In en, this message translates to:
  /// **'Select the lendings whose due date shall be extended.'**
  String get openLendingsSelectHint;

  /// No description provided for @openLendingsExtendSelected.
  ///
  /// In en, this message translates to:
  /// **'Extend ({count})'**
  String openLendingsExtendSelected(int count);

  /// No description provided for @historyActionsShown.
  ///
  /// In en, this message translates to:
  /// **'{count} action(s) shown'**
  String historyActionsShown(int count);

  /// No description provided for @historyLoadMore.
  ///
  /// In en, this message translates to:
  /// **'Load more'**
  String get historyLoadMore;

  /// No description provided for @historyNoResults.
  ///
  /// In en, this message translates to:
  /// **'No history entries found.'**
  String get historyNoResults;

  /// No description provided for @historyActionAcquisition.
  ///
  /// In en, this message translates to:
  /// **'acquisition'**
  String get historyActionAcquisition;

  /// No description provided for @historyActionLiquidation.
  ///
  /// In en, this message translates to:
  /// **'liquidation'**
  String get historyActionLiquidation;

  /// No description provided for @historyActionLending.
  ///
  /// In en, this message translates to:
  /// **'lending'**
  String get historyActionLending;

  /// No description provided for @historyActionReturn.
  ///
  /// In en, this message translates to:
  /// **'return'**
  String get historyActionReturn;

  /// No description provided for @historyActionReservation.
  ///
  /// In en, this message translates to:
  /// **'reservation'**
  String get historyActionReservation;

  /// No description provided for @historyLabelDateTime.
  ///
  /// In en, this message translates to:
  /// **'Date/time'**
  String get historyLabelDateTime;

  /// No description provided for @historyLabelAction.
  ///
  /// In en, this message translates to:
  /// **'Action'**
  String get historyLabelAction;

  /// No description provided for @historyLabelItem.
  ///
  /// In en, this message translates to:
  /// **'Item'**
  String get historyLabelItem;

  /// No description provided for @historyLabelUser.
  ///
  /// In en, this message translates to:
  /// **'Person'**
  String get historyLabelUser;

  /// No description provided for @historyLabelCollector.
  ///
  /// In en, this message translates to:
  /// **'Collector'**
  String get historyLabelCollector;

  /// No description provided for @historyLabelRemarks.
  ///
  /// In en, this message translates to:
  /// **'Remarks'**
  String get historyLabelRemarks;

  /// No description provided for @historyDetailTitle.
  ///
  /// In en, this message translates to:
  /// **'Action details'**
  String get historyDetailTitle;

  /// No description provided for @historyDeleteTitle.
  ///
  /// In en, this message translates to:
  /// **'Delete action'**
  String get historyDeleteTitle;

  /// No description provided for @historyDeleteConfirm.
  ///
  /// In en, this message translates to:
  /// **'Delete this action permanently?'**
  String get historyDeleteConfirm;

  /// No description provided for @historyDeleteWithConflicts.
  ///
  /// In en, this message translates to:
  /// **'Deleting this action requires also deleting {n} dependent action(s):'**
  String historyDeleteWithConflicts(int n);

  /// No description provided for @historyDeleteAll.
  ///
  /// In en, this message translates to:
  /// **'Delete all'**
  String get historyDeleteAll;

  /// No description provided for @historyDeleteSuccess.
  ///
  /// In en, this message translates to:
  /// **'Action(s) deleted.'**
  String get historyDeleteSuccess;

  /// No description provided for @historyDeleteFailed.
  ///
  /// In en, this message translates to:
  /// **'Failed to delete the action.'**
  String get historyDeleteFailed;

  /// No description provided for @historyEditTitle.
  ///
  /// In en, this message translates to:
  /// **'Edit action'**
  String get historyEditTitle;

  /// No description provided for @historyEditSave.
  ///
  /// In en, this message translates to:
  /// **'Save'**
  String get historyEditSave;

  /// No description provided for @historyEditSuccess.
  ///
  /// In en, this message translates to:
  /// **'Action updated.'**
  String get historyEditSuccess;

  /// No description provided for @historyEditFailed.
  ///
  /// In en, this message translates to:
  /// **'Failed to update the action.'**
  String get historyEditFailed;

  /// No description provided for @historyChangeConflicts.
  ///
  /// In en, this message translates to:
  /// **'Saving this change affects {n} other action(s). Continue?'**
  String historyChangeConflicts(int n);

  /// No description provided for @historyChangeConflictsTitle.
  ///
  /// In en, this message translates to:
  /// **'Dependent actions'**
  String get historyChangeConflictsTitle;

  /// No description provided for @personsSearchHint.
  ///
  /// In en, this message translates to:
  /// **'Search by name…'**
  String get personsSearchHint;

  /// No description provided for @personsNoResults.
  ///
  /// In en, this message translates to:
  /// **'No persons found.'**
  String get personsNoResults;

  /// No description provided for @personsAddTooltip.
  ///
  /// In en, this message translates to:
  /// **'New person'**
  String get personsAddTooltip;

  /// No description provided for @personCreateTitle.
  ///
  /// In en, this message translates to:
  /// **'New person'**
  String get personCreateTitle;

  /// No description provided for @personSectionDetails.
  ///
  /// In en, this message translates to:
  /// **'Person details'**
  String get personSectionDetails;

  /// No description provided for @personFormLastname.
  ///
  /// In en, this message translates to:
  /// **'Last name'**
  String get personFormLastname;

  /// No description provided for @personFormFirstname.
  ///
  /// In en, this message translates to:
  /// **'First name'**
  String get personFormFirstname;

  /// No description provided for @personFormTitle.
  ///
  /// In en, this message translates to:
  /// **'Title'**
  String get personFormTitle;

  /// No description provided for @personFormSchoolClass.
  ///
  /// In en, this message translates to:
  /// **'Class'**
  String get personFormSchoolClass;

  /// No description provided for @personFormBarcode.
  ///
  /// In en, this message translates to:
  /// **'Barcode'**
  String get personFormBarcode;

  /// No description provided for @personFormEmail.
  ///
  /// In en, this message translates to:
  /// **'Email (comma-separated)'**
  String get personFormEmail;

  /// No description provided for @personFormTelephone.
  ///
  /// In en, this message translates to:
  /// **'Telephone'**
  String get personFormTelephone;

  /// No description provided for @personFormStreet.
  ///
  /// In en, this message translates to:
  /// **'Street'**
  String get personFormStreet;

  /// No description provided for @personFormZip.
  ///
  /// In en, this message translates to:
  /// **'ZIP'**
  String get personFormZip;

  /// No description provided for @personFormCity.
  ///
  /// In en, this message translates to:
  /// **'City'**
  String get personFormCity;

  /// No description provided for @personFormCountry.
  ///
  /// In en, this message translates to:
  /// **'Country'**
  String get personFormCountry;

  /// No description provided for @personFormRemarks.
  ///
  /// In en, this message translates to:
  /// **'Remarks'**
  String get personFormRemarks;

  /// No description provided for @personFormExternalId.
  ///
  /// In en, this message translates to:
  /// **'External ID (UUID)'**
  String get personFormExternalId;

  /// No description provided for @personSave.
  ///
  /// In en, this message translates to:
  /// **'Save'**
  String get personSave;

  /// No description provided for @personSaved.
  ///
  /// In en, this message translates to:
  /// **'Person saved.'**
  String get personSaved;

  /// No description provided for @personConflict.
  ///
  /// In en, this message translates to:
  /// **'Conflict — the record was modified externally. Please reload.'**
  String get personConflict;

  /// No description provided for @personSaveFailed.
  ///
  /// In en, this message translates to:
  /// **'Failed to save person.'**
  String get personSaveFailed;

  /// No description provided for @personDelete.
  ///
  /// In en, this message translates to:
  /// **'Delete'**
  String get personDelete;

  /// No description provided for @personDeleteConfirm.
  ///
  /// In en, this message translates to:
  /// **'Delete this person permanently?'**
  String get personDeleteConfirm;

  /// No description provided for @personDeleteFailed.
  ///
  /// In en, this message translates to:
  /// **'Cannot delete — return all open lendings first.'**
  String get personDeleteFailed;

  /// No description provided for @personDeleteSuccess.
  ///
  /// In en, this message translates to:
  /// **'Person deleted.'**
  String get personDeleteSuccess;

  /// No description provided for @personLoginSection.
  ///
  /// In en, this message translates to:
  /// **'Login'**
  String get personLoginSection;

  /// No description provided for @personUsernameLabel.
  ///
  /// In en, this message translates to:
  /// **'Username'**
  String get personUsernameLabel;

  /// No description provided for @personUsernameNotSet.
  ///
  /// In en, this message translates to:
  /// **'No username set'**
  String get personUsernameNotSet;

  /// No description provided for @personUsernameSetButton.
  ///
  /// In en, this message translates to:
  /// **'Set'**
  String get personUsernameSetButton;

  /// No description provided for @personUsernameClearButton.
  ///
  /// In en, this message translates to:
  /// **'Remove'**
  String get personUsernameClearButton;

  /// No description provided for @personUsernameSaved.
  ///
  /// In en, this message translates to:
  /// **'Username saved.'**
  String get personUsernameSaved;

  /// No description provided for @personUsernameRemoved.
  ///
  /// In en, this message translates to:
  /// **'Username removed.'**
  String get personUsernameRemoved;

  /// No description provided for @personUsernameTaken.
  ///
  /// In en, this message translates to:
  /// **'Username already taken.'**
  String get personUsernameTaken;

  /// No description provided for @personPasswordSection.
  ///
  /// In en, this message translates to:
  /// **'Password'**
  String get personPasswordSection;

  /// No description provided for @personPasswordIsSet.
  ///
  /// In en, this message translates to:
  /// **'Password is set'**
  String get personPasswordIsSet;

  /// No description provided for @personPasswordNotSet.
  ///
  /// In en, this message translates to:
  /// **'No password set'**
  String get personPasswordNotSet;

  /// No description provided for @personPasswordSetButton.
  ///
  /// In en, this message translates to:
  /// **'Set password'**
  String get personPasswordSetButton;

  /// No description provided for @personPasswordRemoveButton.
  ///
  /// In en, this message translates to:
  /// **'Remove password'**
  String get personPasswordRemoveButton;

  /// No description provided for @personPasswordSetTitle.
  ///
  /// In en, this message translates to:
  /// **'Set password'**
  String get personPasswordSetTitle;

  /// No description provided for @personPasswordNew.
  ///
  /// In en, this message translates to:
  /// **'New password'**
  String get personPasswordNew;

  /// No description provided for @personPasswordConfirm.
  ///
  /// In en, this message translates to:
  /// **'Confirm password'**
  String get personPasswordConfirm;

  /// No description provided for @personPasswordMismatch.
  ///
  /// In en, this message translates to:
  /// **'Passwords do not match.'**
  String get personPasswordMismatch;

  /// No description provided for @personPasswordSaved.
  ///
  /// In en, this message translates to:
  /// **'Password saved.'**
  String get personPasswordSaved;

  /// No description provided for @personPasswordRemoved.
  ///
  /// In en, this message translates to:
  /// **'Password removed.'**
  String get personPasswordRemoved;

  /// No description provided for @navChangePassword.
  ///
  /// In en, this message translates to:
  /// **'Change password'**
  String get navChangePassword;

  /// No description provided for @navChangePasswords.
  ///
  /// In en, this message translates to:
  /// **'Manage passwords'**
  String get navChangePasswords;

  /// No description provided for @changePasswordTitle.
  ///
  /// In en, this message translates to:
  /// **'Change password'**
  String get changePasswordTitle;

  /// No description provided for @changePasswordCurrent.
  ///
  /// In en, this message translates to:
  /// **'Current password'**
  String get changePasswordCurrent;

  /// No description provided for @changePasswordWrong.
  ///
  /// In en, this message translates to:
  /// **'Wrong password.'**
  String get changePasswordWrong;

  /// No description provided for @changePasswordSubmit.
  ///
  /// In en, this message translates to:
  /// **'Change password'**
  String get changePasswordSubmit;

  /// No description provided for @changePasswordsTitle.
  ///
  /// In en, this message translates to:
  /// **'Manage passwords'**
  String get changePasswordsTitle;

  /// No description provided for @changePasswordsSelectPrompt.
  ///
  /// In en, this message translates to:
  /// **'Select a person to manage their credentials.'**
  String get changePasswordsSelectPrompt;

  /// No description provided for @changePasswordsNoUsername.
  ///
  /// In en, this message translates to:
  /// **'No username set'**
  String get changePasswordsNoUsername;

  /// No description provided for @changePasswordsDeleteConfirm.
  ///
  /// In en, this message translates to:
  /// **'Delete the password for {name}?'**
  String changePasswordsDeleteConfirm(String name);

  /// No description provided for @personPermissionsSection.
  ///
  /// In en, this message translates to:
  /// **'Permissions'**
  String get personPermissionsSection;

  /// No description provided for @personPermissionsApply.
  ///
  /// In en, this message translates to:
  /// **'Apply'**
  String get personPermissionsApply;

  /// No description provided for @personPermissionsSaved.
  ///
  /// In en, this message translates to:
  /// **'Permissions saved.'**
  String get personPermissionsSaved;

  /// No description provided for @permEditPersons.
  ///
  /// In en, this message translates to:
  /// **'Edit persons'**
  String get permEditPersons;

  /// No description provided for @permCreateLenders.
  ///
  /// In en, this message translates to:
  /// **'Create borrowers'**
  String get permCreateLenders;

  /// No description provided for @permCreateDataCollectors.
  ///
  /// In en, this message translates to:
  /// **'Create data collectors'**
  String get permCreateDataCollectors;

  /// No description provided for @permCollectLendings.
  ///
  /// In en, this message translates to:
  /// **'Record lendings'**
  String get permCollectLendings;

  /// No description provided for @permCollectReturns.
  ///
  /// In en, this message translates to:
  /// **'Record returns'**
  String get permCollectReturns;

  /// No description provided for @permCollectAcquisitions.
  ///
  /// In en, this message translates to:
  /// **'Record acquisitions'**
  String get permCollectAcquisitions;

  /// No description provided for @permCollectLiquidations.
  ///
  /// In en, this message translates to:
  /// **'Record liquidations'**
  String get permCollectLiquidations;

  /// No description provided for @permViewHistory.
  ///
  /// In en, this message translates to:
  /// **'View history'**
  String get permViewHistory;

  /// No description provided for @permViewPersons.
  ///
  /// In en, this message translates to:
  /// **'View person lists'**
  String get permViewPersons;

  /// No description provided for @permViewItems.
  ///
  /// In en, this message translates to:
  /// **'View item lists'**
  String get permViewItems;

  /// No description provided for @permEditPreferences.
  ///
  /// In en, this message translates to:
  /// **'Edit preferences'**
  String get permEditPreferences;

  /// No description provided for @permPrintIds.
  ///
  /// In en, this message translates to:
  /// **'Print ID cards'**
  String get permPrintIds;

  /// No description provided for @permPrintBarcodes.
  ///
  /// In en, this message translates to:
  /// **'Print barcodes'**
  String get permPrintBarcodes;

  /// No description provided for @permEditCategories.
  ///
  /// In en, this message translates to:
  /// **'Edit categories'**
  String get permEditCategories;

  /// No description provided for @permViewPersonalData.
  ///
  /// In en, this message translates to:
  /// **'View personal data'**
  String get permViewPersonalData;

  /// No description provided for @permManageLabelLayouts.
  ///
  /// In en, this message translates to:
  /// **'Manage label layouts'**
  String get permManageLabelLayouts;

  /// No description provided for @permCalibratePrinters.
  ///
  /// In en, this message translates to:
  /// **'Calibrate printers'**
  String get permCalibratePrinters;

  /// No description provided for @permEditIdLayout.
  ///
  /// In en, this message translates to:
  /// **'Edit ID card layout'**
  String get permEditIdLayout;

  /// No description provided for @permPrintSignatures.
  ///
  /// In en, this message translates to:
  /// **'Print signature labels'**
  String get permPrintSignatures;

  /// No description provided for @permChangeActions.
  ///
  /// In en, this message translates to:
  /// **'Change and delete actions'**
  String get permChangeActions;

  /// No description provided for @permEditProgramSettings.
  ///
  /// In en, this message translates to:
  /// **'Edit program settings'**
  String get permEditProgramSettings;

  /// No description provided for @permManageClasses.
  ///
  /// In en, this message translates to:
  /// **'Manage classes'**
  String get permManageClasses;

  /// No description provided for @permReserve.
  ///
  /// In en, this message translates to:
  /// **'Reserve media'**
  String get permReserve;

  /// No description provided for @permEditReservations.
  ///
  /// In en, this message translates to:
  /// **'Edit reservations'**
  String get permEditReservations;

  /// No description provided for @permAdminPermissions.
  ///
  /// In en, this message translates to:
  /// **'Edit item settings'**
  String get permAdminPermissions;

  /// No description provided for @barcodeDispatchTitle.
  ///
  /// In en, this message translates to:
  /// **'Scan Result'**
  String get barcodeDispatchTitle;

  /// No description provided for @barcodeDispatchLoading.
  ///
  /// In en, this message translates to:
  /// **'Looking up barcode…'**
  String get barcodeDispatchLoading;

  /// No description provided for @barcodeDispatchPersonSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Actions for {name}'**
  String barcodeDispatchPersonSubtitle(String name);

  /// No description provided for @barcodeDispatchItemSubtitle.
  ///
  /// In en, this message translates to:
  /// **'Actions for: {name}'**
  String barcodeDispatchItemSubtitle(String name);

  /// No description provided for @barcodeDispatchActionViewLendings.
  ///
  /// In en, this message translates to:
  /// **'Open lendings & reservations'**
  String get barcodeDispatchActionViewLendings;

  /// No description provided for @barcodeDispatchActionEditPerson.
  ///
  /// In en, this message translates to:
  /// **'Edit person'**
  String get barcodeDispatchActionEditPerson;

  /// No description provided for @barcodeDispatchActionReturnPersonItems.
  ///
  /// In en, this message translates to:
  /// **'Return items'**
  String get barcodeDispatchActionReturnPersonItems;

  /// No description provided for @barcodeDispatchActionLendToPerson.
  ///
  /// In en, this message translates to:
  /// **'Lend item to this person'**
  String get barcodeDispatchActionLendToPerson;

  /// No description provided for @barcodeDispatchActionReturnItem.
  ///
  /// In en, this message translates to:
  /// **'Return this item'**
  String get barcodeDispatchActionReturnItem;

  /// No description provided for @barcodeDispatchActionLendItem.
  ///
  /// In en, this message translates to:
  /// **'Lend this item'**
  String get barcodeDispatchActionLendItem;

  /// No description provided for @barcodeDispatchActionAcquireItem.
  ///
  /// In en, this message translates to:
  /// **'Acquire / edit item'**
  String get barcodeDispatchActionAcquireItem;

  /// No description provided for @barcodeDispatchActionLiquidateItem.
  ///
  /// In en, this message translates to:
  /// **'Liquidate item'**
  String get barcodeDispatchActionLiquidateItem;

  /// No description provided for @barcodeDispatchNotRecognized.
  ///
  /// In en, this message translates to:
  /// **'Barcode not recognized'**
  String get barcodeDispatchNotRecognized;

  /// No description provided for @barcodeDispatchNoOpenLendings.
  ///
  /// In en, this message translates to:
  /// **'No open lendings'**
  String get barcodeDispatchNoOpenLendings;

  /// No description provided for @thumbnailFromFile.
  ///
  /// In en, this message translates to:
  /// **'From file'**
  String get thumbnailFromFile;

  /// No description provided for @thumbnailSearchAmazon.
  ///
  /// In en, this message translates to:
  /// **'Search on Amazon'**
  String get thumbnailSearchAmazon;

  /// No description provided for @thumbnailRemove.
  ///
  /// In en, this message translates to:
  /// **'Remove'**
  String get thumbnailRemove;

  /// No description provided for @thumbnailNone.
  ///
  /// In en, this message translates to:
  /// **'No cover image'**
  String get thumbnailNone;

  /// No description provided for @amazonBrowserHint.
  ///
  /// In en, this message translates to:
  /// **'Tap a cover image to use it'**
  String get amazonBrowserHint;

  /// No description provided for @amazonBrowserDownloadError.
  ///
  /// In en, this message translates to:
  /// **'Could not download image'**
  String get amazonBrowserDownloadError;

  /// No description provided for @certDialogTitle.
  ///
  /// In en, this message translates to:
  /// **'Untrusted Certificate'**
  String get certDialogTitle;

  /// No description provided for @certDialogBody.
  ///
  /// In en, this message translates to:
  /// **'The server \"{host}\" uses a certificate that could not be verified.\n\nSubject: {subject}\nValid: {validFrom} – {validUntil}\nSHA-256:\n{fingerprint}\n\nTrust this server?'**
  String certDialogBody(
    String host,
    String subject,
    String validFrom,
    String validUntil,
    String fingerprint,
  );

  /// No description provided for @certDialogTrust.
  ///
  /// In en, this message translates to:
  /// **'Trust'**
  String get certDialogTrust;

  /// No description provided for @certDialogCancel.
  ///
  /// In en, this message translates to:
  /// **'Cancel'**
  String get certDialogCancel;

  /// No description provided for @titleSearchTitle.
  ///
  /// In en, this message translates to:
  /// **'Title search'**
  String get titleSearchTitle;

  /// No description provided for @titleSearchButton.
  ///
  /// In en, this message translates to:
  /// **'Search internet by title'**
  String get titleSearchButton;

  /// No description provided for @titleSearchOfferButton.
  ///
  /// In en, this message translates to:
  /// **'Search the internet for \"{query}\"'**
  String titleSearchOfferButton(String query);

  /// No description provided for @titleSearchFieldHint.
  ///
  /// In en, this message translates to:
  /// **'Title…'**
  String get titleSearchFieldHint;

  /// No description provided for @titleSearchKindBook.
  ///
  /// In en, this message translates to:
  /// **'Book'**
  String get titleSearchKindBook;

  /// No description provided for @titleSearchKindCd.
  ///
  /// In en, this message translates to:
  /// **'CD'**
  String get titleSearchKindCd;

  /// No description provided for @titleSearchKindDvd.
  ///
  /// In en, this message translates to:
  /// **'DVD'**
  String get titleSearchKindDvd;

  /// No description provided for @titleSearchKindBluray.
  ///
  /// In en, this message translates to:
  /// **'Blu-ray'**
  String get titleSearchKindBluray;

  /// No description provided for @titleSearchKindBoardGame.
  ///
  /// In en, this message translates to:
  /// **'Board game'**
  String get titleSearchKindBoardGame;

  /// No description provided for @titleSearchEmptyPrompt.
  ///
  /// In en, this message translates to:
  /// **'Enter a title and start the search.'**
  String get titleSearchEmptyPrompt;

  /// No description provided for @titleSearchNoResults.
  ///
  /// In en, this message translates to:
  /// **'No results found for \"{query}\".'**
  String titleSearchNoResults(String query);

  /// No description provided for @titleSearchDetailFailed.
  ///
  /// In en, this message translates to:
  /// **'Could not retrieve details from the selected source.'**
  String get titleSearchDetailFailed;

  /// No description provided for @settingsSruTitleField.
  ///
  /// In en, this message translates to:
  /// **'Title field'**
  String get settingsSruTitleField;

  /// No description provided for @navIdCardLayout.
  ///
  /// In en, this message translates to:
  /// **'ID card layout'**
  String get navIdCardLayout;

  /// No description provided for @navIdCardPrint.
  ///
  /// In en, this message translates to:
  /// **'Print ID cards'**
  String get navIdCardPrint;

  /// No description provided for @idCardLayoutTitle.
  ///
  /// In en, this message translates to:
  /// **'ID card layout'**
  String get idCardLayoutTitle;

  /// No description provided for @idCardLayoutCardSize.
  ///
  /// In en, this message translates to:
  /// **'Card size'**
  String get idCardLayoutCardSize;

  /// No description provided for @idCardLayoutWidthMm.
  ///
  /// In en, this message translates to:
  /// **'Width (mm)'**
  String get idCardLayoutWidthMm;

  /// No description provided for @idCardLayoutHeightMm.
  ///
  /// In en, this message translates to:
  /// **'Height (mm)'**
  String get idCardLayoutHeightMm;

  /// No description provided for @idCardLayoutUseLogo.
  ///
  /// In en, this message translates to:
  /// **'Use logo'**
  String get idCardLayoutUseLogo;

  /// No description provided for @idCardLayoutMarginMm.
  ///
  /// In en, this message translates to:
  /// **'Free margin (mm)'**
  String get idCardLayoutMarginMm;

  /// No description provided for @idCardLayoutZoneBarcode.
  ///
  /// In en, this message translates to:
  /// **'Barcode'**
  String get idCardLayoutZoneBarcode;

  /// No description provided for @idCardLayoutZoneText.
  ///
  /// In en, this message translates to:
  /// **'Text'**
  String get idCardLayoutZoneText;

  /// No description provided for @idCardLayoutZoneLogo.
  ///
  /// In en, this message translates to:
  /// **'Logo'**
  String get idCardLayoutZoneLogo;

  /// No description provided for @idCardLayoutRotate.
  ///
  /// In en, this message translates to:
  /// **'Rotate 90°'**
  String get idCardLayoutRotate;

  /// No description provided for @idCardLayoutMaximize.
  ///
  /// In en, this message translates to:
  /// **'Maximize'**
  String get idCardLayoutMaximize;

  /// No description provided for @idCardLayoutAlignLeft.
  ///
  /// In en, this message translates to:
  /// **'Align left'**
  String get idCardLayoutAlignLeft;

  /// No description provided for @idCardLayoutAlignHCenter.
  ///
  /// In en, this message translates to:
  /// **'Centre horizontally'**
  String get idCardLayoutAlignHCenter;

  /// No description provided for @idCardLayoutAlignRight.
  ///
  /// In en, this message translates to:
  /// **'Align right'**
  String get idCardLayoutAlignRight;

  /// No description provided for @idCardLayoutAlignTop.
  ///
  /// In en, this message translates to:
  /// **'Align top'**
  String get idCardLayoutAlignTop;

  /// No description provided for @idCardLayoutAlignVCenter.
  ///
  /// In en, this message translates to:
  /// **'Centre vertically'**
  String get idCardLayoutAlignVCenter;

  /// No description provided for @idCardLayoutAlignBottom.
  ///
  /// In en, this message translates to:
  /// **'Align bottom'**
  String get idCardLayoutAlignBottom;

  /// No description provided for @idCardLayoutRotateBlocked.
  ///
  /// In en, this message translates to:
  /// **'The zone does not fit on the card after rotation.'**
  String get idCardLayoutRotateBlocked;

  /// No description provided for @idCardLayoutSave.
  ///
  /// In en, this message translates to:
  /// **'Save'**
  String get idCardLayoutSave;

  /// No description provided for @idCardLayoutReset.
  ///
  /// In en, this message translates to:
  /// **'Reset'**
  String get idCardLayoutReset;

  /// No description provided for @idCardLayoutSaved.
  ///
  /// In en, this message translates to:
  /// **'Layout saved.'**
  String get idCardLayoutSaved;

  /// No description provided for @idCardSampleName.
  ///
  /// In en, this message translates to:
  /// **'Sample Name'**
  String get idCardSampleName;

  /// No description provided for @idCardIssuedOn.
  ///
  /// In en, this message translates to:
  /// **'issued on: {date}'**
  String idCardIssuedOn(String date);

  /// No description provided for @idCardPrintTitle.
  ///
  /// In en, this message translates to:
  /// **'Print ID cards'**
  String get idCardPrintTitle;

  /// No description provided for @idCardPrintSearchHint.
  ///
  /// In en, this message translates to:
  /// **'Search persons'**
  String get idCardPrintSearchHint;

  /// No description provided for @idCardPrintSelectAll.
  ///
  /// In en, this message translates to:
  /// **'Select all'**
  String get idCardPrintSelectAll;

  /// No description provided for @idCardPrintDeselectAll.
  ///
  /// In en, this message translates to:
  /// **'Deselect all'**
  String get idCardPrintDeselectAll;

  /// No description provided for @idCardPrintSelectedCount.
  ///
  /// In en, this message translates to:
  /// **'{count} selected'**
  String idCardPrintSelectedCount(int count);

  /// No description provided for @idCardPrintButton.
  ///
  /// In en, this message translates to:
  /// **'Print'**
  String get idCardPrintButton;

  /// No description provided for @idCardPrintNoSelection.
  ///
  /// In en, this message translates to:
  /// **'Select at least one person.'**
  String get idCardPrintNoSelection;

  /// No description provided for @idCardPrintNoLayout.
  ///
  /// In en, this message translates to:
  /// **'No ID-card layout configured.'**
  String get idCardPrintNoLayout;

  /// No description provided for @idCardPrintArrangement.
  ///
  /// In en, this message translates to:
  /// **'Label arrangement'**
  String get idCardPrintArrangement;

  /// No description provided for @idCardPrintPaperSize.
  ///
  /// In en, this message translates to:
  /// **'Paper size'**
  String get idCardPrintPaperSize;

  /// No description provided for @idCardPrintOrientation.
  ///
  /// In en, this message translates to:
  /// **'Orientation'**
  String get idCardPrintOrientation;

  /// No description provided for @idCardPrintPortrait.
  ///
  /// In en, this message translates to:
  /// **'Portrait'**
  String get idCardPrintPortrait;

  /// No description provided for @idCardPrintLandscape.
  ///
  /// In en, this message translates to:
  /// **'Landscape'**
  String get idCardPrintLandscape;

  /// No description provided for @idCardPrintColumns.
  ///
  /// In en, this message translates to:
  /// **'Columns'**
  String get idCardPrintColumns;

  /// No description provided for @idCardPrintRows.
  ///
  /// In en, this message translates to:
  /// **'Rows'**
  String get idCardPrintRows;

  /// No description provided for @idCardPrintMarginLeft.
  ///
  /// In en, this message translates to:
  /// **'Left margin (mm)'**
  String get idCardPrintMarginLeft;

  /// No description provided for @idCardPrintMarginTop.
  ///
  /// In en, this message translates to:
  /// **'Top margin (mm)'**
  String get idCardPrintMarginTop;

  /// No description provided for @idCardPrintGapH.
  ///
  /// In en, this message translates to:
  /// **'Horizontal gap (mm)'**
  String get idCardPrintGapH;

  /// No description provided for @idCardPrintGapV.
  ///
  /// In en, this message translates to:
  /// **'Vertical gap (mm)'**
  String get idCardPrintGapV;

  /// No description provided for @idCardPrintDrawBorder.
  ///
  /// In en, this message translates to:
  /// **'Draw border around each card'**
  String get idCardPrintDrawBorder;

  /// No description provided for @navThermalLabelPrint.
  ///
  /// In en, this message translates to:
  /// **'Print labels'**
  String get navThermalLabelPrint;

  /// No description provided for @navLabelPrint.
  ///
  /// In en, this message translates to:
  /// **'Print labels'**
  String get navLabelPrint;

  /// No description provided for @navLabelPresets.
  ///
  /// In en, this message translates to:
  /// **'Label layouts'**
  String get navLabelPresets;

  /// No description provided for @navSignaturePrint.
  ///
  /// In en, this message translates to:
  /// **'Print signatures'**
  String get navSignaturePrint;

  /// No description provided for @signaturePrintTitle.
  ///
  /// In en, this message translates to:
  /// **'Print signatures'**
  String get signaturePrintTitle;

  /// No description provided for @signaturePrintTop.
  ///
  /// In en, this message translates to:
  /// **'Top signature'**
  String get signaturePrintTop;

  /// No description provided for @signaturePrintBottom.
  ///
  /// In en, this message translates to:
  /// **'Bottom signature'**
  String get signaturePrintBottom;

  /// No description provided for @signaturePrintCount.
  ///
  /// In en, this message translates to:
  /// **'Count'**
  String get signaturePrintCount;

  /// No description provided for @signaturePrintButton.
  ///
  /// In en, this message translates to:
  /// **'Print'**
  String get signaturePrintButton;

  /// No description provided for @signaturePrintPickItem.
  ///
  /// In en, this message translates to:
  /// **'Pick an item to prefill the signatures (optional)'**
  String get signaturePrintPickItem;

  /// No description provided for @signaturePrintEmpty.
  ///
  /// In en, this message translates to:
  /// **'Enter a signature first.'**
  String get signaturePrintEmpty;

  /// No description provided for @labelPrintTitle.
  ///
  /// In en, this message translates to:
  /// **'Print labels'**
  String get labelPrintTitle;

  /// No description provided for @labelPrintButton.
  ///
  /// In en, this message translates to:
  /// **'Print'**
  String get labelPrintButton;

  /// No description provided for @labelPrintNext.
  ///
  /// In en, this message translates to:
  /// **'Next: positions'**
  String get labelPrintNext;

  /// No description provided for @labelPrintSearchHint.
  ///
  /// In en, this message translates to:
  /// **'Search items'**
  String get labelPrintSearchHint;

  /// No description provided for @labelPrintCopies.
  ///
  /// In en, this message translates to:
  /// **'Copies per item'**
  String get labelPrintCopies;

  /// No description provided for @labelPrintSelectAll.
  ///
  /// In en, this message translates to:
  /// **'Select all'**
  String get labelPrintSelectAll;

  /// No description provided for @labelPrintDeselectAll.
  ///
  /// In en, this message translates to:
  /// **'Deselect all'**
  String get labelPrintDeselectAll;

  /// No description provided for @labelPrintSelectedCount.
  ///
  /// In en, this message translates to:
  /// **'{count} selected'**
  String labelPrintSelectedCount(int count);

  /// No description provided for @labelPrintNoItems.
  ///
  /// In en, this message translates to:
  /// **'No items found.'**
  String get labelPrintNoItems;

  /// No description provided for @labelPrintNoSelection.
  ///
  /// In en, this message translates to:
  /// **'Select at least one item.'**
  String get labelPrintNoSelection;

  /// No description provided for @labelPrintNoPrinter.
  ///
  /// In en, this message translates to:
  /// **'No USB printer connected.'**
  String get labelPrintNoPrinter;

  /// No description provided for @labelPrintPrinterConnected.
  ///
  /// In en, this message translates to:
  /// **'Connected via USB'**
  String get labelPrintPrinterConnected;

  /// No description provided for @labelPrintRefresh.
  ///
  /// In en, this message translates to:
  /// **'Rescan for printer'**
  String get labelPrintRefresh;

  /// No description provided for @labelPrintPreset.
  ///
  /// In en, this message translates to:
  /// **'Layout'**
  String get labelPrintPreset;

  /// No description provided for @labelPrintPositions.
  ///
  /// In en, this message translates to:
  /// **'Label positions on the sheet'**
  String get labelPrintPositions;

  /// No description provided for @labelPrintPositionsHint.
  ///
  /// In en, this message translates to:
  /// **'Pick which positions on the sheet to use — at most as many as selected items.'**
  String get labelPrintPositionsHint;

  /// No description provided for @labelPrintCellsSelected.
  ///
  /// In en, this message translates to:
  /// **'{count} of {limit} positions selected'**
  String labelPrintCellsSelected(int count, int limit);

  /// No description provided for @labelPrintTooManyCells.
  ///
  /// In en, this message translates to:
  /// **'You cannot select more positions than selected items.'**
  String get labelPrintTooManyCells;

  /// No description provided for @labelPrintRemaining.
  ///
  /// In en, this message translates to:
  /// **'{count} item(s) remaining — pick positions for the next sheet.'**
  String labelPrintRemaining(int count);

  /// No description provided for @labelPrintNoCells.
  ///
  /// In en, this message translates to:
  /// **'Select at least one label position.'**
  String get labelPrintNoCells;

  /// No description provided for @labelPrintPermissionDenied.
  ///
  /// In en, this message translates to:
  /// **'USB permission denied.'**
  String get labelPrintPermissionDenied;

  /// No description provided for @labelPrintSent.
  ///
  /// In en, this message translates to:
  /// **'{count} label(s) sent to the printer.'**
  String labelPrintSent(int count);

  /// No description provided for @cancel.
  ///
  /// In en, this message translates to:
  /// **'Cancel'**
  String get cancel;

  /// No description provided for @ok.
  ///
  /// In en, this message translates to:
  /// **'OK'**
  String get ok;

  /// No description provided for @signatureSample.
  ///
  /// In en, this message translates to:
  /// **'Sample 4'**
  String get signatureSample;

  /// No description provided for @labelPresetTitle.
  ///
  /// In en, this message translates to:
  /// **'Label layouts'**
  String get labelPresetTitle;

  /// No description provided for @labelPresetPreset.
  ///
  /// In en, this message translates to:
  /// **'Layout'**
  String get labelPresetPreset;

  /// No description provided for @labelPresetNew.
  ///
  /// In en, this message translates to:
  /// **'New layout'**
  String get labelPresetNew;

  /// No description provided for @labelPresetRename.
  ///
  /// In en, this message translates to:
  /// **'Rename'**
  String get labelPresetRename;

  /// No description provided for @labelPresetDelete.
  ///
  /// In en, this message translates to:
  /// **'Delete'**
  String get labelPresetDelete;

  /// No description provided for @labelPresetName.
  ///
  /// In en, this message translates to:
  /// **'Name'**
  String get labelPresetName;

  /// No description provided for @labelPresetPurpose.
  ///
  /// In en, this message translates to:
  /// **'Purpose'**
  String get labelPresetPurpose;

  /// No description provided for @labelPresetPrinter.
  ///
  /// In en, this message translates to:
  /// **'Printer'**
  String get labelPresetPrinter;

  /// No description provided for @labelPresetBarWidthReduction.
  ///
  /// In en, this message translates to:
  /// **'Bar width reduction (mm)'**
  String get labelPresetBarWidthReduction;

  /// Width of the narrowest Code128 bar (X-dimension) in millimetres; 0.33 mm is the recommended default
  ///
  /// In en, this message translates to:
  /// **'Barcode bar width (mm)'**
  String get labelBarcodeUnit;

  /// No description provided for @labelPresetEditArrangement.
  ///
  /// In en, this message translates to:
  /// **'Edit label arrangement'**
  String get labelPresetEditArrangement;

  /// No description provided for @labelPresetArrangement.
  ///
  /// In en, this message translates to:
  /// **'Label arrangement'**
  String get labelPresetArrangement;

  /// No description provided for @labelPresetSave.
  ///
  /// In en, this message translates to:
  /// **'Save'**
  String get labelPresetSave;

  /// No description provided for @labelPresetSaved.
  ///
  /// In en, this message translates to:
  /// **'Layout saved.'**
  String get labelPresetSaved;

  /// No description provided for @labelPresetDarkness.
  ///
  /// In en, this message translates to:
  /// **'Darkness'**
  String get labelPresetDarkness;

  /// No description provided for @labelPresetGap.
  ///
  /// In en, this message translates to:
  /// **'Gap (mm)'**
  String get labelPresetGap;

  /// No description provided for @labelPresetXOffset.
  ///
  /// In en, this message translates to:
  /// **'Horizontal offset (mm)'**
  String get labelPresetXOffset;

  /// No description provided for @labelPresetColumns.
  ///
  /// In en, this message translates to:
  /// **'Columns'**
  String get labelPresetColumns;

  /// No description provided for @labelPresetRows.
  ///
  /// In en, this message translates to:
  /// **'Rows'**
  String get labelPresetRows;

  /// No description provided for @labelPresetLeftMargin.
  ///
  /// In en, this message translates to:
  /// **'Left margin (mm)'**
  String get labelPresetLeftMargin;

  /// No description provided for @labelPresetTopMargin.
  ///
  /// In en, this message translates to:
  /// **'Top margin (mm)'**
  String get labelPresetTopMargin;

  /// No description provided for @labelPresetHGap.
  ///
  /// In en, this message translates to:
  /// **'Horizontal gap (mm)'**
  String get labelPresetHGap;

  /// No description provided for @labelPresetVGap.
  ///
  /// In en, this message translates to:
  /// **'Vertical gap (mm)'**
  String get labelPresetVGap;

  /// No description provided for @labelPresetPaperSize.
  ///
  /// In en, this message translates to:
  /// **'Paper size'**
  String get labelPresetPaperSize;

  /// No description provided for @labelPresetLandscape.
  ///
  /// In en, this message translates to:
  /// **'Landscape'**
  String get labelPresetLandscape;

  /// No description provided for @labelPurposeBookLabel.
  ///
  /// In en, this message translates to:
  /// **'Book label'**
  String get labelPurposeBookLabel;

  /// No description provided for @labelPurposeSignature.
  ///
  /// In en, this message translates to:
  /// **'Signature'**
  String get labelPurposeSignature;

  /// No description provided for @labelPurposeIdCard.
  ///
  /// In en, this message translates to:
  /// **'ID card'**
  String get labelPurposeIdCard;

  /// No description provided for @labelPrinterThermal.
  ///
  /// In en, this message translates to:
  /// **'USB thermal printer'**
  String get labelPrinterThermal;

  /// No description provided for @labelPrinterSheet.
  ///
  /// In en, this message translates to:
  /// **'Normal/sheet printer'**
  String get labelPrinterSheet;

  /// No description provided for @navBookLabelLayout.
  ///
  /// In en, this message translates to:
  /// **'Book label layout'**
  String get navBookLabelLayout;

  /// No description provided for @bookLabelLayoutTitle.
  ///
  /// In en, this message translates to:
  /// **'Book label layout'**
  String get bookLabelLayoutTitle;

  /// No description provided for @bookLabelLayoutSaved.
  ///
  /// In en, this message translates to:
  /// **'Book label layout saved.'**
  String get bookLabelLayoutSaved;

  /// No description provided for @bookLabelSampleOrganization.
  ///
  /// In en, this message translates to:
  /// **'Sample Library'**
  String get bookLabelSampleOrganization;

  /// No description provided for @bookLabelSampleTitle.
  ///
  /// In en, this message translates to:
  /// **'Sample Title'**
  String get bookLabelSampleTitle;

  /// No description provided for @bookLabelSampleAuthor.
  ///
  /// In en, this message translates to:
  /// **'Sample Author'**
  String get bookLabelSampleAuthor;

  /// No description provided for @thermalPrintTitle.
  ///
  /// In en, this message translates to:
  /// **'Print item labels'**
  String get thermalPrintTitle;

  /// No description provided for @thermalPrintButton.
  ///
  /// In en, this message translates to:
  /// **'Print on USB printer'**
  String get thermalPrintButton;

  /// No description provided for @thermalPrintSystemButton.
  ///
  /// In en, this message translates to:
  /// **'Print on a normal printer'**
  String get thermalPrintSystemButton;

  /// No description provided for @thermalPrintSearchHint.
  ///
  /// In en, this message translates to:
  /// **'Search items'**
  String get thermalPrintSearchHint;

  /// No description provided for @thermalPrintSelectAll.
  ///
  /// In en, this message translates to:
  /// **'Select all'**
  String get thermalPrintSelectAll;

  /// No description provided for @thermalPrintDeselectAll.
  ///
  /// In en, this message translates to:
  /// **'Deselect all'**
  String get thermalPrintDeselectAll;

  /// No description provided for @thermalPrintSelectedCount.
  ///
  /// In en, this message translates to:
  /// **'{count} selected'**
  String thermalPrintSelectedCount(int count);

  /// No description provided for @thermalPrintNoItems.
  ///
  /// In en, this message translates to:
  /// **'No items found.'**
  String get thermalPrintNoItems;

  /// No description provided for @thermalPrintNoSelection.
  ///
  /// In en, this message translates to:
  /// **'Select at least one item.'**
  String get thermalPrintNoSelection;

  /// No description provided for @thermalPrintNoPrinter.
  ///
  /// In en, this message translates to:
  /// **'No USB printer connected.'**
  String get thermalPrintNoPrinter;

  /// No description provided for @thermalPrintPrinterConnected.
  ///
  /// In en, this message translates to:
  /// **'Connected via USB'**
  String get thermalPrintPrinterConnected;

  /// No description provided for @thermalPrintRefresh.
  ///
  /// In en, this message translates to:
  /// **'Rescan for printer'**
  String get thermalPrintRefresh;

  /// No description provided for @thermalPrintLabelSettings.
  ///
  /// In en, this message translates to:
  /// **'Label & printer settings'**
  String get thermalPrintLabelSettings;

  /// No description provided for @thermalPrintWidthMm.
  ///
  /// In en, this message translates to:
  /// **'Label width (mm)'**
  String get thermalPrintWidthMm;

  /// No description provided for @thermalPrintHeightMm.
  ///
  /// In en, this message translates to:
  /// **'Label height (mm)'**
  String get thermalPrintHeightMm;

  /// No description provided for @thermalPrintGapMm.
  ///
  /// In en, this message translates to:
  /// **'Gap between labels (mm)'**
  String get thermalPrintGapMm;

  /// No description provided for @thermalPrintXOffsetMm.
  ///
  /// In en, this message translates to:
  /// **'Horizontal offset (mm)'**
  String get thermalPrintXOffsetMm;

  /// No description provided for @thermalPrintDarkness.
  ///
  /// In en, this message translates to:
  /// **'Darkness'**
  String get thermalPrintDarkness;

  /// No description provided for @thermalPrintPermissionDenied.
  ///
  /// In en, this message translates to:
  /// **'USB permission denied.'**
  String get thermalPrintPermissionDenied;

  /// No description provided for @thermalPrintSent.
  ///
  /// In en, this message translates to:
  /// **'{count} label(s) sent to the printer.'**
  String thermalPrintSent(int count);

  /// No description provided for @thermalPrintAndroidOnly.
  ///
  /// In en, this message translates to:
  /// **'Direct USB label printing is only available on Android.'**
  String get thermalPrintAndroidOnly;

  /// No description provided for @calibrationButton.
  ///
  /// In en, this message translates to:
  /// **'Calibrate printer'**
  String get calibrationButton;

  /// No description provided for @calibrationTitle.
  ///
  /// In en, this message translates to:
  /// **'Calibrate printer'**
  String get calibrationTitle;

  /// No description provided for @calibrationIntro.
  ///
  /// In en, this message translates to:
  /// **'Correct the print position of a normal (sheet) printer. Print the calibration sheet at 100 % (no fit-to-page), measure the three crosses with a ruler and enter the values. The correction is saved with this preset and applied to every sheet print.'**
  String get calibrationIntro;

  /// No description provided for @calibrationPrintSheet.
  ///
  /// In en, this message translates to:
  /// **'Print calibration sheet'**
  String get calibrationPrintSheet;

  /// No description provided for @calibrationMeasureHeading.
  ///
  /// In en, this message translates to:
  /// **'Measured positions'**
  String get calibrationMeasureHeading;

  /// No description provided for @calibrationMeasureHint.
  ///
  /// In en, this message translates to:
  /// **'Measure the distance from the top/left paper edge to the centre of each cross, in millimetres.'**
  String get calibrationMeasureHint;

  /// No description provided for @calibrationMeasureAX.
  ///
  /// In en, this message translates to:
  /// **'Cross A – X'**
  String get calibrationMeasureAX;

  /// No description provided for @calibrationMeasureAY.
  ///
  /// In en, this message translates to:
  /// **'Cross A – Y'**
  String get calibrationMeasureAY;

  /// No description provided for @calibrationMeasureBX.
  ///
  /// In en, this message translates to:
  /// **'Cross B – X'**
  String get calibrationMeasureBX;

  /// No description provided for @calibrationMeasureCY.
  ///
  /// In en, this message translates to:
  /// **'Cross C – Y'**
  String get calibrationMeasureCY;

  /// No description provided for @calibrationCompute.
  ///
  /// In en, this message translates to:
  /// **'Compute correction'**
  String get calibrationCompute;

  /// No description provided for @calibrationResultHeading.
  ///
  /// In en, this message translates to:
  /// **'Computed correction'**
  String get calibrationResultHeading;

  /// No description provided for @calibrationOffsetX.
  ///
  /// In en, this message translates to:
  /// **'Offset X'**
  String get calibrationOffsetX;

  /// No description provided for @calibrationOffsetY.
  ///
  /// In en, this message translates to:
  /// **'Offset Y'**
  String get calibrationOffsetY;

  /// No description provided for @calibrationScaleX.
  ///
  /// In en, this message translates to:
  /// **'Scale X'**
  String get calibrationScaleX;

  /// No description provided for @calibrationScaleY.
  ///
  /// In en, this message translates to:
  /// **'Scale Y'**
  String get calibrationScaleY;

  /// No description provided for @calibrationReset.
  ///
  /// In en, this message translates to:
  /// **'Reset'**
  String get calibrationReset;

  /// No description provided for @calibrationSave.
  ///
  /// In en, this message translates to:
  /// **'Save'**
  String get calibrationSave;

  /// No description provided for @calibrationSaved.
  ///
  /// In en, this message translates to:
  /// **'Calibration saved.'**
  String get calibrationSaved;
}

class _AppLocalizationsDelegate
    extends LocalizationsDelegate<AppLocalizations> {
  const _AppLocalizationsDelegate();

  @override
  Future<AppLocalizations> load(Locale locale) {
    return SynchronousFuture<AppLocalizations>(lookupAppLocalizations(locale));
  }

  @override
  bool isSupported(Locale locale) =>
      <String>['de', 'en'].contains(locale.languageCode);

  @override
  bool shouldReload(_AppLocalizationsDelegate old) => false;
}

AppLocalizations lookupAppLocalizations(Locale locale) {
  // Lookup logic when only language code is specified.
  switch (locale.languageCode) {
    case 'de':
      return AppLocalizationsDe();
    case 'en':
      return AppLocalizationsEn();
  }

  throw FlutterError(
    'AppLocalizations.delegate failed to load unsupported locale "$locale". This is likely '
    'an issue with the localizations generation tool. Please file an issue '
    'on GitHub with a reproducible sample app and the gen-l10n configuration '
    'that was used.',
  );
}
