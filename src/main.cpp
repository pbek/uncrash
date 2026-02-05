#include "client/daemonclient.h"
#include "settingsmanager.h"
#include "systemtrayicon.h"
#include "utils/cli.h"
#include "version.h"
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[]) {
  // First, check if we're running in CLI-only mode (help, version, or
  // completion) Use QCoreApplication for these to avoid GUI initialization
  // overhead
  bool isCliOnlyMode = false;
  bool debugMode = false;
  for (int i = 1; i < argc; i++) {
    QString arg = QString::fromLocal8Bit(argv[i]);
    if (arg == "--help" || arg == "-h" || arg == "--version" || arg == "-v" ||
        arg == "--completion-bash" || arg == "--completion-fish") {
      isCliOnlyMode = true;
      break;
    }
    if (arg == "--debug") {
      debugMode = true;
    }
  }

  if (isCliOnlyMode) {
    // Use QCoreApplication for CLI-only operations
    QCoreApplication coreApp(argc, argv);
    QCoreApplication::setOrganizationName("uncrash");
    QCoreApplication::setOrganizationDomain("uncrash.app");
    QCoreApplication::setApplicationName("uncrash");
    QCoreApplication::setApplicationVersion(UNCRASH_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        "A KDE Kirigami application for preventing "
        "desktop crashes due to temperature and power issues.");

    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();

    QCommandLineOption completionBashOption(
        QStringList() << "completion-bash",
        "Generate Bash completion script and exit");
    parser.addOption(completionBashOption);

    QCommandLineOption completionFishOption(
        QStringList() << "completion-fish",
        "Generate Fish completion script and exit");
    parser.addOption(completionFishOption);

    QCommandLineOption debugOption(
        QStringList() << "debug",
        "Run in debug mode with separate settings and data directories");
    parser.addOption(debugOption);

    parser.process(coreApp);

    QList<QCommandLineOption> options;
    options << helpOption << versionOption << completionBashOption
            << completionFishOption << debugOption;

    if (parser.isSet(completionBashOption)) {
      Utils::Cli::generateBashCompletionScript(options, "uncrash");
      return 0;
    }

    if (parser.isSet(completionFishOption)) {
      Utils::Cli::generateFishCompletionScript(options, "uncrash");
      return 0;
    }

    // Help and version are automatically handled by parser.process()
    return 0;
  }

  // GUI mode - use QApplication
  QApplication app(argc, argv);

  QApplication::setOrganizationName("uncrash");
  QApplication::setOrganizationDomain("uncrash.app");
  QApplication::setApplicationVersion(UNCRASH_VERSION);

  // Use different organization/app name for debug mode to separate settings
  if (debugMode) {
    QApplication::setApplicationName("uncrash-debug");
    QApplication::setApplicationDisplayName("Uncrash (Debug) " +
                                            QStringLiteral(UNCRASH_VERSION));
  } else {
    QApplication::setApplicationName("uncrash");
    QApplication::setApplicationDisplayName("Uncrash " +
                                            QStringLiteral(UNCRASH_VERSION));
  }

  qDebug() << "Starting uncrash application...";

  KLocalizedString::setApplicationDomain("uncrash");
  QApplication::setWindowIcon(QIcon::fromTheme("uncrash"));

  // Don't quit when last window closes - we have a tray icon
  QApplication::setQuitOnLastWindowClosed(false);

  QQmlApplicationEngine engine;

  // Create and register SettingsManager (for GUI preferences only)
  SettingsManager settingsManager;
  engine.rootContext()->setContextProperty("settingsManager", &settingsManager);

  // Create and register DaemonClient (connects to uncrashd via DBus)
  DaemonClient daemonClient;
  engine.rootContext()->setContextProperty("daemonClient", &daemonClient);

  // Make version and debug mode available to QML
  engine.rootContext()->setContextProperty("appVersion",
                                           QString(UNCRASH_VERSION));
  engine.rootContext()->setContextProperty("isDebugMode", debugMode);

  engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

  qDebug() << "Loading QML from qrc:/qt/qml/uncrash/src/main.qml";

  // Load QML
  const QUrl url(u"qrc:/qt/qml/uncrash/src/main.qml"_s);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
          qCritical() << "ERROR: Failed to load QML file from" << url;
          QCoreApplication::exit(-1);
        }
      },
      Qt::QueuedConnection);

  engine.load(url);

  if (engine.rootObjects().isEmpty()) {
    qCritical() << "ERROR: Failed to load QML file - no root objects created";
    return -1;
  }

  qDebug() << "QML loaded successfully, starting application...";

  // Get the main window
  QObject *rootObject = engine.rootObjects().first();
  QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);

  if (!window) {
    qCritical() << "ERROR: Root object is not a QQuickWindow";
    return -1;
  }

  // Connect QML quit signal to actually quit the application
  QObject::connect(rootObject, SIGNAL(quitRequested()), &app, SLOT(quit()));

  // Create and setup system tray icon
  SystemTrayIcon trayIcon(&daemonClient);

  // Connect tray icon signals
  QObject::connect(&trayIcon, &SystemTrayIcon::showWindowRequested, window,
                   [window]() {
                     if (window->isVisible()) {
                       window->hide();
                     } else {
                       window->show();
                       window->raise();
                       window->requestActivate();
                     }
                   });

  QObject::connect(&trayIcon, &SystemTrayIcon::quitRequested, &app,
                   &QApplication::quit);

  // Make tray icon accessible from QML for close-to-tray behavior
  engine.rootContext()->setContextProperty("systemTrayIcon", &trayIcon);

  // Show the tray icon
  trayIcon.show();

  return app.exec();
}
