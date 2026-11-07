// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026

// ignore: unused_import
import 'package:intl/intl.dart' as intl;
import 'app_localizations.dart';

// ignore_for_file: type=lint

/// The translations for German (`de`).
class AppLocalizationsDe extends AppLocalizations {
  AppLocalizationsDe([String locale = 'de']) : super(locale);

  @override
  String get appTitle => 'Colman';

  @override
  String get signOut => 'Abmelden';

  @override
  String get navLend => 'Ausleihen';

  @override
  String get navReturn => 'Zurückgeben';

  @override
  String get navOpenLendings => 'Fällig';

  @override
  String get navMyLendings => 'Ausgeliehen';

  @override
  String get navMyReservations => 'Reserviert';

  @override
  String get myReservationsEmpty => 'Keine aktiven Reservierungen.';

  @override
  String get myReservationsColExpires => 'Gültig bis';

  @override
  String get openLendingsPersonLabel => 'Person';

  @override
  String get openLendingsAllClasses => 'Alle Klassen';

  @override
  String get openLendingsLabelClass => 'Klasse';

  @override
  String get openLendingsClearFilter => 'Filter löschen';

  @override
  String get openLendingsPrint => 'Drucken';

  @override
  String get openLendingsColDueDate => 'Fälligkeit';

  @override
  String get openLendingsColBorrower => 'Ausleiher';

  @override
  String get openLendingsColCount => 'Anzahl';

  @override
  String get openLendingsColItem => 'Titel';

  @override
  String get navHistory => 'Verlauf';

  @override
  String get navPersons => 'Benutzer';

  @override
  String get navAcquire => 'Anschaffen';

  @override
  String get navLiquidate => 'Aussondern';

  @override
  String get navSearch => 'Suchen';

  @override
  String get navScan => 'Scannen';

  @override
  String get navClasses => 'Klassen';

  @override
  String get navCategories => 'Kategorien';

  @override
  String get categoriesTitle => 'Kategorien verwalten';

  @override
  String get categoriesNoResults => 'Keine Kategorien vorhanden.';

  @override
  String get categoriesAddTitle => 'Neue Kategorie';

  @override
  String get categoriesEditTitle => 'Kategorie bearbeiten';

  @override
  String get categoriesNameLabel => 'Kategoriename';

  @override
  String get categoriesSearchCategory => 'Suchkategorie';

  @override
  String get categoriesFields => 'Felder';

  @override
  String get categoriesNoFields => 'Noch keine Felder.';

  @override
  String get categoriesAddField => 'Feld hinzufügen';

  @override
  String get categoriesEditFieldTitle => 'Feld';

  @override
  String get categoriesFieldName => 'Feldname';

  @override
  String get categoriesFieldType => 'Feldtyp';

  @override
  String get categoriesFieldRepeatable => 'Wiederholbar';

  @override
  String get categoriesFieldDelete => 'Feld entfernen';

  @override
  String get categoriesDeleteTitle => 'Kategorie löschen';

  @override
  String categoriesDeleteConfirm(String name) {
    return 'Kategorie \'$name\' löschen? Alle Medien dieses Typs und ihr Ausleihverlauf werden dauerhaft entfernt.';
  }

  @override
  String get categoriesSaveSuccess => 'Kategorie gespeichert.';

  @override
  String get categoriesSaveFailed =>
      'Die Kategorie konnte nicht gespeichert werden. Der Name ist möglicherweise bereits vergeben.';

  @override
  String get categoriesDeleteSuccess => 'Kategorie gelöscht.';

  @override
  String get mediaKindUnknown => 'Keine';

  @override
  String get mediaKindBook => 'Buch';

  @override
  String get mediaKindCd => 'CD';

  @override
  String get mediaKindDvd => 'DVD';

  @override
  String get mediaKindBluRay => 'Blu-ray';

  @override
  String get mediaKindBoardGame => 'Brettspiel';

  @override
  String get dataTypeString => 'Text (mehrzeilig)';

  @override
  String get dataTypeShortString => 'Text';

  @override
  String get dataTypeDate => 'Datum';

  @override
  String get dataTypeTime => 'Uhrzeit';

  @override
  String get dataTypeInteger => 'Ganzzahl';

  @override
  String get dataTypeFloat => 'Dezimalzahl';

  @override
  String get dataTypeDuration => 'Dauer';

  @override
  String get dataTypeTable => 'Liste';

  @override
  String get dataTypeIntegerRanges => 'Zahlenbereiche';

  @override
  String get navSettings => 'Einstellungen';

  @override
  String get loginSubtitle => 'Bibliotheksverwaltung';

  @override
  String get loginSearching => 'Suche läuft…';

  @override
  String get loginNoServers => 'Keine Server gefunden';

  @override
  String get loginDiscoveryUnavailable => 'Erkennung nicht verfügbar';

  @override
  String get loginDiscoverOnLan => 'Im Netzwerk suchen';

  @override
  String get loginRefresh => 'Aktualisieren';

  @override
  String get loginEnterUrlManually => 'URL manuell eingeben';

  @override
  String get loginBackToDiscovery => 'Zurück zur Suche';

  @override
  String get loginServerUrl => 'Server-URL';

  @override
  String get loginServerUrlHint => 'https://colman.example.com';

  @override
  String get loginUsername => 'Benutzername';

  @override
  String get loginPassword => 'Passwort';

  @override
  String get loginRequired => 'Pflichtfeld';

  @override
  String get loginSignIn => 'Anmelden';

  @override
  String loginFailed(String error) {
    return 'Anmeldung fehlgeschlagen: $error';
  }

  @override
  String get itemsTitle => 'Medien suchen';

