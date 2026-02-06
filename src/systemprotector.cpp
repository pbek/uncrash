#include "systemprotector.h"
#include <QDebug>

SystemProtector::SystemProtector(QObject *parent) : QObject(parent) {
  m_powerMonitor = new PowerMonitor(this);
  m_cpuController = new CpuController(this);
  m_cooldownTimer = new QTimer(this);
  m_cooldownTimer->setSingleShot(true);

  // Connect power monitor threshold changes to CPU regulation
  connect(m_powerMonitor, &PowerMonitor::thresholdExceededChanged, this,
          &SystemProtector::handleThresholdChange);

  // Connect cooldown timer
  connect(m_cooldownTimer, &QTimer::timeout, this,
          &SystemProtector::onCooldownExpired);
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
    m_cooldownTimer->stop();
    m_cpuController->removeFrequencyLimit();
    m_limitWasAutoApplied = false;
  }
}

void SystemProtector::setCooldownSeconds(int seconds) {
  if (m_cooldownSeconds == seconds)
    return;

  m_cooldownSeconds = seconds;
  emit cooldownSecondsChanged();
}

void SystemProtector::handleThresholdChange() {
  if (!m_autoProtection)
    return;

  if (m_powerMonitor->thresholdExceeded()) {
    // Threshold exceeded - apply limit immediately
    qDebug() << "GPU power threshold exceeded, applying CPU frequency limit";
    m_cpuController->applyFrequencyLimit();
    m_limitWasAutoApplied = true;

    // Stop any pending cooldown timer since we're re-applying
    m_cooldownTimer->stop();
  } else {
    // Threshold not exceeded - only remove if limit was auto-applied
    if (m_limitWasAutoApplied) {
      qDebug() << "GPU power below threshold, starting cooldown timer for"
               << m_cooldownSeconds << "seconds";
      // Start cooldown timer
      m_cooldownTimer->start(m_cooldownSeconds * 1000);
    }
  }
}

void SystemProtector::onCooldownExpired() {
  // Only remove if threshold is still not exceeded and limit was auto-applied
  if (!m_powerMonitor->thresholdExceeded() && m_limitWasAutoApplied) {
    qDebug() << "Cooldown expired, removing CPU frequency limit";
    m_cpuController->removeFrequencyLimit();
    m_limitWasAutoApplied = false;
  }
}
