#include "daemonservice.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QSettings>

DaemonService::DaemonService(QObject *parent) : QObject(parent) {
  m_protector = new SystemProtector(this);
  m_powerMonitor = m_protector->powerMonitor();
  m_cpuController = m_protector->cpuController();
  m_temperatureMonitor = new TemperatureMonitor(this);

  // Connect internal signals to DBus signals
  connect(m_powerMonitor, &PowerMonitor::gpuPowerChanged, this,
          &DaemonService::onGpuPowerChanged);
  connect(m_powerMonitor, &PowerMonitor::gpuPowerThresholdChanged, this,
          &DaemonService::onGpuPowerThresholdChanged);
  connect(m_powerMonitor, &PowerMonitor::thresholdExceededChanged, this,
          &DaemonService::onThresholdExceededChanged);

  connect(m_cpuController, &CpuController::currentMaxFrequencyChanged, this,
          &DaemonService::onCurrentMaxFrequencyChanged);
  connect(m_cpuController, &CpuController::currentFrequencyChanged, this,
          &DaemonService::onCurrentFrequencyChanged);
  connect(m_cpuController, &CpuController::maxFrequencyChanged, this,
          &DaemonService::onMaxFrequencyChanged);
  connect(m_cpuController, &CpuController::regulationEnabledChanged, this,
          &DaemonService::onRegulationEnabledChanged);
  connect(m_cpuController, &CpuController::cpuLimitAppliedChanged, this,
          &DaemonService::onCpuLimitAppliedChanged);

  connect(m_protector, &SystemProtector::autoProtectionChanged, this,
          &DaemonService::onAutoProtectionChanged);
  connect(m_protector, &SystemProtector::cooldownSecondsChanged, this,
          &DaemonService::onCooldownSecondsChanged);

  // Connect temperature monitor signals
  connect(m_temperatureMonitor, &TemperatureMonitor::gpuTemperatureChanged,
          this, &DaemonService::GpuTemperatureChanged);
  connect(m_temperatureMonitor, &TemperatureMonitor::cpuTemperatureChanged,
          this, &DaemonService::CpuTemperatureChanged);
  connect(m_temperatureMonitor,
          &TemperatureMonitor::motherboardTemperatureChanged, this,
          &DaemonService::MotherboardTemperatureChanged);
  connect(m_temperatureMonitor, &TemperatureMonitor::fanSpeedsChanged, this,
          [this]() {
            emit CpuFanSpeedChanged(cpuFanSpeed());
            emit GpuFanSpeedChanged(gpuFanSpeed());
          });

  // Start temperature monitoring (update every 2 seconds)
  m_temperatureMonitor->startMonitoring(2000);

  // Emit initial GPU vendor/name
  emit GpuVendorChanged(m_temperatureMonitor->gpuVendor());
  emit GpuNameChanged(m_temperatureMonitor->gpuName());

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

double DaemonService::currentFrequency() const {
  return m_cpuController->currentFrequency();
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

int DaemonService::cooldownSeconds() const {
  return m_protector->cooldownSeconds();
}

bool DaemonService::thresholdExceeded() const {
  return m_powerMonitor->thresholdExceeded();
}

bool DaemonService::cpuLimitApplied() const {
  return m_cpuController->cpuLimitApplied();
}

// Temperature getters
double DaemonService::gpuTemperature() const {
  return m_temperatureMonitor->gpuTemperature();
}

int DaemonService::gpuFanSpeed() const {
  return m_temperatureMonitor->gpuFanSpeed();
}

double DaemonService::cpuTemperature() const {
  return m_temperatureMonitor->cpuTemperature();
}

int DaemonService::cpuFanSpeed() const {
  return m_temperatureMonitor->cpuFanSpeed();
}

double DaemonService::motherboardTemperature() const {
  return m_temperatureMonitor->motherboardTemperature();
}

QString DaemonService::gpuVendor() const {
  return m_temperatureMonitor->gpuVendor();
}

QString DaemonService::gpuName() const {
  return m_temperatureMonitor->gpuName();
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

void DaemonService::setCooldownSeconds(int seconds) {
  m_protector->setCooldownSeconds(seconds);
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
  status["currentFrequency"] = currentFrequency();
  status["maxFrequency"] = maxFrequency();
  status["regulationEnabled"] = regulationEnabled();
  status["autoProtection"] = autoProtection();
  status["cooldownSeconds"] = cooldownSeconds();
  status["thresholdExceeded"] = thresholdExceeded();
  status["cpuLimitApplied"] = cpuLimitApplied();

  // Add temperature data
  status["gpuTemperature"] = gpuTemperature();
  status["gpuFanSpeed"] = gpuFanSpeed();
  status["cpuTemperature"] = cpuTemperature();
  status["cpuFanSpeed"] = cpuFanSpeed();
  status["motherboardTemperature"] = motherboardTemperature();
  status["gpuVendor"] = gpuVendor();
  status["gpuName"] = gpuName();

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

void DaemonService::onCurrentFrequencyChanged() {
  emit CurrentFrequencyChanged(currentFrequency());
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

void DaemonService::onCooldownSecondsChanged() {
  emit CooldownSecondsChanged(cooldownSeconds());
}

void DaemonService::onThresholdExceededChanged() {
  emit ThresholdExceededChanged(thresholdExceeded());
}

void DaemonService::onCpuLimitAppliedChanged() {
  emit CpuLimitAppliedChanged(cpuLimitApplied());
}

void DaemonService::loadSettings() {
  QSettings settings("/etc/uncrash/uncrash.conf", QSettings::IniFormat);

  double threshold = settings.value("gpuPowerThreshold", 100.0).toDouble();
  double frequency = settings.value("cpuMaxFrequency", 3.5).toDouble();
  bool autoProtect = settings.value("autoProtection", true).toBool();
  int cooldown = settings.value("cooldownSeconds", 5).toInt();

  m_powerMonitor->setGpuPowerThreshold(threshold);
  m_cpuController->setMaxFrequency(frequency);
  m_protector->setAutoProtection(autoProtect);
  m_protector->setCooldownSeconds(cooldown);

  qInfo() << "Settings loaded from /etc/uncrash/uncrash.conf";
}

void DaemonService::saveSettings() {
  QSettings settings("/etc/uncrash/uncrash.conf", QSettings::IniFormat);

  settings.setValue("gpuPowerThreshold", gpuPowerThreshold());
  settings.setValue("cpuMaxFrequency", maxFrequency());
  settings.setValue("autoProtection", autoProtection());
  settings.setValue("cooldownSeconds", cooldownSeconds());

  settings.sync();
}