  @override
  String get itemsSearchHint => 'Nach Name, Stichwort suchen…';

  @override
  String get itemsEmptyPrompt => 'Suchbegriff oben eingeben.';

  @override
  String get itemsNotFound => 'Keine Medien gefunden.';

  @override
  String errorPrefix(String error) {
    return 'Fehler: $error';
  }

  @override
  String get errorDetails => 'Details';

  @override
  String get errorConnectionFailed => 'Keine Verbindung zum Server möglich.';

  @override
  String get errorAccessDenied =>
      'Sie haben keine Berechtigung für diese Aktion.';

  @override
  String get errorInvalidCredentials => 'Benutzername oder Passwort falsch.';

  @override
  String get errorServerError => 'Der Server hat einen Fehler gemeldet.';

  @override
  String get errorRequestFailed => 'Die Anfrage ist fehlgeschlagen.';

  @override
  String errorIncompatibleServer(String serverVersion, String requiredVersion) {
    return 'Dieser Server ist mit dieser App nicht kompatibel (Server $serverVersion, App benötigt $requiredVersion). Bitte aktualisieren Sie die App oder den Server.';
  }

  @override
  String get retry => 'Erneut versuchen';

  @override
  String get itemDetailFallbackTitle => 'Medium';

  @override
  String get labelName => 'Name';

  @override
  String get labelType => 'Medientyp';

  @override
  String get labelPlace => 'Standort';

  @override
  String get labelBarcode => 'Colman-Barcode';

  @override
  String get labelDescription => 'Beschreibung';

  @override
  String get labelAvailability => 'Verfügbarkeit';

  @override
  String availabilityOf(int available, int count) {
    return '$available von $count verfügbar';
  }

  @override
  String get lendingTitle => 'Ausleihe';

  @override
  String get lendingComingSoon => 'Ausleihe – kommt bald';

  @override
  String get lendingTabNew => 'Neu';

  @override
  String get lendingTabOpen => 'Offen';

  @override
  String get lendingTabDue => 'Fällig';

  @override
  String get lendingStepPerson => 'Person';

  @override
  String get lendingStepItem => 'Medium';

  @override
  String get lendingStepDetails => 'Details';

  @override
  String get lendingStep1Title => 'Benutzer auswählen';

  @override
  String get lendingStep2Title => 'Medium auswählen';

  @override
  String get lendingStep3Title => 'Ausleihdetails';

  @override
  String get lendingItemSearchHint => 'Nach Name suchen…';

  @override
  String get lendingItemNotFound => 'Keine Medien gefunden.';

  @override
  String get lendingItemSearchPrompt => 'Medienname eingeben.';

  @override
  String lendingSuccessLendId(int id) {
    return 'Ausleihe #$id erstellt';
  }

  @override
  String lendingSuccessReserveId(int id) {
    return 'Reservierung #$id erstellt';
  }

  @override
  String get lendingNewLending => 'Neue Ausleihe';

  @override
  String get lendingNewItemSamePerson => 'Neues Medium, gleiche Person';

  @override
  String lendingConflictReserved(int count) {
    return 'Nur $count nicht reserviertes Exemplar verfügbar';
  }

  @override
  String get lendingOpenNoResults => 'Keine offenen Ausleihen.';

  @override
  String lendingOpenLentReturned(int lent, int returned) {
    return '$lent ausgeliehen, $returned zurückgegeben';
  }

  @override
  String get lendingDueNoResults => 'Keine fälligen Ausleihen.';

  @override
  String lendingDueOutstanding(int count) {
    return '$count ausstehend';
  }

  @override
  String get returnTitle => 'Rückgabe';

  @override
  String get returnComingSoon => 'Rückgabe – kommt bald';

  @override
  String get returnTabNew => 'Neu';

  @override
  String get returnTabOpen => 'Offen';

  @override
  String get returnStepLending => 'Ausleihe';

  @override
  String get returnStepDetails => 'Details';

  @override
  String get returnStep1Title => 'Medium auswählen';

  @override
  String get returnStep2Title => 'Ausleihe auswählen';

  @override
  String get returnStep3Title => 'Rückgabedetails';

  @override
  String get returnStep2NoResults =>
      'Keine offenen Ausleihen für dieses Medium.';

  @override
  String get returnOpenNoResults => 'Keine offenen Ausleihen.';

  @override
  String returnSuccessId(int id) {
    return 'Rückgabe #$id gespeichert';
  }

  @override
  String get returnNewReturn => 'Neue Rückgabe';

  @override
  String get returnSameItem => 'Gleiches Medium';

  @override
  String get acquisitionTitle => 'Anschaffung';

  @override
  String get acquisitionComingSoon => 'Anschaffung – kommt bald';

  @override
  String get acquisitionStep1Title => 'Medium auswählen';

  @override
  String get acquisitionStep2Title => 'Anschaffungsdetails';

  @override
  String acquisitionSuccessId(int id) {
    return 'Anschaffung #$id gespeichert';
  }

  @override
  String get acquisitionNewAcquisition => 'Neue Anschaffung';

  @override
  String get acquisitionCreateItemTitle => 'Neues Medium';

  @override
  String get acquisitionCreateItemButton => 'Neues Medium anlegen';

  @override
  String get acquisitionBarcodeHint => 'Barcode scannen oder eingeben';

  @override
  String get acquisitionBarcodeSearchingInternet => 'Suche im Internet…';

  @override
  String get acquisitionBarcodeNotFound => 'Barcode nicht gefunden';

  @override
  String get barcodeScanTitle => 'Barcode scannen';

  @override
  String get barcodeScanHint => 'Kamera auf Barcode halten';

  @override
  String get barcodeScanTooltip => 'Mit Kamera scannen';

