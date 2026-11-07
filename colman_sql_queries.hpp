// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// SQLQueries: IQueries implementation backed by a QSqlDatabase.
/**
   @file
   @author Christoph Pesch
   @see colman_sql_queries_types.hpp, colman_sql_queries_items.hpp, colman_sql_queries_persons.hpp, colman_sql_queries_movements.hpp, colman_sql_queries_history.hpp.
 */

#ifndef COLMAN_SQL_QUERIES_HPP
#  define COLMAN_SQL_QUERIES_HPP

#  ifndef SYSTEM_QSQLDATABASE
#    include <QSqlDatabase>
#    define SYSTEM_QSQLDATABASE
#  endif
#  ifndef COLMAN_IQUERIES_HPP
#    include "colman_iqueries.hpp"
#  endif
#  ifndef SYSTEM_FUNCTIONAL
#    include <functional>
#    define SYSTEM_FUNCTIONAL
#  endif

namespace colman
{
  /// IQueries implementation that executes queries against a QSqlDatabase.
  class SQLQueries : public IQueries
  {
  public:
    /// Tag type: construct a file-backed database connection.
    struct FileDb
    {};

    /// Registers the QSQLITE driver; file opened later via load_database().
    explicit SQLQueries(FileDb,
                        std::function<void()> on_backup_error = {},
                        std::function<bool()> on_old_backups = {});
    /// Opens an in-memory SQLite database with the given connection name.
    explicit SQLQueries(QString const& conn_name);
    ~SQLQueries() override;
    SQLQueries(SQLQueries const&) = delete;
    SQLQueries& operator=(SQLQueries const&) = delete;
    SQLQueries(SQLQueries&&) = delete;
    SQLQueries& operator=(SQLQueries&&) = delete;

    // -----------------------------------------------------------------------
    // Database file management
    // -----------------------------------------------------------------------

    bool create_new_database(Collection& collection) override;
    bool create_new_database(Collection& collection,
                             QString const& fname) override;
    bool create_new_database(Collection& collection, QString const& fname,
                             QString const& key) override;
    bool add_administrator() override;
    bool load_database(QString const& fname) override;
    bool load_database(QString const& fname, QString const& key) override;
    bool save_database(QString const& fname) override;
    bool supports_encryption() const override;
    bool is_encrypted() const override;
    bool encrypt_database(QString const& new_key) override;
    bool change_encryption_key(QString const& old_key,
                               QString const& new_key) override;
    bool remove_encryption(QString const& current_key) override;
    bool create_tables() override;
    std::vector<QString> get_table_columns(QString const& table,
                                           int start_index);
    bool is_open() const override;
    bool is_locale_supported() const override;

    // -----------------------------------------------------------------------
    // Collection metadata
    // -----------------------------------------------------------------------

    bool create_collection(Collection const& collection) override;
    bool change_collection(Collection const& collection) override;
    Collection read_collection() override;
    QString read_id_label_layout_name() override;
    QString read_item_card_label_layout_name() override;

    // -----------------------------------------------------------------------
    // Signature categories
    // -----------------------------------------------------------------------

    std::vector<QString> read_signature_top_categories() override;
    bool add_signature_top_category(QString const& name) override;
    bool remove_signature_top_category(QString const& name) override;
    std::vector<QString> read_signature_bottom_categories(
        QString const& top) override;
    bool add_signature_bottom_category(QString const& top,
                                       QString const& name) override;
    bool remove_signature_bottom_category(QString const& top,
                                          QString const& name) override;

    // -----------------------------------------------------------------------
    // Item types
    // -----------------------------------------------------------------------

    int add_new_type(Type const& type,
                     QString const& tablename = {}) override;
    int get_type_id(QString const& type) override;
    int type_id_for_media_kind(MediaKind kind) override;
    MediaKind media_kind_for_type_id(int type_id) override;
    Type get_type(int type_id) override;
    QString get_type_name(int type_id) override;
    QString get_type_tablename(int type_id);
    std::vector<std::pair<int, QString>> get_type_locale(
        QLocale::Language lang = QLocale::AnyLanguage) override;
    QString get_type_locale(
        int type_id,
        QLocale::Language lang = QLocale::AnyLanguage) override;
    QString get_type_locale(
        QString const& type,
        QLocale::Language lang = QLocale::AnyLanguage) override;
    bool change_type(int type_id, Type const& new_type,
                     QMap<QString, QString> const& display_names) override;
    bool remove_type(int type_id) override;
    QStringList get_combined_field_order(int type_id) override;
    QStringList get_default_common_field_order() override;
    bool save_field_order(int type_id, QStringList const& field_order) override;
    std::vector<QString> get_type_names() override;
    QString get_type_field_label(int type_id, QString const& field,
                                 QLocale::Language lang) override;
    QString get_tablename(int type_id);
    QString get_translation(QString const& table, QString const& column,
                            QString const& language);
    QString get_default_name(QString const& table,
                             QString const& column);

