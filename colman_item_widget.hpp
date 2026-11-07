// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class ItemWidget.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ITEM_WIDGET_HPP
#  define COLMAN_ITEM_WIDGET_HPP

#  ifndef COLMAN_ITEM_HPP
#    include "colman_item.hpp"
#  endif
#  ifndef COLMAN_LIST_VALUE_WIDGET_HPP
#    include "colman_list_value_widget.hpp"
#  endif
#  ifndef COLMAN_ITEM_WIDGET_UI_HPP
#    include "colman_item_widget_ui.hpp"
#  endif
#  ifndef SYSTEM_QCOMBOBOX
#    include <QComboBox>
#    define SYSTEM_QCOMBOBOX
#  endif
#  ifndef SYSTEM_QCHECKBOX
#    include <QCheckBox>
#    define SYSTEM_QCHECKBOX
#  endif
#  ifndef SYSTEM_QSPINBOX
#    include <QSpinBox>
#    define SYSTEM_QSPINBOX
#  endif
#  ifndef SYSTEM_MAP
#    include <map>
#    define SYSTEM_MAP
#  endif

namespace colman
{
  class AppData;

  /// Qt widget for editing and displaying item data.
  /**
     Provides form fields for all item properties including type-specific
     attributes, thumbnail, and school-item fields.
     Emits field_changed() whenever the user modifies any field.
  */
  class ItemWidget : public QWidget
  {
    Q_OBJECT

  Q_SIGNALS:
    /// Emitted when any input field is modified.
    void field_changed();

  public Q_SLOTS:
    /// Clears all input fields to their default state.
    void clear_fields();
    /// Looks up item data from the internet using the ISBN field.
    void internet_lookup(QString const& fallback_name = QString());

  public:
    /// Constructs the widget with the given parent.
    ItemWidget(QWidget* parent = nullptr);
    /// Destructor.
    virtual ~ItemWidget() = default;

    /// Connects the widget to the application data.
    /**
       @param data the central application data object
    */
    void setup(AppData& data);
    /// Hides the type selector combo box (for contexts with an external type control).
    void hide_type_selector();
    /// Repopulates the item type combo box from the database.
    void reset_item_type();
    /// Repopulates the item type combo box and selects \a type_id.
    void reset_item_type(int type_id);

    /// Snapshot of all editable field values.
    /**
       Used to preserve user input across an item-type switch: capture the
       values with save_field_values() before the switch and write them back
       with restore_field_values() afterwards.
    */
    struct SavedFieldValues
    {
      QString name_; ///< item name field
      QString place_; ///< place field
      QString description_; ///< description field
      QStringList keywords_; ///< keyword list values
      QString signature_top_; ///< top signature combo text
      QString signature_bottom_; ///< bottom signature combo text
      QString subject_; ///< school subject field
      QString school_type_; ///< school type field
      QString academic_year_; ///< academic year field
      bool is_schoolitem_=false; ///< school-item checkbox state
      int thumbnail_id_=0; ///< loaded thumbnail id, 0 if none
      std::map<QString, QString> attribute_values_; ///< type attributes
      std::map<QString, QStringList> attribute_list_values_; ///< list attrs
    };

    /// Captures all current field values for a later restore.
    /**
       @return snapshot of common fields, type-specific attribute values
               (keyed by database column), and the thumbnail id
    */
    SavedFieldValues save_field_values() const;
    /// Writes back the field values captured by save_field_values().
    /**
       Non-empty common fields and type-specific attributes whose database
       column exists in the current type are restored; everything else is
       left untouched.
       @param saved snapshot returned by save_field_values()
    */
    void restore_field_values(SavedFieldValues const& saved);
    /// Installs auto-completers on relevant input fields.
    void set_completers();
    /// Removes auto-completers from input fields.
    void remove_completers();

    /// Populates all form fields from \a item.
    /**
       @param item the item whose data is to be displayed
       @param type_name internal type name string (may be empty)
    */
    void item_to_fields(Item const& item, QString const& type_name);
    /// Populates the form fields from an internet lookup result.
    /**
       Like item_to_fields(), but empty signatures in @p item keep the
       signatures currently shown in the widget (e.g. the default set via
       set_default_signatures()); non-empty signatures from the lookup win.
       @param item the item retrieved by the lookup
    */
    void lookup_result_to_fields(Item item);
    /// Sets the signatures pre-filled whenever the fields are reset.
    /**
       The defaults are applied by clear_fields() and on every item-type
       switch.  Empty strings mean no default.
       @param top    default top signature
       @param bottom default bottom signature
    */
    void set_default_signatures(QString const& top, QString const& bottom);
    /// Reads all form fields into \a item.
    /**
       @param item output item receiving the field values
       @return true on success, false if any field contains invalid input
    */
    bool fields_to_item(Item& item) const;
    /// Copies the current field values to another ItemWidget.
    /**
       @param target the target widget to copy into
       @return true on success
    */
    bool copy_to(ItemWidget* target) const;

