#include "daemonservice.h"
#include <QCoreApplication>
#include <QDebug>
#include <csignal>

static QCoreApplication *app = nullptr;

void signalHandler(int signal) {
  if (signal == SIGTERM || signal == SIGINT) {
    qInfo() << "Received signal" << signal << ", shutting down...";
    if (app) {
      app->quit();
    }
  }
}

int main(int argc, char *argv[]) {
  QCoreApplication application(argc, argv);
  app = &application;

  QCoreApplication::setOrganizationName("uncrash");
  QCoreApplication::setApplicationName("uncrashd");
  QCoreApplication::setApplicationVersion("0.0.1");

  // Set up signal handlers
  std::signal(SIGTERM, signalHandler);
  std::signal(SIGINT, signalHandler);

  qInfo() << "Starting Uncrash daemon...";

  DaemonService service;
  if (!service.registerService()) {
    qCritical() << "Failed to register DBus service, exiting.";
    return 1;
  }

  qInfo() << "Uncrash daemon started successfully";

  return application.exec();
}
