// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Declaration of struct MainWindowUi.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_MAIN_WINDOW_UI_HPP
#  define COLMAN_MAIN_WINDOW_UI_HPP

#  ifndef SYSTEM_QCOREAPPLICATION
#    include <QCoreApplication>
#    define SYSTEM_QCOREAPPLICATION
#  endif

// The data members are pointers and would only need forward declarations, but
// this header replaces the generated ui_colman.h, which pulled in the full Qt
// widget definitions.  Code reaching the widgets through AppData::mw_ relies on
// that, so the same headers are included here to keep this a drop-in.
#  ifndef SYSTEM_QACTION
#    include <QAction>
#    define SYSTEM_QACTION
#  endif
#  ifndef SYSTEM_QCHECKBOX
#    include <QCheckBox>
#    define SYSTEM_QCHECKBOX
#  endif
#  ifndef SYSTEM_QCOMBOBOX
#    include <QComboBox>
#    define SYSTEM_QCOMBOBOX
#  endif
#  ifndef SYSTEM_QDATEEDIT
#    include <QDateEdit>
#    define SYSTEM_QDATEEDIT
#  endif
#  ifndef SYSTEM_QDATETIMEEDIT
#    include <QDateTimeEdit>
#    define SYSTEM_QDATETIMEEDIT
#  endif
#  ifndef SYSTEM_QFORMLAYOUT
#    include <QFormLayout>
#    define SYSTEM_QFORMLAYOUT
#  endif
#  ifndef SYSTEM_QHBOXLAYOUT
#    include <QHBoxLayout>
#    define SYSTEM_QHBOXLAYOUT
#  endif
#  ifndef SYSTEM_QHEADERVIEW
#    include <QHeaderView>
#    define SYSTEM_QHEADERVIEW
#  endif
#  ifndef SYSTEM_QLABEL
#    include <QLabel>
#    define SYSTEM_QLABEL
#  endif
#  ifndef SYSTEM_QLINEEDIT
#    include <QLineEdit>
#    define SYSTEM_QLINEEDIT
#  endif
#  ifndef SYSTEM_QLISTWIDGET
#    include <QListWidget>
#    define SYSTEM_QLISTWIDGET
#  endif
#  ifndef SYSTEM_QMAINWINDOW
#    include <QMainWindow>
#    define SYSTEM_QMAINWINDOW
#  endif
#  ifndef SYSTEM_QPLAINTEXTEDIT
#    include <QPlainTextEdit>
#    define SYSTEM_QPLAINTEXTEDIT
#  endif
#  ifndef SYSTEM_QPUSHBUTTON
#    include <QPushButton>
#    define SYSTEM_QPUSHBUTTON
#  endif
#  ifndef SYSTEM_QRADIOBUTTON
#    include <QRadioButton>
#    define SYSTEM_QRADIOBUTTON
#  endif
#  ifndef SYSTEM_QSCROLLAREA
#    include <QScrollArea>
#    define SYSTEM_QSCROLLAREA
#  endif
#  ifndef SYSTEM_QSPINBOX
#    include <QSpinBox>
#    define SYSTEM_QSPINBOX
#  endif
#  ifndef SYSTEM_QDOUBLESPINBOX
#    include <QDoubleSpinBox>
#    define SYSTEM_QDOUBLESPINBOX
#  endif
#  ifndef SYSTEM_QSPLITTER
#    include <QSplitter>
#    define SYSTEM_QSPLITTER
#  endif
#  ifndef SYSTEM_QSTACKEDWIDGET
#    include <QStackedWidget>
#    define SYSTEM_QSTACKEDWIDGET
#  endif
#  ifndef SYSTEM_QTABLEVIEW
#    include <QTableView>
#    define SYSTEM_QTABLEVIEW
#  endif
#  ifndef SYSTEM_QTABLEWIDGET
#    include <QTableWidget>
#    define SYSTEM_QTABLEWIDGET
#  endif
#  ifndef SYSTEM_QTEXTEDIT
#    include <QTextEdit>
#    define SYSTEM_QTEXTEDIT
#  endif
#  ifndef SYSTEM_QTOOLBUTTON
#    include <QToolButton>
#    define SYSTEM_QTOOLBUTTON
#  endif
#  ifndef SYSTEM_QTREEWIDGET
#    include <QTreeWidget>
#    define SYSTEM_QTREEWIDGET
#  endif
#  ifndef SYSTEM_QVBOXLAYOUT
#    include <QVBoxLayout>
#    define SYSTEM_QVBOXLAYOUT
#  endif
#  ifndef SYSTEM_QWIDGET
#    include <QWidget>
#    define SYSTEM_QWIDGET
#  endif
#  ifndef COLMAN_ITEM_WIDGET_HPP
#    include "colman_item_widget.hpp"
#  endif
#  ifndef COLMAN_PERSON_LIST_WIDGET_HPP
#    include "colman_person_list_widget.hpp"
#  endif
#  ifndef COLMAN_PERSON_CHECK_LIST_WIDGET_HPP
#    include "colman_person_check_list_widget.hpp"
#  endif
#  ifndef COLMAN_PERSON_WIDGET_HPP
#    include "colman_person_widget.hpp"
#  endif
#  ifndef COLMAN_PIXMAP_LABEL_HPP
#    include "colman_pixmap_label.hpp"
#  endif
#  ifndef COLMAN_SCALING_LABEL_HPP
#    include "colman_scaling_label.hpp"
#  endif

