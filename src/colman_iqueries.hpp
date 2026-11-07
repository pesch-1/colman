// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Abstract interface for all colman SQL query operations.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_IQUERIES_HPP
#  define COLMAN_IQUERIES_HPP

#  ifndef SYSTEM_QSTRING
#    include <QString>
#    define SYSTEM_QSTRING
#  endif
#  ifndef SYSTEM_QSTRINGLIST
#    include <QStringList>
#    define SYSTEM_QSTRINGLIST
#  endif
#  ifndef SYSTEM_QDATETIME
#    include <QDateTime>
#    define SYSTEM_QDATETIME
#  endif
#  ifndef SYSTEM_VECTOR
#    include <vector>
#    define SYSTEM_VECTOR
#  endif
#  ifndef SYSTEM_CSTDDEF
#    include <cstddef>
#    define SYSTEM_CSTDDEF
#  endif
#  ifndef SYSTEM_QMAP
#    include <QMap>
#    define SYSTEM_QMAP
#  endif
#  ifndef SYSTEM_QLOCALE
#    include <QLocale>
#    define SYSTEM_QLOCALE
#  endif
#  ifndef SYSTEM_QBYTEARRAY
#    include <QByteArray>
#    define SYSTEM_QBYTEARRAY
#  endif
#  ifndef COLMAN_TABLE_MODELS_HPP
#    include "colman_table_models.hpp"
#  endif
#  ifndef COLMAN_COLLECTION_HPP
#    include "colman_collection.hpp"
#  endif
#  ifndef COLMAN_ITEM_HPP
#    include "colman_item.hpp"
#  endif
#  ifndef COLMAN_LENDING_HPP
#    include "colman_lending.hpp"
#  endif
#  ifndef COLMAN_RETURN_HPP
#    include "colman_return.hpp"
#  endif
#  ifndef COLMAN_ACQUISITION_HPP
#    include "colman_acquisition.hpp"
#  endif
#  ifndef COLMAN_LIQUIDATION_HPP
#    include "colman_liquidation.hpp"
#  endif


namespace colman
{
  /// Abstract interface for all SQL database operations.
  /**
     Combines CRUD operations for all domain entities with the read-only
     query methods that back the view models.  Implementations hold the
     actual database connection; callers depend only on this interface.
  */
  class IQueries
  {
  public:
    virtual ~IQueries() = default;

    // -----------------------------------------------------------------------
    // Database file management
    // -----------------------------------------------------------------------

    /// Creates a new SQLite database at \a fname with the given \a collection.
    /** Fills in barcode_prefix and data_version on \a collection.
        @return true on success */
    virtual bool create_new_database(Collection& collection,
                                     QString const& fname) = 0;
    /// Creates a new encrypted database at \a fname; \a key must be non-empty.
    /** @return true on success */
    virtual bool create_new_database(Collection& collection,
                                     QString const& fname,
                                     QString const& key) = 0;
    /// creates a new SQLite database in memory with given \a collection.
    virtual bool create_new_database(Collection& collection) = 0;
    /// Inserts the Administrator (person id 0) and their default permissions.
    /** Must be called after create_tables() on a freshly created database.
        @return true on success */
    virtual bool add_administrator() = 0;
    /// Opens an existing .col database file (unencrypted).
    /** @return true on success */
    virtual bool load_database(QString const& fname) = 0;
    /// Opens an existing encrypted .col database file.
    /** @param key  decryption passphrase
        @return true on success; false also on wrong passphrase */
    virtual bool load_database(QString const& fname,
                               QString const& key) = 0;
    /// Saves a compact copy of the database to \a fname (always unencrypted).
    /** @return true on success */
    virtual bool save_database(QString const& fname) = 0;

    // -----------------------------------------------------------------------
    // Database encryption management (admin only)
    // -----------------------------------------------------------------------

    /// Returns true if this backend supports local database encryption.
    virtual bool supports_encryption() const = 0;
    /// Returns true if the database was opened with an encryption key.
    virtual bool is_encrypted() const = 0;
    /// Encrypts an unencrypted database with \a new_key.
    /** @return true on success */
    virtual bool encrypt_database(QString const& new_key) = 0;
    /// Changes the encryption key from \a old_key to \a new_key.
    /** @return true on success */
    virtual bool change_encryption_key(QString const& old_key,
                                       QString const& new_key) = 0;
    /// Removes encryption; \a current_key must match the current passphrase.
    /** @return true on success */
    virtual bool remove_encryption(QString const& current_key) = 0;
    /// Creates all required tables.
    /** @return true on success */
    virtual bool create_tables() = 0;

    // -----------------------------------------------------------------------
    // Collection metadata
    // -----------------------------------------------------------------------

    /// Inserts collection metadata into the database.
    /** @return true on success */
    virtual bool create_collection(Collection const& collection) = 0;
    /// Updates the stored collection metadata.
    /** @return true on success */
    virtual bool change_collection(Collection const& collection) = 0;
    /// Reads and returns the collection metadata from the database.
    virtual Collection read_collection() = 0;
    /// Reads the name of the last-used ID-label sheet layout.
    /** Returns an empty string if no value has been stored yet.  Card and
        label layouts are kept per client (locally); this read-only accessor
        exists only for the desktop's one-time migration into local storage. */
    virtual QString read_id_label_layout_name() = 0;
    /// Reads the name of the last-used item-card label sheet.
    /** Returns an empty string if no value has been stored yet.  Read-only,
        for the desktop's one-time migration into local storage. */
    virtual QString read_item_card_label_layout_name() = 0;