  @override
  String get liquidationTitle => 'Aussonderung';

  @override
  String get liquidationComingSoon => 'Aussonderung – kommt bald';

  @override
  String get liquidationStep1Title => 'Medium auswählen';

  @override
  String get liquidationStep2Title => 'Aussonderungsdetails';

  @override
  String liquidationSuccessId(int id) {
    return 'Aussonderung #$id gespeichert';
  }

  @override
  String get liquidationNewLiquidation => 'Neue Aussonderung';

  @override
  String get historyTitle => 'Verlauf';

  @override
  String get historyComingSoon => 'Verlauf – kommt bald';

  @override
  String get personsTitle => 'Benutzer';

  @override
  String get personsComingSoon => 'Benutzer – kommt bald';

  @override
  String get classesTitle => 'Klassen';

  @override
  String get classesComingSoon => 'Klassen – kommt bald';

  @override
  String get classesNoResults => 'Keine Klassen vorhanden.';

  @override
  String get classesAddTooltip => 'Neue Klasse';

  @override
  String get classesAddTitle => 'Neue Klasse';

  @override
  String get classesAddNameLabel => 'Klassenname';

  @override
  String get classesRenameTitle => 'Klasse umbenennen';

  @override
  String get classesRenameNewNameLabel => 'Neuer Name';

  @override
  String classesDeleteConfirm(String name) {
    return 'Klasse \'$name\' löschen?';
  }

  @override
  String get classesGradeRange => 'Jahrgangsstufen';

  @override
  String get classesGradeRangeMin => 'Min';

  @override
  String get classesGradeRangeMax => 'Max';

  @override
  String get classesNewSchoolYear => 'Neues Schuljahr';

  @override
  String get classesNewSchoolYearStep1 => 'Klassenumbenennung';

  @override
  String get classesNewSchoolYearStep2 => 'Betroffene Personen';

  @override
  String get classesNewSchoolYearDissolved => '(aufgelöst)';

  @override
  String get classesNewSchoolYearAddClass => 'Klasse hinzufügen';

  @override
  String get classesNewSchoolYearNoPersons => 'Keine betroffenen Personen.';

  @override
  String get classesNewSchoolYearNoClass => '(keine Klasse)';

  @override
  String get classesNewSchoolYearDeletePerson => 'Person löschen';

  @override
  String get classesApply => 'Anwenden';

  @override
  String get classesNext => 'Weiter';

  @override
  String get settingsTitle => 'Einstellungen';

  @override
  String get settingsComingSoon => 'Einstellungen – kommt bald';

  @override
  String get settingsInternetServicesSection => 'Internetdienste';

  @override
  String get settingsNetworkSection => 'Netzwerk';

  @override
  String get settingsSruServersSection => 'SRU-Bibliotheksserver';

  @override
  String get settingsOpenLibrary => 'Open Library';

  @override
  String get settingsGoogleBooks => 'Google Books';

  @override
  String get settingsGoogleApisKey => 'API-Schlüssel';

  @override
  String get settingsMusicBrainz => 'MusicBrainz';

  @override
  String get settingsBgg => 'BoardGameGeek';

  @override
  String get settingsBggToken => 'BGG-Token';

  @override
  String get settingsOpenGtindb => 'OpenGTINdb';

  @override
  String get settingsOpenGtindbUserId => 'Benutzer-ID';

  @override
  String get settingsDownloadThumbnails => 'Cover-Bilder herunterladen';

  @override
  String get settingsTimeoutSeconds => 'Zeitlimit (Sek.)';

  @override
  String get settingsPollIntervalSeconds => 'Aktualisierungsintervall (Sek.)';

  @override
  String get settingsPollIntervalHint =>
      'Wie oft die App den Server auf Änderungen prüft (5–300 Sek.).';

  @override
  String get settingsSruName => 'Name';

  @override
  String get settingsSruEndpoint => 'Endpunkt';

  @override
  String get settingsSruIsbnField => 'ISBN-Feld';

  @override
  String get settingsSruEanField => 'EAN-Feld';

  @override
  String get settingsSruSchema => 'Schema';

  @override
  String get settingsSruAddServer => 'Server hinzufügen';

  @override
  String get settingsSruDeleteServer => 'Server löschen';

  @override
  String get settingsSruEditServer => 'Server bearbeiten';

  @override
  String get settingsSave => 'Speichern';

  @override
  String get settingsRevert => 'Zurücksetzen';

  @override
  String get settingsSaved => 'Einstellungen gespeichert';

  @override
  String get serverUnreachableBanner =>
      'Server nicht erreichbar – die angezeigten Daten sind möglicherweise veraltet.';

  @override
  String get dismiss => 'Schließen';

  @override
  String get personDetailFallbackTitle => 'Benutzer';

  @override
  String personDetailComingSoon(int id) {
    return 'Benutzer $id – kommt bald';
  }

  @override
  String get labelSignatureTop => 'Signatur (oben)';

  @override
  String get labelSignatureBottom => 'Signatur (unten)';

  @override
  String get labelKeywords => 'Stichworte';

  @override
  String get labelSubject => 'Fach';

  @override
  String get labelAcademicYear => 'Schuljahr';

  @override
  String get itemsAdvancedSearch => 'Erweiterte Suche';

  @override
  String get labelAllTypes => 'Alle Medientypen';

  @override
  String get labelCount => 'Anzahl';

  @override
  String get labelExpiryDate => 'Läuft ab am';

  @override
  String get labelDueDate => 'Fälligkeitsdatum';

  @override
  String get labelPerson => 'Person';

  @override
  String get labelPersonSearch => 'Person suchen…';

  @override
  String get actionReserve => 'Reservieren';

  @override
  String get actionLend => 'Ausleihen';

