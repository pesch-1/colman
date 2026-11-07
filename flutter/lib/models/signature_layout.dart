// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
import 'id_card_layout.dart';

/// Layout of a signature (spine call-number) label: the label size, a free
/// margin and a single positioned text zone. Signatures carry no barcode or
/// logo, mirroring the desktop `SignatureLabelLayout`.
class SignatureLayout {
  const SignatureLayout({
    this.cardWidthMm = 50.0,
    this.cardHeightMm = 20.0,
    this.marginMm = 2.0,
    this.text = const IdCardElement(
        xMm: 2.0, yMm: 2.0, widthMm: 46.0, heightMm: 16.0),
  });

  /// Label width in millimetres.
  final double cardWidthMm;

  /// Label height in millimetres.
  final double cardHeightMm;

  /// Free margin around the label edge in millimetres.
  final double marginMm;

  /// The single text zone (position, size and orientation).
  final IdCardElement text;

  SignatureLayout copyWith({
    double? cardWidthMm,
    double? cardHeightMm,
    double? marginMm,
    IdCardElement? text,
  }) {
    return SignatureLayout(
      cardWidthMm: cardWidthMm ?? this.cardWidthMm,
      cardHeightMm: cardHeightMm ?? this.cardHeightMm,
      marginMm: marginMm ?? this.marginMm,
      text: text ?? this.text,
    );
  }

  factory SignatureLayout.fromJson(Map<String, dynamic> json) {
    const d = SignatureLayout();
    return SignatureLayout(
      cardWidthMm: (json['card_width_mm'] as num?)?.toDouble() ?? d.cardWidthMm,
      cardHeightMm:
          (json['card_height_mm'] as num?)?.toDouble() ?? d.cardHeightMm,
      marginMm: (json['margin_mm'] as num?)?.toDouble() ?? d.marginMm,
      text: json['text'] is Map<String, dynamic>
          ? IdCardElement.fromJson(json['text'] as Map<String, dynamic>)
          : d.text,
    );
  }

  Map<String, dynamic> toJson() => {
        'card_width_mm': cardWidthMm,
        'card_height_mm': cardHeightMm,
        'margin_mm': marginMm,
        'text': text.toJson(),
      };
}