    /// database open?
    virtual bool is_open() const = 0;
    /// Returns true if the current system locale has translations in the data.
    virtual bool is_locale_supported() const = 0;

    // -----------------------------------------------------------------------
    // Signature categories
    // -----------------------------------------------------------------------

    /// Returns all signature-top category names, sorted alphabetically.
    virtual std::vector<QString> read_signature_top_categories() = 0;
    /// Inserts a new signature-top category.
    /** @return true on success */
    virtual bool add_signature_top_category(QString const& name) = 0;
    /// Removes a signature-top category by name.
    /** @return true on success */
    virtual bool remove_signature_top_category(QString const& name) = 0;
    /// Returns all bottom-category names for the given top category, sorted.
    virtual std::vector<QString>
    read_signature_bottom_categories(QString const& top) = 0;
    /// Inserts a new signature-bottom category for the given top category.
    /** @return true on success */
    virtual bool add_signature_bottom_category(QString const& top,
                                               QString const& name) = 0;
    /// Removes a signature-bottom category by top category and name.
    /** @return true on success */
    virtual bool remove_signature_bottom_category(QString const& top,
                                                  QString const& name) = 0;

    // -----------------------------------------------------------------------
    // Item types
    // -----------------------------------------------------------------------

    /// Inserts a new type and returns its database id, or -1 on failure.
    /** @param tablename SQLite table name for the attribute table; derived
        from the type name if empty. */
    virtual int add_new_type(Type const& type,
                             QString const& tablename = {}) = 0;
    /// Re-inserts a type with a specific \a id (for undo/redo only).
    /// Returns the database id of the type with the given \a type name.
    virtual int get_type_id(QString const& type) = 0;
    /// Returns the lowest type id carrying media \a kind, or -1 if none.
    virtual int type_id_for_media_kind(MediaKind kind) = 0;
    /// Returns the media kind of \a type_id, or MediaKind::Unknown.
    virtual MediaKind media_kind_for_type_id(int type_id) = 0;
    /// Returns the Type object for \a type_id.
    virtual Type get_type(int type_id) = 0;
    /// Returns the display name of the type with \a type_id.
    virtual QString get_type_name(int type_id) = 0;
    /// Returns all (id, localised_name) pairs for all types.
    virtual std::vector<std::pair<int, QString>>
    get_type_locale(
        QLocale::Language lang = QLocale::AnyLanguage) = 0;
    /// Returns the localised name of the type with \a type_id.
    virtual QString get_type_locale(
        int type_id,
        QLocale::Language lang = QLocale::AnyLanguage) = 0;
    /// Returns the localised name of the type with the given \a type name.
    virtual QString get_type_locale(
        QString const& type,
        QLocale::Language lang = QLocale::AnyLanguage) = 0;
    /// Updates an existing type definition.
    /**
     * @param display_names maps sanitized column name to display name for
     *        the current system language; used to update translations for
     *        both new and existing fields.
     * @return true on success
     */
    virtual bool change_type(int type_id, Type const& new_type,
                             QMap<QString, QString> const& display_names) = 0;
    /// Removes a type and its associated item table.
    /** @return true on success */
    virtual bool remove_type(int type_id) = 0;
    /// Returns the combined field display order for the given type.
    /**
     * Returns a list of field identifiers in display order.  Each entry is
     * either a common-field sentinel (e.g. \c "__name__") or a type-specific
     * column name (as stored in the \c data_types table).  Common fields that
     * have no stored position are placed before type-specific fields.
     */
    virtual QStringList get_combined_field_order(int type_id) = 0;
    /// Returns the default common-field order for new types.
    /**
     * Returns the common-field sentinels (e.g. \c "__name__") sorted by their
     * default positions.  Used to pre-populate the field table when creating a
     * new type before it has a database id.
     */
    virtual QStringList get_default_common_field_order() = 0;
    /// Persists the combined field display order for the given type.
    /**
     * @param type_id    database id of the type.
     * @param field_order ordered list of field identifiers (common sentinels
     *                    first character is \c '_', type-specific are column
     *                    names).
     * @return true on success.
     */
    virtual bool save_field_order(int type_id,
                                  QStringList const& field_order) = 0;
    /// Returns the display names of all types.
    virtual std::vector<QString> get_type_names() = 0;
    /// Returns the localised display name for \a field of type \a type_id.
    virtual QString get_type_field_label(int type_id, QString const& field,
                                         QLocale::Language lang) = 0;
    // -----------------------------------------------------------------------
    // Items
    // -----------------------------------------------------------------------

