# Colman Flutter Client

Mobile REST client for the Colman library management system, targeting Android
and iOS on both smartphones and tablets.

## Goal

Replicate the functionality of the existing Qt-based Colman REST desktop client
as a native-feeling mobile application. The Flutter client communicates with the
same Colman REST API server and supports all core workflows.

## Target Platforms

- Android (smartphones and tablets) — supported
- iOS (iPhones and iPads) — planned; not built or tested yet, not offered

## Planned Features

### Authentication
- Login with server URL, username, and password (HTTP Basic Auth / JWT)
- Server discovery via mDNS on the local network

### Core Workflows
- **Lending** — lend items to borrowers (multi-step wizard)
- **Return** — process item returns
- **Acquisition** — add new items to the collection
- **Liquidation** — remove items from the collection

### Management
- **Items** — search, view, create, and edit library items with type-specific
  fields; barcode scanning
- **Persons** — manage borrowers (students, teachers); class assignments
- **Item Types** — configure item types and their custom attributes
- **Classes** — school class management
- **History** — audit trail of all transactions
- **Settings** — user preferences and account management

### Live Sync
- Poll `/api/table-versions` periodically to detect remote changes and refresh
  views automatically

## Running

```bash
# Linux desktop (requires g++-14 and libsecret-1-dev)
flutter run -d linux

# Android device (USB connected, USB debugging enabled)
flutter run -d android

# Web (requires CHROME_EXECUTABLE to be set)
flutter run -d chrome
```

On first build, the `native_assets/linux` directory is created automatically
via `linux/CMakeLists.txt`. No manual setup needed.

## Building an APK for Android

### Prerequisites

- Flutter SDK (see <https://docs.flutter.dev/get-started/install>)
- Android SDK and build tools (installed via Android Studio or `sdkmanager`)
- Java Development Kit (JDK 17 recommended)

The `ANDROID_HOME` (or `ANDROID_SDK_ROOT`) environment variable must point to
your Android SDK installation, e.g.:

```bash
export ANDROID_HOME=$HOME/Android/Sdk
```

### Debug APK (for sideloading and testing)

```bash
cd flutter
flutter build apk --debug
```

Output: `build/app/outputs/flutter-apk/app-debug.apk`

### Release APK (for distribution)

A release build must be signed. Create a keystore once:

```bash
keytool -genkey -v -keystore ~/colman-release.jks \
  -keyalg RSA -keysize 2048 -validity 10000 \
  -alias colman
```

Create `flutter/android/key.properties` (do **not** commit this file):

```
storePassword=<your-store-password>
keyPassword=<your-key-password>
keyAlias=colman
storeFile=<absolute-path-to-colman-release.jks>
```

Then build the signed release APK:

```bash
cd flutter
flutter build apk --release
```

Output: `build/app/outputs/flutter-apk/app-release.apk`

For a split APK per ABI (smaller download size):

```bash
flutter build apk --release --split-per-abi
```

This produces separate APKs for `armeabi-v7a`, `arm64-v8a`, and `x86_64`.

### Installing on a connected device

```bash
flutter install          # installs the most recently built APK
# or explicitly:
adb install build/app/outputs/flutter-apk/app-release.apk
```

## Technology Stack

| Component | Choice |
|---|---|
| Framework | Flutter (Dart) |
| HTTP client | `dio` |
| JSON serialization | `json_serializable` / `freezed` |
| State management | `riverpod` |
| Navigation | `go_router` |
| Adaptive layout | `flutter_adaptive_scaffold` |

## API Reference

The client targets the Colman REST API. Key endpoint families:

- `GET/POST /api/items` — item CRUD
- `GET/POST /api/persons` — person CRUD
- `POST /api/lendings`, `POST /api/returns` — lending workflows
- `GET /api/table-versions` — concurrency / live sync
- `GET /api/classes`, `GET /api/history` — school classes, audit trail
- `GET /api/collection`, `GET /api/me` — server info, current user

Versioned updates use `If-Match` headers for optimistic concurrency control,
consistent with the desktop client.
