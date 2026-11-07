// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
/// Command-line ISBN query tool.
/**
   @file
   @author Christoph Pesch
*/

#include "colman_isbn_queries.hpp"
#ifndef COLMAN_SRU_SERVER_HPP
#  include "colman_sru_server.hpp"
#endif
#ifndef COLMAN_ITEM_HPP
#  include "colman_item.hpp"
#endif
#ifndef SYSTEM_QCOREAPPLICATION
#  include <QCoreApplication>
#  define SYSTEM_QCOREAPPLICATION
#endif
#ifndef SYSTEM_QCOMMANDLINEPARSER
#  include <QCommandLineParser>
#  define SYSTEM_QCOMMANDLINEPARSER
#endif
#ifndef SYSTEM_QTEXTSTREAM
#  include <QTextStream>
#  define SYSTEM_QTEXTSTREAM
#endif

namespace
{
} // namespace


/**
  Entry point for the colman_query_isbn command-line tool.

  Parses command-line arguments, configures an IsbnQueries object, runs
  a blocking ISBN lookup, and prints the merged item fields and cover URL
  to stdout.

  @param argc  Argument count.
  @param argv  Argument values.
  @return 0 on success, 1 on usage error or lookup failure.
*/
int main(int argc, char* argv[])
{
  try {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("colman_query_isbn");

    QCommandLineParser parser;
    parser.setApplicationDescription(
        "Query ISBN data from online sources and print the result.");
    parser.addHelpOption();
    parser.addPositionalArgument("isbn", "ISBN-10 or ISBN-13 to look up.");

    QCommandLineOption opt_ol("with-openlibrary", "Query Open Library.");
    QCommandLineOption opt_ga("with-googleapis", "Query Google Books.");
    QCommandLineOption opt_ga_key(
        "googleapis-key", "Google Books API key (optional).", "key");

    parser.addOption(opt_ol);
    parser.addOption(opt_ga);
    parser.addOption(opt_ga_key);

    parser.process(app);

    QStringList const args = parser.positionalArguments();
    if(args.isEmpty()) {
      parser.showHelp(1);
    }
    QString const isbn = args.first();

    colman::IsbnQueries q;
    if(parser.isSet(opt_ga_key)) {
      q.set_googleapis_key(parser.value(opt_ga_key));
    }
    q.use_openlibrary(parser.isSet(opt_ol));
    q.use_googleapis(parser.isSet(opt_ga));
    q.set_sru_servers(colman::load_sru_servers());

    QTextStream out(stdout);
    if(!q.query_isbn(isbn)) {
      out << "No services enabled — use --with-openlibrary, --with-googleapis,"
             " or enable SRU servers in settings.\n";
      return 1;
    }

    colman::Item const item = q.get_book_data();
    colman::print_item(item);

    QString const cover = q.get_thumbnail_url();
    if(!cover.isEmpty()) {
      out << "cover-url:   " << cover << "\n";
    }
  } catch(std::exception const& e) {
    qCritical() << e.what();
    return 1;
  }
  return 0;
}

/* EOF */
