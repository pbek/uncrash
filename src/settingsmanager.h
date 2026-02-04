#pragma once

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(int windowWidth READ windowWidth WRITE setWindowWidth NOTIFY
                 windowWidthChanged)
  Q_PROPERTY(int windowHeight READ windowHeight WRITE setWindowHeight NOTIFY
                 windowHeightChanged)
  Q_PROPERTY(double gpuPowerThreshold READ gpuPowerThreshold WRITE
                 setGpuPowerThreshold NOTIFY gpuPowerThresholdChanged)
  Q_PROPERTY(double cpuMaxFrequency READ cpuMaxFrequency WRITE
                 setCpuMaxFrequency NOTIFY cpuMaxFrequencyChanged)
  Q_PROPERTY(bool autoProtection READ autoProtection WRITE setAutoProtection
                 NOTIFY autoProtectionChanged)

public:
  explicit SettingsManager(QObject *parent = nullptr);

  // Window geometry
  int windowWidth() const;
  void setWindowWidth(int width);

  int windowHeight() const;
  void setWindowHeight(int height);

  // GPU settings
  double gpuPowerThreshold() const;
  void setGpuPowerThreshold(double threshold);

  // CPU settings
  double cpuMaxFrequency() const;
  void setCpuMaxFrequency(double frequency);

  // Protection settings
  bool autoProtection() const;
  void setAutoProtection(bool enabled);

signals:
  void windowWidthChanged();
  void windowHeightChanged();
  void gpuPowerThresholdChanged();
  void cpuMaxFrequencyChanged();
  void autoProtectionChanged();

private:
  QSettings m_settings;
};
