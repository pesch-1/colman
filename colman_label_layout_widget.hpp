// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of LabelLayoutPreset and LabelLayoutWidget.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_LABEL_LAYOUT_WIDGET_HPP
#  define COLMAN_LABEL_LAYOUT_WIDGET_HPP

#  ifndef COLMAN_ID_CARD_LAYOUT_HPP
#    include "colman_id_card_layout.hpp"
#  endif
#  ifndef COLMAN_SIGNATURE_LABEL_LAYOUT_HPP
#    include "colman_signature_label_layout.hpp"
#  endif
#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QLIST
#    include <QList>
#    define SYSTEM_QLIST
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;

namespace colman
{

  /// Print flow a layout was created for.
  /**
     User layouts are tagged with the purpose of the editor in which they
     were created so that each print dialog only offers layouts meant for
     it.  @c Any is used for built-in templates and for legacy layouts that
     predate this categorisation; they remain visible everywhere.
  */
  enum struct LabelLayoutPurpose {
    Any,        ///< untagged / template: usable in every editor and flow
    IdCard,     ///< person ID-card printing
    ItemLabel,  ///< item label printing
    Signature   ///< signature-label printing
  };

  /// Converts a purpose to its QSettings string key (empty string for Any).
  QString to_settings_key(LabelLayoutPurpose purpose);

  /// Parses a QSettings string key into a purpose (unknown/empty => Any).
  LabelLayoutPurpose purpose_from_settings_key(QString const& key);

  /// All geometry parameters that describe one label sheet format or label printer.
  /**
     All lengths are in millimetres.  @c builtin is @c true for the
     factory-supplied presets and @c false for user-defined ones.

     When @c is_label_printer is @c true, @c printer_cups_name and
     @c printer_dpi are relevant.  In that case @c columns, @c rows,
     @c left_margin, @c top_margin, @c h_gap and @c v_gap are ignored; only
     @c label_width and @c label_height describe the label dimensions.
  */
  struct LabelLayoutPreset
  {
    QString name;               ///< display name (unique key for user presets)
    int columns = 4;          ///< number of label columns per sheet
    int rows = 10;            ///< number of label rows per sheet
    double label_width = 48.5;   ///< label width in mm
    double label_height = 25.4;  ///< label height in mm
    double left_margin = 4.72;   ///< page left edge → left edge of first label (mm)
    double top_margin = 10.65;   ///< page top edge → top edge of first label (mm)
    double h_gap = 2.56;         ///< horizontal gap between adjacent labels (mm)
    double v_gap = 0.0;          ///< vertical gap between adjacent labels (mm)
    double page_width_mm = 210.0;  ///< printable page width in mm (default A4)
    double page_height_mm = 297.0; ///< printable page height in mm (default A4)
    bool builtin = false;      ///< true = factory preset, cannot be deleted
    bool is_label_printer = false;  ///< true = thermal/label printer preset
    QString printer_cups_name;       ///< CUPS printer name (label printer only)
    int printer_dpi = 203;         ///< printer resolution in dpi (label printer only)
    double bar_width_reduction_mm = 0.0; ///< barcode bar width reduction in mm
                                         ///< (thermal ink-spread compensation)
    int darkness = -1;             ///< thermal print darkness (CUPS Darkness
                                   ///< option); -1 = keep printer default
    bool rotate_180 = false;       ///< rotate printed content by 180°
                                   ///< (label printer only)
    LabelLayoutPurpose purpose = LabelLayoutPurpose::Any; ///< intended print flow
    IdCardLayout card_arrangement;       ///< element zones (IdCard/ItemLabel)
    SignatureLabelLayout sig_arrangement; ///< text zone (Signature)
  };

  /// Widget for selecting and managing label sheet layout presets.
  /**
     Provides a combo box listing all known layouts (factory presets plus
     user-defined ones stored in QSettings).  The user can create new
     presets, edit and save existing user presets, and delete them.
     Clicking <b>Apply</b> emits @c layout_applied with the current preset.

     Factory presets are read-only (Save and Delete are disabled).
     User presets are fully editable.

     @par QSettings storage
     User presets are stored under the group
     @c "label_layouts/<name>/" with the keys
     @c columns, @c rows, @c label_width, @c label_height,
     @c left_margin, @c top_margin, @c h_gap, @c v_gap.
  */
  class LabelLayoutWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit LabelLayoutWidget(QWidget* parent = nullptr);

    /// Returns the preset currently shown in the fields.
    LabelLayoutPreset current_preset() const;

    /// Selects the preset named @p name in the combo box (no-op if not found).
    void select_preset(QString const& name);

    /// Returns the "New" button so an embedding editor can reposition it.
    QPushButton* new_button() const { return new_btn_; }
    /// Returns the "Delete" button so an embedding editor can reposition it.
    QPushButton* delete_button() const { return delete_btn_; }

    /// Sets the purpose used to tag saved layouts and to filter the combo.
    /**
       New layouts saved here are tagged with @p purpose; the combo lists
       only user layouts of that purpose (plus untagged @c Any ones) while
       always keeping the built-in templates.  Repopulates immediately.
    */
    void set_purpose(LabelLayoutPurpose purpose);

    /// Returns all factory-supplied built-in presets.
    static QList<LabelLayoutPreset> builtin_layouts();