    // -----------------------------------------------------------------------
    // Items
    // -----------------------------------------------------------------------

    int add_new_item(Item const& item) override;
    bool get_item(int id, Item& item, QString& type_name) override;
    bool remove_item(int id) override;
    bool force_remove_item(int id) override;
    bool change_item(int id, Item const& item) override;
    void restrict_item_to_type(Item& item) override;
    unsigned int available(int item_id) override;
    unsigned int stock_size(int item_id) override;
    bool have_lendings(int item_id) override;
    std::vector<std::pair<int, int>> open_lendings_for_item(
        int item_id) override;
    std::vector<std::pair<int, int>> open_lendings_for_person(
        int person_id) override;
    std::vector<int> items_by_isbn(QString const& isbn) override;
    std::vector<int> items_by_ean(QString const& ean) override;
    QString compute_item_barcode(int type_id, int item_id) override;
    bool parse_item_barcode(QString const& barcode, int& type_id,
                            int& item_id) override;
    bool is_item_barcode(QString const& barcode) override;
    std::vector<std::pair<int, QString>> get_items_ordered_by_name() override;
    std::vector<int> get_recently_acquired_item_ids(int count) override;
    std::vector<int> get_items_acquired_between(QDateTime const& from,
                                                QDateTime const& to) override;
    std::vector<ItemTypeThumbnail> get_items_with_type_and_thumbnails()
        override;

    // -----------------------------------------------------------------------
    // Acquisitions
    // -----------------------------------------------------------------------

    int add_new_acquisition(Acquisition const& acquisition) override;
    bool get_acquisition(int id, Acquisition& acquisition) override;
    bool change_acquisition(int id, Acquisition const& acquisition) override;
    bool remove_acquisition(int id) override;

    // -----------------------------------------------------------------------
    // Liquidations
    // -----------------------------------------------------------------------

    int add_new_liquidation(Liquidation const& liquidation) override;
    bool get_liquidation(int id, Liquidation& liquidation) override;
    bool change_liquidation(int id, Liquidation const& liquidation) override;
    bool remove_liquidation(int id) override;

    // -----------------------------------------------------------------------
    // Returns
    // -----------------------------------------------------------------------

    int add_new_return(Return const& ret) override;
    bool get_return(int return_id, Return& ret) override;
    bool change_return(int return_id, Return const& ret) override;
    bool remove_return(int return_id) override;

    // -----------------------------------------------------------------------
    // Lendings
    // -----------------------------------------------------------------------

    int add_new_lending(Lending const& lending) override;
    bool get_lending(int id, Lending& lending) override;
    bool change_lending(int id, Lending const& lending) override;
    bool remove_lending(int id) override;
    bool lending_balance(int lending_id, std::size_t& lent,
                         std::size_t& returned) override;
    GroupLendingResult add_group_lending(
        Lending const& lending,
        std::vector<int> const& person_ids) override;
    GroupReturnResult add_group_return(
        int item_id, std::vector<int> const& person_ids,
        Return const& tmpl) override;
    std::vector<LendingInfo> query_lendings(int person_id) override;

    // -----------------------------------------------------------------------
    // Persons
    // -----------------------------------------------------------------------

    int add_new_person(Person const& person) override;
    bool get_person(int id, Person& person) override;
    int get_person(QString const& barcode) override;
    bool change_person(int id, Person const& person) override;
    bool person_known(QString const& lastname,
                      QString const& firstname) override;
    bool email_known(QString const& emails) override;
    bool username_known(QString const& username) override;
    bool remove_person(int id) override;
    bool has_non_admin_persons() override;
    QString compute_person_barcode(int person_id) override;
    bool is_person_barcode(QString const& str) override;
    bool parse_person_barcode(QString const& barcode, int& person_id) override;
    int get_person_by_external_id(QString const& sub) override;
    bool set_person_external_id(int person_id, QString const& sub) override;
    int check_local_credentials(QString const& username,
                                QString const& password) override;
    bool set_local_credentials(int person_id,
                               QString const& username,
                               QString const& password_hash) override;
    bool delete_local_credentials(int person_id) override;
    QString get_username(int person_id) override;
    bool set_username(int person_id, QString const& username) override;
    bool remove_username(int person_id) override;

    // -----------------------------------------------------------------------
    // Permissions
    // -----------------------------------------------------------------------

