#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QTimer>

struct SensorData {
  double temperature = 0.0; // In Celsius
  int fanSpeed = 0;         // In RPM
  bool valid = false;
};

class TemperatureMonitor : public QObject {
  Q_OBJECT

public:
  explicit TemperatureMonitor(QObject *parent = nullptr);
  ~TemperatureMonitor() override = default;

  // GPU sensors
  double gpuTemperature() const { return m_gpuData.temperature; }
  int gpuFanSpeed() const { return m_gpuData.fanSpeed; }
  int gpuFanPercent() const { return m_gpuFanPercent; }

  // CPU sensors
  double cpuTemperature() const { return m_cpuData.temperature; }
  int cpuFanSpeed() const { return m_cpuData.fanSpeed; }

  // Case/Motherboard sensors
  double motherboardTemperature() const {
    return m_motherboardData.temperature;
  }
  double cpuSocketTemperature() const { return m_cpuSocketTemperature; }

  // Additional case fans
  QMap<QString, int> caseFanSpeeds() const { return m_caseFanSpeeds; }

  // GPU type detection
  QString gpuVendor() const { return m_gpuVendor; }
  QString gpuName() const { return m_gpuName; }

  void startMonitoring(int intervalMs = 2000);
  void stopMonitoring();

signals:
  void temperaturesUpdated();
  void gpuTemperatureChanged(double temperature);
  void cpuTemperatureChanged(double temperature);
  void motherboardTemperatureChanged(double temperature);
  void fanSpeedsChanged();

private slots:
  void updateSensors();

private:
  // Helper methods
  SensorData readGpuSensors();
  SensorData readNvidiaGpu();
  SensorData readAmdGpu();
  SensorData readCpuSensors();
  SensorData readMotherboardSensors();
  void readFanSensors();
  void detectGpuVendor();

  double readHwmonTemp(const QString &hwmonPath, const QString &tempFile);
  int readHwmonFan(const QString &hwmonPath, const QString &fanFile);
  QString findHwmonByName(const QString &name);
  QStringList findAllHwmonByPattern(const QString &pattern);
  QString readHwmonLabel(const QString &hwmonPath, const QString &labelFile);

  // Sensor data
  SensorData m_gpuData;
  SensorData m_cpuData;
  SensorData m_motherboardData;

  double m_cpuSocketTemperature = 0.0;
  int m_gpuFanPercent = 0;
  QMap<QString, int> m_caseFanSpeeds;

  // GPU info
  QString m_gpuVendor; // "NVIDIA", "AMD", or "Unknown"
  QString m_gpuName;

  // Hwmon paths (cached for performance)
  QString m_cpuTempPath;  // CPU temperature (k10temp, coretemp, etc.)
  QString m_gpuHwmonPath; // AMD GPU hwmon path (if AMD)
  QString
      m_motherboardPath; // Motherboard sensors (asus_wmi, nct6775, it87, etc.)

  QTimer *m_updateTimer;
};