class QMainWindow;

namespace colman
{

  /// Hand-written main-window UI (replaces the generated colman.ui form).
  /**
     Builds the application main window: the central QStackedWidget with all
     all pages, the menu bar with its actions and the status
     bar.  The data members mirror the widget names that the rest of the code
     base addresses through AppData::mw_, so callers are unaffected by the
     switch away from the generated Ui_MainWindow.  setup_ui() constructs the
     whole tree; the page-specific build_*() helpers keep it readable.
  */
  struct MainWindowUi
  {
    QAction* action_open_ = nullptr;
    QAction* action_new_ = nullptr;
    QAction* action_close_ = nullptr;
    QWidget* centralwidget = nullptr;
    QStackedWidget* pages_stack_ = nullptr;
    QWidget* start_page_ = nullptr;
    QWidget* central_page_ = nullptr;
    QWidget* item_type_page_ = nullptr;
    QPushButton* item_type_back_ = nullptr;
    QTableView* item_type_type_view_ = nullptr;
    QPushButton* item_type_remove_ = nullptr;
    QPushButton* item_type_change_ = nullptr;
    QLineEdit* item_type_name_ = nullptr;
    QComboBox* item_type_media_kind_ = nullptr;
    QTableWidget* item_type_fields_ = nullptr;
    QPushButton* item_type_remove_field_ = nullptr;
    QPushButton* item_type_add_field_ = nullptr;
    QPushButton* item_type_move_up_ = nullptr;
    QPushButton* item_type_move_down_ = nullptr;
    QToolButton* item_type_marc21_template_ = nullptr;
    QPushButton* item_type_revert_ = nullptr;
    QPushButton* item_type_insert_ = nullptr;
    QWidget* person_page_ = nullptr;
    QPushButton* back_button2_ = nullptr;
    PersonListWidget* person_person_list_ = nullptr;
    QVBoxLayout* verticalLayout_9 = nullptr;
    PersonWidget* person_person_ = nullptr;
    QPushButton* person_clear_ = nullptr;
    QPushButton* remove_person_ = nullptr;
    QPushButton* change_person_ = nullptr;
    QPushButton* insert_person_ = nullptr;
    QWidget* person_barcode_page_ = nullptr;
    QPushButton* person_barcode_back_ = nullptr;
    QLineEdit* person_barcode_search_ = nullptr;
    QComboBox* person_barcode_class_ = nullptr;
    QPushButton* person_barcode_select_all_ = nullptr;
    QPushButton* person_barcode_deselect_all_ = nullptr;
    QCheckBox* person_barcode_recent_check_ = nullptr;
    QSpinBox* person_barcode_recent_count_ = nullptr;
    QCheckBox* person_barcode_show_border_ = nullptr;
    QStackedWidget* person_barcode_stack_ = nullptr;
    QTreeWidget* person_barcode_tree_ = nullptr;
    PersonWidget* person_barcode_person_ = nullptr;
    QLabel* person_barcode_selection_count_ = nullptr;
    QPushButton* person_card_print_ = nullptr;
    QWidget* person_barcode_print_page_ = nullptr;
    QWidget* item_barcode_page_ = nullptr;
    QPushButton* item_barcode_back_ = nullptr;
    QHBoxLayout* horizontalLayout_ibc_top = nullptr;
    QLabel* item_barcode_logo_ = nullptr;
    QLineEdit* item_barcode_search_ = nullptr;
    QPushButton* item_barcode_select_all_ = nullptr;
    QPushButton* item_barcode_deselect_all_ = nullptr;
    QCheckBox* item_barcode_recent_check_ = nullptr;
    QSpinBox* item_barcode_recent_count_ = nullptr;
    QCheckBox* item_barcode_daterange_check_ = nullptr;
    QDateEdit* item_barcode_date_from_ = nullptr;
    QLabel* item_barcode_date_to_label_ = nullptr;
    QDateEdit* item_barcode_date_to_ = nullptr;
    QStackedWidget* item_barcode_stack_ = nullptr;
    QTreeWidget* item_barcode_tree_ = nullptr;
    ItemWidget* item_barcode_item_widget_ = nullptr;
    QLabel* item_barcode_selection_count_ = nullptr;
    QLabel* item_barcode_visible_count_ = nullptr;
    QPushButton* item_card_print_ = nullptr;
    QWidget* item_barcode_print_page_ = nullptr;
    QWidget* person_view_page_ = nullptr;
    QPushButton* person_view_back_ = nullptr;
    QCheckBox* person_view_with_items_ = nullptr;
    QCheckBox* person_view_only_selected_ = nullptr;
    QPushButton* person_view_generate_ = nullptr;
    PersonListWidget* person_view_person_list_ = nullptr;
    QTextEdit* person_view_preview_ = nullptr;
    QPushButton* person_view_print_ = nullptr;
    QWidget* history_page_ = nullptr;
    QPushButton* back_button_ = nullptr;
    QLabel* history_from_label_ = nullptr;
    QDateEdit* history_from_date_ = nullptr;
    QLabel* history_to_label_ = nullptr;
    QDateEdit* history_to_date_ = nullptr;
    QPushButton* history_apply_button_ = nullptr;
    QCheckBox* history_show_all_ = nullptr;
    QComboBox* history_action_filter_ = nullptr;
    QTableView* combined_history_table_ = nullptr;
    QLabel* history_count_label_ = nullptr;
    QPushButton* history_load_more_ = nullptr;
    QPushButton* history_details_button_ = nullptr;
    QPushButton* history_change_button_ = nullptr;
    QPushButton* history_delete_button_ = nullptr;
    QWidget* settings_page_ = nullptr;
    QPushButton* settings_back_ = nullptr;
    QLineEdit* settings_collection_name_ = nullptr;
    QLineEdit* settings_organization_ = nullptr;
    QLineEdit* settings_responsible_ = nullptr;
    QPushButton* settings_browse_logo_ = nullptr;
    QLabel* settings_logo_ = nullptr;
    QCheckBox* settings_use_logo_ = nullptr;
    PixmapLabel* settings_logo_pixmap_ = nullptr;
    QCheckBox* settings_use_lending_period_ = nullptr;
    QSpinBox* settings_lending_period_years_ = nullptr;
    QSpinBox* settings_lending_period_months_ = nullptr;
    QSpinBox* settings_lending_period_days_ = nullptr;
    QCheckBox* settings_allow_reservations_ = nullptr;
    QSpinBox* settings_reservation_period_days_ = nullptr;
    QComboBox* settings_default_type_ = nullptr;
    QCheckBox* settings_default_is_schoolitem_ = nullptr;
    QListWidget* settings_sig_top_categories_ = nullptr;
    QPushButton* settings_add_sig_top_cat_ = nullptr;
    QPushButton* settings_remove_sig_top_cat_ = nullptr;
    QListWidget* settings_sig_bottom_categories_ = nullptr;
    QPushButton* settings_add_sig_bottom_cat_ = nullptr;
    QPushButton* settings_remove_sig_bottom_cat_ = nullptr;
    QPushButton* settings_revert_ = nullptr;
    QPushButton* settings_apply_ = nullptr;
    QWidget* preferences_page_ = nullptr;
    QPushButton* preferences_back_ = nullptr;
    QLineEdit* pref_thumbnail_dir_ = nullptr;
    QPushButton* pref_thumbnail_dir_browse_ = nullptr;
    QCheckBox* pref_search_openlibrary_ = nullptr;
    QCheckBox* pref_search_googleapis_ = nullptr;
    QTableWidget* pref_sru_servers_ = nullptr;
    QPushButton* pref_sru_add_ = nullptr;
    QPushButton* pref_sru_remove_ = nullptr;
    QCheckBox* pref_search_musicbrainz_ = nullptr;
    QCheckBox* pref_search_bgg_ = nullptr;
    QLineEdit* pref_bgg_token_ = nullptr;
    QLabel* pref_bgg_logo_ = nullptr;
    QCheckBox* pref_search_opengtindb_ = nullptr;
    QLineEdit* pref_opengtindb_userid_ = nullptr;
    QLineEdit* pref_proxy_user_ = nullptr;
    QLineEdit* pref_proxy_password_ = nullptr;
    QCheckBox* pref_download_thumbnails_ = nullptr;
    QSpinBox* pref_timeout_ = nullptr;
    QSpinBox* pref_poll_interval_ = nullptr;
    QSpinBox* pref_search_threshold_ = nullptr;
    QDoubleSpinBox* pref_barcode_unit_ = nullptr;
    QLineEdit* pref_googleapis_key_ = nullptr;
    QPushButton* pref_revert_ = nullptr;
    QPushButton* pref_apply_ = nullptr;
    QWidget* lending_person_ = nullptr;
    QPushButton* lending_person_back_ = nullptr;
    PersonListWidget* lending_person_person_list_ = nullptr;
    PersonWidget* lending_person_person_ = nullptr;
    QPushButton* lending_person_extend_ = nullptr;
    QPushButton* lending_person_next_ = nullptr;
    QWidget* lending_item_ = nullptr;
    QPushButton* lending_item_back_ = nullptr;
    ScalingLabel* label_472 = nullptr;
    QLineEdit* lending_item_barcode_ = nullptr;
    QToolButton* lending_item_camera_ = nullptr;
    QTableView* lending_item_item_view_ = nullptr;
    ItemWidget* lending_item_item_widget_ = nullptr;
    QLabel* lending_item_item_count_ = nullptr;
    QPushButton* lending_item_load_more_ = nullptr;
    QPushButton* lending_item_next_ = nullptr;
    QWidget* lending_details_ = nullptr;
    QPushButton* lending_details_back_ = nullptr;
    ScalingLabel* label_494 = nullptr;
    QLineEdit* lending_details_person_name_ = nullptr;
    QLineEdit* lending_details_item_name_ = nullptr;
    QLabel* lending_details_available_ = nullptr;
    QRadioButton* lending_details_lend_radio_ = nullptr;
    QRadioButton* lending_details_reserve_radio_ = nullptr;
    QSpinBox* lending_details_count_ = nullptr;
    QLabel* label_150 = nullptr;
    QDateTimeEdit* lending_details_datetime_ = nullptr;
    QWidget* lending_details_duedate_row_ = nullptr;
    QCheckBox* lending_details_due_date_check_ = nullptr;
    QDateEdit* lending_details_due_date_ = nullptr;
    QWidget* lending_details_expires_row_ = nullptr;
    QDateEdit* lending_details_expires_date_ = nullptr;
    QLabel* label_151 = nullptr;
    QPlainTextEdit* lending_details_remarks_ = nullptr;
    QPushButton* lending_details_next_ = nullptr;
    QWidget* lending_confirm_ = nullptr;
    QPushButton* lending_confirm_back_ = nullptr;
    QLineEdit* lending_confirm_person_ = nullptr;
    QLineEdit* lending_confirm_item_ = nullptr;
    QLineEdit* lending_confirm_number_ = nullptr;
    QLineEdit* lending_confirm_due_date_ = nullptr;
    QPlainTextEdit* lending_confirm_remarks_ = nullptr;
    QPushButton* lending_confirm_finish_ = nullptr;
    QWidget* lending_done_ = nullptr;
    QPushButton* lending_done_back_ = nullptr;
    QLabel* lending_done_question_ = nullptr;
    QPushButton* lending_done_yes_ = nullptr;
    QPushButton* lending_done_no_ = nullptr;
    QWidget* return_item_ = nullptr;
    QPushButton* return_item_back_ = nullptr;
    QLineEdit* return_item_barcode_ = nullptr;
    QToolButton* return_item_camera_ = nullptr;
    QTableView* return_item_item_view_ = nullptr;
    ItemWidget* return_item_item_widget_ = nullptr;
    QPushButton* return_item_next_ = nullptr;
    QWidget* return_borrower_ = nullptr;
    QPushButton* return_borrower_back_ = nullptr;
    PersonCheckListWidget* return_borrower_person_list_ = nullptr;
    QLabel* return_borrower_status_ = nullptr;
    QLabel* return_borrower_message_ = nullptr;
    PersonWidget* return_borrower_person_ = nullptr;
    QPushButton* return_borrower_next_ = nullptr;
    QWidget* return_lending_ = nullptr;
    QPushButton* return_lending_back_ = nullptr;
    QLabel* return_lending_person_name_ = nullptr;
    QLabel* return_lending_item_name_ = nullptr;
    QTableView* return_lending_lendings_view_ = nullptr;
    QPushButton* return_lending_next_ = nullptr;
    QWidget* return_details_ = nullptr;
    QPushButton* return_details_back_ = nullptr;
    ScalingLabel* label_164 = nullptr;
    QLabel* return_details_item_name_ = nullptr;
    QLabel* return_details_lent_count_ = nullptr;
    QLabel* return_details_returned_count_ = nullptr;
    QLabel* return_details_remaining_count_ = nullptr;
    QLabel* return_details_person_name_ = nullptr;
    QSpinBox* return_details_count_ = nullptr;
    QLabel* label_169 = nullptr;
    QDateTimeEdit* return_details_datetime_ = nullptr;
    QPlainTextEdit* return_details_remarks_ = nullptr;
    QPushButton* return_details_extend_ = nullptr;
    QFormLayout* return_details_counts_form_ = nullptr;
    QFormLayout* return_details_count_form_ = nullptr;
    QLabel* return_details_summary_ = nullptr;
    QPushButton* return_details_next_ = nullptr;
    QWidget* return_confirm_ = nullptr;
    QPushButton* return_confirm_back_ = nullptr;
    QLineEdit* return_confirm_person_ = nullptr;
    QLineEdit* return_confirm_item_ = nullptr;
    QLineEdit* return_confirm_number_ = nullptr;
    QLineEdit* return_confirm_datetime_ = nullptr;
    QPlainTextEdit* return_confirm_remarks_ = nullptr;
    QFormLayout* return_confirm_form_ = nullptr;
    QListWidget* return_confirm_persons_ = nullptr;
    QPushButton* return_confirm_finish_ = nullptr;
    QWidget* return_done_ = nullptr;
    QPushButton* return_done_back_ = nullptr;
    QPushButton* return_done_yes_ = nullptr;
    QPushButton* return_done_no_ = nullptr;
    QWidget* acquisition_select_ = nullptr;
    QPushButton* acquisition_select_back_ = nullptr;
    QLineEdit* acquisition_select_search_ = nullptr;
    QToolButton* acquisition_select_camera_ = nullptr;
    QComboBox* acquisition_select_type_ = nullptr;
    QPushButton* acquisition_select_clear_ = nullptr;
    QPushButton* acquisition_select_internet_search_ = nullptr;
    QTableView* acquisition_select_items_view_ = nullptr;
    ItemWidget* acquisition_select_item_ = nullptr;
    QLabel* acquisition_select_item_count_ = nullptr;
    QPushButton* acquisition_select_load_more_ = nullptr;
    QPushButton* acquisition_select_restore_ = nullptr;
    QPushButton* acquisition_select_change_item_ = nullptr;
    QPushButton* acquisition_select_new_item_ = nullptr;
    QPushButton* acquisition_select_more_copies_ = nullptr;
    QWidget* acquisition_count_ = nullptr;
    QPushButton* acquisition_count_back_ = nullptr;
    ScalingLabel* acquisition_count_item_name_ = nullptr;
    QSpinBox* acquisition_count_count_ = nullptr;
    QLabel* label_14 = nullptr;
    QDateTimeEdit* acquisition_count_datetime_ = nullptr;
    QPushButton* acquisition_count_finish_ = nullptr;
    QWidget* acquisition_done_ = nullptr;
    QPushButton* acquisition_done_yes_ = nullptr;
    QPushButton* acquisition_done_no_ = nullptr;
    QPushButton* acquisition_done_print_card_ = nullptr;
    QPushButton* acquisition_done_print_top_ = nullptr;
    QPushButton* acquisition_done_print_bottom_ = nullptr;
    QWidget* liquidation_item_ = nullptr;
    QPushButton* liquidation_item_back_ = nullptr;
    QLineEdit* liquidation_item_search_ = nullptr;
    QToolButton* liquidation_item_camera_ = nullptr;
    QTableView* liquidation_item_items_view_ = nullptr;
    ItemWidget* liquidation_item_item_ = nullptr;
    QLabel* liquidation_item_item_count_ = nullptr;
    QPushButton* liquidation_item_load_more_ = nullptr;
    QPushButton* liquidation_item_next_ = nullptr;
    QWidget* liquidation_count_ = nullptr;
    QPushButton* liquidation_count_back_ = nullptr;
    QLabel* liquidation_count_item_name_ = nullptr;
    QLabel* liquidation_count_available_ = nullptr;
    QSpinBox* liquidation_count_count_ = nullptr;
    QLabel* label_23 = nullptr;
    QDateTimeEdit* liquidation_count_datetime_ = nullptr;
    QPushButton* liquidation_count_previous_ = nullptr;
    QPushButton* liquidation_count_finish_ = nullptr;
    QWidget* liquidation_done_ = nullptr;
    QPushButton* liquidation_done_yes_ = nullptr;
    QPushButton* liquidation_done_no_ = nullptr;
    QWidget* group_lending_item_ = nullptr;
    QPushButton* group_lending_item_back_ = nullptr;
    QLineEdit* group_lending_item_barcode_ = nullptr;
    QToolButton* group_lending_item_camera_ = nullptr;
    QTableView* group_lending_item_view_ = nullptr;
    ItemWidget* group_lending_item_widget_ = nullptr;
    QLabel* group_lending_item_count_ = nullptr;
    QPushButton* group_lending_item_load_more_ = nullptr;
    QPushButton* group_lending_item_next_ = nullptr;
    QWidget* group_lending_persons_ = nullptr;
    QPushButton* group_lending_persons_back_ = nullptr;
    QLabel* group_lending_persons_item_name_ = nullptr;
    PersonCheckListWidget* group_lending_persons_list_ = nullptr;
    QLabel* group_lending_persons_status_ = nullptr;
    QLabel* group_lending_persons_message_ = nullptr;
    QPushButton* group_lending_persons_next_ = nullptr;
    QWidget* group_lending_details_ = nullptr;
    QPushButton* group_lending_details_back_ = nullptr;
    QLineEdit* group_lending_details_item_name_ = nullptr;
    QLineEdit* group_lending_details_persons_ = nullptr;
    QCheckBox* group_lending_details_due_date_check_ = nullptr;
    QDateEdit* group_lending_details_due_date_ = nullptr;
    QPlainTextEdit* group_lending_details_remarks_ = nullptr;
    QPushButton* group_lending_details_next_ = nullptr;
    QWidget* group_lending_confirm_ = nullptr;
    QPushButton* group_lending_confirm_back_ = nullptr;
    QLineEdit* group_lending_confirm_item_ = nullptr;
    QLineEdit* group_lending_confirm_number_ = nullptr;
    QLineEdit* group_lending_confirm_due_date_ = nullptr;
    QPlainTextEdit* group_lending_confirm_remarks_ = nullptr;
    QListWidget* group_lending_confirm_persons_ = nullptr;
    QPushButton* group_lending_confirm_finish_ = nullptr;
    QWidget* group_lending_done_ = nullptr;
    QPushButton* group_lending_done_back_ = nullptr;
    QLabel* group_lending_done_message_ = nullptr;
    QPushButton* group_lending_done_print_ = nullptr;
    QPushButton* group_lending_done_again_ = nullptr;
    QPushButton* group_lending_done_close_ = nullptr;