    /// Inserts a new item and returns its database id, or -1 on failure.
    virtual int add_new_item(Item const& item) = 0;
    /// Reads the item with \a id into \a item and fills \a type_name.
    /** @return true on success */
    virtual bool get_item(int id, Item& item, QString& type_name) = 0;
    /// Removes the item with \a id (fails if open lendings exist).
    /** @return true on success */
    virtual bool remove_item(int id) = 0;
    /// Removes the item with \a id regardless of existing lendings.
    /** @return true on success */
    virtual bool force_remove_item(int id) = 0;
    /// Updates the item with \a id.
    /** @return true on success */
    virtual bool change_item(int id, Item const& item) = 0;
    /// Removes all key-value pairs from \a item that do not belong to its type.
    virtual void restrict_item_to_type(Item& item) = 0;
    /// Returns the number of currently available (not lent) copies of \a item_id.
    virtual unsigned int available(int item_id) = 0;
    /// Returns the total stock count of \a item_id.
    virtual unsigned int stock_size(int item_id) = 0;
    /// Returns true if \a item_id has any (open or closed) lendings.
    virtual bool have_lendings(int item_id) = 0;
    /// Returns (lending_id, person_id) pairs for all open lendings of \a item_id.
    virtual std::vector<std::pair<int, int>>
    open_lendings_for_item(int item_id) = 0;
    /// Returns (lending_id, item_id) pairs for all open lendings of \a person_id.
    virtual std::vector<std::pair<int, int>>
    open_lendings_for_person(int person_id) = 0;
    /// Returns the item ids of all items whose ISBN10 or ISBN13 equals \a isbn.
    virtual std::vector<int> items_by_isbn(QString const& isbn) = 0;
    /// Returns the item ids of all items whose EAN matches \a ean.
    /** Searches all type-specific tables that have an "ean" column.
        Both the given form and, when \a ean is 13 digits starting with '0',
        the 12-digit form (without leading zero) are checked. */
    virtual std::vector<int> items_by_ean(QString const& ean) = 0;
    /// Computes the barcode string for an item identified by type and item id.
    virtual QString compute_item_barcode(int type_id, int item_id) = 0;
    /// Parses a barcode string and fills \a type_id and \a item_id.
    /** @return true if \a barcode is a valid item barcode */
    virtual bool parse_item_barcode(QString const& barcode,
                                    int& type_id, int& item_id) = 0;
    /// Returns true if \a barcode is a valid item barcode for this collection.
    virtual bool is_item_barcode(QString const& barcode) = 0;
    virtual std::vector<std::pair<int, QString>> get_items_ordered_by_name() = 0;
    /// Returns the ids of the \a count most recently acquired items.
    /** Items are ordered by their latest acquisition date/time, newest
        first. Items without any acquisition record are not included.
        @param count maximum number of item ids to return
        @return item ids, newest acquisition first (at most \a count) */
    virtual std::vector<int> get_recently_acquired_item_ids(int count) = 0;
    /// Returns the ids of items acquired within the time span [from, to].
    /** Items whose acquisition date/time falls in the inclusive range
        [\a from, \a to] are returned, each id at most once. Items without
        any matching acquisition record are not included.
        @param from start of the time span (inclusive)
        @param to   end of the time span (inclusive)
        @return item ids ordered by id */
    virtual std::vector<int>
    get_items_acquired_between(QDateTime const& from,
                               QDateTime const& to) = 0;
    struct ItemTypeThumbnail
    {
      int item_id_ = 0;
      QString item_name_;
      QString type_name_;
      int thumb_id_ = 0;
    };
    virtual std::vector<ItemTypeThumbnail> get_items_with_type_and_thumbnails() = 0;

    // -----------------------------------------------------------------------
    // Acquisitions
    // -----------------------------------------------------------------------

    /// Inserts a new acquisition and returns its database id, or -1 on failure.
    virtual int add_new_acquisition(Acquisition const& acquisition) = 0;
    /// Reads the acquisition with \a id into \a acquisition.
    /** @return true on success */
    virtual bool get_acquisition(int id, Acquisition& acquisition) = 0;
    /// Updates the acquisition with \a id.
    /** @return true on success */
    virtual bool change_acquisition(int id,
                                    Acquisition const& acquisition) = 0;
    /// Removes the acquisition with \a id.
    /** @return true on success */
    virtual bool remove_acquisition(int id) = 0;

    // -----------------------------------------------------------------------
    // Liquidations
    // -----------------------------------------------------------------------

    /// Inserts a new liquidation and returns its database id, or -1 on failure.
    virtual int add_new_liquidation(Liquidation const& liquidation) = 0;
    /// Reads the liquidation with \a id into \a liquidation.
    /** @return true on success */
    virtual bool get_liquidation(int id, Liquidation& liquidation) = 0;
    /// Updates the liquidation with \a id.
    /** @return true on success */
    virtual bool change_liquidation(int id,
                                    Liquidation const& liquidation) = 0;
    /// Removes the liquidation with \a id.
    /** @return true on success */
    virtual bool remove_liquidation(int id) = 0;

    // -----------------------------------------------------------------------
    // Returns
    // -----------------------------------------------------------------------

    /// Inserts a new return and returns its database id, or -1 on failure.
    virtual int add_new_return(Return const& ret) = 0;
    /// Reads the return with \a return_id into \a ret.
    /** @return true on success */
    virtual bool get_return(int return_id, Return& ret) = 0;
    /// Updates the return with \a return_id.
    /** @return true on success */
    virtual bool change_return(int return_id, Return const& ret) = 0;
    /// Removes the return with \a return_id.
    /** @return true on success */
    virtual bool remove_return(int return_id) = 0;

    /// Result of add_group_return().
    struct GroupReturnResult
    {
      /// Outcome of a group return.
      enum struct Status {
        Ok,          ///< all open copies returned
        NotLent,     ///< a person has no open lending of the item
        Unsupported, ///< server does not offer group returns
        Error        ///< database or communication error
      };
      Status status_ = Status::Error; ///< outcome
      std::vector<int> return_ids_;   ///< new return ids (Ok only)
      int returned_count_ = 0;        ///< copies returned (Ok only)
      int person_id_ = -1;            ///< offending person (NotLent)
    };

