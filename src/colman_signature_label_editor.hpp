// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of SignatureLabelEditor.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_SIGNATURE_LABEL_EDITOR_HPP
#  define COLMAN_SIGNATURE_LABEL_EDITOR_HPP

#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef COLMAN_IQUERIES_HPP
#    include "colman_iqueries.hpp"
#  endif
#  ifndef COLMAN_LABEL_LAYOUT_WIDGET_HPP
#    include "colman_label_layout_widget.hpp"
#  endif

class QGraphicsItem;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QPushButton;
class QSlider;

namespace colman
{

/// Widget for interactively designing the layout of signature labels.
/**
   Embeds a LabelLayoutWidget at the top so the user can pick a label
   size.  Below it shows a scaled preview of the label with one
   draggable, resizable text zone that will hold the shelf signature.

   No barcode and no logo zones are present — signature labels carry
   only a single line of text (the signature).

   Call save_layout() via the "Save" button to persist the layout in
   the database so it can be used as a default for later signature
   printing.
*/
  class SignatureLabelEditor : public QWidget
  {
    Q_OBJECT

  public:
    explicit SignatureLabelEditor(IQueries& queries,
                                  QWidget* parent = nullptr);

  protected:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void showEvent(QShowEvent* e) override;

  private:
  /// Rebuilds the scene for a label of @p w_mm × @p h_mm mm.
  /**
     If @p saved is non-null its element positions are used;
     otherwise default placement is applied.
  */
    void rebuild_card(double w_mm, double h_mm,
                      SignatureLabelLayout const* saved = nullptr);
  /// Loads @p p's dimensions and text-zone arrangement into the preview.
    void apply_preset_to_preview(LabelLayoutPreset const& p);
  /// Returns the text-zone arrangement currently shown in the scene.
    SignatureLabelLayout current_sig_arrangement() const;
  /// Saves the current layout (geometry + arrangement) as a user layout.
    void save_layout();
  /// Resets the scene to the selected layout's arrangement (or defaults).
    void reset_layout();

    LabelLayoutWidget* layout_w_ = nullptr; ///< embedded geometry/printer chooser
    QString current_layout_name_;        ///< name of the selected label preset
    QGraphicsScene* scene_ = nullptr;   ///< editor scene
    QGraphicsView* view_ = nullptr;    ///< view for the scene
    QGraphicsItem* text_item_ = nullptr; ///< CardElementItem (text)
    double card_w_mm_ = 57.0;  ///< current label width in mm
    double card_h_mm_ = 32.0;  ///< current label height in mm
    double margin_mm_ = 0.0;   ///< free margin around label edge in mm
    QPushButton* save_btn_ = nullptr; ///< "Save" button
    QPushButton* reset_btn_ = nullptr; ///< "Reset" button
    QSlider* margin_slider_ = nullptr; ///< margin thickness slider
    QLabel* margin_value_label_ = nullptr; ///< shows the margin value in mm
  };

} // namespace colman

#endif /* #ifndef COLMAN_SIGNATURE_LABEL_EDITOR_HPP */

/* EOF */