    /// Builds the complete main-window widget tree into @p window.
    void setup_ui(QMainWindow& window);

  private:
    // Page builders, one per stack page; each populates @p page.
    void build_item_type_page(QWidget* page);
    void build_person_page(QWidget* page);
    void build_person_barcode_page(QWidget* page);
    void build_item_barcode_page(QWidget* page);
    void build_person_view_page(QWidget* page);
    void build_history_page(QWidget* page);
    void build_settings_page(QWidget* page);
    void build_preferences_page(QWidget* page);
    void build_lending_person(QWidget* page);
    void build_lending_item(QWidget* page);
    void build_lending_details(QWidget* page);
    void build_lending_confirm(QWidget* page);
    void build_lending_done(QWidget* page);
    void build_return_item(QWidget* page);
    void build_return_borrower(QWidget* page);
    void build_return_lending(QWidget* page);
    void build_return_details(QWidget* page);
    void build_return_confirm(QWidget* page);
    void build_return_done(QWidget* page);
    void build_acquisition_select(QWidget* page);
    void build_acquisition_count(QWidget* page);
    void build_acquisition_done(QWidget* page);
    void build_liquidation_item(QWidget* page);
    void build_liquidation_count(QWidget* page);
    void build_liquidation_done(QWidget* page);
    void build_group_lending_item(QWidget* page);
    void build_group_lending_persons(QWidget* page);
    void build_group_lending_details(QWidget* page);
    void build_group_lending_confirm(QWidget* page);
    void build_group_lending_done(QWidget* page);

  public:
    /// Translates in the historic "MainWindow" context.
    /**
       Declared after the data members and setup_ui() because the macro ends
       with a private: access specifier; this keeps the members public.  Using
       the "MainWindow" context preserves the translations generated from the
       former colman.ui file.
    */
    Q_DECLARE_TR_FUNCTIONS(MainWindow)
  };

} // namespace colman

#endif /* #ifndef COLMAN_MAIN_WINDOW_UI_HPP */
/* EOF */