    /// Returns all open copies of an item held by several persons at once.
    /**
       All returns are inserted in a single transaction: either every open
       lending of the item held by one of the persons is closed or nothing
       is changed.  One return is recorded per open lending, covering its
       remaining count.
       @param item_id    the item to return
       @param person_ids the returning persons (no duplicates)
       @param tmpl       template carrying date/time, remarks and data
                         collector; lending and count are ignored
       @return outcome with the new return ids on success */
    virtual GroupReturnResult add_group_return(
        int item_id, std::vector<int> const& person_ids,
        Return const& tmpl) = 0;

    // -----------------------------------------------------------------------
    // Lendings
    // -----------------------------------------------------------------------

    /// Inserts a new lending and returns its database id, or -1 on failure.
    virtual int add_new_lending(Lending const& lending) = 0;
    /// Reads the lending with \a id into \a lending.
    /** @return true on success */
    virtual bool get_lending(int id, Lending& lending) = 0;
    /// Updates the lending with \a id.
    /** @return true on success */
    virtual bool change_lending(int id, Lending const& lending) = 0;
    /// Removes the lending with \a id.
    /** @return true on success */
    virtual bool remove_lending(int id) = 0;
    /// Fills \a lent and \a returned with the counts for \a lending_id.
    /** @return true on success */
    virtual bool lending_balance(int lending_id, std::size_t& lent,
                                 std::size_t& returned) = 0;

    /// Result of add_group_lending().
    struct GroupLendingResult
    {
      /// Outcome of a group lending.
      enum struct Status {
        Ok,                 ///< all lendings inserted
        NotEnoughAvailable, ///< more persons than unreserved copies
        AlreadyLent,        ///< a person already holds the item
        Unsupported,        ///< server does not offer group lendings
        Error               ///< database or communication error
      };
      Status status_ = Status::Error; ///< outcome
      std::vector<int> lending_ids_;  ///< new lending ids (Ok only)
      int lendable_ = 0;   ///< copies that can be lent (NotEnoughAvailable)
      int person_id_ = -1; ///< offending person (AlreadyLent)
    };

    /// Lends one copy of an item to each of several persons at once.
    /**
       All lendings are inserted in a single transaction: either every
       person gets a lending or nothing is changed.  Each person's own
       reservation of the item is consumed like in add_new_lending().
       Persons that already hold an open lending of the item are rejected.
       @param lending    template carrying item, date/time, due date,
                         remarks and data collector; person and count are
                         ignored (count is always 1)
       @param person_ids the borrowers (no duplicates)
       @return outcome with the new lending ids on success */
    virtual GroupLendingResult add_group_lending(
        Lending const& lending, std::vector<int> const& person_ids) = 0;
    struct LendingInfo
    {
      int lending_id_ = 0;
      int lending_count_ = 0;
      QString item_name_;
    };
    virtual std::vector<LendingInfo> query_lendings(int person_id) = 0;

    // -----------------------------------------------------------------------
    // Persons
    // -----------------------------------------------------------------------

    /// Inserts a new person and returns the new database id, or -1 on failure.
    virtual int add_new_person(Person const& person) = 0;
    /// Reads the person with \a id into \a person.
    /** @return true on success */
    virtual bool get_person(int id, Person& person) = 0;
    /// Returns the database id of the person with the given \a barcode.
    virtual int get_person(QString const& barcode) = 0;
    /// Updates the person with \a id.
    /** @return true on success */
    virtual bool change_person(int id, Person const& person) = 0;
    /// Returns true if a person with the given name already exists.
    virtual bool person_known(QString const& lastname,
                              QString const& firstname) = 0;
    /// Returns true if any person already has \a emails as their email field.
    virtual bool email_known(QString const& emails) = 0;
    /// Returns true if \a username is already taken by any person.
    virtual bool username_known(QString const& username) = 0;
    /// Removes the person with \a id.
    /** @return true on success */
    virtual bool remove_person(int id) = 0;
    /// Returns true if any person other than the Administrator (id 0) exists.
    virtual bool has_non_admin_persons() = 0;
    /// Computes the barcode string for \a person_id.
    virtual QString compute_person_barcode(int person_id) = 0;
    /// Returns true if \a str is a valid person barcode for this collection.
    virtual bool is_person_barcode(QString const& str) = 0;
    /// Parses a person barcode string and fills \a person_id.
    /** @return true if \a barcode is a valid person barcode */
    virtual bool parse_person_barcode(QString const& barcode,
                                      int& person_id) = 0;
    /// Returns the person_id whose external_id equals \a sub, or -1.
    virtual int get_person_by_external_id(QString const& sub) = 0;
    /// Stores \a sub as the external_id for an existing person.
    /** @return true on success */
    virtual bool set_person_external_id(int person_id,
                                        QString const& sub) = 0;

    // -----------------------------------------------------------------------
    // Local REST credentials
    // -----------------------------------------------------------------------

    /// Checks local REST credentials; returns person_id on success, -1 on failure.
    virtual int check_local_credentials(QString const& username,
                                        QString const& password) = 0;
    /// Creates or replaces the local REST account for \a person_id.
    /** @return true on success */
    virtual bool set_local_credentials(int person_id,
                                       QString const& username,
                                       QString const& password_hash) = 0;
    /// Removes the local REST account for \a person_id.
    /** @return true on success */
    virtual bool delete_local_credentials(int person_id) = 0;
    /// Returns the stored username for \a person_id, or an empty string.
    virtual QString get_username(int person_id) = 0;
    /// Stores \a username for \a person_id (upsert).
    /** @return false if the username is already taken by another person */
    virtual bool set_username(int person_id, QString const& username) = 0;
    /// Clears the username for \a person_id.
    /** @return true on success */
    virtual bool remove_username(int person_id) = 0;

