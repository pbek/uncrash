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
  Q_PROPERTY(double maxFrequency READ maxFrequency WRITE setMaxFrequency NOTIFY
                 maxFrequencyChanged)
  Q_PROPERTY(bool regulationEnabled READ regulationEnabled WRITE
                 setRegulationEnabled NOTIFY regulationEnabledChanged)
  Q_PROPERTY(bool autoProtection READ autoProtection WRITE setAutoProtection
                 NOTIFY autoProtectionChanged)
  Q_PROPERTY(bool thresholdExceeded READ thresholdExceeded NOTIFY
                 thresholdExceededChanged)
  Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

public:
  explicit DaemonClient(QObject *parent = nullptr);

  // Property getters
  double gpuPower() const { return m_gpuPower; }
  double gpuPowerThreshold() const { return m_gpuPowerThreshold; }
  double currentMaxFrequency() const { return m_currentMaxFrequency; }
  double maxFrequency() const { return m_maxFrequency; }
  bool regulationEnabled() const { return m_regulationEnabled; }
  bool autoProtection() const { return m_autoProtection; }
  bool thresholdExceeded() const { return m_thresholdExceeded; }
  bool connected() const { return m_connected; }

  // Property setters
  void setGpuPowerThreshold(double threshold);
  void setMaxFrequency(double frequency);
  void setRegulationEnabled(bool enabled);
  void setAutoProtection(bool enabled);

  Q_INVOKABLE void applyFrequencyLimit();
  Q_INVOKABLE void removeFrequencyLimit();
  Q_INVOKABLE void refreshStatus();

signals:
  void gpuPowerChanged();
  void gpuPowerThresholdChanged();
  void currentMaxFrequencyChanged();
  void maxFrequencyChanged();
  void regulationEnabledChanged();
  void autoProtectionChanged();
  void thresholdExceededChanged();
  void connectedChanged();
  void error(const QString &message);

private slots:
  void onGpuPowerChanged(double power);
  void onGpuPowerThresholdChanged(double threshold);
  void onCurrentMaxFrequencyChanged(double frequency);
  void onMaxFrequencyChanged(double frequency);
  void onRegulationEnabledChanged(bool enabled);
  void onAutoProtectionChanged(bool enabled);
  void onThresholdExceededChanged(bool exceeded);
  void onServiceOwnerChanged(const QString &name, const QString &oldOwner,
                             const QString &newOwner);

private:
  void connectToService();
  void updateConnectionStatus(bool status);

  QDBusAbstractInterface *m_interface;

  double m_gpuPower = 0.0;
  double m_gpuPowerThreshold = 100.0;
  double m_currentMaxFrequency = 0.0;
  double m_maxFrequency = 3.5;
  bool m_regulationEnabled = true;
  bool m_autoProtection = true;
  bool m_thresholdExceeded = false;
  bool m_connected = false;
};