  @override
  String get actionCancelReservation => 'Reservierung stornieren';

  @override
  String get actionEditItem => 'Gegenstand bearbeiten';

  @override
  String get actionDeleteItem => 'Gegenstand löschen';

  @override
  String deleteItemConfirmMessage(String name) {
    return '\"$name\" und alle zugehörigen vergangenen Anschaffungen, Aussortierungen, Ausleihen und Rückgaben endgültig löschen?\n\nDies kann nicht rückgängig gemacht werden.';
  }

  @override
  String get itemDeleted => 'Gegenstand gelöscht.';

  @override
  String get itemsScanNotFound => 'Kein Gegenstand zu diesem Barcode gefunden';

  @override
  String get itemEditKeywordsHint => 'kommagetrennt';

  @override
  String get itemSaved => 'Gegenstand gespeichert.';

  @override
  String get reserveDialogTitle => 'Medium reservieren';

  @override
  String get reserveSelfHint =>
      'Das Medium wird auf den eigenen Namen reserviert.';

  @override
  String get lendDialogTitle => 'Medium ausleihen';

  @override
  String get cancelReservationDialogTitle => 'Reservierung stornieren';

  @override
  String get actionConfirm => 'Bestätigen';

  @override
  String get reservationCreated => 'Reservierung erstellt';

  @override
  String get lendingCreated => 'Ausleihe erstellt';

  @override
  String get reservationCancelled => 'Reservierung storniert';

  @override
  String get errorAlreadyReserved =>
      'Eine Reservierung für diese Person existiert bereits';

  @override
  String get errorNotEnoughAvailable => 'Nicht genügend Exemplare verfügbar';

  @override
  String get errorReservationsDisabled =>
      'Reservierungen für dieses Medium sind deaktiviert';

  @override
  String get noActiveReservations => 'Keine aktiven Reservierungen';

  @override
  String get selectReservationToCancel =>
      'Reservierung zum Stornieren auswählen';

  @override
  String get historyFromDate => 'Von';

  @override
  String get historyToDate => 'Bis';

  @override
  String get historyApply => 'Anwenden';

  @override
  String get historyShowAll => 'Alles anzeigen';

  @override
  String get historyFilterAllActions => 'Alle Aktionen';

  @override
  String get extendDueDateButton => 'Frist verlängern';

  @override
  String get extendDueDateTitle => 'Frist verlängern';

  @override
  String get extendDueDateAction => 'Verlängern';

  @override
  String get extendDueDateNoLendings =>
      'Diese Person hat keine offenen Ausleihen.';

  @override
  String get extendDueDateSelectHint =>
      'Wähle die Ausleihen aus, deren Frist geändert werden soll:';

  @override
  String get extendDueDateNewDate => 'Neues Fälligkeitsdatum';

  @override
  String extendDueDateFixedHint(int count) {
    String _temp0 = intl.Intl.pluralLogic(
      count,
      locale: localeName,
      other: '$count Ausleihen werden verlängert:',
      one: '1 Ausleihe wird verlängert:',
    );
    return '$_temp0';
  }

  @override
  String openLendingsSelectedCount(int count) {
    return '$count ausgewählt';
  }

  @override
  String get openLendingsSelectAll => 'Alle auswählen';

  @override
  String get openLendingsDeselectAll => 'Auswahl aufheben';

  @override
  String get openLendingsSelectHint =>
      'Wähle die Ausleihen aus, deren Frist verlängert werden soll.';

  @override
  String openLendingsExtendSelected(int count) {
    return 'Verlängern ($count)';
  }

  @override
  String historyActionsShown(int count) {
    return '$count Aktion(en) angezeigt';
  }

  @override
  String get historyLoadMore => 'Mehr laden';

  @override
  String get historyNoResults => 'Keine Einträge gefunden.';

  @override
  String get historyActionAcquisition => 'Anschaffung';

  @override
  String get historyActionLiquidation => 'Aussonderung';

  @override
  String get historyActionLending => 'Ausleihe';

  @override
  String get historyActionReturn => 'Rückgabe';

  @override
  String get historyActionReservation => 'Reservierung';

  @override
  String get historyLabelDateTime => 'Datum/Uhrzeit';

  @override
  String get historyLabelAction => 'Aktion';

  @override
  String get historyLabelItem => 'Medium';

  @override
  String get historyLabelUser => 'Person';

  @override
  String get historyLabelCollector => 'Erfasser';

  @override
  String get historyLabelRemarks => 'Anmerkungen';

  @override
  String get historyDetailTitle => 'Aktionsdetails';

  @override
  String get historyDeleteTitle => 'Aktion löschen';

  @override
  String get historyDeleteConfirm => 'Diese Aktion endgültig löschen?';

  @override
  String historyDeleteWithConflicts(int n) {
    return 'Das Löschen dieser Aktion erfordert auch das Löschen von $n abhängigen Aktion(en):';
  }

  @override
  String get historyDeleteAll => 'Alle löschen';

  @override
  String get historyDeleteSuccess => 'Aktion(en) gelöscht.';

  @override
  String get historyDeleteFailed => 'Aktion konnte nicht gelöscht werden.';

  @override
  String get historyEditTitle => 'Aktion bearbeiten';

  @override
  String get historyEditSave => 'Speichern';

  @override
  String get historyEditSuccess => 'Aktion aktualisiert.';

  @override
  String get historyEditFailed => 'Aktion konnte nicht aktualisiert werden.';

  @override
  String historyChangeConflicts(int n) {
    return 'Diese Änderung betrifft $n weitere Aktion(en). Fortfahren?';
  }

  @override
  String get historyChangeConflictsTitle => 'Abhängige Aktionen';

