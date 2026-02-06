#include "daemonclient.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

DaemonClient::DaemonClient(QObject *parent)
    : QObject(parent), m_interface(nullptr) {
  connectToService();

  // Monitor service availability
  QDBusConnection bus = QDBusConnection::systemBus();
  bus.connect("org.freedesktop.DBus", "/org/freedesktop/DBus",
              "org.freedesktop.DBus", "NameOwnerChanged", this,
              SLOT(onServiceOwnerChanged(QString, QString, QString)));
}

void DaemonClient::connectToService() {
  QDBusConnection bus = QDBusConnection::systemBus();

  if (m_interface) {
    delete m_interface;
    m_interface = nullptr;
  }

  m_interface = new QDBusInterface("org.uncrash.Daemon", "/org/uncrash/Daemon",
                                   "org.uncrash.Daemon", bus, this);

  if (!m_interface->isValid()) {
    qWarning() << "Failed to connect to uncrash daemon:"
               << bus.lastError().message();
    updateConnectionStatus(false);
    return;
  }

  // Connect to daemon signals
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "GpuPowerChanged", this, SLOT(onGpuPowerChanged(double)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "GpuPowerThresholdChanged", this,
              SLOT(onGpuPowerThresholdChanged(double)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "CurrentMaxFrequencyChanged", this,
              SLOT(onCurrentMaxFrequencyChanged(double)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "CurrentFrequencyChanged", this,
              SLOT(onCurrentFrequencyChanged(double)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "MaxFrequencyChanged", this, SLOT(onMaxFrequencyChanged(double)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "RegulationEnabledChanged", this,
              SLOT(onRegulationEnabledChanged(bool)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "AutoProtectionChanged", this,
              SLOT(onAutoProtectionChanged(bool)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "CooldownSecondsChanged", this,
              SLOT(onCooldownSecondsChanged(int)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "ThresholdExceededChanged", this,
              SLOT(onThresholdExceededChanged(bool)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "CpuLimitAppliedChanged", this,
              SLOT(onCpuLimitAppliedChanged(bool)));

  // Connect to temperature signals
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "GpuTemperatureChanged", this,
              SLOT(onGpuTemperatureChanged(double)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "GpuFanSpeedChanged", this, SLOT(onGpuFanSpeedChanged(int)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "CpuTemperatureChanged", this,
              SLOT(onCpuTemperatureChanged(double)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "CpuFanSpeedChanged", this, SLOT(onCpuFanSpeedChanged(int)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "MotherboardTemperatureChanged", this,
              SLOT(onMotherboardTemperatureChanged(double)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "GpuVendorChanged", this, SLOT(onGpuVendorChanged(QString)));
  bus.connect("org.uncrash.Daemon", "/org/uncrash/Daemon", "org.uncrash.Daemon",
              "GpuNameChanged", this, SLOT(onGpuNameChanged(QString)));

  updateConnectionStatus(true);
  refreshStatus();
}

void DaemonClient::updateConnectionStatus(bool status) {
  if (m_connected != status) {
    m_connected = status;
    emit connectedChanged();
  }
}

void DaemonClient::setGpuPowerThreshold(double threshold) {
  if (!m_interface || !m_interface->isValid()) {
    emit error("Not connected to daemon");
    return;
  }

  m_interface->setProperty("GpuPowerThreshold", threshold);
}

void DaemonClient::setMaxFrequency(double frequency) {
  if (!m_interface || !m_interface->isValid()) {
    emit error("Not connected to daemon");
    return;
  }

  m_interface->setProperty("MaxFrequency", frequency);
}

void DaemonClient::setRegulationEnabled(bool enabled) {
  if (!m_interface || !m_interface->isValid()) {
    emit error("Not connected to daemon");
    return;
  }

  m_interface->setProperty("RegulationEnabled", enabled);
}

void DaemonClient::setAutoProtection(bool enabled) {
  if (!m_interface || !m_interface->isValid()) {
    emit error("Not connected to daemon");
    return;
  }

  m_interface->setProperty("AutoProtection", enabled);
}

void DaemonClient::setCooldownSeconds(int seconds) {
  if (!m_interface || !m_interface->isValid()) {
    emit error("Not connected to daemon");
    return;
  }

  m_interface->setProperty("CooldownSeconds", seconds);
}

void DaemonClient::applyFrequencyLimit() {
  if (!m_interface || !m_interface->isValid()) {
    emit error("Not connected to daemon");
    return;
  }

  QDBusReply<bool> reply = m_interface->call("ApplyFrequencyLimit");
  if (!reply.isValid()) {
    emit error("Failed to apply frequency limit: " + reply.error().message());
  } else {
    // Refresh status to update thresholdExceeded and other properties
    refreshStatus();
  }
}

void DaemonClient::removeFrequencyLimit() {
  if (!m_interface || !m_interface->isValid()) {
    emit error("Not connected to daemon");
    return;
  }

  QDBusReply<bool> reply = m_interface->call("RemoveFrequencyLimit");
  if (!reply.isValid()) {
    emit error("Failed to remove frequency limit: " + reply.error().message());
  } else {
    // Refresh status to update thresholdExceeded and other properties
    refreshStatus();
  }
}

void DaemonClient::refreshStatus() {
  if (!m_interface || !m_interface->isValid()) {
    return;
  }

  QDBusReply<QVariantMap> reply = m_interface->call("GetStatus");
  if (reply.isValid()) {
    QVariantMap status = reply.value();

    m_gpuPower = status["gpuPower"].toDouble();
    m_gpuPowerThreshold = status["gpuPowerThreshold"].toDouble();
    m_currentMaxFrequency = status["currentMaxFrequency"].toDouble();
    m_currentFrequency = status["currentFrequency"].toDouble();
    m_maxFrequency = status["maxFrequency"].toDouble();
    m_regulationEnabled = status["regulationEnabled"].toBool();
    m_autoProtection = status["autoProtection"].toBool();
    m_cooldownSeconds = status["cooldownSeconds"].toInt();
    m_thresholdExceeded = status["thresholdExceeded"].toBool();
    m_cpuLimitApplied = status["cpuLimitApplied"].toBool();

    // Read temperature data
    m_gpuTemperature = status["gpuTemperature"].toDouble();
    m_gpuFanSpeed = status["gpuFanSpeed"].toInt();
    m_cpuTemperature = status["cpuTemperature"].toDouble();
    m_cpuFanSpeed = status["cpuFanSpeed"].toInt();
    m_motherboardTemperature = status["motherboardTemperature"].toDouble();
    m_gpuVendor = status["gpuVendor"].toString();
    m_gpuName = status["gpuName"].toString();

    emit gpuPowerChanged();
    emit gpuPowerThresholdChanged();
    emit currentMaxFrequencyChanged();
    emit currentFrequencyChanged();
    emit maxFrequencyChanged();
    emit regulationEnabledChanged();
    emit autoProtectionChanged();
    emit cooldownSecondsChanged();
    emit thresholdExceededChanged();
    emit cpuLimitAppliedChanged();

    // Emit temperature signals
    emit gpuTemperatureChanged();
    emit gpuFanSpeedChanged();
    emit cpuTemperatureChanged();
    emit cpuFanSpeedChanged();
    emit motherboardTemperatureChanged();
    emit gpuVendorChanged();
    emit gpuNameChanged();
  }
}

void DaemonClient::onGpuPowerChanged(double power) {
  m_gpuPower = power;
  emit gpuPowerChanged();
}

void DaemonClient::onGpuPowerThresholdChanged(double threshold) {
  m_gpuPowerThreshold = threshold;
  emit gpuPowerThresholdChanged();
}

void DaemonClient::onCurrentMaxFrequencyChanged(double frequency) {
  m_currentMaxFrequency = frequency;
  emit currentMaxFrequencyChanged();
}

void DaemonClient::onCurrentFrequencyChanged(double frequency) {
  m_currentFrequency = frequency;
  emit currentFrequencyChanged();
}

void DaemonClient::onMaxFrequencyChanged(double frequency) {
  m_maxFrequency = frequency;
  emit maxFrequencyChanged();
}

void DaemonClient::onRegulationEnabledChanged(bool enabled) {
  m_regulationEnabled = enabled;
  emit regulationEnabledChanged();
}

void DaemonClient::onAutoProtectionChanged(bool enabled) {
  m_autoProtection = enabled;
  emit autoProtectionChanged();
}

void DaemonClient::onCooldownSecondsChanged(int seconds) {
  m_cooldownSeconds = seconds;
  emit cooldownSecondsChanged();
}

void DaemonClient::onThresholdExceededChanged(bool exceeded) {
  m_thresholdExceeded = exceeded;
  emit thresholdExceededChanged();
}

void DaemonClient::onCpuLimitAppliedChanged(bool applied) {
  m_cpuLimitApplied = applied;
  emit cpuLimitAppliedChanged();
}

void DaemonClient::onServiceOwnerChanged(const QString &name,
                                         const QString & /*oldOwner*/,
                                         const QString &newOwner) {
  if (name == "org.uncrash.Daemon") {
    if (newOwner.isEmpty()) {
      qWarning() << "Uncrash daemon disconnected";
      updateConnectionStatus(false);
    } else {
      qInfo() << "Uncrash daemon connected";
      connectToService();
    }
  }
}

// Temperature slot implementations
void DaemonClient::onGpuTemperatureChanged(double temperature) {
  if (m_gpuTemperature != temperature) {
    m_gpuTemperature = temperature;
    emit gpuTemperatureChanged();
  }
}

void DaemonClient::onGpuFanSpeedChanged(int speed) {
  if (m_gpuFanSpeed != speed) {
    m_gpuFanSpeed = speed;
    emit gpuFanSpeedChanged();
  }
}

void DaemonClient::onCpuTemperatureChanged(double temperature) {
  if (m_cpuTemperature != temperature) {
    m_cpuTemperature = temperature;
    emit cpuTemperatureChanged();
  }
}

void DaemonClient::onCpuFanSpeedChanged(int speed) {
  if (m_cpuFanSpeed != speed) {
    m_cpuFanSpeed = speed;
    emit cpuFanSpeedChanged();
  }
}

void DaemonClient::onMotherboardTemperatureChanged(double temperature) {
  if (m_motherboardTemperature != temperature) {
    m_motherboardTemperature = temperature;
    emit motherboardTemperatureChanged();
  }
}

void DaemonClient::onGpuVendorChanged(const QString &vendor) {
  if (m_gpuVendor != vendor) {
    m_gpuVendor = vendor;
    emit gpuVendorChanged();
  }
}

void DaemonClient::onGpuNameChanged(const QString &name) {
  if (m_gpuName != name) {
    m_gpuName = name;
    emit gpuNameChanged();
  }
}
