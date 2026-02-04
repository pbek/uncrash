#pragma once

#include "cpucontroller.h"
#include "powermonitor.h"
#include <QObject>

class SystemProtector : public QObject {
  Q_OBJECT
  Q_PROPERTY(PowerMonitor *powerMonitor READ powerMonitor CONSTANT)
  Q_PROPERTY(CpuController *cpuController READ cpuController CONSTANT)
  Q_PROPERTY(bool autoProtection READ autoProtection WRITE setAutoProtection
                 NOTIFY autoProtectionChanged)

public:
  explicit SystemProtector(QObject *parent = nullptr);

  PowerMonitor *powerMonitor() const { return m_powerMonitor; }
  CpuController *cpuController() const { return m_cpuController; }
  bool autoProtection() const { return m_autoProtection; }

  void setAutoProtection(bool enabled);

private slots:
  void handleThresholdChange();

private:
  PowerMonitor *m_powerMonitor;
  CpuController *m_cpuController;
  bool m_autoProtection = true;

signals:
  void autoProtectionChanged();
};