  @override
  String get personsSearchHint => 'Nach Name suchen…';

  @override
  String get personsNoResults => 'Keine Personen gefunden.';

  @override
  String get personsAddTooltip => 'Neue Person';

  @override
  String get personCreateTitle => 'Neue Person';

  @override
  String get personSectionDetails => 'Personendaten';

  @override
  String get personFormLastname => 'Nachname';

  @override
  String get personFormFirstname => 'Vorname';

  @override
  String get personFormTitle => 'Titel';

  @override
  String get personFormSchoolClass => 'Klasse';

  @override
  String get personFormBarcode => 'Barcode';

  @override
  String get personFormEmail => 'E-Mail (durch Komma getrennt)';

  @override
  String get personFormTelephone => 'Telefon';

  @override
  String get personFormStreet => 'Straße';

  @override
  String get personFormZip => 'PLZ';

  @override
  String get personFormCity => 'Ort';

  @override
  String get personFormCountry => 'Land';

  @override
  String get personFormRemarks => 'Anmerkungen';

  @override
  String get personFormExternalId => 'Externe ID (UUID)';

  @override
  String get personSave => 'Speichern';

  @override
  String get personSaved => 'Person gespeichert.';

  @override
  String get personConflict =>
      'Konflikt — der Datensatz wurde extern geändert. Bitte neu laden.';

  @override
  String get personSaveFailed => 'Person konnte nicht gespeichert werden.';

  @override
  String get personDelete => 'Löschen';

  @override
  String get personDeleteConfirm => 'Diese Person endgültig löschen?';

  @override
  String get personDeleteFailed =>
      'Löschen nicht möglich — bitte zuerst alle offenen Ausleihen zurückgeben.';

  @override
  String get personDeleteSuccess => 'Person gelöscht.';

  @override
  String get personLoginSection => 'Anmeldung';

  @override
  String get personUsernameLabel => 'Benutzername';

  @override
  String get personUsernameNotSet => 'Kein Benutzername gesetzt';

  @override
  String get personUsernameSetButton => 'Setzen';

  @override
  String get personUsernameClearButton => 'Entfernen';

  @override
  String get personUsernameSaved => 'Benutzername gespeichert.';

  @override
  String get personUsernameRemoved => 'Benutzername entfernt.';

  @override
  String get personUsernameTaken => 'Benutzername bereits vergeben.';

  @override
  String get personPasswordSection => 'Passwort';

  @override
  String get personPasswordIsSet => 'Passwort ist gesetzt';

  @override
  String get personPasswordNotSet => 'Kein Passwort gesetzt';

  @override
  String get personPasswordSetButton => 'Passwort setzen';

  @override
  String get personPasswordRemoveButton => 'Passwort entfernen';

  @override
  String get personPasswordSetTitle => 'Passwort setzen';

  @override
  String get personPasswordNew => 'Neues Passwort';

  @override
  String get personPasswordConfirm => 'Bestätigen';

  @override
  String get personPasswordMismatch => 'Passwörter stimmen nicht überein.';

  @override
  String get personPasswordSaved => 'Passwort gespeichert.';

  @override
  String get personPasswordRemoved => 'Passwort entfernt.';

  @override
  String get navChangePassword => 'Passwort ändern';

  @override
  String get navChangePasswords => 'Passwörter verwalten';

  @override
  String get changePasswordTitle => 'Passwort ändern';

  @override
  String get changePasswordCurrent => 'Aktuelles Passwort';

  @override
  String get changePasswordWrong => 'Falsches Passwort.';

  @override
  String get changePasswordSubmit => 'Passwort ändern';

  @override
  String get changePasswordsTitle => 'Passwörter verwalten';

  @override
  String get changePasswordsSelectPrompt =>
      'Person auswählen, um deren Zugangsdaten zu verwalten.';

  @override
  String get changePasswordsNoUsername => 'Kein Benutzername gesetzt';

  @override
  String changePasswordsDeleteConfirm(String name) {
    return 'Passwort für $name löschen?';
  }

  @override
  String get personPermissionsSection => 'Berechtigungen';

  @override
  String get personPermissionsApply => 'Übernehmen';

  @override
  String get personPermissionsSaved => 'Berechtigungen gespeichert.';

  @override
  String get permEditPersons => 'Personen bearbeiten';

  @override
  String get permCreateLenders => 'Ausleihende erstellen';

  @override
  String get permCreateDataCollectors => 'Datenerfasser erstellen';

  @override
  String get permCollectLendings => 'Ausleihen erfassen';

  @override
  String get permCollectReturns => 'Rückgaben erfassen';

  @override
  String get permCollectAcquisitions => 'Anschaffungen erfassen';

  @override
  String get permCollectLiquidations => 'Aussonderungen erfassen';

  @override
  String get permViewHistory => 'Verlauf anzeigen';

  @override
  String get permViewPersons => 'Personenlisten anzeigen';

  @override
  String get permViewItems => 'Medienlisten anzeigen';

  @override
  String get permEditPreferences => 'Einstellungen bearbeiten';

  @override
  String get permPrintIds => 'Ausweise drucken';

  @override
  String get permPrintBarcodes => 'Barcodes drucken';

  @override
  String get permEditCategories => 'Kategorien bearbeiten';

  @override
  String get permViewPersonalData => 'Persönliche Daten anzeigen';

  @override
  String get permManageLabelLayouts => 'Etikettenlayouts verwalten';

  @override
  String get permCalibratePrinters => 'Drucker kalibrieren';

  @override
  String get permEditIdLayout => 'Ausweislayout bearbeiten';

  @override
  String get permPrintSignatures => 'Signaturschilder drucken';

  @override
  String get permChangeActions => 'Aktionen ändern und löschen';