    /// Loads all user-defined presets from QSettings.
    static QList<LabelLayoutPreset> load_user_layouts();

    /// Sets the element arrangement reported back by current_preset().
    /**
       The arrangement is edited in the surrounding card/signature editor;
       call this before saving so the assembled preset carries it.
    */
    void set_current_card_arrangement(IdCardLayout const& arrangement);
    void set_current_sig_arrangement(
        SignatureLabelLayout const& arrangement); ///< @copydoc set_current_card_arrangement

    /// Repopulates the combo and reselects @p name after an external save.
    void reselect_after_save(QString const& name);

    /// Validates a layout name for saving (non-empty, not a built-in name).
    /**
       @param name the proposed layout name.
       @param err  receives a human-readable error when validation fails.
       @return true when @p name may be saved.
    */
    static bool validate_save_name(QString const& name, QString* err);

    /// Saves (or overwrites) @p preset in QSettings.
    static void save_user_layout(LabelLayoutPreset const& preset);

    /// Removes the user preset named @p name from QSettings.
    static void delete_user_layout(QString const& name);

    /// Returns the name of the layout most recently saved or applied.
    /**
       Stored in QSettings so it survives application restarts.
       Returns an empty string if the user has never saved or applied a layout.
    */
    static QString last_selected_layout_name();

    /// Remembers \a name as the layout most recently selected for printing.
    /**
       Stored in QSettings so it survives application restarts.  Used by the
       print dialogs to restore the last sheet on the next run, replacing the
       former database-backed last-used-layout name.
    */
    static void set_last_selected_layout_name(QString const& name);

  Q_SIGNALS:
    /// Emitted whenever the selected layout changes (combo change or New).
    /**
       Carries the full preset including its loaded element arrangement so
       the surrounding editor can rebuild its preview and load the
       arrangement into its scene.
    */
    void layout_applied(colman::LabelLayoutPreset const& preset);

    /// Emitted on live geometry edits (label size etc.) of the same layout.
    /**
       The surrounding editor should resize its preview to the new
       dimensions while keeping the arrangement currently in its scene.
    */
    void geometry_changed(colman::LabelLayoutPreset const& preset);

  protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

  private Q_SLOTS:
    void on_layout_changed(int index);
    void on_printer_changed(int index);
    void on_new();
    void on_delete();
    void update_preview();

  private:
    void populate_combo();
    void populate_printer_combo();
    void load_into_fields(LabelLayoutPreset const& p);
    bool is_current_builtin() const;
    static QString settings_group(QString const& name);

    void set_printer_mode(bool printer_mode);

    QComboBox* layout_combo_ = nullptr;
    QLineEdit* name_edit_ = nullptr;
    QSpinBox* columns_spin_ = nullptr;
    QSpinBox* rows_spin_ = nullptr;
    QDoubleSpinBox* label_width_spin_ = nullptr;
    QDoubleSpinBox* label_height_spin_ = nullptr;
    QDoubleSpinBox* left_margin_spin_ = nullptr;
    QDoubleSpinBox* top_margin_spin_ = nullptr;
    QDoubleSpinBox* h_gap_spin_ = nullptr;
    QDoubleSpinBox* v_gap_spin_ = nullptr;
    QDoubleSpinBox* bar_width_reduction_spin_ = nullptr; ///< barcode BWR in mm
    QSpinBox* darkness_spin_ = nullptr;    ///< thermal print darkness
    QLabel* darkness_label_ = nullptr;     ///< label for darkness (printer only)
    QCheckBox* rotate_check_ = nullptr;    ///< rotate output by 180°
                                           ///< (printer only)
    QPushButton* new_btn_ = nullptr;
    QPushButton* delete_btn_ = nullptr;
    QLabel* count_label_ = nullptr;        ///< shows "N labels per sheet"
    QLabel* columns_label_ = nullptr;      ///< grid row label (hidden in printer mode)
    QLabel* rows_label_ = nullptr;         ///< grid row label (hidden in printer mode)
    QLabel* left_margin_label_ = nullptr;  ///< grid row label (hidden in printer mode)
    QLabel* top_margin_label_ = nullptr;   ///< grid row label (hidden in printer mode)
    QLabel* h_gap_label_ = nullptr;        ///< grid row label (hidden in printer mode)
    QLabel* v_gap_label_ = nullptr;        ///< grid row label (hidden in printer mode)
    QComboBox* printer_combo_ = nullptr;   ///< chooses the bound label printer
    QLabel* printer_label_ = nullptr;      ///< "Printer:" caption (printer mode)
    QString current_cups_name_;          ///< CUPS name of currently shown printer
    int current_printer_dpi_ = 203;    ///< DPI of currently shown printer
    bool current_is_printer_ = false;   ///< true when in label-printer mode
    LabelLayoutPurpose purpose_ = LabelLayoutPurpose::Any; ///< editor purpose
    IdCardLayout loaded_card_arrangement_;       ///< arrangement of selected layout
    SignatureLabelLayout loaded_sig_arrangement_; ///< arrangement of selected layout
    QGraphicsScene* scene_ = nullptr;
    QGraphicsView* preview_view_ = nullptr;
  };

} // namespace colman

#endif /* #ifndef COLMAN_LABEL_LAYOUT_WIDGET_HPP */

/* EOF */
