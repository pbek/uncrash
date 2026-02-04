#pragma once

#include <QObject>
#include <QTimer>

class PowerMonitor : public QObject {
  Q_OBJECT
  Q_PROPERTY(double gpuPower READ gpuPower NOTIFY gpuPowerChanged)
  Q_PROPERTY(double gpuPowerThreshold READ gpuPowerThreshold WRITE
                 setGpuPowerThreshold NOTIFY gpuPowerThresholdChanged)
  Q_PROPERTY(bool thresholdExceeded READ thresholdExceeded NOTIFY
                 thresholdExceededChanged)

public:
  explicit PowerMonitor(QObject *parent = nullptr);

  double gpuPower() const { return m_gpuPower; }
  double gpuPowerThreshold() const { return m_gpuPowerThreshold; }
  bool thresholdExceeded() const { return m_thresholdExceeded; }

  void setGpuPowerThreshold(double threshold);

signals:
  void gpuPowerChanged();
  void gpuPowerThresholdChanged();
  void thresholdExceededChanged();

private slots:
  void updateGpuPower();

private:
  double readGpuPowerFromSysfs();

  QTimer *m_updateTimer;
  double m_gpuPower = 0.0;
  double m_gpuPowerThreshold = 100.0;
  bool m_thresholdExceeded = false;
};
