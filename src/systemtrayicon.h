#pragma once

#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>

class DaemonClient;

class SystemTrayIcon : public QObject {
  Q_OBJECT

public:
  explicit SystemTrayIcon(DaemonClient *daemonClient,
                          QObject *parent = nullptr);
  ~SystemTrayIcon() override;

  void show();
  void hide();
  bool isVisible() const;

signals:
  void showWindowRequested();
  void quitRequested();

private slots:
  void onActivated(QSystemTrayIcon::ActivationReason reason);
  void onAutoProtectionChanged();
  void onCpuLimitAppliedChanged();
  void updateIcon();
  void updateTooltip();

private:
  void setupTrayIcon();
  void setupMenu();
  QPixmap createCpuIcon(bool active, bool disconnected = false);

  QSystemTrayIcon *m_trayIcon;
  QMenu *m_trayMenu;
  DaemonClient *m_daemonClient;

  QAction *m_showAction;
  QAction *m_quitAction;
};
