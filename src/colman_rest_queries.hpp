// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// RestQueries: IQueries implementation that talks to a colman REST server.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_REST_QUERIES_HPP
#  define COLMAN_REST_QUERIES_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif
#  ifndef SYSTEM_QNETWORKACCESSMANAGER
#    include <QNetworkAccessManager>
#    define SYSTEM_QNETWORKACCESSMANAGER
#  endif
#  ifndef SYSTEM_QJSONOBJECT
#    include <QJsonObject>
#    define SYSTEM_QJSONOBJECT
#  endif
#  ifndef COLMAN_IQUERIES_HPP
#    include "colman_iqueries.hpp"
#  endif
#  ifndef COLMAN_REST_API_VERSION_HPP
#    include "colman_rest_api_version.hpp"
#  endif

namespace colman
{

/// Connection configuration for a remote colman REST server.
  struct RestConfig
  {
    QString base_url;  ///< Base URL, e.g. "https://host:8080" (no trailing slash)
    QString token;     ///< Bearer JWT (takes priority over username/password)
    QString username;  ///< HTTP Basic Auth username (used when token is empty)
    QString password;  ///< HTTP Basic Auth password (used when token is empty)
    int timeout_ms = 10000; ///< Network timeout in milliseconds
  };

/// Result of test_connection() — HTTP status plus parsed server error text.
  struct RestReply
  {
    int status = 0;         ///< HTTP status code; 0 on network/timeout error
    QString error_message;  ///< JSON "error" or "message" from body; empty if absent
  };

/// IQueries implementation that delegates all operations to a REST server.
/**
   Sends HTTP requests to a colman REST server and deserializes JSON
   responses back into the same domain types used by SQLQueries.  Methods
   that have no meaningful remote equivalent (database file management,
   local thumbnails, encryption) are implemented as stubs that return
   false / -1 / empty.

   Every public method that performs network I/O blocks the calling thread
   by running a local QEventLoop until the QNetworkReply finishes or the
   configured timeout elapses.
*/
  class RestQueries : public IQueries
  {
  public:
  /// Constructs an unconnected instance; call config() before use.
    RestQueries();
    ~RestQueries() override = default;
    RestQueries(RestQueries const&) = delete;
    RestQueries& operator=(RestQueries const&) = delete;
    RestQueries(RestQueries&&) = delete;
    RestQueries& operator=(RestQueries&&) = delete;

  /// Replaces the active configuration (base URL, token, timeout).
    void set_config(RestConfig const& config);
  /// Returns the active configuration.
    RestConfig const& config() const;
  /// Sends GET /api/collection; returns status and any JSON error message.
    RestReply test_connection();
  /// Fetches GET /api/version; returns {0,0} on network or parse error.
    ApiVersion fetch_api_version();
  /// Fetches GET /api/me; returns person_id (0=admin, >0=user, -1=error).
    int fetch_logged_in_person_id();

  // -------------------------------------------------------------------------
  // Stubs — not applicable to a remote client
  // -------------------------------------------------------------------------

  /// Not applicable to REST; always returns false.
    bool create_new_database(Collection&,
                             QString const&,
                             QString const&) override
    {
      return false;
    }
  /// Not applicable to REST; always returns false.
    bool create_new_database(Collection&, QString const&) override
    {
      return false;
    }
  /// Not applicable to REST; always returns false.
    bool create_new_database(Collection&) override { return false; }
  /// Not applicable to REST; always returns false.
    bool add_administrator() override { return false; }
  /// Not applicable to REST; always returns false.
    bool load_database(QString const&) override { return false; }
  /// Not applicable to REST; always returns false.
    bool load_database(QString const&, QString const&) override
    {
      return false;
    }
  /// Not applicable to REST; always returns false.
    bool save_database(QString const&) override { return false; }
  /// Not applicable to REST; always returns false.
    bool supports_encryption() const override { return false; }
  /// Not applicable to REST; always returns false.
    bool is_encrypted() const override { return false; }
  /// Not applicable to REST; always returns false.
    bool encrypt_database(QString const&) override { return false; }
  /// Not applicable to REST; always returns false.
    bool change_encryption_key(QString const&,
                               QString const&) override
    {
      return false;
    }
  /// Not applicable to REST; always returns false.
    bool remove_encryption(QString const&) override { return false; }
  /// Not applicable to REST; always returns false.
    bool create_tables() override { return false; }
  /// Not applicable to REST; always returns false.
    bool create_collection(Collection const&) override { return false; }
  /// No card/label layout is stored remotely; always empty.
    QString read_id_label_layout_name() override { return {}; }
  /// No card/label layout is stored remotely; always empty.
    QString read_item_card_label_layout_name() override { return {}; }
  /// No-op for REST; item is left unchanged.
    void restrict_item_to_type(Item&) override {}
  /// Not applicable to REST; always returns -1.
    int use_local_thumbnail(QString const&) override { return -1; }
  /// Not applicable to REST; always returns false.
    bool have_local_thumbnail(int) override { return false; }
  // -------------------------------------------------------------------------
  // Local behaviour — no HTTP needed
  // -------------------------------------------------------------------------