  @override
  String get permEditProgramSettings => 'Programm-Einstellungen bearbeiten';

  @override
  String get permManageClasses => 'Klassen verwalten';

  @override
  String get permReserve => 'Medien reservieren';

  @override
  String get permEditReservations => 'Reservierungen bearbeiten';

  @override
  String get permAdminPermissions => 'Item-Einstellungen bearbeiten';

  @override
  String get barcodeDispatchTitle => 'Scan-Ergebnis';

  @override
  String get barcodeDispatchLoading => 'Barcode wird gesucht…';

  @override
  String barcodeDispatchPersonSubtitle(String name) {
    return 'Aktionen für $name';
  }

  @override
  String barcodeDispatchItemSubtitle(String name) {
    return 'Aktionen für: $name';
  }

  @override
  String get barcodeDispatchActionViewLendings =>
      'Offene Ausleihen & Reservierungen';

  @override
  String get barcodeDispatchActionEditPerson => 'Person bearbeiten';

  @override
  String get barcodeDispatchActionReturnPersonItems =>
      'Gegenstände zurückgeben';

  @override
  String get barcodeDispatchActionLendToPerson => 'Gegenstand ausleihen';

  @override
  String get barcodeDispatchActionReturnItem => 'Diesen Gegenstand zurückgeben';

  @override
  String get barcodeDispatchActionLendItem => 'Diesen Gegenstand ausleihen';

  @override
  String get barcodeDispatchActionAcquireItem => 'Anschaffen / bearbeiten';

  @override
  String get barcodeDispatchActionLiquidateItem => 'Gegenstand aussondern';

  @override
  String get barcodeDispatchNotRecognized => 'Barcode nicht erkannt';

  @override
  String get barcodeDispatchNoOpenLendings => 'Keine offenen Ausleihen';

  @override
  String get thumbnailFromFile => 'Aus Datei';

  @override
  String get thumbnailSearchAmazon => 'Bei Amazon suchen';

  @override
  String get thumbnailRemove => 'Entfernen';

  @override
  String get thumbnailNone => 'Kein Titelbild';

  @override
  String get amazonBrowserHint => 'Auf Titelbild tippen um es zu übernehmen';

  @override
  String get amazonBrowserDownloadError => 'Bild konnte nicht geladen werden';

  @override
  String get certDialogTitle => 'Nicht vertrauenswürdiges Zertifikat';

  @override
  String certDialogBody(
    String host,
    String subject,
    String validFrom,
    String validUntil,
    String fingerprint,
  ) {
    return 'Der Server \"$host\" verwendet ein Zertifikat, das nicht verifiziert werden konnte.\n\nBetreff: $subject\nGültig: $validFrom – $validUntil\nSHA-256:\n$fingerprint\n\nDiesem Server vertrauen?';
  }

  @override
  String get certDialogTrust => 'Vertrauen';

  @override
  String get certDialogCancel => 'Abbrechen';

  @override
  String get titleSearchTitle => 'Titelsuche';

  @override
  String get titleSearchButton => 'Im Internet nach Titel suchen';

  @override
  String titleSearchOfferButton(String query) {
    return 'Im Internet nach \"$query\" suchen';
  }

  @override
  String get titleSearchFieldHint => 'Titel…';

  @override
  String get titleSearchKindBook => 'Buch';

  @override
  String get titleSearchKindCd => 'CD';

  @override
  String get titleSearchKindDvd => 'DVD';

  @override
  String get titleSearchKindBluray => 'Blu-ray';

  @override
  String get titleSearchKindBoardGame => 'Gesellschaftsspiel';

  @override
  String get titleSearchEmptyPrompt => 'Titel eingeben und Suche starten.';

  @override
  String titleSearchNoResults(String query) {
    return 'Keine Ergebnisse für \"$query\" gefunden.';
  }

  @override
  String get titleSearchDetailFailed =>
      'Details konnten nicht aus der gewählten Quelle geladen werden.';

  @override
  String get settingsSruTitleField => 'Titelfeld';

  @override
  String get navIdCardLayout => 'Ausweis-Layout';

  @override
  String get navIdCardPrint => 'Ausweise drucken';

  @override
  String get idCardLayoutTitle => 'Ausweis-Layout';

  @override
  String get idCardLayoutCardSize => 'Kartengröße';

  @override
  String get idCardLayoutWidthMm => 'Breite (mm)';

  @override
  String get idCardLayoutHeightMm => 'Höhe (mm)';

  @override
  String get idCardLayoutUseLogo => 'Logo verwenden';

  @override
  String get idCardLayoutMarginMm => 'Freier Rand (mm)';

  @override
  String get idCardLayoutZoneBarcode => 'Barcode';

  @override
  String get idCardLayoutZoneText => 'Text';

  @override
  String get idCardLayoutZoneLogo => 'Logo';

  @override
  String get idCardLayoutRotate => 'Um 90° drehen';

  @override
  String get idCardLayoutMaximize => 'Maximieren';

  @override
  String get idCardLayoutAlignLeft => 'Linksbündig ausrichten';

  @override
  String get idCardLayoutAlignHCenter => 'Horizontal zentrieren';

  @override
  String get idCardLayoutAlignRight => 'Rechtsbündig ausrichten';

  @override
  String get idCardLayoutAlignTop => 'Oben ausrichten';

  @override
  String get idCardLayoutAlignVCenter => 'Vertikal zentrieren';

  @override
  String get idCardLayoutAlignBottom => 'Unten ausrichten';

  @override
  String get idCardLayoutRotateBlocked =>
      'Die Zone passt nach dem Drehen nicht mehr auf die Karte.';

  @override
  String get idCardLayoutSave => 'Speichern';

