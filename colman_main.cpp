// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) Christoph Pesch 2026
#ifndef COLMAN_APP_DATA_HPP
#  include "colman_app_data.hpp"
#endif
#ifndef COLMAN_UI_SNAPSHOT_HPP
#  include "colman_ui_snapshot.hpp"
#endif
#ifndef COLMAN_START_PAGE_HPP
#  include "colman_start_page.hpp"
#endif
#ifndef COLMAN_DB_UTIL_HPP
#  include "colman_db_util.hpp"
#endif
#ifndef COLMAN_PREFERENCES_PAGE_HPP
#  include "colman_preferences_page.hpp"
#endif
#ifndef COLMAN_SCREENSHOT_HPP
#  include "colman_screenshot.hpp"
#endif
#ifndef COLMAN_TRACE_HPP
#  include "colman_trace.hpp"
#endif
#ifndef COLMAN_TUTORIAL_SCREENSHOTS_HPP
#  include "colman_tutorial_screenshots.hpp"
#endif
#ifndef SYSTEM_IOSTREAM
#  include <iostream>
#  define SYSTEM_IOSTREAM
#endif
#ifndef SYSTEM_QTDEBUG
#  include <QtDebug>
#  define SYSTEM_QTDEBUG
#endif
#ifndef SYSTEM_QSQLDRIVER
#  include <QSqlDriver>
#  define SYSTEM_QSQLDRIVER
#endif
#ifndef SYSTEM_QTRANSLATOR
#  include <QTranslator>
#  define SYSTEM_QTRANSLATOR
#endif
#ifndef SYSTEM_QSETTINGS
#  include <QSettings>
#  define SYSTEM_QSETTINGS
#endif
#ifndef SYSTEM_QLIBRARYINFO
#  include <QLibraryInfo>
#  define SYSTEM_QLIBRARYINFO
#endif
#ifndef SYSTEM_QNETWORKPROXY
#  include <QNetworkProxy>
#  define SYSTEM_QNETWORKPROXY
#endif
#ifndef SYSTEM_QCOMMANDLINEPARSER
#  include <QCommandLineParser>
#  define SYSTEM_QCOMMANDLINEPARSER
#endif
#ifndef SYSTEM_QGRAPHICSVIEW
#  include <QGraphicsView>
#  define SYSTEM_QGRAPHICSVIEW
#endif
#ifndef SYSTEM_QWHEELEVENT
#  include <QWheelEvent>
#  define SYSTEM_QWHEELEVENT
#endif
#ifndef SYSTEM_QKEYEVENT
#  include <QKeyEvent>
#  define SYSTEM_QKEYEVENT
#endif
#ifndef SYSTEM_QTIMER
#  include <QTimer>
#  define SYSTEM_QTIMER
#endif
#ifndef SYSTEM_QDIR
#  include <QDir>
#  define SYSTEM_QDIR
#endif
#ifndef SYSTEM_QTEMPORARYDIR
#  include <QTemporaryDir>
#  define SYSTEM_QTEMPORARYDIR
#endif
#ifndef SYSTEM_MEMORY
#  include <memory>
#  define SYSTEM_MEMORY
#endif
#ifndef SYSTEM_QICON
#  include <QIcon>
#  define SYSTEM_QICON
#endif


namespace
{
  /// Name of the option that sets the trace level.
  char constexpr TraceLevelOption[] = "trace-level";

  /// Name of the developer option that renders UI snapshots.
  char constexpr UiSnapshotOption[] = "ui-snapshot";

  /// Name of the developer option that renders the tutorial screenshots.
  char constexpr TutorialScreenshotsOption[] = "tutorial-screenshots";

  /// Name of the option naming the tutorial sample data file.
  char constexpr TutorialDataOption[] = "tutorial-data";