  /// Returns true; the REST server always supports the system locale.
    bool is_locale_supported() const override { return true; }
  /// Returns true when a base URL has been configured.
    bool is_open() const override;

  // -------------------------------------------------------------------------
  // Collection
  // -------------------------------------------------------------------------

    Collection read_collection() override;
    bool change_collection(Collection const& collection) override;

  // -------------------------------------------------------------------------
  // Signature categories
  // -------------------------------------------------------------------------

    std::vector<QString> read_signature_top_categories() override;
    bool add_signature_top_category(QString const& name) override;
    bool remove_signature_top_category(QString const& name) override;
    std::vector<QString>
    read_signature_bottom_categories(QString const& top) override;
    bool add_signature_bottom_category(QString const& top,
                                       QString const& name) override;
    bool remove_signature_bottom_category(QString const& top,
                                          QString const& name) override;

  // -------------------------------------------------------------------------
  // Item types
  // -------------------------------------------------------------------------

    int add_new_type(Type const& type,
                     QString const& tablename = {}) override;
    int get_type_id(QString const& type) override;
    int type_id_for_media_kind(MediaKind kind) override;
    MediaKind media_kind_for_type_id(int type_id) override;
    Type get_type(int type_id) override;
    QString get_type_name(int type_id) override;
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
    bool save_field_order(int type_id,
                          QStringList const& field_order) override;
    std::vector<QString> get_type_names() override;
    QString get_type_field_label(int type_id, QString const& field,
                                 QLocale::Language lang) override;
    std::vector<TypeFieldMeta> get_type_field_metas(int type_id) override;

  // -------------------------------------------------------------------------
  // Items
  // -------------------------------------------------------------------------

    int add_new_item(Item const& item) override;
    bool get_item(int id, Item& item, QString& type_name) override;
    bool remove_item(int id) override;
    bool force_remove_item(int id) override;
    bool change_item(int id, Item const& item) override;
    unsigned int available(int item_id) override;
    unsigned int stock_size(int item_id) override;
    bool have_lendings(int item_id) override;
    std::vector<std::pair<int, int>>
    open_lendings_for_item(int item_id) override;
    std::vector<std::pair<int, int>>
    open_lendings_for_person(int person_id) override;
    std::vector<int> items_by_isbn(QString const& isbn) override;
    std::vector<int> items_by_ean(QString const& ean) override;
    QString compute_item_barcode(int type_id, int item_id) override;
    bool parse_item_barcode(QString const& barcode,
                            int& type_id, int& item_id) override;
    bool is_item_barcode(QString const& barcode) override;
    std::vector<std::pair<int, QString>>
    get_items_ordered_by_name() override;
    std::vector<int> get_recently_acquired_item_ids(int count) override;
    std::vector<int> get_items_acquired_between(QDateTime const& from,
                                                QDateTime const& to) override;
    std::vector<ItemTypeThumbnail>
    get_items_with_type_and_thumbnails() override;

  // -------------------------------------------------------------------------
  // Acquisitions
  // -------------------------------------------------------------------------

    int add_new_acquisition(Acquisition const& acquisition) override;
    bool get_acquisition(int id, Acquisition& acquisition) override;
    bool change_acquisition(int id,
                            Acquisition const& acquisition) override;
    bool remove_acquisition(int id) override;

  // -------------------------------------------------------------------------
  // Liquidations
  // -------------------------------------------------------------------------

    int add_new_liquidation(Liquidation const& liquidation) override;
    bool get_liquidation(int id, Liquidation& liquidation) override;
    bool change_liquidation(int id,
                            Liquidation const& liquidation) override;
    bool remove_liquidation(int id) override;

  // -------------------------------------------------------------------------
  // Returns
  // -------------------------------------------------------------------------

    int add_new_return(Return const& ret) override;
    bool get_return(int return_id, Return& ret) override;
    bool change_return(int return_id, Return const& ret) override;
    bool remove_return(int return_id) override;

  // -------------------------------------------------------------------------
  // Lendings
  // -------------------------------------------------------------------------

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

