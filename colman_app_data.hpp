// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of class AppData.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_APPDATA_HPP
#  define COLMAN_APPDATA_HPP

#  ifndef COLMAN_ISBN_QUERIES_HPP
#    include "colman_isbn_queries.hpp"
#  endif
#  ifndef COLMAN_EAN_QUERIES_HPP
#    include "colman_ean_queries.hpp"
#  endif
#  ifndef COLMAN_BGG_QUERIES_HPP
#    include "colman_bgg_queries.hpp"
#  endif
#  ifndef COLMAN_NAME_QUERIES_HPP
#    include "colman_name_queries.hpp"
#  endif
#  ifndef COLMAN_IQUERIES_HPP
#    include "colman_iqueries.hpp"
#  endif
#  ifndef COLMAN_MAIN_WINDOW_UI_HPP
#    include "colman_main_window_ui.hpp"
#  endif
#  ifndef SYSTEM_FUNCTIONAL
#    include <functional>
#    define SYSTEM_FUNCTIONAL
#  endif
#  ifndef SYSTEM_MEMORY
#    include <memory>
#    define SYSTEM_MEMORY
#  endif
#  ifndef SYSTEM_QLOCKFILE
#    include <QLockFile>
#    define SYSTEM_QLOCKFILE
#  endif
#  ifndef SYSTEM_QTOOLBUTTON
#    include <QToolButton>
#    define SYSTEM_QTOOLBUTTON
#  endif
#  ifndef SYSTEM_QTABLEWIDGET
#    include <QTableWidget>
#    define SYSTEM_QTABLEWIDGET
#  endif
#  ifndef SYSTEM_QSET
#    include <QSet>
#    define SYSTEM_QSET
#  endif

namespace colman
{
  /// Central application state passed by reference to all page functions.
  /**
     Aggregates the SQLite database connection, the generated
     main-window UI, all view-model instances (one per view/page), and the
     ISBN-query helper.  There is exactly one AppData instance per program run;
     no global variables are used.
  */
  class AppData
  {
  public:
    /// Page-transition callback; receives a mutable reference to AppData.
    using AppFunction = std::function<void(AppData&)>;

    /// Constructs AppData with a file-backed SQLite database.
    AppData();
    /// Constructs AppData with a named in-memory SQLite database (for tests).
    explicit AppData(QString const& conn_name);
    /// Constructs AppData with an externally supplied IQueries implementation.
    explicit AppData(std::unique_ptr<IQueries> queries, bool show_external_id_ui = true);
    ~AppData() = default;

    /// Sets the directory for storing thumbnail files and propagates it to the
    /// underlying IQueries implementation if it is an SQLQueries instance.
    void set_thumbnail_dir(QString const& dir);
    /// Returns the current thumbnail directory (empty if not set).
    QString thumbnail_dir() const;

    QString filename_;         ///< path of the currently open .col file
    int current_data_collector_ = -1;   ///< 0=admin, person_id=user, -1=not logged in
    MainWindowUi mw_;          ///< hand-written main-window UI
    QMainWindow w_;            ///< the main window widget
    IsbnQueries isbn_queries_; ///< ISBN look-up helper
    EanQueries ean_queries_;   ///< EAN product barcode look-up helper
    BggQueries bgg_queries_;   ///< BGG board-game look-up helper
    NameQueries name_queries_; ///< multi-source title look-up helper
    std::unique_ptr<IQueries> queries_; ///< SQL query facade

    QToolButton* open_button_ = nullptr;           ///< start page: open collection
    QToolButton* new_button_ = nullptr;            ///< start page: new collection
    QToolButton* exit_button_ = nullptr;           ///< start page: quit program
    QTableWidget* recently_opened_ = nullptr;       ///< start page: recently-opened table

    QToolButton* new_person_button_ = nullptr;    ///< central page: edit persons
    QToolButton* acquisition_button_ = nullptr;   ///< central page: record & acquire
    QToolButton* lend_button_ = nullptr;          ///< central page: lend item
    QToolButton* group_lend_button_ = nullptr;    ///< central page: group lending
    QToolButton* item_view_button_ = nullptr;     ///< central page: view items
    QToolButton* item_type_button_ = nullptr;     ///< central page: edit item type
    QToolButton* liquidation_button_ = nullptr;   ///< central page: liquidate
    QToolButton* return_button_ = nullptr;        ///< central page: return item
    QToolButton* person_view_button_ = nullptr;   ///< central page: view persons
    QToolButton* settings_button_ = nullptr;      ///< central page: change settings
    QToolButton* print_id_button_ = nullptr;      ///< central page: print id
    QToolButton* print_barcode_button_ = nullptr; ///< central page: print barcode
    QToolButton* history_button_ = nullptr;       ///< central page: view history
    QToolButton* edit_id_button_ = nullptr;             ///< central page: edit ID layout
    QToolButton* edit_item_card_button_ = nullptr;     ///< central page: edit item card layout
    QToolButton* edit_signature_layout_button_ = nullptr; ///< central page: edit signature label layout
    QToolButton* print_signatures_button_ = nullptr;   ///< central page: print signature labels
    QToolButton* calibrate_printer_button_ = nullptr;  ///< central page: calibrate printer
    QToolButton* search_button_ = nullptr;             ///< central page: search items
    QToolButton* open_lendings_button_ = nullptr;      ///< central page: open lendings
    QToolButton* my_reservations_button_ = nullptr;    ///< central page: my reservations
    QToolButton* class_management_button_ = nullptr;   ///< central page: class management
    QToolButton* scan_button_ = nullptr;               ///< central page: scan barcode with camera
    QWidget* class_management_page_ = nullptr;     ///< class management main page
    QWidget* item_card_editor_page_ = nullptr; ///< item card editor page widget
    QWidget* camera_scan_page_ = nullptr;      ///< camera barcode scan page widget
    QWidget* search_item_detail_page_ = nullptr; ///< item detail page (search)
    /// Reloads the currently shown item on search_item_detail_page_ (for polling).
    std::function<void()> refresh_search_item_detail_;