  /// Name of the option enabling tutorial screenshots that need internet.
  char constexpr TutorialNetworkOption[] = "tutorial-network";

/// Adds colman's command-line options to \a parser.
/**
   Called before and after the translators are installed: the first,
   untranslated parse only looks for the UI snapshot option so that the
   pixel baseline does not depend on the system language.
   @param parser parser to configure.
*/
  void add_options(QCommandLineParser& parser)
  {
    parser.setApplicationDescription(QCoreApplication::translate(
        "colman::CommandLine", "Collection Manager for school libraries"));
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(
        QString::fromLatin1(TraceLevelOption),
        QCoreApplication::translate(
            "colman::CommandLine",
            "Print diagnostic traces on the console (0 = silent, "
            "2 = internet search)."),
        QCoreApplication::translate("colman::CommandLine", "number"),
        QStringLiteral("0")));
    QCommandLineOption snapshot_option(
        QString::fromLatin1(UiSnapshotOption),
        QCoreApplication::translate(
            "colman::CommandLine",
            "Render every main-window page to a PNG in <dir> and exit."),
        QCoreApplication::translate("colman::CommandLine", "dir"));
    snapshot_option.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(snapshot_option);
    // Developer options for tools/tutorial_screenshots.sh; hidden and
    // untranslated like --ui-snapshot.
    QCommandLineOption tutorial_option(
        QString::fromLatin1(TutorialScreenshotsOption),
        QStringLiteral("Render the tutorial screenshots into <dir> and exit."),
        QStringLiteral("dir"));
    tutorial_option.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(tutorial_option);
    QCommandLineOption tutorial_data_option(
        QString::fromLatin1(TutorialDataOption),
        QStringLiteral("Sample data (JSON) for --tutorial-screenshots."),
        QStringLiteral("file"));
    tutorial_data_option.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(tutorial_data_option);
    QCommandLineOption tutorial_network_option(
        QString::fromLatin1(TutorialNetworkOption),
        QStringLiteral("Also render tutorial screenshots that need internet."));
    tutorial_network_option.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(tutorial_network_option);
    parser.addPositionalArgument(
        QCoreApplication::translate("colman::CommandLine", "file"),
        QCoreApplication::translate("colman::CommandLine",
                                    "Collection file (.col) to open."),
        QCoreApplication::translate("colman::CommandLine", "[file]"));
  }


/// Clamps \a pt to [8, 24], applies it as the application font, and persists it.
  void apply_font_size(int pt)
  {
    pt = qBound(8, pt, 24);
    QFont f = QApplication::font();
    f.setPointSize(pt);
    QApplication::setFont(f);
    QSettings().setValue("font_size", pt);
  }

/// Global event filter: Ctrl+Wheel and Ctrl++/−/0 change the application font size.
/**
 * Installed on QApplication so it receives events before any widget.
 * Ctrl+Wheel is suppressed when the target widget is inside a QGraphicsView
 * so that the existing per-view zoom handlers are not disrupted.
 */
  class FontSizeFilter : public QObject
  {
  public:
    explicit FontSizeFilter(QObject* parent = nullptr)
        : QObject(parent) {}

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
      if(event->type() == QEvent::Wheel) {
        auto* we = static_cast<QWheelEvent*>(event);
        if(we->modifiers() & Qt::ControlModifier) {
          if(inside_graphics_view(watched)) {
            return false;
          }
          int const delta = we->angleDelta().y() > 0 ? 1 : -1;
          apply_font_size(QApplication::font().pointSize() + delta);
          return true;
        }
      }
      if(event->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(event);
        if(ke->modifiers() & Qt::ControlModifier) {
          int const key = ke->key();
          if(key == Qt::Key_Plus || key == Qt::Key_Equal) {
            apply_font_size(QApplication::font().pointSize() + 1);
            return true;
          }
          if(key == Qt::Key_Minus) {
            apply_font_size(QApplication::font().pointSize() - 1);
            return true;
          }
          if(key == Qt::Key_0) {
            apply_font_size(12);
            return true;
          }
          if(key == Qt::Key_P) {
            QTimer::singleShot(0, []() { colman::take_screenshot(); });
            return true;
          }
        }
      }
      return false;
    }

  private:
  /// Returns true when \a obj is a widget inside a QGraphicsView hierarchy.
    static bool inside_graphics_view(QObject* obj)
    {
      QWidget* w = qobject_cast<QWidget*>(obj);
      if(!w) {
        w = QApplication::widgetAt(QCursor::pos());
      }
      while(w) {
        if(qobject_cast<QGraphicsView*>(w)) {
          return true;
        }
        w = w->parentWidget();
      }
      return false;
    }
  };

} // namespace


