// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// colman_gen_testdb: generates a colman database with random items for testing.
/**
   @file
   @author Christoph Pesch
*/

#ifndef COLMAN_ACQUISITION_HPP
#  include "colman_acquisition.hpp"
#  define COLMAN_ACQUISITION_HPP
#endif
#ifndef COLMAN_COLLECTION_HPP
#  include "colman_collection.hpp"
#  define COLMAN_COLLECTION_HPP
#endif
#ifndef COLMAN_ITEM_HPP
#  include "colman_item.hpp"
#  define COLMAN_ITEM_HPP
#endif
#ifndef COLMAN_PERSON_HPP
#  include "colman_person.hpp"
#  define COLMAN_PERSON_HPP
#endif
#ifndef COLMAN_PASSWORDS_HPP
#  include "colman_passwords.hpp"
#  define COLMAN_PASSWORDS_HPP
#endif
#ifndef COLMAN_SQL_QUERIES_HPP
#  include "colman_sql_queries.hpp"
#  define COLMAN_SQL_QUERIES_HPP
#endif
#ifndef SYSTEM_QCOREAPPLICATION
#  include <QCoreApplication>
#  define SYSTEM_QCOREAPPLICATION
#endif
#ifndef SYSTEM_QCOMMANDLINEPARSER
#  include <QCommandLineParser>
#  define SYSTEM_QCOMMANDLINEPARSER
#endif
#ifndef SYSTEM_QDATETIME
#  include <QDateTime>
#  define SYSTEM_QDATETIME
#endif
#ifndef SYSTEM_QFILE
#  include <QFile>
#  define SYSTEM_QFILE
#endif
#ifndef SYSTEM_QRANDOMGENERATOR
#  include <QRandomGenerator>
#  define SYSTEM_QRANDOMGENERATOR
#endif
#ifndef SYSTEM_QTEXTSTREAM
#  include <QTextStream>
#  define SYSTEM_QTEXTSTREAM
#endif

namespace
{

  QString const Authors[] = {
    "Smith, J.", "Müller, K.", "Johnson, A.", "Schmidt, H.",
    "Williams, R.", "Fischer, M.", "Brown, T.", "Weber, P.",
    "Jones, D.", "Wagner, S.", "Garcia, L.", "Becker, C.",
  };
  int constexpr AuthorsN = 12;

  QString const Publishers[] = {
    "Klett",     "Cornelsen", "Westermann", "Duden",
    "Springer",  "Heyne",     "dtv",        "Rowohlt",
    "S. Fischer","Hanser",    "Beck",       "Reclam",
  };
  int constexpr PublishersN = 12;

  QString const Subjects[] = {
    "Mathematics", "History",    "Biology",   "Physics",
    "Chemistry",   "Geography",  "German",    "English",
    "Art",         "Music",      "Sports",    "Computer Science",
  };
  int constexpr SubjectsN = 12;

  QString const Adjectives[] = {
    "Great", "Essential", "Complete", "Advanced",
    "Modern", "Practical", "New", "Classic",
  };
  int constexpr AdjectivesN = 8;

  QString const Nouns[] = {
    "Guide", "Handbook", "Introduction", "Companion",
    "Atlas", "Reader", "Compendium", "Overview",
  };
  int constexpr NounsN = 8;

  /// Returns a random element from an array of size \a n.
  template <typename T>
  T const& pick(T const* arr, int n)
  {
    return arr[QRandomGenerator::global()->bounded(n)];
  }

  /// Generates a random item name (always contains "1" for easy bulk search).
  QString random_name(int index)
  {
    return pick(Adjectives, AdjectivesN) + " " +
           pick(Subjects, SubjectsN) + " " +
           pick(Nouns, NounsN) + " 1-" +
           QString::number(index);
  }

  /// Generates a random year between 1980 and 2025 as a string.
  QString random_year()
  {
    return QString::number(1980 + QRandomGenerator::global()->bounded(46));
  }

} // namespace

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("colman_gen_testdb");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "Generate a colman database populated with random items.");
  parser.addHelpOption();
  parser.addVersionOption();
  QCommandLineOption const password_opt(
      QStringList() << "p" << "password",
      "Admin password for new databases (default: \"admin\").",
      "password",
      "admin");
  parser.addOption(password_opt);
  parser.addPositionalArgument("filename", "Path to the .col database file.");
  parser.addPositionalArgument("count",    "Number of items to insert.");
  parser.process(app);

  auto const args = parser.positionalArguments();
  if(args.size() != 2) {
    parser.showHelp(1);
  }

  auto const filename = args[0];
  bool ok = false;
  auto const count = args[1].toInt(&ok);
  if(!ok || count < 0) {
    QTextStream(stderr) << "Error: count must be a non-negative integer.\n";
    return 1;
  }

  colman::SQLQueries sq(colman::SQLQueries::FileDb{});

  if(!QFile::exists(filename)) {
    colman::Collection coll;
    coll.set_name("Test");
    coll.set_organization("Test");
    if(!sq.create_new_database(coll, filename)) {
      QTextStream(stderr) << "Error: could not create database " << filename
                          << "\n";
      return 1;
    }
    auto const pw = parser.value(password_opt);
    if(!sq.set_local_credentials(0, "admin", colman::hash_password(pw))) {
      QTextStream(stderr) << "Warning: could not set admin credentials.\n";
    }
    colman::Person tester;
    tester.set_lastname("Tester");
    tester.set_firstname("Theo");
    auto const tester_id = sq.add_new_person(tester);
    if(tester_id < 0) {
      QTextStream(stderr) << "Warning: could not add test user.\n";
    }
    else {
      sq.set_local_credentials(
          tester_id, "tester", colman::hash_password("tester"));
    }
    QTextStream(stdout) << "Created " << filename
                        << " (admin password: \"" << pw << "\")\n";
  }
  else {
    if(!sq.load_database(filename)) {
      QTextStream(stderr) << "Error: could not open database " << filename
                          << "\n";
      return 1;
    }
    QTextStream(stdout) << "Opened " << filename << "\n";
  }

  auto const types = sq.query_types(-1);
  if(types.empty()) {
    QTextStream(stderr) << "Error: no item types found in database.\n";
    return 1;
  }

  QTextStream out(stdout);
  for(int i = 0; i < count; ++i) {
    auto const& type = types[static_cast<std::size_t>(i) % types.size()];
    colman::Item item;
    item.set_type_id(type.get_id());
    item.set_name(random_name(i + 1));
    item.set_value("authors",          pick(Authors, AuthorsN));
    item.set_value("publisher",        pick(Publishers, PublishersN));
    item.set_value("publication_date", random_year());
    item.set_value("subject",          pick(Subjects, SubjectsN));
    auto const item_id = sq.add_new_item(item);
    if(item_id < 0) {
      out << "Warning: failed to insert item " << (i + 1) << "\n";
    }
    else {
      colman::Acquisition acq;
      acq.set_item_id(item_id);
      acq.set_count(1);
      acq.set_date_time(QDateTime::currentDateTime());
      if(sq.add_new_acquisition(acq) < 0) {
        out << "Warning: failed to insert acquisition for item "
            << (i + 1) << "\n";
      }
    }
    if((i + 1) % 100 == 0 || i + 1 == count) {
      out << "Inserted " << (i + 1) << " / " << count << " items\r";
      out.flush();
    }
  }
  out << "\nDone.\n";
  return 0;
}

/* EOF */