    // -----------------------------------------------------------------------
    // Permissions
    // -----------------------------------------------------------------------

    virtual std::vector<QString> get_permissions() const = 0;
    virtual std::vector<bool> get_user_permissions(int person_id) = 0;
    /// Creates default permissions for \a person_id.
    /** @return true on success */
    virtual bool create_default_permissions(int person_id) = 0;
    /// Returns true if \a person_id has the permission named \a column.
    virtual bool has_permission(int person_id, QString const& column) = 0;
    /// Sets a single permission flag for \a person_id.
    /** @return true on success */
    virtual bool set_permission(int person_id, QString const& column,
                                bool value) = 0;
    /// Removes all permissions for \a person_id.
    /** @return true on success */
    virtual bool remove_permissions(int person_id) = 0;
    /// Returns true if \a person_id has at most reserve-only permissions.
    /**
     * A person is reserve-only when every permission column other than
     * view_personal_data, reserve, and view_items is false.
     * Used to gate barcode-only (password-less) login.
     */
    virtual bool is_reserve_only(int person_id) = 0;

    // -----------------------------------------------------------------------
    // Passwords
    // -----------------------------------------------------------------------

    /// Inserts or replaces the password hash for \a person_id.
    /** @return true on success */
    virtual bool set_password(int person_id, QString const& hash) = 0;
    /// Reads the stored password hash for \a person_id into \a hash.
    /** @return true if an entry was found */
    virtual bool get_password(int person_id, QString& hash) = 0;
    /// Returns true if a password entry exists for \a person_id.
    virtual bool has_password(int person_id) = 0;
    /// Removes the password entry for \a person_id.
    /** @return true on success */
    virtual bool remove_password(int person_id) = 0;
    /// Changes \a person_id's own password after verifying \a old_password.
    /**
       Verifies \a old_password against the stored hash and, only on success,
       replaces it with the already-hashed \a new_hash.  Used for self-service
       password changes where the caller proves knowledge of the current
       password.
       @param person_id    the person changing their own password
       @param old_password the current password in clear text
       @param new_hash     the new password, already Argon2id-hashed
       @return true if the old password matched and the new one was stored
    */
    virtual bool change_own_password(int person_id,
                                     QString const& old_password,
                                     QString const& new_hash) = 0;

    // -----------------------------------------------------------------------
    // Thumbnails
    // -----------------------------------------------------------------------

    /// Stores a thumbnail pixmap and returns its database id, or -1 on failure.
    /**
       @param thumbnail the image to store
       @param key       logical key used to identify the thumbnail (e.g. ISBN)
       @param url       source URL of the image (informational only)
       @param overwrite if true, replaces an existing thumbnail for \a key
    */
    virtual int store_thumbnail(QByteArray const& png, QString const& key,
                                QString const& url,
                                bool overwrite = false) = 0;
    /// Reads the PNG bytes of thumbnail \a thumbnail_id into \a png.
    /** @return true on success */
    virtual bool get_thumbnail(int thumbnail_id, QByteArray& png) = 0;
    /// Returns the id of a locally cached thumbnail for \a key, or -1 if none.
    virtual int use_local_thumbnail(QString const& key) = 0;
    /// Returns true if thumbnail \a thumbnail_id is stored locally on disk.
    virtual bool have_local_thumbnail(int thumbnail_id) = 0;

    // -----------------------------------------------------------------------
    // View-model queries (existing)
    // -----------------------------------------------------------------------

    /// Queries the types table and returns the result as a list of Type objects.
    /** Pass type_id=-1 to retrieve all types. */
    virtual std::vector<Type> query_types(
        int type_id,
        QLocale::Language lang = QLocale::AnyLanguage) = 0;

    /// Queries the persons table and returns a list of Person objects.
    /** Pass person_id=-1 and an empty \a search to retrieve all persons. */
    virtual std::vector<Person> query_persons(int person_id,
                                              QString const& search) = 0;
    /// Returns the ids of the \a count most recently created persons.
    /** Persons are ordered by creation, newest first.  As the persons table
        carries no creation timestamp, the (monotonically increasing) person id
        is used as the creation-order proxy.
        @param count maximum number of person ids to return
        @return person ids, newest first (at most \a count) */
    virtual std::vector<int> get_recently_created_person_ids(int count) = 0;
    /// Queries persons eligible to receive a new lending matching the filters.
    virtual std::vector<Person> query_lending_persons(
        int item_id, int lending_id,
        QString const& person_search,
        QString const& item_search) = 0;
    /// Queries persons with password flag for the login page.
    /** Pass an empty \a search to retrieve all persons. */
    virtual std::vector<StartPagePersonListModel::Row>
    query_start_page_persons(QString const& search) = 0;

