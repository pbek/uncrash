#pragma once

#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QObject>

class DaemonClient : public QObject {
  Q_OBJECT

  Q_PROPERTY(double gpuPower READ gpuPower NOTIFY gpuPowerChanged)
  Q_PROPERTY(double gpuPowerThreshold READ gpuPowerThreshold WRITE
                 setGpuPowerThreshold NOTIFY gpuPowerThresholdChanged)
  Q_PROPERTY(double currentMaxFrequency READ currentMaxFrequency NOTIFY
                 currentMaxFrequencyChanged)
  Q_PROPERTY(double currentFrequency READ currentFrequency NOTIFY
                 currentFrequencyChanged)
  Q_PROPERTY(double maxFrequency READ maxFrequency WRITE setMaxFrequency NOTIFY
                 maxFrequencyChanged)
  Q_PROPERTY(bool regulationEnabled READ regulationEnabled WRITE
                 setRegulationEnabled NOTIFY regulationEnabledChanged)
  Q_PROPERTY(bool autoProtection READ autoProtection WRITE setAutoProtection
                 NOTIFY autoProtectionChanged)
  Q_PROPERTY(int cooldownSeconds READ cooldownSeconds WRITE setCooldownSeconds
                 NOTIFY cooldownSecondsChanged)
  Q_PROPERTY(bool thresholdExceeded READ thresholdExceeded NOTIFY
                 thresholdExceededChanged)
  Q_PROPERTY(
      bool cpuLimitApplied READ cpuLimitApplied NOTIFY cpuLimitAppliedChanged)
  Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

  // Temperature properties
  Q_PROPERTY(
      double gpuTemperature READ gpuTemperature NOTIFY gpuTemperatureChanged)
  Q_PROPERTY(int gpuFanSpeed READ gpuFanSpeed NOTIFY gpuFanSpeedChanged)
  Q_PROPERTY(
      double cpuTemperature READ cpuTemperature NOTIFY cpuTemperatureChanged)
  Q_PROPERTY(int cpuFanSpeed READ cpuFanSpeed NOTIFY cpuFanSpeedChanged)
  Q_PROPERTY(double motherboardTemperature READ motherboardTemperature NOTIFY
                 motherboardTemperatureChanged)
  Q_PROPERTY(QString gpuVendor READ gpuVendor NOTIFY gpuVendorChanged)
  Q_PROPERTY(QString gpuName READ gpuName NOTIFY gpuNameChanged)

public:
  explicit DaemonClient(QObject *parent = nullptr);

  // Property getters
  double gpuPower() const { return m_gpuPower; }
  double gpuPowerThreshold() const { return m_gpuPowerThreshold; }
  double currentMaxFrequency() const { return m_currentMaxFrequency; }
  double currentFrequency() const { return m_currentFrequency; }
  double maxFrequency() const { return m_maxFrequency; }
  bool regulationEnabled() const { return m_regulationEnabled; }
  bool autoProtection() const { return m_autoProtection; }
  int cooldownSeconds() const { return m_cooldownSeconds; }
  bool thresholdExceeded() const { return m_thresholdExceeded; }
  bool cpuLimitApplied() const { return m_cpuLimitApplied; }
  bool connected() const { return m_connected; }

  // Temperature getters
  double gpuTemperature() const { return m_gpuTemperature; }
  int gpuFanSpeed() const { return m_gpuFanSpeed; }
  double cpuTemperature() const { return m_cpuTemperature; }
  int cpuFanSpeed() const { return m_cpuFanSpeed; }
  double motherboardTemperature() const { return m_motherboardTemperature; }
  QString gpuVendor() const { return m_gpuVendor; }
  QString gpuName() const { return m_gpuName; }

  // Property setters
  void setGpuPowerThreshold(double threshold);
  void setMaxFrequency(double frequency);
  void setRegulationEnabled(bool enabled);
  void setAutoProtection(bool enabled);
  void setCooldownSeconds(int seconds);

  Q_INVOKABLE void applyFrequencyLimit();
  Q_INVOKABLE void removeFrequencyLimit();
  Q_INVOKABLE void refreshStatus();

signals:
  void gpuPowerChanged();
  void gpuPowerThresholdChanged();
  void currentMaxFrequencyChanged();
  void currentFrequencyChanged();
  void maxFrequencyChanged();
  void regulationEnabledChanged();
  void autoProtectionChanged();
  void cooldownSecondsChanged();
  void thresholdExceededChanged();
  void cpuLimitAppliedChanged();
  void connectedChanged();
  void error(const QString &message);

  // Temperature signals
  void gpuTemperatureChanged();
  void gpuFanSpeedChanged();
  void cpuTemperatureChanged();
  void cpuFanSpeedChanged();
  void motherboardTemperatureChanged();
  void gpuVendorChanged();
  void gpuNameChanged();

private slots:
  void onGpuPowerChanged(double power);
  void onGpuPowerThresholdChanged(double threshold);
  void onCurrentMaxFrequencyChanged(double frequency);
  void onCurrentFrequencyChanged(double frequency);
  void onMaxFrequencyChanged(double frequency);
  void onRegulationEnabledChanged(bool enabled);
  void onAutoProtectionChanged(bool enabled);
  void onCooldownSecondsChanged(int seconds);
  void onThresholdExceededChanged(bool exceeded);
  void onCpuLimitAppliedChanged(bool applied);
  void onServiceOwnerChanged(const QString &name, const QString &oldOwner,
                             const QString &newOwner);
  // Temperature slots
  void onGpuTemperatureChanged(double temperature);
  void onGpuFanSpeedChanged(int speed);
  void onCpuTemperatureChanged(double temperature);
  void onCpuFanSpeedChanged(int speed);
  void onMotherboardTemperatureChanged(double temperature);
  void onGpuVendorChanged(const QString &vendor);
  void onGpuNameChanged(const QString &name);

private:
  void connectToService();
  void updateConnectionStatus(bool status);

  QDBusAbstractInterface *m_interface;

  double m_gpuPower = 0.0;
  double m_gpuPowerThreshold = 100.0;
  double m_currentMaxFrequency = 0.0;
  double m_currentFrequency = 0.0;
  double m_maxFrequency = 3.5;
  bool m_regulationEnabled = true;
  bool m_autoProtection = true;
  int m_cooldownSeconds = 5;
  bool m_thresholdExceeded = false;
  bool m_cpuLimitApplied = false;
  bool m_connected = false;

  // Temperature data
  double m_gpuTemperature = 0.0;
  int m_gpuFanSpeed = 0;
  double m_cpuTemperature = 0.0;
  int m_cpuFanSpeed = 0;
  double m_motherboardTemperature = 0.0;
  QString m_gpuVendor;
  QString m_gpuName;
};
