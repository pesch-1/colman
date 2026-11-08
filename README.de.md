[English](README.md) | **Deutsch**

<p align="center">
  <img src="flutter/assets/icons/colman_logo.jpeg" alt="colman-Logo: eine lesende Eule" width="200">
</p>

# colman — Verwaltung für Büchereien und Sammlungen

colman ist eine Desktop-Anwendung zur Verwaltung einer Schulbücherei.
Erfasst werden Medien (Bücher und andere Medien), Ausleiher, Ausleihen,
Rückgaben, Zugänge und Abgänge.  colman ist mit Qt 6 geschrieben und
speichert seine Daten in einer SQLite-Datenbank.

**Plattformen:** colman läuft unter **Linux** (auch auf dem Raspberry Pi);
eine **Windows**-Version gibt es als **Vorschau** (siehe
[INSTALL.md](INSTALL.md#windows-preview), englisch).  Der mobile Client läuft
auf **Android**-Geräten.  macOS wird noch nicht unterstützt.

Ursprünglich wurde colman für die Ausleihe von Klassensätzen entwickelt —
etwa Arbeitshefte, die eine Schule in vielen Exemplaren besitzt.  Mehrere
Exemplare desselben Titels lassen sich in einem Schritt erfassen,
ausleihen, zurücknehmen oder ausscheiden, ohne dass jedes Exemplar einzeln
registriert werden muss.

colman ist nicht auf Schulbüchereien beschränkt: Es eignet sich auch für
private Sammlungen von Büchern, CDs, DVDs, Brettspielen oder anderen
Gegenständen und behält im Blick, was an wen verliehen ist.  Schwerpunkt
und am gründlichsten getestet ist allerdings der Einsatz in der
Schulbücherei; private Sammlungen werden unterstützt, sind aber noch nicht
so ausführlich getestet.

## Funktionen im Überblick

- Geführte Abläufe für Ausleihe, Rückgabe, Zugang und Abgang, auch für
  viele Exemplare auf einmal
- Barcodes für Medien und Leserausweise; Druck von Ausweisen, Etiketten
  und Signaturschildern (auch auf Thermo-Etikettendruckern)
- Übernahme von Titeldaten und Covern aus dem Internet per ISBN oder EAN
  (u. a. Deutsche Nationalbibliothek, Open Library, MusicBrainz,
  BoardGameGeek)
- Reservierungen, Leihfristen und Verlängerungen, Ausleihhistorie
- Klassenverwaltung und Wechsel ins neue Schuljahr
- Fein abgestufte Berechtigungen für Bibliotheksteam, Lehrkräfte und
  Schülerinnen und Schüler
- Optional verschlüsselte Datenbankdatei

## Thermo-Etikettendrucker

colman druckt Etiketten und Signaturschilder auf Thermo-Etikettendruckern
pixelgenau, als TSPL-Druckauftrag direkt über USB.  Bislang sind zwei Modelle
getestet und werden automatisch erkannt:

| Modell | USB-ID | Warnung bei leerer Rolle |
|---|---|---|
| Vevor Y486 | `5958:0486` | ja |
| Munbyn RW403B | `0d28:ccdd` | nein – die Firmware meldet keinen Status |

Nur diese beiden werden angeboten, wenn sie per USB angeschlossen sind, ohne im
Drucksystem eingerichtet zu sein.  Andere in CUPS eingetragene
Etikettendrucker erkennt colman an ihrem Namen und an etikettengroßen
Papierformaten und druckt über CUPS – getestet sind sie aber nicht.

Hilfe beim Einbinden und Testen weiterer Modelle ist hochwillkommen – siehe
[CONTRIBUTING.md](CONTRIBUTING.md#thermal-label-printers) (englisch).  Und wer
dem Autor dafür ein konkretes Druckermodell zur Verfügung stellen möchte,
meldet sich gern über ein Issue :-)

## Betriebsarten

- **Einzelplatz:** Alle Daten liegen in einer lokalen `.col`-Datei; es wird
  weder Server noch Netzwerk benötigt.
- **Mehrplatz:** `colman_rest_server` stellt die Sammlung im Netzwerk
  bereit (z. B. auf einem Raspberry Pi, siehe
  [doc/raspberry_pi.md](doc/raspberry_pi.md)); beliebig viele
  `colman_rest_client`-Arbeitsplätze greifen gleichzeitig darauf zu.
- **App:** Ein Flutter-Client (Verzeichnis [flutter/](flutter/)) bietet die
  wichtigsten Abläufe auf Android-Smartphones und -Tablets, inklusive
  Barcode-Scan mit der Kamera.

Einzelheiten zu Server, Anmeldung (lokale Benutzer, OIDC/JWT,
Barcode-Login) und den Kommandozeilen-Werkzeugen stehen im
[englischen README](README.md).

## Installation

Ausführliche Anleitungen für Debian/Ubuntu und openSUSE stehen in
[INSTALL.md](INSTALL.md).  Kurzfassung für Debian/Ubuntu:

```bash
sudo apt install cmake pkg-config libargon2-dev libssl-dev \
  libzxing-dev qt6-base-dev qt6-svg-dev qt6-tools-dev qt6-l10n-tools \
  qt6-httpserver-dev qt6-websockets-dev qt6-multimedia-dev \
  qt6-webengine-dev qt6-webchannel-dev

cmake -B bld && cmake --build bld -j4
./bld/colman
```

Auf der Startseite lässt sich eine neue Sammlung anlegen; dabei fragt
colman nach einem Administratorpasswort.  Die Oberfläche richtet sich nach der Systemsprache
(Deutsch oder Englisch).

## Dokumentation

Das Tutorial führt Schritt für Schritt durch alle Funktionen – von der
ersten Sammlung über Ausleihe und Etikettendruck bis zu Berechtigungen
und Mehrplatzbetrieb.  Es gibt es auf Deutsch und Englisch, online
unter <https://pesch-1.github.io/colman/> (HTML und PDF).  Lokal erzeugt
wird es mit [Doxygen](https://www.doxygen.nl/):

```bash
cmake --build bld --target tutorial      # Tutorial, alle Sprachen (tutorial_de/tutorial_en: nur eine)
cmake --build bld --target doc           # Entwicklerdoku (englisch)
```

- Tutorial: `bld/tutorial/de/html/index.html` bzw.
  `bld/tutorial/de/latex/colman_tutorial.pdf`
- Entwicklerdoku: `bld/doc/html/index.html`

## Datenschutz

colman speichert personenbezogene Daten von Schülerinnen, Schülern und
Lehrkräften.  Alle Daten bleiben auf dem eigenen Rechner bzw. dem eigenen
Server; colman sendet keine Daten an Dritte.  Bei der Internetsuche nach
Titeldaten wird nur die gesuchte ISBN, EAN bzw. der Titel an den
jeweiligen Dienst übermittelt.

## Lizenz, Mitwirken und Sicherheit

**Autor:** Christoph Pesch  
**Lizenz:** [GNU General Public License v3.0 oder später](LICENSE)

Fremdmaterial und dessen Lizenzen sind in [NOTICE](NOTICE) aufgeführt;
[ACKNOWLEDGEMENTS.md](ACKNOWLEDGEMENTS.md) (englisch) dankt allen, auf
deren Arbeit colman aufbaut.
Beiträge sind willkommen, siehe [CONTRIBUTING.md](CONTRIBUTING.md);
Sicherheitslücken bitte wie in [SECURITY.md](SECURITY.md) beschrieben
melden.  Issues und Pull Requests dürfen gern auf Deutsch verfasst
werden.

Wie colman entstanden ist und wie und warum bei der Entwicklung
KI-Assistenten eingesetzt wurden, erzählt [STORY.md](STORY.md)
(englisch).