    bool create_default_permissions(int person_id) override;
    bool has_permission(int person_id, QString const& column) override;
    bool set_permission(int person_id, QString const& column,
                        bool value) override;
    bool remove_permissions(int person_id) override;
    bool is_reserve_only(int person_id) override;
    std::vector<QString> get_permissions() const override;
    std::vector<bool> get_user_permissions(int person_id) override;

    // -----------------------------------------------------------------------
    // Passwords
    // -----------------------------------------------------------------------

    bool set_password(int person_id, QString const& hash) override;
    bool get_password(int person_id, QString& hash) override;
    bool has_password(int person_id) override;
    bool remove_password(int person_id) override;
    bool change_own_password(int person_id, QString const& old_password,
                             QString const& new_hash) override;

    // -----------------------------------------------------------------------
    // Thumbnails
    // -----------------------------------------------------------------------

    void set_thumbnail_dir(QString const& dir);
    QString thumbnail_dir() const;
    int store_thumbnail(QByteArray const& png, QString const& key,
                        QString const& url, bool overwrite = false) override;
    bool get_thumbnail(int thumbnail_id, QByteArray& png) override;
    int use_local_thumbnail(QString const& key) override;
    bool have_local_thumbnail(int thumbnail_id) override;

    // -----------------------------------------------------------------------
    // Translations
    // -----------------------------------------------------------------------

    bool add_translation(QString const& tablename, QString const& column,
                         QLocale::Language language,
                         QString const& name);
    QString get_translation(QString const& tablename, QString const& column,
                            QLocale::Language language);

    // -----------------------------------------------------------------------
    // View-model queries (existing)
    // -----------------------------------------------------------------------

    std::vector<Type> query_types(
        int type_id,
        QLocale::Language lang = QLocale::AnyLanguage) override;

    std::vector<Person> query_persons(int person_id,
                                      QString const& search) override;
    std::vector<int> get_recently_created_person_ids(int count) override;
    std::vector<Person> query_lending_persons(
        int item_id, int lending_id, QString const& person_search,
        QString const& item_search) override;

    std::vector<StartPagePersonListModel::Row> query_start_page_persons(
        QString const& search) override;

    int count_items() override;
    std::vector<ItemListModel::Row> query_items(
        int item_id, QString const& search,
        QLocale::Language lang = QLocale::AnyLanguage) override;
    std::vector<ItemListModel::Row> query_acquired_items(
        int item_id, QString const& search,
        int limit = 0, int offset = 0,
        QLocale::Language lang = QLocale::AnyLanguage) override;
    std::vector<ItemListModel::Row> query_available_items(
        int item_id, QString const& search,
        int limit = 0, int offset = 0,
        QLocale::Language lang = QLocale::AnyLanguage) override;
    std::vector<ItemListModel::Row> query_lent_items(
        int person_id, int lending_id, QString const& person_search,
        QString const& item_search,
        QLocale::Language lang = QLocale::AnyLanguage) override;
    std::vector<ItemListModel::Row> query_items_with_type(
        int type_id, QString const& search,
        int limit = 0, int offset = 0,
        QLocale::Language lang = QLocale::AnyLanguage) override;

    std::vector<TypeFieldMeta> get_type_field_metas(int type_id) override;

    std::vector<SearchItemRow> query_items_for_search(
        QString const& general, QString const& name, QString const& subject,
        QString const& description, QString const& keywords, int type_id,
        std::vector<TypeFieldFilter> const& type_fields,
        int grade_min, int grade_max,
        int limit, int offset,
        QLocale::Language lang = QLocale::AnyLanguage) override;

    std::vector<OpenLendingListModel::Row> query_open_lendings(
        int person_id, int item_id) override;
    std::vector<LendingPersonInfo> query_item_lendings(int item_id) override;

    QStringList query_type_field_completions(
        int type_id, QString const& field) override;
    QStringList query_person_field_completions(
        QString const& field) override;
    QStringList query_item_field_completions(
        QString const& field) override;
    QStringList query_school_item_field_completions(
        QString const& field) override;

    std::vector<HistoryCombinedModel::Row> query_combined_history(
        bool is_admin = false, QDate const& from_date = QDate(),
        QDate const& to_date = QDate(),
        bool show_all = false,
        int limit = 0, int offset = 0,
        QString const& action_filter = {}) override;

    std::vector<ChangePasswordsPersonListModel::Row>
    query_change_passwords_persons() override;

    std::vector<DueLendingListModel::Row> query_due_lendings(
        QDate const& from_date = QDate(), QDate const& to_date = QDate(),
        int person_id = -1, QString const& school_class_filter = {},
        QString const& person_name_filter = {}) override;

    bool extend_lendings(std::vector<int> const& lending_ids,
                         QDate const& duedate) override;