    /// Returns the item type ID currently selected in the type combo box.
    int get_type_id() const;
    /// Returns true if the name field is non-empty.
    bool has_name() const;
    /// Returns true if any field value differs from \a item.
    /**
       @param item the item to compare against
    */
    bool modified(Item const& item) const;
    /// Sets all input fields to read-only or editable.
    /**
       @param ro true to make fields read-only
    */
    void set_read_only(bool ro);
    /// Performs an internet lookup for the given ISBN or title string.
    /**
       @param search ISBN or title string to look up
       @return true if data was retrieved successfully
    */
    bool internet_lookup_string(QString const& search);
    /// Opens the BGG search page with \a name as the initial query.
    /**
       Configures the BGG service from QSettings and navigates to a
       SearchSelectPage pre-filled with \a name; when the user applies a
       result the retrieved game data is copied into the widget.  When
       \a ean is non-empty it is stored as the item's EAN field.  The
       lookup is asynchronous (returns after navigating to the page).
       @param name  Initial search query for the BGG search.
       @param ean   Optional EAN-13 to store on the retrieved item.
    */
    void internet_bgg_name_lookup(QString const& name,
                                  QString const& ean = QString());
    /// Opens the multi-source title search page with \a name as query.
    /**
       For book, CD, DVD, and Blu-ray item types: configures the title
       search services from QSettings and navigates to a SearchSelectPage
       pre-filled with \a name; when the user applies a result the
       retrieved data is copied into the widget.  When \a ean is non-empty
       it is stored as the item's EAN field.  The lookup is asynchronous
       (returns after navigating to the page).
       @param name  Initial search query for the title search.
       @param ean   Optional EAN-13 to store on the retrieved item.
    */
    void internet_name_lookup(QString const& name,
                              QString const& ean = QString());

  private:
    void select_item_type(AppData& data, int type_id, QString const& trans);
    /// Repopulates signature_bottom_ based on the current signature_top_ value.
    void refresh_signature_bottom();
    /// Writes the default signatures into the signature combo boxes.
    void apply_default_signatures();
    void set_is_schoolitem(bool b);
    void some_field_changed();
    void browse_thumbnail();
    void scan_thumbnail();
    void amazon_thumbnail();
    void zoom_thumbnail();
    void remove_thumbnail();
    void use_thumbnail(int tid);

    /// Makes @p w scroll the field list with the wheel until it is focused.
    /**
       Sets @p w to strong focus (no wheel focus) and installs the shared
       wheel guard so that, while the widget is not the active field, mouse
       wheel events scroll the surrounding list instead of changing the
       widget's value or popping a calendar.  Has no effect on null.
       @param w the input widget to guard, or nullptr.
    */
    void install_wheel_guard(QWidget* w);
    /// Installs the wheel guard on every input widget in the item grid.
    void install_wheel_guards_in_grid();

    ItemWidgetUi widget_;
    QObject* wheel_guard_ = nullptr; ///< shared wheel-event filter
    colman::AppData* data_ = nullptr; ///< pointer to application data
    colman::ListValueWidget* keywords_lv_ = nullptr; ///< keyword list editor
    bool read_only_ = false; ///< whether fields are read-only
    QString last_lookup_key_; ///< EAN or ISBN from the last internet lookup
    QString last_lookup_thumbnail_url_; ///< cover URL found for this item
    QString default_signature_top_; ///< top signature pre-filled on reset
    QString default_signature_bottom_; ///< bottom signature pre-filled on reset
    QComboBox* item_allow_reservations_combo_ = nullptr; ///< per-item allow override
    QCheckBox* item_reservation_period_override_check_ = nullptr; ///< period override checkbox
    QSpinBox* item_reservation_period_days_spin_ = nullptr; ///< period override spinbox
  };

} // namespace colman

#endif /* #ifndef COLMAN_ITEM_WIDGET_HPP */

/* EOF */