/**
   Application entry point.
   Initializes the Qt application, loads German translations, sets up the
   start and preferences pages, shows the main window, and runs the event loop.
   Closes the database connection on exit.
   @param argc command-line argument count.
   @param argv command-line argument values.
   @return Qt application exit code.
*/
int main(int argc, char* argv[])
{
  colman::set_trace_level(0);
  QCoreApplication::setOrganizationName("cpesch");
  QCoreApplication::setApplicationName("colman");
  QApplication app(argc, argv);
  // Scratch directory for --tutorial-screenshots: holds the throw-away
  // settings and the sample collection file.
  std::unique_ptr<QTemporaryDir> tutorial_dir;
  colman::TutorialScreenshotOptions tutorial_options;
  // Developer hook: --ui-snapshot=<dir> renders every main-window page to a
  // PNG and exits.  Used as a pixel baseline to guard the hand-written
  // main-window UI against layout regressions.
  {
    QCommandLineParser early_parser;
    add_options(early_parser);
    static_cast<void>(early_parser.parse(app.arguments()));
    QString const snapshot_option = QString::fromLatin1(UiSnapshotOption);
    if(early_parser.isSet(snapshot_option)) {
      QString const dir = early_parser.value(snapshot_option);
      return colman::render_ui_snapshots(dir) < 0 ? 1 : 0;
    }
    // Developer hook: --tutorial-screenshots=<dir> drives the application
    // through the tutorial (see colman_tutorial_screenshots.hpp).  Settings
    // go to a temporary directory so that the user's configuration stays
    // untouched and every run starts from the same state.
    QString const tutorial_option =
        QString::fromLatin1(TutorialScreenshotsOption);
    if(early_parser.isSet(tutorial_option)) {
      tutorial_dir = std::make_unique<QTemporaryDir>();
      if(!tutorial_dir->isValid()) {
        std::cerr << "cannot create a temporary directory\n";
        return 1;
      }
      QSettings::setDefaultFormat(QSettings::IniFormat);
      QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                         tutorial_dir->filePath("settings"));
      // Qt's own settings (e.g. the file dialog sidebar) use NativeFormat.
      QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                         tutorial_dir->filePath("settings"));
      QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
      tutorial_options.out_dir_ =
          QDir(early_parser.value(tutorial_option)).absolutePath();
      tutorial_options.sample_data_file_ = early_parser.value(
          QString::fromLatin1(TutorialDataOption));
      tutorial_options.work_dir_ = tutorial_dir->filePath("collections");
      tutorial_options.with_network_ = early_parser.isSet(
          QString::fromLatin1(TutorialNetworkOption));
      QDir().mkpath(tutorial_options.work_dir_);
    }
  }
  // Qt built-in translations (file dialogs, message boxes, …).
  QTranslator translator_qt;
  static_cast<void>(
      translator_qt.load(QLocale::system(), "qtbase", "_",
                         QLibraryInfo::path(QLibraryInfo::TranslationsPath),
                         ".qm"));
  // Application translations (embedded as Qt resources).
  QTranslator translator;
  static_cast<void>(
      translator.load(QLocale::system(), "colman", "_", ":/translations",
                      ".qm"));
  app.installTranslator(&translator_qt);
  app.installTranslator(&translator);
  QCommandLineParser parser;
  add_options(parser);
  // parse() instead of process(): the latter calls exit(), which makes Qt
  // complain about thread storage destroyed too early.
  if(!parser.parse(app.arguments())) {
    std::cerr << qPrintable(parser.errorText()) << '\n';
    return 1;
  }
  if(parser.isSet(QStringLiteral("help"))
     || parser.isSet(QStringLiteral("help-all")))
  {
    std::cout << qPrintable(parser.helpText()) << std::flush;
    return 0;
  }
  {
    QString const level_text =
        parser.value(QString::fromLatin1(TraceLevelOption));
    if(!colman::set_trace_level(level_text)) {
      std::cerr << qPrintable(QCoreApplication::translate(
                       "colman::CommandLine",
                       "Invalid trace level: %1").arg(level_text))
                << '\n';
      return 1;
    }
  }
  QStringList const positional = parser.positionalArguments();
  QString const collection_file =
      positional.isEmpty() ? QString() : positional.first();
  {
    QSettings settings;
    QVariant const stored = settings.value("font_size");
    if(stored.isValid()) {
      QFont f = app.font();
      f.setPointSize(stored.toInt());
      app.setFont(f);
    }
  }
  app.installEventFilter(new FontSizeFilter(&app));
  app.setWindowIcon(QIcon(":/icons/owls/colman_logo.jpeg"));
  colman::AppData data;
  {
    QSettings settings;
    data.set_thumbnail_dir(settings.value("thumbnail_dir").toString());
  }
  QNetworkProxyFactory::setUseSystemConfiguration(true);
  data.mw_.setup_ui(data.w_);
  data.w_.setMinimumSize(1024, 600);
  data.w_.resize(1200, 824);
  colman::setup_start_page(app, data, collection_file);
  colman::setup_preferences_page(app, data);
  if(tutorial_dir) {
    data.w_.resize(1100, 800);
    data.w_.show();
    QTimer::singleShot(0, &app, [&app, &data, &tutorial_options]() {
      app.exit(colman::run_tutorial_screenshots(data, tutorial_options));
    });
    return app.exec();
  }
  data.w_.show();
  return app.exec();
}
