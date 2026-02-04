#include "daemonservice.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QSettings>

DaemonService::DaemonService(QObject *parent) : QObject(parent) {
  m_protector = new SystemProtector(this);
  m_powerMonitor = m_protector->powerMonitor();
  m_cpuController = m_protector->cpuController();

  // Connect internal signals to DBus signals
  connect(m_powerMonitor, &PowerMonitor::gpuPowerChanged, this,
          &DaemonService::onGpuPowerChanged);
  connect(m_powerMonitor, &PowerMonitor::gpuPowerThresholdChanged, this,
          &DaemonService::onGpuPowerThresholdChanged);
  connect(m_powerMonitor, &PowerMonitor::thresholdExceededChanged, this,
          &DaemonService::onThresholdExceededChanged);

  connect(m_cpuController, &CpuController::currentMaxFrequencyChanged, this,
          &DaemonService::onCurrentMaxFrequencyChanged);
  connect(m_cpuController, &CpuController::maxFrequencyChanged, this,
          &DaemonService::onMaxFrequencyChanged);
  connect(m_cpuController, &CpuController::regulationEnabledChanged, this,
          &DaemonService::onRegulationEnabledChanged);

  connect(m_protector, &SystemProtector::autoProtectionChanged, this,
          &DaemonService::onAutoProtectionChanged);

  // Load settings
  loadSettings();
}

DaemonService::~DaemonService() { saveSettings(); }

bool DaemonService::registerService() {
  QDBusConnection bus = QDBusConnection::systemBus();

  if (!bus.registerService("org.uncrash.Daemon")) {
    qWarning() << "Failed to register DBus service:"
               << bus.lastError().message();
    return false;
  }

  if (!bus.registerObject("/org/uncrash/Daemon", this,
                          QDBusConnection::ExportAllProperties |
                              QDBusConnection::ExportAllSlots |
                              QDBusConnection::ExportAllSignals)) {
    qWarning() << "Failed to register DBus object:"
               << bus.lastError().message();
    return false;
  }

  qInfo() << "DBus service registered: org.uncrash.Daemon";
  return true;
}

// Property getters
double DaemonService::gpuPower() const { return m_powerMonitor->gpuPower(); }

double DaemonService::gpuPowerThreshold() const {
  return m_powerMonitor->gpuPowerThreshold();
}

double DaemonService::currentMaxFrequency() const {
  return m_cpuController->currentMaxFrequency();
}

double DaemonService::maxFrequency() const {
  return m_cpuController->maxFrequency();
}

bool DaemonService::regulationEnabled() const {
  return m_cpuController->regulationEnabled();
}

bool DaemonService::autoProtection() const {
  return m_protector->autoProtection();
}

bool DaemonService::thresholdExceeded() const {
  return m_powerMonitor->thresholdExceeded();
}

// Property setters
void DaemonService::setGpuPowerThreshold(double threshold) {
  m_powerMonitor->setGpuPowerThreshold(threshold);
  saveSettings();
}

void DaemonService::setMaxFrequency(double frequency) {
  m_cpuController->setMaxFrequency(frequency);
  saveSettings();
}

void DaemonService::setRegulationEnabled(bool enabled) {
  m_cpuController->setRegulationEnabled(enabled);
  saveSettings();
}

void DaemonService::setAutoProtection(bool enabled) {
  m_protector->setAutoProtection(enabled);
  saveSettings();
}

// DBus methods
bool DaemonService::ApplyFrequencyLimit() {
  m_cpuController->applyFrequencyLimit();
  emit FrequencyLimitApplied(m_cpuController->maxFrequency());
  return true;
}

bool DaemonService::RemoveFrequencyLimit() {
  m_cpuController->removeFrequencyLimit();
  emit FrequencyLimitRemoved();
  return true;
}

QVariantMap DaemonService::GetStatus() {
  QVariantMap status;
  status["gpuPower"] = gpuPower();
  status["gpuPowerThreshold"] = gpuPowerThreshold();
  status["currentMaxFrequency"] = currentMaxFrequency();
  status["maxFrequency"] = maxFrequency();
  status["regulationEnabled"] = regulationEnabled();
  status["autoProtection"] = autoProtection();
  status["thresholdExceeded"] = thresholdExceeded();
  return status;
}

// Signal forwarding slots
void DaemonService::onGpuPowerChanged() { emit GpuPowerChanged(gpuPower()); }

void DaemonService::onGpuPowerThresholdChanged() {
  emit GpuPowerThresholdChanged(gpuPowerThreshold());
}

void DaemonService::onCurrentMaxFrequencyChanged() {
  emit CurrentMaxFrequencyChanged(currentMaxFrequency());
}

void DaemonService::onMaxFrequencyChanged() {
  emit MaxFrequencyChanged(maxFrequency());
}

void DaemonService::onRegulationEnabledChanged() {
  emit RegulationEnabledChanged(regulationEnabled());
}

void DaemonService::onAutoProtectionChanged() {
  emit AutoProtectionChanged(autoProtection());
}

void DaemonService::onThresholdExceededChanged() {
  emit ThresholdExceededChanged(thresholdExceeded());
}

void DaemonService::loadSettings() {
  QSettings settings("/etc/uncrash/uncrash.conf", QSettings::IniFormat);

  double threshold = settings.value("gpuPowerThreshold", 100.0).toDouble();
  double frequency = settings.value("cpuMaxFrequency", 3.5).toDouble();
  bool autoProtect = settings.value("autoProtection", true).toBool();

  m_powerMonitor->setGpuPowerThreshold(threshold);
  m_cpuController->setMaxFrequency(frequency);
  m_protector->setAutoProtection(autoProtect);

  qInfo() << "Settings loaded from /etc/uncrash/uncrash.conf";
}

void DaemonService::saveSettings() {
  QSettings settings("/etc/uncrash/uncrash.conf", QSettings::IniFormat);

  settings.setValue("gpuPowerThreshold", gpuPowerThreshold());
  settings.setValue("cpuMaxFrequency", maxFrequency());
  settings.setValue("autoProtection", autoProtection());

  settings.sync();
}
