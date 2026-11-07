// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class CardEditorBase — shared base for card layout editors.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_CARD_EDITOR_BASE_HPP
#  define COLMAN_CARD_EDITOR_BASE_HPP

#  ifndef COLMAN_ID_CARD_LAYOUT_HPP
#    include "colman_id_card_layout.hpp"
#  endif
#  ifndef COLMAN_IQUERIES_HPP
#    include "colman_iqueries.hpp"
#  endif
#  ifndef COLMAN_LABEL_LAYOUT_WIDGET_HPP
#    include "colman_label_layout_widget.hpp"
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_UTILITY
#    include <utility>
#    define SYSTEM_UTILITY
#  endif

class QCheckBox;
class QGraphicsItem;
class QGraphicsScene;
class QGraphicsView;
class QPushButton;
class QSlider;

namespace colman
{

  class CardElementItem;
  class Collection;

/// Abstract base class shared by IdEditor and ItemCardEditor.
/**
   Provides the full UI scaffold (group box, preview view, margin slider,
   save/reset buttons) and implements current_layout(), clamp_layout(),
   rebuild_card(), save_layout(), reset_layout(), eventFilter(), and
   showEvent().

   Derived classes implement the variant behaviour via pure virtual
   methods and call setup_editor() from their constructors to complete
   the initialisation.
*/
  class CardEditorBase : public QWidget
  {
    Q_OBJECT

  public:
    explicit CardEditorBase(IQueries& queries, QWidget* parent = nullptr);

  /// Returns the layout as currently arranged in the editor.
    IdCardLayout current_layout() const;

  protected:
  /// Builds the UI and performs the initial rebuild.
  /**
     Derived class constructors call this once the vtable is fully set up.
     @param group_label    label on the card-size group box.
     @param margin_tooltip tooltip for the margin slider.
     @param settings_key   QSettings prefix (e.g. "id_editor").
     @param purpose        purpose tag for layouts created in this editor.
  */
    void setup_editor(QString const& group_label,
                      QString const& margin_tooltip,
                      QString const& settings_key,
                      LabelLayoutPurpose purpose);

    void rebuild_card(double w_mm, double h_mm,
                      IdCardLayout const* layout = nullptr);
  /// Loads @p p's dimensions and element arrangement into the preview.
    void apply_preset_to_preview(LabelLayoutPreset const& p);
    void save_layout();
    void reset_layout();

  /// Returns @p layout with all elements clamped to fit within @p w_mm × @p h_mm.
    static IdCardLayout clamp_layout(IdCardLayout layout,
                                     double w_mm, double h_mm);

    bool eventFilter(QObject* obj, QEvent* e) override;
    void showEvent(QShowEvent* e) override;

  // ------------------------------------------------------------------
  // Virtual interface — subclasses implement these
  // ------------------------------------------------------------------

  /// Returns card dimensions (w_mm, h_mm) for the given preset.
    virtual std::pair<double, double> card_dims(LabelLayoutPreset const& p) const = 0;

  /// Returns the sample barcode text for the preview.
    virtual QString preview_barcode(Collection const& col) const = 0;

  /// Returns the sample text lines for the text zone.
    virtual QStringList preview_text_lines(Collection const& col) const = 0;

  /// Returns the index of the emphasized text line (-1 for none).
    virtual int text_emph_line() const = 0;

  /// Returns the fraction of card height allocated to the text/logo zone.
    virtual double default_text_height_fraction() const = 0;

  /// Applies type-specific settings to the text item (default: no-op).
    virtual void configure_text_item(CardElementItem* tx) const;

  // ------------------------------------------------------------------
  // Shared data members
  // ------------------------------------------------------------------

    IQueries& queries_;                ///< database queries
    LabelLayoutWidget* layout_w_ = nullptr; ///< embedded geometry/printer chooser
    QGraphicsScene* scene_ = nullptr; ///< editor scene
    QGraphicsView* view_ = nullptr; ///< view for the scene
    QGraphicsItem* barcode_item_ = nullptr; ///< CardElementItem (barcode)
    QGraphicsItem* text_item_ = nullptr; ///< CardElementItem (text)
    QGraphicsItem* logo_item_ = nullptr; ///< CardElementItem (logo)
    double card_w_mm_ = 0.0;     ///< current card width in mm
    double card_h_mm_ = 0.0;     ///< current card height in mm
    double margin_mm_ = 0.0;     ///< free margin around card edge in mm
    QPushButton* save_btn_ = nullptr; ///< "Save" button
    QPushButton* reset_btn_ = nullptr; ///< "Reset" button
    QCheckBox* logo_check_ = nullptr; ///< "Show logo" toggle
    QSlider* margin_slider_ = nullptr; ///< margin thickness slider
    QString settings_key_;           ///< QSettings prefix
  };

} // namespace colman

#endif /* #ifndef COLMAN_CARD_EDITOR_BASE_HPP */

/* EOF */
