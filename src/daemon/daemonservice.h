#pragma once

#include "../cpucontroller.h"
#include "../powermonitor.h"
#include "../systemprotector.h"
#include "../temperaturemonitor.h"
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QObject>

class DaemonService : public QObject {
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.uncrash.Daemon")

  // DBus properties
  Q_PROPERTY(double GpuPower READ gpuPower NOTIFY GpuPowerChanged)
  Q_PROPERTY(double GpuPowerThreshold READ gpuPowerThreshold WRITE
                 setGpuPowerThreshold NOTIFY GpuPowerThresholdChanged)
  Q_PROPERTY(double CurrentMaxFrequency READ currentMaxFrequency NOTIFY
                 CurrentMaxFrequencyChanged)
  Q_PROPERTY(double MaxFrequency READ maxFrequency WRITE setMaxFrequency NOTIFY
                 MaxFrequencyChanged)
  Q_PROPERTY(bool RegulationEnabled READ regulationEnabled WRITE
                 setRegulationEnabled NOTIFY RegulationEnabledChanged)
  Q_PROPERTY(bool AutoProtection READ autoProtection WRITE setAutoProtection
                 NOTIFY AutoProtectionChanged)
  Q_PROPERTY(bool ThresholdExceeded READ thresholdExceeded NOTIFY
                 ThresholdExceededChanged)
  Q_PROPERTY(
      bool CpuLimitApplied READ cpuLimitApplied NOTIFY CpuLimitAppliedChanged)

  // Temperature sensors
  Q_PROPERTY(
      double GpuTemperature READ gpuTemperature NOTIFY GpuTemperatureChanged)
  Q_PROPERTY(int GpuFanSpeed READ gpuFanSpeed NOTIFY GpuFanSpeedChanged)
  Q_PROPERTY(
      double CpuTemperature READ cpuTemperature NOTIFY CpuTemperatureChanged)
  Q_PROPERTY(int CpuFanSpeed READ cpuFanSpeed NOTIFY CpuFanSpeedChanged)
  Q_PROPERTY(double MotherboardTemperature READ motherboardTemperature NOTIFY
                 MotherboardTemperatureChanged)
  Q_PROPERTY(QString GpuVendor READ gpuVendor NOTIFY GpuVendorChanged)
  Q_PROPERTY(QString GpuName READ gpuName NOTIFY GpuNameChanged)

public:
  explicit DaemonService(QObject *parent = nullptr);
  ~DaemonService();

  bool registerService();

  // Property getters
  double gpuPower() const;
  double gpuPowerThreshold() const;
  double currentMaxFrequency() const;
  double maxFrequency() const;
  bool regulationEnabled() const;
  bool autoProtection() const;
  bool thresholdExceeded() const;
  bool cpuLimitApplied() const;

  // Temperature getters
  double gpuTemperature() const;
  int gpuFanSpeed() const;
  double cpuTemperature() const;
  int cpuFanSpeed() const;
  double motherboardTemperature() const;
  QString gpuVendor() const;
  QString gpuName() const;

  // Property setters
  void setGpuPowerThreshold(double threshold);
  void setMaxFrequency(double frequency);
  void setRegulationEnabled(bool enabled);
  void setAutoProtection(bool enabled);

public slots:
  // DBus methods
  bool ApplyFrequencyLimit();
  bool RemoveFrequencyLimit();
  QVariantMap GetStatus();

signals:
  // DBus signals
  void GpuPowerChanged(double power);
  void GpuPowerThresholdChanged(double threshold);
  void CurrentMaxFrequencyChanged(double frequency);
  void MaxFrequencyChanged(double frequency);
  void RegulationEnabledChanged(bool enabled);
  void AutoProtectionChanged(bool enabled);
  void ThresholdExceededChanged(bool exceeded);
  void CpuLimitAppliedChanged(bool applied);
  void FrequencyLimitApplied(double frequency);
  void FrequencyLimitRemoved();

  // Temperature signals
  void GpuTemperatureChanged(double temperature);
  void GpuFanSpeedChanged(int speed);
  void CpuTemperatureChanged(double temperature);
  void CpuFanSpeedChanged(int speed);
  void MotherboardTemperatureChanged(double temperature);
  void GpuVendorChanged(const QString &vendor);
  void GpuNameChanged(const QString &name);

private slots:
  void onGpuPowerChanged();
  void onGpuPowerThresholdChanged();
  void onCurrentMaxFrequencyChanged();
  void onMaxFrequencyChanged();
  void onRegulationEnabledChanged();
  void onAutoProtectionChanged();
  void onThresholdExceededChanged();
  void onCpuLimitAppliedChanged();

private:
  void loadSettings();
  void saveSettings();

  SystemProtector *m_protector;
  PowerMonitor *m_powerMonitor;
  CpuController *m_cpuController;
  TemperatureMonitor *m_temperatureMonitor;
};
