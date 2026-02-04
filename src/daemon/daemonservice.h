#pragma once

#include "../cpucontroller.h"
#include "../powermonitor.h"
#include "../systemprotector.h"
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
  void FrequencyLimitApplied(double frequency);
  void FrequencyLimitRemoved();

private slots:
  void onGpuPowerChanged();
  void onGpuPowerThresholdChanged();
  void onCurrentMaxFrequencyChanged();
  void onMaxFrequencyChanged();
  void onRegulationEnabledChanged();
  void onAutoProtectionChanged();
  void onThresholdExceededChanged();

private:
  void loadSettings();
  void saveSettings();

  SystemProtector *m_protector;
  PowerMonitor *m_powerMonitor;
  CpuController *m_cpuController;
};