    /// Returns the total number of items in the database.
    virtual int count_items() = 0;
    /// Queries items filtered by id and text search.
    virtual std::vector<ItemListModel::Row> query_items(
        int item_id, QString const& search,
        QLocale::Language lang = QLocale::AnyLanguage) = 0;
    /// Queries items that have at least one acquisition record.
    virtual std::vector<ItemListModel::Row> query_acquired_items(
        int item_id, QString const& search,
        int limit = 0, int offset = 0,
        QLocale::Language lang = QLocale::AnyLanguage) = 0;
    /// Queries items that have available copies.
    virtual std::vector<ItemListModel::Row> query_available_items(
        int item_id, QString const& search,
        int limit = 0, int offset = 0,
        QLocale::Language lang = QLocale::AnyLanguage) = 0;
    /// Queries items currently lent, filtered by person, lending and text search.
    virtual std::vector<ItemListModel::Row> query_lent_items(
        int person_id, int lending_id,
        QString const& person_search,
        QString const& item_search,
        QLocale::Language lang = QLocale::AnyLanguage) = 0;
    /// Queries items filtered by type and text search.
    /**
     * @param lang  language for type-name translation; AnyLanguage means
     *              use the server/process locale (system_language())
     */
    virtual std::vector<ItemListModel::Row> query_items_with_type(
        int type_id, QString const& search,
        int limit = 0, int offset = 0,
        QLocale::Language lang = QLocale::AnyLanguage) = 0;
    /// One row returned by query_items_for_search().
    struct SearchItemRow
    {
      int id = 0;
      QString name;        ///< items.name
      QString type_locale; ///< localised type name
      int count = 0;         ///< total stock count
      int available = 0;     ///< available (not lent) copies
      int type_id = 0;       ///< items.type (foreign key into types table)
      int total_reserved = 0; ///< sum of all active reservation counts for this item
    };
    /// Metadata for one type-specific column (for print/export field list).
    struct TypeFieldMeta
    {
      QString column;     ///< DB column name (SQL-normalised user-entered name)
      QString marc21_tag; ///< MARC21 identifier, e.g. "100$a", or empty
    };
    /// Returns field metadata for all columns of \a type_id.
    /** Columns are ordered by their stored position.
        @return vector of TypeFieldMeta ordered by position. */
    virtual std::vector<TypeFieldMeta> get_type_field_metas(
        int type_id) = 0;
    /// One type-specific field filter passed to query_items_for_search().
    struct TypeFieldFilter
    {
      QString column_; ///< column name in item_list_values or type-specific table
      QString value_;  ///< search text (empty filters are ignored)
    };
    /// Searches items by multiple optional fields (all AND-combined).
    /**
       For \a general, each space-separated word must appear in at least
       one indexed field.  Empty strings are ignored.
       @param type_id     restrict to this type id, or -1 for all types
       @param tablename   SQLite table for \a type_id (empty when type_id==-1)
       @param type_fields per-column filters; when type_id>0, each filter
                          checks both item_list_values and \a tablename
       @param grade_min   lowest wanted grade level (school items), 0 = no
                          lower bound
       @param grade_max   highest wanted grade level (school items), 0 = no
                          upper bound; when any bound is set, items without a
                          grade level are excluded
    */
    virtual std::vector<SearchItemRow> query_items_for_search(
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
        QLocale::Language lang = QLocale::AnyLanguage) = 0;

    /// Queries open (unreturned) lendings filtered by person and item.
    virtual std::vector<OpenLendingListModel::Row> query_open_lendings(
        int person_id, int item_id) = 0;
    struct LendingPersonInfo
    {
      int lending_id_ = 0;
      QString person_last_name_;
      QString person_first_name_;
      QString person_title_;
    };
    /// queries lendings for a given item with focus on lender.
    virtual std::vector<LendingPersonInfo> query_item_lendings(int item_id) = 0;

    /// Queries distinct values of \a field in the type-specific table for
    /// \a type_id (for auto-completers on dynamic item attributes).
    virtual QStringList query_type_field_completions(
        int type_id, QString const& field) = 0;
    /// Queries distinct values of \a field in the persons table.
    virtual QStringList query_person_field_completions(
        QString const& field) = 0;
    /// Queries distinct values of \a field in the common items table.
    virtual QStringList query_item_field_completions(
        QString const& field) = 0;
    /// Queries distinct values of \a field in the schoolitems table.
    virtual QStringList query_school_item_field_completions(
        QString const& field) = 0;

    /// Queries combined history across all four action types.
    /**
       Filters by optional date range \a from_date and \a to_date
       (inclusive when non-null).  Includes collector name when \a is_admin.
       Rows are ordered by datetime descending.
       @param action_filter restrict to one action type ("acquisition",
                            "liquidation", "lending", "return",
                            "reservation"); empty = all types
    */
    virtual std::vector<HistoryCombinedModel::Row>
    query_combined_history(bool is_admin = false,
                           QDate const& from_date = QDate(),
                           QDate const& to_date = QDate(),
                           bool show_all = false,
                           int limit = 0, int offset = 0,
                           QString const& action_filter = {}) = 0;

    /// Queries persons for the change-passwords admin page.
    /**
       Returns the Administrator (id 0) as the first row, followed by all
       regular persons (id > 0) ordered by lastname, firstname.
    */
    virtual std::vector<ChangePasswordsPersonListModel::Row>
    query_change_passwords_persons() = 0;

    /// Returns all history rows that directly conflict with deleting \a action.
    /**
       Conflict rules applied:
       - acquisition → lendings/liquidations that needed the added copies
       - lending     → returns that reference this lending
       - return      → lendings/liquidations that needed the freed copies
       - liquidation → no conflicts (deleting a liquidation only increases
                        available stock, which never invalidates later actions)
       @param action the history row to be deleted
       @return vector of conflicting rows (may be empty)
    */
    virtual std::vector<HistoryCombinedModel::Row>
    find_conflicting_actions(HistoryCombinedModel::Row const& action) = 0;