  @override
  String get idCardLayoutReset => 'Zurücksetzen';

  @override
  String get idCardLayoutSaved => 'Layout gespeichert.';

  @override
  String get idCardSampleName => 'Max Mustermann';

  @override
  String idCardIssuedOn(String date) {
    return 'ausgestellt am: $date';
  }

  @override
  String get idCardPrintTitle => 'Ausweise drucken';

  @override
  String get idCardPrintSearchHint => 'Personen suchen';

  @override
  String get idCardPrintSelectAll => 'Alle auswählen';

  @override
  String get idCardPrintDeselectAll => 'Auswahl aufheben';

  @override
  String idCardPrintSelectedCount(int count) {
    return '$count ausgewählt';
  }

  @override
  String get idCardPrintButton => 'Drucken';

  @override
  String get idCardPrintNoSelection =>
      'Bitte mindestens eine Person auswählen.';

  @override
  String get idCardPrintNoLayout => 'Kein Ausweis-Layout konfiguriert.';

  @override
  String get idCardPrintArrangement => 'Etiketten-Anordnung';

  @override
  String get idCardPrintPaperSize => 'Papiergröße';

  @override
  String get idCardPrintOrientation => 'Ausrichtung';

  @override
  String get idCardPrintPortrait => 'Hochformat';

  @override
  String get idCardPrintLandscape => 'Querformat';

  @override
  String get idCardPrintColumns => 'Spalten';

  @override
  String get idCardPrintRows => 'Zeilen';

  @override
  String get idCardPrintMarginLeft => 'Rand links (mm)';

  @override
  String get idCardPrintMarginTop => 'Rand oben (mm)';

  @override
  String get idCardPrintGapH => 'Abstand horizontal (mm)';

  @override
  String get idCardPrintGapV => 'Abstand vertikal (mm)';

  @override
  String get idCardPrintDrawBorder => 'Rahmen um jede Karte zeichnen';

  @override
  String get navThermalLabelPrint => 'Etiketten drucken';

  @override
  String get navLabelPrint => 'Etiketten drucken';

  @override
  String get navLabelPresets => 'Etiketten-Layouts';

  @override
  String get navSignaturePrint => 'Signaturen drucken';

  @override
  String get signaturePrintTitle => 'Signaturen drucken';

  @override
  String get signaturePrintTop => 'Signatur oben';

  @override
  String get signaturePrintBottom => 'Signatur unten';

  @override
  String get signaturePrintCount => 'Anzahl';

  @override
  String get signaturePrintButton => 'Drucken';

  @override
  String get signaturePrintPickItem =>
      'Artikel zur Vorbelegung der Signaturen wählen (optional)';

  @override
  String get signaturePrintEmpty => 'Zuerst eine Signatur eingeben.';

  @override
  String get labelPrintTitle => 'Etiketten drucken';

  @override
  String get labelPrintButton => 'Drucken';

  @override
  String get labelPrintNext => 'Weiter: Plätze';

  @override
  String get labelPrintSearchHint => 'Artikel suchen';

  @override
  String get labelPrintCopies => 'Kopien pro Artikel';

  @override
  String get labelPrintSelectAll => 'Alle auswählen';

  @override
  String get labelPrintDeselectAll => 'Auswahl aufheben';

  @override
  String labelPrintSelectedCount(int count) {
    return '$count ausgewählt';
  }

  @override
  String get labelPrintNoItems => 'Keine Artikel gefunden.';

  @override
  String get labelPrintNoSelection => 'Mindestens einen Artikel auswählen.';

  @override
  String get labelPrintNoPrinter => 'Kein USB-Drucker verbunden.';

  @override
  String get labelPrintPrinterConnected => 'Über USB verbunden';

  @override
  String get labelPrintRefresh => 'Erneut nach Drucker suchen';

  @override
  String get labelPrintPreset => 'Layout';

  @override
  String get labelPrintPositions => 'Etikettenplätze auf dem Bogen';

  @override
  String get labelPrintPositionsHint =>
      'Wähle die zu nutzenden Plätze auf dem Bogen — höchstens so viele wie ausgewählte Artikel.';

  @override
  String labelPrintCellsSelected(int count, int limit) {
    return '$count von $limit Plätzen ausgewählt';
  }

  @override
  String get labelPrintTooManyCells =>
      'Es können nicht mehr Plätze als ausgewählte Artikel gewählt werden.';

  @override
  String labelPrintRemaining(int count) {
    return '$count Artikel verbleiben — Plätze für den nächsten Bogen wählen.';
  }

  @override
  String get labelPrintNoCells => 'Mindestens einen Etikettenplatz auswählen.';

  @override
  String get labelPrintPermissionDenied => 'USB-Berechtigung verweigert.';

  @override
  String labelPrintSent(int count) {
    return '$count Etikett(en) an den Drucker gesendet.';
  }

  @override
  String get cancel => 'Abbrechen';

  @override
  String get ok => 'OK';

  @override
  String get signatureSample => 'Bsp 4';

  @override
  String get labelPresetTitle => 'Etiketten-Layouts';

  @override
  String get labelPresetPreset => 'Layout';

  @override
  String get labelPresetNew => 'Neues Layout';

  @override
  String get labelPresetRename => 'Umbenennen';

  @override
  String get labelPresetDelete => 'Löschen';

  @override
  String get labelPresetName => 'Name';

  @override
  String get labelPresetPurpose => 'Verwendung';

  @override
  String get labelPresetPrinter => 'Drucker';

  @override
  String get labelPresetBarWidthReduction => 'Balkenbreiten-Reduktion (mm)';

  @override
  String get labelBarcodeUnit => 'Barcode-Strichbreite (mm)';

