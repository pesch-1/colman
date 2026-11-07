// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Reusable widget for entering an encryption password with a strength meter.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ENCRYPTION_PASSWORD_WIDGET_HPP
#  define COLMAN_ENCRYPTION_PASSWORD_WIDGET_HPP

#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif

class QLabel;
class QLineEdit;

namespace colman
{
  /// Returns a strength score 0..5 and a descriptive label for \a pw.
  /**
     Scores one point each for: length >= 8, uppercase letter, lowercase
     letter, digit, non-alphanumeric character.
     @param pw     password to evaluate
     @param label  filled with a human-readable strength description
     @param color  filled with a CSS colour string matching the strength
     @return score 0..5
  */
  int password_strength(QString const& pw, QString& label, QString& color);

  /// Password + confirmation fields with a live strength indicator.
  /**
     Emits changed() whenever either field is edited.  is_valid() is true
     when the password is non-empty and both fields match.
  */
  class EncryptionPasswordWidget : public QWidget
  {
    Q_OBJECT

  public:
    /// Constructs the widget.
    explicit EncryptionPasswordWidget(QWidget* parent = nullptr);

    /// Returns the entered password.
    QString password() const;
    /// Returns true when the password is non-empty and both fields match.
    bool is_valid() const;
    /// Clears both fields and the strength label.
    void clear();

  Q_SIGNALS:
    /// Emitted whenever either field changes.
    void changed();

  private:
    void update_strength();

    QLineEdit* pw_edit_ = nullptr;       ///< password field
    QLineEdit* confirm_edit_ = nullptr;  ///< confirmation field
    QLabel*    strength_label_ = nullptr; ///< strength indicator
  };

} // namespace colman

#endif /* #ifndef COLMAN_ENCRYPTION_PASSWORD_WIDGET_HPP */

/* EOF */