  // -------------------------------------------------------------------------
  // Persons
  // -------------------------------------------------------------------------

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
    bool parse_person_barcode(QString const& barcode,
                              int& person_id) override;
    int get_person_by_external_id(QString const& sub) override;
    bool set_person_external_id(int person_id,
                                QString const& sub) override;
    int check_local_credentials(QString const& username,
                                QString const& password) override;
    bool set_local_credentials(int person_id,
                               QString const& username,
                               QString const& password_hash) override;
    bool delete_local_credentials(int person_id) override;
    QString get_username(int person_id) override;
    bool set_username(int person_id, QString const& username) override;
    bool remove_username(int person_id) override;

  // -------------------------------------------------------------------------
  // Permissions
  // -------------------------------------------------------------------------

    std::vector<QString> get_permissions() const override;
    std::vector<bool> get_user_permissions(int person_id) override;
    bool create_default_permissions(int person_id) override;
    bool has_permission(int person_id,
                        QString const& column) override;
    bool set_permission(int person_id, QString const& column,
                        bool value) override;
    bool remove_permissions(int person_id) override;
    bool is_reserve_only(int person_id) override;
    bool set_password(int person_id, QString const& hash) override;
    bool get_password(int person_id, QString& hash) override;
    bool has_password(int person_id) override;
    bool remove_password(int person_id) override;
    bool change_own_password(int person_id, QString const& old_password,
                             QString const& new_hash) override;

  // -------------------------------------------------------------------------
  // Thumbnails
  // -------------------------------------------------------------------------

    int store_thumbnail(QByteArray const& png, QString const& key,
                        QString const& url,
                        bool overwrite = false) override;
    bool get_thumbnail(int thumbnail_id, QByteArray& png) override;

  // -------------------------------------------------------------------------
  // View-model queries
  // -------------------------------------------------------------------------

    std::vector<Type> query_types(
        int type_id,
        QLocale::Language lang = QLocale::AnyLanguage) override;
    std::vector<Person> query_persons(int person_id,
                                      QString const& search) override;
    std::vector<int> get_recently_created_person_ids(int count) override;
    std::vector<Person> query_lending_persons(
        int item_id, int lending_id,
        QString const& person_search,
        QString const& item_search) override;
    std::vector<StartPagePersonListModel::Row>
    query_start_page_persons(QString const& search) override;
    int count_items() override;
    std::vector<ItemListModel::Row>
    query_items(int item_id, QString const& search,
                QLocale::Language lang = QLocale::AnyLanguage) override;
    std::vector<ItemListModel::Row>
    query_acquired_items(int item_id, QString const& search,
                         int limit = 0, int offset = 0,
                         QLocale::Language lang =
                             QLocale::AnyLanguage) override;
    std::vector<ItemListModel::Row>
    query_available_items(int item_id,
                          QString const& search,
                          int limit = 0, int offset = 0,
                          QLocale::Language lang =
                              QLocale::AnyLanguage) override;
    std::vector<ItemListModel::Row>
    query_lent_items(int person_id, int lending_id,
                     QString const& person_search,
                     QString const& item_search,
                     QLocale::Language lang =
                         QLocale::AnyLanguage) override;
    std::vector<ItemListModel::Row>
    query_items_with_type(int type_id,
                          QString const& search,
                          int limit = 0,
                          int offset = 0,
                          QLocale::Language lang =
                              QLocale::AnyLanguage) override;
    std::vector<SearchItemRow> query_items_for_search(
        QString const& general,
        QString const& name,
        QString const& subject,
        QString const& description,
        QString const& keywords,
        int type_id,
        std::vector<TypeFieldFilter> const& type_fields,
        int grade_min,
        int grade_max,
        int limit,
        int offset,
        QLocale::Language lang = QLocale::AnyLanguage) override;
    std::vector<OpenLendingListModel::Row>
    query_open_lendings(int person_id, int item_id) override;
    std::vector<LendingPersonInfo>
    query_item_lendings(int item_id) override;
    QStringList query_type_field_completions(
        int type_id, QString const& field) override;
    QStringList query_person_field_completions(
        QString const& field) override;
    QStringList query_item_field_completions(
        QString const& field) override;
    QStringList query_school_item_field_completions(
        QString const& field) override;
    std::vector<HistoryCombinedModel::Row>
    query_combined_history(bool is_admin = false,
                           QDate const& from_date = QDate(),
                           QDate const& to_date = QDate(),
                           bool show_all = false,
                           int limit = 0, int offset = 0,
                           QString const& action_filter = {}) override;
    std::vector<ChangePasswordsPersonListModel::Row>
    query_change_passwords_persons() override;
    std::vector<HistoryCombinedModel::Row>
    find_conflicting_actions(
        HistoryCombinedModel::Row const& action) override;
    bool batch_delete_history_actions(
        std::vector<HistoryCombinedModel::Row> const& actions) override;
    std::vector<HistoryCombinedModel::Row>
    find_change_conflicts(
        HistoryCombinedModel::Row const& old_action,
        HistoryCombinedModel::Row const& new_action) override;
    bool apply_action_change(
        HistoryCombinedModel::Row const& old_action,
        HistoryCombinedModel::Row const& new_action) override;
    std::vector<DueLendingListModel::Row>
    query_due_lendings(
        QDate const& from_date = QDate(),
        QDate const& to_date = QDate(),
        int person_id = -1,
        QString const& school_class_filter = {},
        QString const& person_name_filter = {}) override;
    bool extend_lendings(std::vector<int> const& lending_ids,
                         QDate const& duedate) override;
    QStringList load_class_names() override;
    bool add_class_name(QString const& name) override;
    bool remove_class_name(QString const& name) override;
    bool rename_class_name(QString const& old_name,
                           QString const& new_name) override;
    std::vector<AffectedPerson>
    get_affected_persons(
        QMap<QString, QString> const& old_to_new) override;
    bool apply_school_year_changes(
        QMap<int, QString> const& person_id_to_new_class,
        QList<int> const& persons_to_delete,
        QStringList const& new_class_list) override;

