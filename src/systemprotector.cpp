#include "systemprotector.h"
#include <QDebug>

SystemProtector::SystemProtector(QObject *parent) : QObject(parent) {
  m_powerMonitor = new PowerMonitor(this);
  m_cpuController = new CpuController(this);

  // Connect power monitor threshold changes to CPU regulation
  connect(m_powerMonitor, &PowerMonitor::thresholdExceededChanged, this,
          &SystemProtector::handleThresholdChange);
}

void SystemProtector::setAutoProtection(bool enabled) {
  if (m_autoProtection == enabled)
    return;

  m_autoProtection = enabled;
  emit autoProtectionChanged();

  // Apply or remove protection based on current state
  if (enabled) {
    handleThresholdChange();
  } else {
    m_cpuController->removeFrequencyLimit();
  }
}

void SystemProtector::handleThresholdChange() {
  if (!m_autoProtection)
    return;

  if (m_powerMonitor->thresholdExceeded()) {
    qDebug() << "GPU power threshold exceeded, applying CPU frequency limit";
    m_cpuController->applyFrequencyLimit();
  } else {
    qDebug() << "GPU power below threshold, removing CPU frequency limit";
    m_cpuController->removeFrequencyLimit();
  }
}