    /// Deletes all history actions in \a actions and recomputes item counters.
    /**
       Executes all deletions in a single transaction.  For each affected item,
       the stock count and available count in the items table are recomputed
       from the remaining acquisition, liquidation, lending, and return rows.
       @param actions rows to delete (order does not matter)
       @return true on success, false if any database error occurred
    */
    virtual bool batch_delete_history_actions(
        std::vector<HistoryCombinedModel::Row> const& actions) = 0;

    /// Returns all history rows that conflict with changing \a old_action.
    /**
       Checks whether replacing \a old_action with \a new_action (updated
       datetime, count, remarks, and collector_id) would violate any
       consistency constraint:
       - Availability: no later lending or liquidation becomes impossible.
       - Self-validity: the action itself cannot execute at the new datetime
         with the new count (insufficient stock for lending/liquidation;
         insufficient outstanding copies for a return).
       - Ordering: returns cannot precede their parent lending; a parent
         lending cannot follow its return.
       - Count: total returns of a lending cannot exceed the new count.
       @param old_action current stored row
       @param new_action proposed replacement (same id and action type)
       @return conflicting rows (empty means the change is safe to apply)
    */
    virtual std::vector<HistoryCombinedModel::Row>
    find_change_conflicts(
        HistoryCombinedModel::Row const& old_action,
        HistoryCombinedModel::Row const& new_action) = 0;

    /// Applies a change to a single history row and recomputes item counters.
    /**
       Updates count, datetime, remarks (for lending/return), and
       data_collector for the row identified by \a old_action.id and
       \a old_action.action.  Runs inside a single transaction; recomputes
       items.count and items.available from the history tables on success.
       @param old_action identifies the row to update (action type and id)
       @param new_action new values (count, datetime, remarks, collector_id)
       @return true on success, false if any database error occurred
    */
    virtual bool apply_action_change(
        HistoryCombinedModel::Row const& old_action,
        HistoryCombinedModel::Row const& new_action) = 0;

    /// Returns open lendings due in [from_date, to_date] (inclusive).
    /**
       When \a from_date is invalid, all open lendings are returned (no date
       filter).  Rows are ordered by duedate ascending; lendings without a
       due date appear last.
       @param from_date          start of the date range (invalid = no filter)
       @param to_date            end of the date range (ignored when from_date
                                 is invalid)
       @param person_id          restrict to this person id when >= 0
       @param school_class_filter restrict to exact school class when non-empty
       @param person_name_filter  restrict by lastname prefix when non-empty
    */
    virtual std::vector<DueLendingListModel::Row> query_due_lendings(
        QDate const& from_date = QDate(),
        QDate const& to_date = QDate(),
        int person_id = -1,
        QString const& school_class_filter = {},
        QString const& person_name_filter = {}) = 0;

    /// Sets a new due date for the given lendings ("extend lending period").
    /**
       All lendings are updated in a single transaction; the version of
       each changed lending is incremented.
       @param lending_ids ids of the lendings to change
       @param duedate     new due date (invalid = no due date)
       @return true on success, false if any lending could not be changed
    */
    virtual bool extend_lendings(std::vector<int> const& lending_ids,
                                 QDate const& duedate) = 0;

    // ---------------------------------------------------------------
    // Class management (Klassenverwaltung)
    // ---------------------------------------------------------------

    /// Loads sorted list of current class names (academic_year='').
    virtual QStringList load_class_names() = 0;

    /// Adds a class name; no-op if already present.
    /** @return true on success */
    virtual bool add_class_name(QString const& name) = 0;

    /// Removes a class name entry (academic_year='' only).
    /** @return true on success */
    virtual bool remove_class_name(QString const& name) = 0;

    /// Renames a class and updates all persons with that class.
    /** Runs in a single transaction.
        @return true on success */
    virtual bool rename_class_name(QString const& old_name,
                                   QString const& new_name) = 0;

    /// One row returned by get_affected_persons().
    struct AffectedPerson
    {
      int person_id_ = 0;
      QString display_name_; ///< "Lastname, Firstname"
      QString old_class_;
      QString new_class_;    ///< empty = no new class
    };

    /// Returns persons whose school_class matches a key in \a old_to_new.
    virtual std::vector<AffectedPerson>
    get_affected_persons(QMap<QString, QString> const& old_to_new) = 0;

    /// Applies per-person class changes and replaces the classes list.
    /**
       Runs in a single transaction.
       @param person_id_to_new_class  person_id → new school_class (empty = clear)
       @param persons_to_delete       person ids to remove (must have no open lendings)
       @param new_class_list          replaces all academic_year='' rows in classes
       @return true on success
    */
    virtual bool apply_school_year_changes(
        QMap<int, QString> const& person_id_to_new_class,
        QList<int> const& persons_to_delete,
        QStringList const& new_class_list) = 0;

    // -----------------------------------------------------------------------
    // Concurrency / ETag support
    // -----------------------------------------------------------------------

    /// Identifies a mutable entity table that carries a \c version column.
    enum struct VersionedEntity {
      Item,
      Person,
      Lending,
      Return,
      Acquisition,
      Liquidation
    };