  // -------------------------------------------------------------------------
  // Concurrency / versioned updates
  // -------------------------------------------------------------------------

    int get_version(VersionedEntity entity, int id) override;
    TableVersions query_table_versions() override;
    UpdateResult change_item_versioned(
        int id, Item const& item,
        int expected_version) override;
    UpdateResult change_person_versioned(
        int id, Person const& person,
        int expected_version) override;
    UpdateResult change_lending_versioned(
        int id, Lending const& lending,
        int expected_version) override;
    UpdateResult change_return_versioned(
        int return_id, Return const& ret,
        int expected_version) override;
    UpdateResult change_acquisition_versioned(
        int id, Acquisition const& acquisition,
        int expected_version) override;
    UpdateResult change_liquidation_versioned(
        int id, Liquidation const& liquidation,
        int expected_version) override;

  // -------------------------------------------------------------------------
  // Reservations
  // -------------------------------------------------------------------------

    ReservationResult add_reservation(int item_id, int person_id, int count,
                                      QDate const& expires_at,
                                      int data_collector_id,
                                      int& new_id) override;
    bool remove_reservation(int reservation_id) override;
    std::vector<Reservation> get_reservations_for_item(int item_id) override;
    bool get_reservation(int item_id, int person_id, Reservation& r) override;
    std::vector<Reservation> get_reservations(int person_id) override;

  private:
  /// Raw HTTP response.
    struct Reply
    {
      int status = 0;   ///< HTTP status code, 0 on network error
      QByteArray body;  ///< Response body bytes
      QByteArray etag;  ///< Value of the ETag response header, or empty
    };

  /// Sends an HTTP request and blocks until the reply arrives or times out.
  /**
     @param method   HTTP verb: "GET", "POST", "PUT", "DELETE"
     @param path     Path including query string, e.g. "/api/items?id=1"
     @param body     Request body bytes (empty for GET/DELETE)
     @param content_type  Content-Type header value for non-empty bodies
     @return Reply with status and body; status==0 on network error
  */
    Reply send(QString const& method, QString const& path,
               QByteArray const& body = {},
               QString const& content_type =
                   QStringLiteral("application/json"));

  /// Sends an HTTP request with an If-Match header for versioned updates.
  /**
     @param method           HTTP verb (typically "PUT")
     @param path             Request path
     @param body             Request body bytes
     @param expected_version Value sent as If-Match header
     @return Reply with status and body; status==0 on network error
  */
    Reply send_versioned(QString const& method, QString const& path,
                         QByteArray const& body,
                         int expected_version);

    RestConfig config_;            ///< Base URL, token and timeout
    QNetworkAccessManager nam_;    ///< Shared network access manager
    int logged_in_person_id_ = -1;         ///< person_id from /api/me (set at login)
    QJsonObject logged_in_permissions_;    ///< permissions from /api/me (set at login)
    QMap<int, QMap<QString, QString>>
        type_label_cache_; ///< Cache: type_id -> (column -> label)
  };

} // namespace colman

#endif /* #ifndef COLMAN_REST_QUERIES_HPP */

/* EOF */