    int last_acquisition_item_id_ = -1;  ///< item ID of the last completed acquisition
    bool lending_reserve_intent_ = false; ///< true when wizard was opened via Reserve action
    int lending_action_item_id_ = -1;    ///< item pre-selected via barcode action, -1 if none
    bool show_external_id_ui_ = false; ///< true in REST-client mode

    HistoryCombinedModel combined_history_model_; ///< model for the history page
    int history_offset_ = 0;       ///< pagination offset for history list
    bool history_has_more_ = false; ///< true when more history rows available
    ItemListModel acquisition_item_model_; ///< item model for acquisition
    int acquisition_item_offset_ = 0;      ///< pagination offset for acquisition item list
    bool acquisition_item_has_more_ = false; ///< true when more acquisition items available
    ItemListModel liquidation_item_model_; ///< item model for liquidation
    int liquidation_item_offset_ = 0;      ///< pagination offset for liquidation item list
    bool liquidation_item_has_more_ = false; ///< true when more liquidation items available
    ItemListModel ibc_item_model_;             ///< item model for item barcode page
    PersonListModel lending_person_model_;  ///< person model for lend page
    ItemListModel lending_item_model_;    ///< item model for lend page
    int lending_item_offset_ = 0;      ///< pagination offset for lending item list
    bool lending_item_has_more_ = false; ///< true when more lending items available
    ItemListModel group_lending_item_model_; ///< item model for group lending
    int group_lending_item_offset_ = 0; ///< pagination offset for group lending items
    bool group_lending_item_has_more_ = false; ///< true when more group lending items available
    int group_lending_persons_item_id_ = -1; ///< item the borrower list was built for
    int group_lending_free_copies_ = 0; ///< copies lendable to borrowers without own reservation
    QSet<int> group_lending_reserving_ids_; ///< borrowers holding a reservation of the item
    int return_borrower_item_id_ = -1; ///< item the return borrower list was built for
    ItemListModel return_item_model_;     ///< item model for return page
    OpenLendingListModel return_lending_model_; ///< lending model for return page
    PersonListModel pbc_person_model_;           ///< person model for person barcode page
    PersonListModel person_model_;               ///< model for person page
    TypeListModel type_model_;                  ///< model for item type page

    /// Sets the callback invoked when the item barcode page Back button is pressed.
    /**
       When set, this overrides the default behaviour of returning to the
       central page.  It is cleared automatically after it fires.
    */
    void set_ibc_back_function(AppFunction func)
    {
      ibc_back_ = func;
    }
    /// Fires the item barcode back callback if set, else goes to central page.
    /**
       Clears the stored callback after invoking it so that subsequent
       visits to the item barcode page use the default back destination.
    */
    void invoke_ibc_back()
    {
      if(ibc_back_) {
        AppFunction f = std::move(ibc_back_);
        ibc_back_ = AppFunction();
        f(*this);
      }
      else {
        mw_.pages_stack_->setCurrentWidget(mw_.central_page_);
      }
    }

    /// Sets the callback invoked when the item card editor Back button is pressed.
    /**
       When set, this overrides the default behaviour of returning to the
       central page.  It is cleared automatically after it fires.
    */
    void set_ice_back_function(AppFunction func)
    {
      ice_back_ = func;
    }
    /// Fires the item card editor back callback if set, else goes to central page.
    /**
       Clears the stored callback after invoking it so that subsequent
       visits to the item card editor use the default back destination.
    */
    void invoke_ice_back()
    {
      if(ice_back_) {
        AppFunction f = std::move(ice_back_);
        ice_back_ = AppFunction();
        f(*this);
      }
      else {
        mw_.pages_stack_->setCurrentWidget(mw_.central_page_);
      }
    }

    /// Sets the callback that discards lazily created editor widgets.
    /**
       Registered by setup_central_page() so close_collection() can
       reach the file-local editor pages.
    */
    void set_clear_lazy_editors_function(std::function<void()> func)
    {
      clear_lazy_editors_ = std::move(func);
    }
    /// Discards lazily created editor widgets if the callback is set.
    /**
       Called when a collection is closed so the editors are rebuilt
       consistent with the next opened collection.
    */
    void clear_lazy_editors()
    {
      if(clear_lazy_editors_) {
        clear_lazy_editors_();
      }
    }

    std::unique_ptr<QLockFile> collection_lock_; ///< exclusive lock on open .col file

  private:
    QString thumbnail_dir_;  ///<
    AppFunction ibc_back_; ///< optional override for the item barcode page back button
    AppFunction ice_back_; ///< optional override for the item card editor back button
    std::function<void()> clear_lazy_editors_; ///< discards lazily created editors
  };

  /// Namespace-level convenience alias for AppData::AppFunction.
  using AppFunction = AppData::AppFunction;

} // namespace colman

#endif /* #ifndef COLMAN_APPDATA_HPP */

/* EOF */