    /// Result code returned by versioned update operations.
    /** Maps directly to HTTP status codes in the REST layer:
        Ok → 200, VersionMismatch → 412, NotFound → 404, Error → 500. */
    enum struct UpdateResult {
      Ok,              ///< update applied; version was incremented
      VersionMismatch, ///< expected_version did not match stored version
      NotFound,        ///< no row with the given id exists
      Error            ///< database error
    };

    /// Returns the current \c version value for the row with \a id in the
    /// table that corresponds to \a entity.
    /** Used by the REST layer to build \c ETag response headers.
        @return the version (>= 1) on success, or -1 if the row does not
                exist or an error occurs */
    virtual int get_version(VersionedEntity entity, int id) = 0;

    /// Updates an item only if its stored version equals \a expected_version.
    /** On success the version is incremented and UpdateResult::Ok is returned.
        @param id               primary key of the item to update
        @param item             new item data
        @param expected_version version from the client's If-Match header
        @return Ok, VersionMismatch, NotFound, or Error */
    virtual UpdateResult change_item_versioned(
        int id, Item const& item, int expected_version) = 0;

    /// Updates a person only if its stored version equals \a expected_version.
    /** @param id               primary key of the person to update
        @param person           new person data
        @param expected_version version from the client's If-Match header
        @return Ok, VersionMismatch, NotFound, or Error */
    virtual UpdateResult change_person_versioned(
        int id, Person const& person, int expected_version) = 0;

    /// Updates a lending only if its stored version equals \a expected_version.
    /** @param id               primary key of the lending to update
        @param lending          new lending data
        @param expected_version version from the client's If-Match header
        @return Ok, VersionMismatch, NotFound, or Error */
    virtual UpdateResult change_lending_versioned(
        int id, Lending const& lending, int expected_version) = 0;

    /// Updates a return only if its stored version equals \a expected_version.
    /** @param return_id        primary key of the return to update
        @param ret              new return data
        @param expected_version version from the client's If-Match header
        @return Ok, VersionMismatch, NotFound, or Error */
    virtual UpdateResult change_return_versioned(
        int return_id, Return const& ret, int expected_version) = 0;

    /// Updates an acquisition only if its stored version equals
    /// \a expected_version.
    /** @param id               primary key of the acquisition to update
        @param acquisition      new acquisition data
        @param expected_version version from the client's If-Match header
        @return Ok, VersionMismatch, NotFound, or Error */
    virtual UpdateResult change_acquisition_versioned(
        int id, Acquisition const& acquisition,
        int expected_version) = 0;

    /// Updates a liquidation only if its stored version equals
    /// \a expected_version.
    /** @param id               primary key of the liquidation to update
        @param liquidation      new liquidation data
        @param expected_version version from the client's If-Match header
        @return Ok, VersionMismatch, NotFound, or Error */
    virtual UpdateResult change_liquidation_versioned(
        int id, Liquidation const& liquidation,
        int expected_version) = 0;

    /// Per-table change counters used by the REST client for polling.
    /** Maps tracked table names to their current change-counter value. */
    using TableVersions = QMap<QString, int>;

    /// Returns the per-table change counters from \c colman_table_versions.
    /** Used by the REST client to detect server-side data changes without
        re-fetching full result sets.
        @return map of table name → current change counter (0 on error) */
    virtual TableVersions query_table_versions() = 0;

    // -----------------------------------------------------------------------
    // Reservations
    // -----------------------------------------------------------------------

    /// Result of add_reservation().
    enum struct ReservationResult {
      Ok,                ///< reservation inserted or topped up successfully
      AlreadyExists,     ///< legacy duplicate result (kept for REST compat)
      NotEnoughAvailable, ///< count exceeds unreserved copies
      Error              ///< database error
    };

    /// One reservation row.
    struct Reservation
    {
      int id = 0;
      int item_id = 0;
      int person_id = 0;
      int count = 1;          ///< number of copies reserved
      QDateTime reserved_at;
      QDate expires_at;
      int version = 1;
      int data_collector_id = 0; ///< person id of the data collector
    };

    /// Inserts a reservation; checks that count <= unreserved available copies.
    /** Uses BEGIN IMMEDIATE.  Purges expired reservations for the item first.
        If the person already has an active reservation for the item, its
        count is increased instead and the existing expiry date is kept.
        @param item_id    item to reserve
        @param person_id  person placing the reservation
        @param count      number of copies to reserve (additionally)
        @param expires_at expiry date for a new reservation
        @param new_id     output: id of the new or topped-up row on success
        @return Ok, NotEnoughAvailable, or Error */
    virtual ReservationResult add_reservation(
        int item_id, int person_id, int count,
        QDate const& expires_at, int data_collector_id,
        int& new_id) = 0;

    /// Deletes the reservation with \a reservation_id.
    /** @return true on success */
    virtual bool remove_reservation(int reservation_id) = 0;

    /// Returns all active (non-expired) reservations for \a item_id.
    /** Lazily deletes expired rows during this call. */
    virtual std::vector<Reservation> get_reservations_for_item(
        int item_id) = 0;

    /// Fills \a r with the active reservation of \a person_id for \a item_id.
    /** Lazily deletes expired reservations for the item.
        @return false if no active reservation exists */
    virtual bool get_reservation(int item_id, int person_id,
                                 Reservation& r) = 0;

    /// Returns all active reservations for \a person_id.
    /** Pass \a person_id == -1 to return all active reservations. */
    virtual std::vector<Reservation> get_reservations(int person_id) = 0;
  };

} // namespace colman

#endif /* #ifndef COLMAN_IQUERIES_HPP */

/* EOF */