  @override
  String get labelPresetEditArrangement => 'Etiketten-Layout bearbeiten';

  @override
  String get labelPresetArrangement => 'Etiketten-Layout';

  @override
  String get labelPresetSave => 'Speichern';

  @override
  String get labelPresetSaved => 'Layout gespeichert.';

  @override
  String get labelPresetDarkness => 'Schwärze';

  @override
  String get labelPresetGap => 'Spalt (mm)';

  @override
  String get labelPresetXOffset => 'Horizontaler Versatz (mm)';

  @override
  String get labelPresetColumns => 'Spalten';

  @override
  String get labelPresetRows => 'Zeilen';

  @override
  String get labelPresetLeftMargin => 'Rand links (mm)';

  @override
  String get labelPresetTopMargin => 'Rand oben (mm)';

  @override
  String get labelPresetHGap => 'Abstand horizontal (mm)';

  @override
  String get labelPresetVGap => 'Abstand vertikal (mm)';

  @override
  String get labelPresetPaperSize => 'Papiergröße';

  @override
  String get labelPresetLandscape => 'Querformat';

  @override
  String get labelPurposeBookLabel => 'Buchetikett';

  @override
  String get labelPurposeSignature => 'Signatur';

  @override
  String get labelPurposeIdCard => 'Ausweis';

  @override
  String get labelPrinterThermal => 'USB-Thermodrucker';

  @override
  String get labelPrinterSheet => 'Normaler/Bogendrucker';

  @override
  String get navBookLabelLayout => 'Buchetiketten-Layout';

  @override
  String get bookLabelLayoutTitle => 'Buchetiketten-Layout';

  @override
  String get bookLabelLayoutSaved => 'Buchetiketten-Layout gespeichert.';

  @override
  String get bookLabelSampleOrganization => 'Beispiel-Bücherei';

  @override
  String get bookLabelSampleTitle => 'Beispieltitel';

  @override
  String get bookLabelSampleAuthor => 'Beispielautor';

  @override
  String get thermalPrintTitle => 'Artikel-Etiketten drucken';

  @override
  String get thermalPrintButton => 'Auf USB-Drucker drucken';

  @override
  String get thermalPrintSystemButton => 'Auf normalem Drucker drucken';

  @override
  String get thermalPrintSearchHint => 'Artikel suchen';

  @override
  String get thermalPrintSelectAll => 'Alle auswählen';

  @override
  String get thermalPrintDeselectAll => 'Auswahl aufheben';

  @override
  String thermalPrintSelectedCount(int count) {
    return '$count ausgewählt';
  }

  @override
  String get thermalPrintNoItems => 'Keine Artikel gefunden.';

  @override
  String get thermalPrintNoSelection => 'Mindestens einen Artikel auswählen.';

  @override
  String get thermalPrintNoPrinter => 'Kein USB-Drucker verbunden.';

  @override
  String get thermalPrintPrinterConnected => 'Über USB verbunden';

  @override
  String get thermalPrintRefresh => 'Erneut nach Drucker suchen';

  @override
  String get thermalPrintLabelSettings => 'Etiketten- & Druckereinstellungen';

  @override
  String get thermalPrintWidthMm => 'Etikettenbreite (mm)';

  @override
  String get thermalPrintHeightMm => 'Etikettenhöhe (mm)';

  @override
  String get thermalPrintGapMm => 'Abstand zwischen Etiketten (mm)';

  @override
  String get thermalPrintXOffsetMm => 'Horizontaler Versatz (mm)';

  @override
  String get thermalPrintDarkness => 'Schwärze';

  @override
  String get thermalPrintPermissionDenied => 'USB-Berechtigung verweigert.';

  @override
  String thermalPrintSent(int count) {
    return '$count Etikett(en) an den Drucker gesendet.';
  }

  @override
  String get thermalPrintAndroidOnly =>
      'Direkter USB-Etikettendruck ist nur unter Android verfügbar.';

  @override
  String get calibrationButton => 'Drucker kalibrieren';

  @override
  String get calibrationTitle => 'Drucker kalibrieren';

  @override
  String get calibrationIntro =>
      'Korrigiere die Druckposition eines normalen (Bogen-)Druckers. Drucke das Kalibrierblatt mit 100 % (ohne Seitenanpassung), miss die drei Kreuze mit einem Lineal und trage die Werte ein. Die Korrektur wird mit diesem Preset gespeichert und bei jedem Bogendruck angewandt.';

  @override
  String get calibrationPrintSheet => 'Kalibrierblatt drucken';

  @override
  String get calibrationMeasureHeading => 'Gemessene Positionen';

  @override
  String get calibrationMeasureHint =>
      'Miss den Abstand von der oberen/linken Blattkante zur Mitte jedes Kreuzes in Millimetern.';

  @override
  String get calibrationMeasureAX => 'Kreuz A – X';

  @override
  String get calibrationMeasureAY => 'Kreuz A – Y';

  @override
  String get calibrationMeasureBX => 'Kreuz B – X';

  @override
  String get calibrationMeasureCY => 'Kreuz C – Y';

  @override
  String get calibrationCompute => 'Versatz berechnen';

  @override
  String get calibrationResultHeading => 'Berechnete Korrektur';

  @override
  String get calibrationOffsetX => 'Versatz X';

  @override
  String get calibrationOffsetY => 'Versatz Y';

  @override
  String get calibrationScaleX => 'Skalierung X';

  @override
  String get calibrationScaleY => 'Skalierung Y';

  @override
  String get calibrationReset => 'Zurücksetzen';

  @override
  String get calibrationSave => 'Speichern';

  @override
  String get calibrationSaved => 'Kalibrierung gespeichert.';
}
