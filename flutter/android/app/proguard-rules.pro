# mobile_scanner: keep all MLKit subpackages (the library's own rule only covers
# the top-level package with *, missing com.google.mlkit.vision.barcode.* etc.)
-keep class com.google.mlkit.** { *; }
-dontwarn com.google.mlkit.**

# mobile_scanner plugin classes
-keep class dev.steenbakker.mobile_scanner.** { *; }

# CameraX
-keep class androidx.camera.** { *; }
-dontwarn androidx.camera.**

# barhopper (bundled model helper used by MLKit barcode scanning)
-keep class com.google.android.libraries.barhopper.** { *; }
-dontwarn com.google.android.libraries.barhopper.**
