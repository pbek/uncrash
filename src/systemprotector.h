#pragma once

#include "cpucontroller.h"
#include "powermonitor.h"
#include <QObject>
#include <QTimer>

class SystemProtector : public QObject {
  Q_OBJECT
  Q_PROPERTY(PowerMonitor *powerMonitor READ powerMonitor CONSTANT)
  Q_PROPERTY(CpuController *cpuController READ cpuController CONSTANT)
  Q_PROPERTY(bool autoProtection READ autoProtection WRITE setAutoProtection
                 NOTIFY autoProtectionChanged)
  Q_PROPERTY(int cooldownSeconds READ cooldownSeconds WRITE setCooldownSeconds
                 NOTIFY cooldownSecondsChanged)

public:
  explicit SystemProtector(QObject *parent = nullptr);

  PowerMonitor *powerMonitor() const { return m_powerMonitor; }
  CpuController *cpuController() const { return m_cpuController; }
  bool autoProtection() const { return m_autoProtection; }
  int cooldownSeconds() const { return m_cooldownSeconds; }

  void setAutoProtection(bool enabled);
  void setCooldownSeconds(int seconds);

private slots:
  void handleThresholdChange();
  void onCooldownExpired();

private:
  PowerMonitor *m_powerMonitor;
  CpuController *m_cpuController;
  QTimer *m_cooldownTimer;
  bool m_autoProtection = true;
  int m_cooldownSeconds = 5;
  bool m_limitWasAutoApplied = false;

signals:
  void autoProtectionChanged();
  void cooldownSecondsChanged();
};