    std::vector<HistoryCombinedModel::Row> find_conflicting_actions(
        HistoryCombinedModel::Row const& action) override;

    bool batch_delete_history_actions(
        std::vector<HistoryCombinedModel::Row> const& actions) override;

    std::vector<HistoryCombinedModel::Row> find_change_conflicts(
        HistoryCombinedModel::Row const& old_action,
        HistoryCombinedModel::Row const& new_action) override;

    bool apply_action_change(
        HistoryCombinedModel::Row const& old_action,
        HistoryCombinedModel::Row const& new_action) override;

    QStringList load_class_names() override;
    bool add_class_name(QString const& name) override;
    bool remove_class_name(QString const& name) override;
    bool rename_class_name(QString const& old_name,
                           QString const& new_name) override;
    std::vector<IQueries::AffectedPerson>
    get_affected_persons(QMap<QString, QString> const& old_to_new) override;
    bool apply_school_year_changes(
        QMap<int, QString> const& person_id_to_new_class,
        QList<int> const& persons_to_delete,
        QStringList const& new_class_list) override;

    int get_version(VersionedEntity entity, int id) override;
    TableVersions query_table_versions() override;

    UpdateResult change_item_versioned(
        int id, Item const& item, int expected_version) override;
    UpdateResult change_person_versioned(
        int id, Person const& person, int expected_version) override;
    UpdateResult change_lending_versioned(
        int id, Lending const& lending, int expected_version) override;
    UpdateResult change_return_versioned(
        int return_id, Return const& ret, int expected_version) override;
    UpdateResult change_acquisition_versioned(
        int id, Acquisition const& acquisition,
        int expected_version) override;
    UpdateResult change_liquidation_versioned(
        int id, Liquidation const& liquidation,
        int expected_version) override;

    // -----------------------------------------------------------------------
    // Reservations
    // -----------------------------------------------------------------------

    ReservationResult add_reservation(int item_id, int person_id, int count,
                                      QDate const& expires_at,
                                      int data_collector_id,
                                      int& new_id) override;
    bool remove_reservation(int reservation_id) override;
    std::vector<Reservation> get_reservations_for_item(int item_id) override;
    bool get_reservation(int item_id, int person_id,
                         Reservation& r) override;
    std::vector<Reservation> get_reservations(int person_id) override;

  private:
    /// Adds the types.media_kind column and back-fills built-in types if the
    /// column is missing.  Idempotent; safe to call on every database open.
    bool ensure_media_kind_column();
    /// Seeds the default signature categories in the UI language.
    bool seed_signature_categories();
    /// Stores German and English display names of the predefined types.
    /** Only fills missing or raw names; idempotent, safe on every open. */
    bool ensure_predefined_translations();
    int get_lend_item(int lend_id);
    bool recompute_item_counts(int item_id);
    QString lending_query_base(int person_id, int item_id, int lending_id,
                               QString const& person_search,
                               QString const& item_search,
                               bool show_returned = true);
    bool add_person(int id, Person const& person);
    bool add_predefined_books_type();
    bool add_predefined_periodical_type();
    bool add_predefined_cd_type();
    bool add_predefined_dvd_type();
    bool add_predefined_bluray_type();
    bool add_predefined_board_game_type();
    bool add_to_available(int item_id, unsigned int count);
    bool subtract_from_available(int item_id, unsigned int count);
    bool add_predefined_types();

    /// Invalidates the collection metadata cache.
    void invalidate_collection_cache();
    /// Rebuilds the FTS5 index row for a single item.
    void rebuild_fts_for_item(int item_id);
    /// Rebuilds the entire items_fts table from scratch.
    void rebuild_all_fts();
    /// Writes a backup unconditionally.
    void backup_once();
    /// Increments the change counter and writes a backup when the threshold is reached.
    void backup_after_change();

    QString thumbnail_dir_;  ///< root directory for thumbnail files on disk
    QString conn_name_;      ///< Qt connection name (empty = default)
    QSqlDatabase db_;        ///< the database connection
    QString encryption_key_; ///< non-empty if database is encrypted
    mutable Collection
        collection_cache_; ///< cached result of read_collection()
    mutable bool collection_cache_valid_ =
        false; ///< true when cache is current
    mutable int collection_cache_version_ =
        -1; ///< collection table version the cache was read at
    std::function<void()> on_backup_error_; ///< called when a backup write fails
    std::function<bool()> on_old_backups_;  ///< called when old backups pile up; return true to delete them
    int changes_since_last_backup_ = 0;     ///< changes since last automatic backup
    bool session_backup_done_ = false;      ///< true after the first backup this session
  };

} // namespace colman

#endif /* #ifndef COLMAN_SQL_QUERIES_HPP */

/* EOF */
